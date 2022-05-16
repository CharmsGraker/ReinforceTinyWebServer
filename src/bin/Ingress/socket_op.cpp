//
// Created by nekonoyume on 2022/5/16.
//

#include "socket_op.h"
#include "args.h"


void yumira::socket_op::dealAccept(yumira::AcceptArgs &args, yumira::AcceptReply &reply) {
    onAccept(args, reply);
    Utils::regist_fd(args.epollFd,reply.connFd, true,0);
    return;
}

void yumira::socket_op::dealRead(const SocketContext &ctx, void *args) {
    Utils::regist_fd(ctx.epollFd, ctx.socketFd(), true, 0);
    (const_cast<SocketContext&>(ctx)).makeHttpResponse();
    try {
        printf("dealRead\n");
        onRead(ctx, args);
        Utils::modfd(ctx.epollFd, ctx.socketFd(), EPOLLIN, 0);
    }catch (std::exception& e) {
        e.what();
        (const_cast<SocketContext&>(ctx)).FreeHttpResponse();
    }
}

void yumira::socket_op::dealWrite(const yumira::SocketContext &ctx, void *args) {
    Utils::regist_fd(ctx.epollFd, ctx.socketFd(), true, 0);
    onWrite(ctx, args);
    Utils::modfd(ctx.epollFd, ctx.socketFd(), EPOLLIN, 0);
    (const_cast<SocketContext&>(ctx)).FreeHttpResponse();
}

void yumira::socket_op::dealClose(const yumira::SocketContext &ctx, void *args) {
    onClose(ctx, args);
    Utils::removefd(ctx.epollFd,ctx.socketFd());
}

