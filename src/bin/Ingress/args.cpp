//
// Created by nekonoyume on 2022/5/16.
//

#include "args.h"
#include "../socket/Socket.h"

int yumira::SocketContext::socketFd()const {
    return socketPtr->fd;
}

yumira::SocketContext::SocketContext(int epollFd_, Socket *socket) : epollFd(epollFd_),
                                              socketPtr(socket),
                                              request(nullptr),
                                              response(nullptr) {};

yumira::SocketContext::SocketContext(int fd) : socketPtr(new Socket(fd)), request(nullptr), response(nullptr) {};

yumira::SocketContext::SocketContext(int epollFd_,int connFd):SocketContext(epollFd_,new Socket(connFd)) {}

void yumira::SocketContext::makeHttpResponse() {
    response = new HttpResponse;
}

void yumira::SocketContext::FreeHttpResponse() {
    delete response;
    response = nullptr;
};
yumira::AcceptArgs::AcceptArgs(int epollFd_, int listenFd_) : epollFd(epollFd_), listenFd(listenFd_) {

}