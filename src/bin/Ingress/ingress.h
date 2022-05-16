//
// Created by nekonoyume on 2022/5/15.
//

#ifndef TINYWEB_INGRESS_OPERATION_H
#define TINYWEB_INGRESS_OPERATION_H

#include <unistd.h>
#include <netinet/in.h>
#include "../http/HttpResponse.h"
#include "args.h"
#include "../all_exception.h"
#include "socket_op.h"
#include "../http/http_request/http_request.h"
#include "../socket/Socket.h"
#include "../timer/lst_timer.h"
#include "../extensions/loadbalancer/RedisLoadBalancer.h"
#include "../debug/dprintf.h"
#include "../config/config.h"
#include <memory>

using namespace std;
using yumira::debug::DPrintf;

namespace yumira {
    class Ingress {
        struct ingress_operation;
    protected:
        void loop() {
            while (!m_stop) {
                int n_fd;
                if ((n_fd = ::epoll_wait(m_epollfd, epollEvents, MAX_EVENT_NUMBER, -1)) < 0
                    && errno != EINTR) {
                    break;
                }
                for (int i = 0; i < n_fd; ++i) {
                    int sockfd = epollEvents[i].data.fd;
                    if (sockfd == m_listenSocket.fd) {

                        // allocate new connection for user if there are some free http connections.
                        AcceptArgs args{m_epollfd, m_listenSocket.fd};
                        args.accept_cb = [this](sockaddr_in sockaddrIn, socklen_t len) {
                            return m_listenSocket.accept(sockaddrIn, len);
                        };
                        AcceptReply reply{-1};
                        // WARNING! if want to use reference, be careful about std::ref
                        m_socket_op->dealAccept(args, reply);
//                        printf("reply.connFd=%d\n",reply.connFd);
                        if (reply.connFd >= 0) {
                            cachedCtx[reply.connFd].reset(makeNewSocketContext(reply.connFd));
                            printf("make context\n");
                        }
                        continue;
                    }

                    auto iter = cachedCtx.find(sockfd);
                    if (iter == cachedCtx.end() || !iter->second) {
                        throw exception();
                    }
                    printf("hello\n");
                    auto &&ctx = *iter->second;
//                    printf("ctx.fd=%d\n", ctx.socketFd);

                    if (false) {
                        m_socket_op->dealClose(ctx, nullptr);
                        cachedCtx.erase(iter);
                    }
                    if (epollEvents[i].events & EPOLLIN) {
                        m_socket_op->dealRead(ctx, nullptr);
                    } else if (epollEvents[i].events & EPOLLOUT) {
                        m_socket_op->dealWrite(ctx, nullptr);
                    }
                }
            }
        }

    public:
        Ingress(const InetAddress &redisIpPort);

        void listen(int port, const std::string &ListenIpPattern = yumira::ipv4::IP_ANY) {
            struct linger tmp = {0, 1}; // { on/off linger, risidual time }
            auto &&address = ipv4::newSockaddrIn(port);
            int REUSE = 1;
            m_listenSocket.setSockOpt(SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp))
                    .setSockOpt(SOL_SOCKET, SO_REUSEADDR, &REUSE, sizeof(REUSE))
                    .bind((struct sockaddr *) &address, sizeof(address))
                    .listen(5);

            m_epollfd = epoll_create(5);

            Utils::regist_fd(m_epollfd, m_listenSocket.fd, false, listenTriggerMode);
            loop();
        };

        ~Ingress();

        SocketContext *makeNewSocketContext(int connFd) {
            return new SocketContext{m_epollfd, connFd};
        }


    protected:
        std::map<int, unique_ptr<SocketContext>> cachedCtx;
    private:
        Socket m_listenSocket;
        int m_epollfd;
        int listenTriggerMode = 0;
        bool m_stop;
        epoll_event epollEvents[MAX_EVENT_NUMBER];
        socket_op *m_socket_op;
    };


}
#endif //TINYWEB_INGRESS_OPERATION_H
