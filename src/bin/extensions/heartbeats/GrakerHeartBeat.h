//
// Created by nekonoyume on 2022/4/12.
//

#ifndef REDISTEST_GRAKERHEARTBEAT_H
#define REDISTEST_GRAKERHEARTBEAT_H

#include "../../socket/ustdnet.h"
#include "../../socket/InetAddress.h"
#include "../redis_utils/redis_utils.h"
#include <map>
#include <functional>
#include <iostream>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <unordered_set>

class CTimer;

namespace yumira {
    namespace heartbeat {
        extern std::unordered_set<std::string> exclude_address_;
    }
}

int
peekHBPack(int conn_fd);

struct HeartBeatPack {
    static int MAGIC;
    int magicNum = MAGIC;
    typedef enum {
        PING = 0, PONG
    } MSG_TYPE;
    MSG_TYPE type;
    int length;

    bool IsHeartBeat(int sockFd) {
        char buf[128];
        int n = recvfrom(sockFd, buf, sizeof buf, MSG_PEEK, nullptr, nullptr);
        if (n < 0) {

        }
        int magic = *(int *) buf;
        if (magic == MAGIC)
            return true;
        return false;
    };

    static void
    makePING(HeartBeatPack &pack_head) {
        pack_head.type = HeartBeatPack::PING;
        pack_head.length = sizeof(HeartBeatPack);
        pack_head.magicNum = HeartBeatPack::MAGIC;
    };

    static void
    makePONG(HeartBeatPack &pack_head) {
        pack_head.type = HeartBeatPack::PONG;
        pack_head.length = sizeof(HeartBeatPack);
        pack_head.magicNum = HeartBeatPack::MAGIC;
    };

};


struct SocketConn {
    InetAddress address;
    int n_timeout;
    int n_error;

    SocketConn();

    SocketConn(int Fd);

    void
    resetExpire();

    bool
    isTimeOut(const time_t &dt);

    int getSockFd();

    void
    setMarker() {
        observe_ = true;
    }

    bool
    isMarked() const {
        return observe_;
    }

    ~SocketConn() {
        printf("MASTER close Fd=%d\n", sockFd);
        close(sockFd);
    }

private:
    int sockFd;
    time_t dt_heart_;
    bool observe_;
private:
    static time_t CLIENT_SPAN_SEC;
};

class HBWorker {
public:
    HBWorker() {};

    bool
    sendData(int fd, void *buffer, int flag = 0) {
        auto n = send(fd, buffer, sizeof buffer, flag);
        if (n == -1) {
            return false;
        }
        if (n == 0) {
            printf("write Nothing\n");
            return false;
        }
        printf("write n_bytes=%ld\n", n);
        return true;

    };

    virtual bool
    reactFd(int Fd) = 0;

//    virtual void
//    addObserverFd(int Fd) = 0;

    virtual void
    doWork() = 0;


    virtual ~HBWorker() {};
};


class HBMaster : public HBWorker {
protected:
    bool
    bind_address(const InetAddress &, int &fd);

public:
    static int
    allocSocket() {
        return socket(AF_INET, SOCK_STREAM, 0);
    }

    HBMaster(const InetAddress &inetAddress) : bindAddress(inetAddress),
                                               listen_fd(-1),
                                               clusterSize_(-1),
                                               redis_conn(nullptr),
                                               m_dontWait(true),
                                               load_self_server_address(nullptr){
    }

    bool
    sendHBtoNewSlave(const int &fd) {
        printf("[HBMaster] sendHBtoNewSlave to %d\n", fd);
        // slave still know master
        HeartBeatPack pack_head;
        HeartBeatPack::makePING(pack_head);
        auto n = send(fd, (void *) &pack_head, sizeof pack_head, 0);
        if (n < 0) {
            printf("master write fail\n");
            return false;
        } else if (n == 0) {
            printf("master write nothing\n");
            return false;
        }
        printf("master write n_bytes=%ld\n", n);
        return true;
    };

    int
    getListenFd() const {
        return listen_fd;
    }

    std::function<InetAddress()>
    load_self_server_address;


    bool reactFd(int Fd) override;


    bool
    setRedisConn(RedisConnection *conn) {
        if (redis_conn)
            return false;
        redis_conn = conn;
        return true;
    }

    void tryDiscoverCluster() {
        return tryDiscoverCluster(clusterKey_);
    };
    // invoke by idle-Thread
    void tryDiscoverCluster(const std::string &clusterKey);

    // invoke by idle-Thread
    void
    doWork()override;

    ~HBMaster() {
        for (auto &&kv: _clientConns) {
            delete kv.second;
        }
    }

private:
    InetAddress bindAddress;
    int listen_fd; // wait other slave to connect
    time_t lastTime;
    std::function<void(SocketConn)> callbackOnDisconnect_;
    // nodeIp -> connect Fd
    std::map<std::string, SocketConn *> _clientConns;
    static int MAX_N_TIMEOUT;
    RedisConnection *redis_conn;
    std::string clusterKey_;
    int clusterSize_;
    // boolean
    bool m_dontWait;
    // load from conf, to exclude self node, so we won't send heartbeat to self server

};

class HBSlave : public HBWorker {
public:
    HBSlave() : HBSlave(-1) {
    };

    explicit HBSlave(int listenFd) : listen_fd(listenFd), n_arrived(0), m_dontWait(true) {
    }

    bool
    setListenFd(int fd) {
        printf("setListenFd=%d\n", fd);

        std::unique_lock<std::mutex> lockGuard(fd_mutex_);
        listen_fd = fd;
        if (listen_fd >= 0) {
            if (m_dontWait) {
                auto old_flag = fcntl(listen_fd, F_GETFL, 0);
                auto ret = fcntl(listen_fd, F_SETFL, old_flag | O_NONBLOCK);
                if (ret < 0) {
                    std::cerr << "has no previllige to set fd=" << listen_fd << std::endl;
                }
            }
            unbind_cond.notify_all();
        }
        return true;
    };

    bool
    sendHeartBeatIfAbsent(int connFd);

    bool
    reactFd(int connFd) override {
        return sendHeartBeatIfAbsent(connFd);
    };

    void addObserverFd(int Fd) {
        assert(n_arrived >= 0);
        ++n_arrived;
    }

    void doWork() override {
        std::cout << "Slave: " << std::this_thread::get_id() << " enter into doWork()\n";

        struct sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        int connFd = -1;
        {
            printf("slave before lock\n");

            std::unique_lock<std::mutex> lockGuard(fd_mutex_);
            while (listen_fd < 0) {
                unbind_cond.wait(lockGuard);
            }
        }
        printf("slave before accept\n");

        connFd = accept(listen_fd, (struct sockaddr *) &peer_addr, &addr_len);
        printf("slave after accept\n");

        if (connFd < 0) {
            // empty
        } else {
            printf("Slave receive ConnFd: %d", connFd);
            int n_sendFail = 0;
            while (!reactFd(connFd) && n_sendFail < 3) {
                ++n_sendFail;
            };

        }
    }
//        if (n_arrived == 0) {
//            printf("[HBSlave] nothing happen.\n");
//        } else if (n_arrived < 0)
//            return;
//
//        while (n_arrived--) {
//            auto connFd = accept(listen_fd, (struct sockaddr *) nullptr, nullptr);
//            reactFd(connFd);
//        }


private:
    int listen_fd;
    int n_arrived;
    std::mutex fd_mutex_;
    std::condition_variable unbind_cond;
    bool m_dontWait;

};

#endif //REDISTEST_GRAKERHEARTBEAT_H
