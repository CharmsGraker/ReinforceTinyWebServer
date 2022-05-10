#include "GrakerHeartBeat.h"
#include <iostream>
#include <arpa/inet.h>
#include "../../timer/CTimer.h"
#include <cstring>

int HBMaster::MAX_N_TIMEOUT = 3;
unordered_set<std::string> yumira::heartbeat::exclude_address_;

bool HBMaster::bind_address(const InetAddress &masterIP, int &sockFd) {
    assert(sockFd >= 0);
    auto ip_ = masterIP.getHost();
    auto port = masterIP.getPort();
    struct sockaddr_in master_addr_;
    bzero(&master_addr_, sizeof master_addr_);
    master_addr_.sin_family = AF_INET;
    master_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    master_addr_.sin_port = htons(port);
    // must set SO_REUSEADDR before bind() !!!!!
    int opt = 1;
    setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    auto ret = bind(sockFd, (struct sockaddr *) &master_addr_, sizeof master_addr_);

    if (ret == -1) {
        std::cerr << "bind() failed\n";
        sockFd = -1;
        return false;
    }
//    ret = listen(listen_fd, LISTENQ);

    return true;
}


bool HBMaster::reactFd(int connFd) {
    HeartBeatPack pack_head;
    HeartBeatPack::makePING(pack_head);

    auto ret = sendData(connFd, (void *) &pack_head);
    if (ret) {
        printf("master send HeartBeat to connected Fd: %d\n", connFd);
    }
    return ret;
}

void
HBMaster::tryDiscoverCluster(const std::string &clusterKey) {
    assert(redis_conn);
    std::cout << std::this_thread::get_id() << " tryDiscoverCluster() with key=" << clusterKey << "\n";
    using namespace yumira::heartbeat;

    // record cluster Key
    clusterKey_ = clusterKey;
    auto &&all_nodes = redis_conn->Zrange(clusterKey, 0, -1);
    // observe all nodes has been register to Redis
    if (!all_nodes.empty()) {
        for (auto &&inetAddressStr: all_nodes) {
            // ignore current server in this process
            if (!exclude_address_.empty() && exclude_address_.find(inetAddressStr) != exclude_address_.end())
                continue;
            // add to client
            // allocate a new socket for master, then bind to interested IP
            printf("find node [%s] registered in Redis\n", inetAddressStr.c_str());
            auto ai = HostServ(inetAddressStr);
            // build a new socket fd for each node TCP connection
            auto iter = _clientConns.find(inetAddressStr);
            if (iter != _clientConns.end()) {
                printf("[HBMaster] find connected socket=%d,not replace.\n", listen_fd);
//                _clientConns.erase(iter);
            } else {
                // not exists

                printf("[HBMaster] try alloc new socket\n");
                int sockFd = allocSocket();
                if (!bind_address(bindAddress, sockFd)) {
                    printf("[HBMaster] error when bind()\n");
                    std::abort();
                };

                int n = connect(sockFd, ai->ai_addr, ai->ai_addrlen);
                printf("[HBMaster] connect() ret=%d\n", n);

                if (n < 0) {
                    auto s_flag = fcntl(sockFd, F_GETFL, 0);
                    if (s_flag & O_NONBLOCK && errno == EINPROGRESS) {
                        // it's okay when we use non-block
                    } else {
                        // error when non-block connect()
                        fprintf(stderr, "[HBMaster] nonblocking error\n");
                    }
                } else {
                    if (sendHBtoNewSlave(sockFd)) {
                        printf("[HBMaster] send PING to new Slave: %s success\n", inetAddressStr.c_str());
                        // if not use new, the element will deconstruct but still exists in container
//                        _clientConns[listen_fd] = SocketConn(listen_fd);
                        _clientConns[inetAddressStr] = new SocketConn(sockFd);
                    } else {
                        printf("[HBMaster] send PING to new Slave: %s fail\n", inetAddressStr.c_str());
                    }
                }
            }
        }
    } else {
        /** no cluster node be found */
        printf("no cluster node be found\n");
    }
}


