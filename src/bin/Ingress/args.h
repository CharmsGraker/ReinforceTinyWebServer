//
// Created by nekonoyume on 2022/5/15.
//

#ifndef TINYWEB_ARGS_H
#define TINYWEB_ARGS_H

#include <memory>
#include "../http/http_request/http_request.h"
#include "../http/HttpResponse.h"
#include "../timer/lst_timer.h"

struct Socket;
namespace yumira {

    struct AcceptArgs {
        AcceptArgs(int i, int i1);

        int epollFd;
        int listenFd;
    private:
        friend class Ingress;

        std::function<long(sockaddr_in, socklen_t)> accept_cb = nullptr;
    public:
        long accept() {
            struct sockaddr_in client_address;
            socklen_t len_client_addr;

            return accept_cb(client_address, len_client_addr);
        }

        ~AcceptArgs() {

        }
    };

    struct AcceptReply {
        int connFd;
    };


    struct SocketContext {
    private:
        SocketContext(int epollFd_, Socket *socket);
    public:
        int epollFd;

        std::unique_ptr<Socket> socketPtr;
        HttpRequest *request = nullptr;
        HttpResponse *response = nullptr;
        typedef HttpResponse response_maker;

        SocketContext() = default;



        SocketContext(int epollFd_, int connFd);

        SocketContext(int fd);

        int socketFd() const;


        ~SocketContext() = default;

        void makeHttpResponse();

        void FreeHttpResponse();
    };
}
#endif //TINYWEB_ARGS_H
