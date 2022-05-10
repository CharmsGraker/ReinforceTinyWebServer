//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_SOCKETACCEPTOR_H
#define TINYWEB_SOCKETACCEPTOR_H

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include "InetAddress.h"
#include "../eventloop/EventLoop.h"

class socketAcceptor: public boost::noncopyable{
public:
    typedef boost::function<void(int sockfd,const InetAddress&)> newConnectionCallback;
    socketAcceptor(EventLoop& eventLoop,const InetAddress& listenAddr) {

    }
};
#endif //TINYWEB_SOCKETACCEPTOR_H