void HBMaster::doWork() {
    // has been lost too many cluster Nodes
    std::cout << "Master: " << std::this_thread::get_id() << "enter into doWork()\n";

    if (_clientConns.empty() || _clientConns.size() < clusterSize_) {
        tryDiscoverCluster();
    }

    std::cout << "Master: " << std::this_thread::get_id()
              << " detect clusterSize=" << _clientConns.size()
              << "\n";
    time_t nowTime = CTimer::GetNowMillSec();
    time_t dt = nowTime - lastTime;
    lastTime = nowTime;

    for (auto &&iter = _clientConns.begin(); iter != _clientConns.end();) {
        bool needRemove = false;

        try {
            auto &&client = iter->second;
            char buf[128];
            if (client->isTimeOut(dt)) {
                needRemove = true;
                goto judge;
            }
            printf("before read\n");
            auto nRead = recv(client->getSockFd(), &buf, sizeof buf, m_dontWait ? MSG_DONTWAIT : 0);
            buf[nRead] = '\0';
            printf("after read,read=%ld\n", nRead);

            if (nRead >= 0) {
                printf("master read from buffer n_byte=%d\n", nRead);
                //  || client.n_timeout > MAX_N_TIMEOUT


                // send new heartBeat
                if (((HeartBeatPack *) buf)->type == HeartBeatPack::PONG) {
                    printf("master receive PONG\n");

                    client->resetExpire();
                    int n_sendFail = 0;

                    while (!needRemove && !sendHBtoNewSlave(client->getSockFd())) {
                        std::cout << "master in retry loop\n";
                        ++n_sendFail;
                        if (n_sendFail > 3) {
                            needRemove = true;
                        }
                    }
                } else if (((HeartBeatPack *) buf)->type == HeartBeatPack::PING) {
                    printf("master receive PING\n");
                    needRemove = true;
                }
            } else {
                if (!m_dontWait) {
                    needRemove = true;
                } else {
                    // as we set non-block IO, wait for next term
                }
            }
        } catch (std::exception &e) {
            e.what();
            needRemove = true;
        }
        judge:
        if (needRemove) {
            std::cout << "Master: " << std::this_thread::get_id() << "remove sockFd="
                      << iter->second->getSockFd()
                      << "\n";
//            if (iter->second->getSockFd() == listen_fd) {
//                listen_fd = -1;
//            }
            // deallocate SockConn on heap
            //            if (callbackOnDisconnect_)
//                callbackOnDisconnect_(iter->second);
            delete iter->second;
            _clientConns.erase(iter++);


            // callback()
        } else {
            ++iter;
        };
    }

    clusterSize_ = _clientConns.size();


}

bool HBSlave::sendHeartBeatIfAbsent(int connFd) {
    struct sockaddr_in peer_addr;
    socklen_t addr_len;
    getpeername(connFd, (struct sockaddr *) &peer_addr, &addr_len);

    char peer_ip_[128];
    inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip_, sizeof peer_ip_);
    char buffer[128];
    auto nByte = recv(connFd, (void *) buffer, sizeof buffer, 0);
    buffer[nByte] = '\0';
    if (((HeartBeatPack *) (buffer))->type == HeartBeatPack::PING) {
        printf("slave receive PING, ");
    }
    printf("slave receive n_byte=%d from peer/master = %s:%d\n", nByte, peer_ip_, ntohs(peer_addr.sin_port));
    HeartBeatPack pong_pkg;
    HeartBeatPack::makePONG(pong_pkg);
    auto ret = sendData(connFd, (void *) &pong_pkg);
    if (ret) {
        printf("slave send HB type=%d to %s:%d success\n", pong_pkg.type, peer_ip_, ntohs(peer_addr.sin_port));
    }
    return ret;
}