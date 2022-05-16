//
// Created by nekonoyume on 2022/5/15.
//

#ifndef TINYWEB_SOCKET_H
#define TINYWEB_SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "../all_exception.h"

struct Socket {
public:
    int fd;

    Socket() {
        fd = socket(PF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            fprintf(stderr, "[WARN] create socket failed!\n%s\n", strerror(errno));
        }
    }

    Socket(int socketFd) : fd(socketFd) {

    }


    Socket(Socket &&other) {
        fd = other.fd;
        other.fd = -1;
    }

    Socket &
    setSockOpt(int __level, int __optname,
               const void *__optval, socklen_t __optlen) {
        auto &&ret = setsockopt(fd, __level, __optname, __optval, __optlen);
        if (ret < 0) {
            throw NetworkExpcetion("[WARN] setSockOpt() failed!");
        }
        return *this;


    };

    Socket &
    bind(__CONST_SOCKADDR_ARG __addr, socklen_t __len) {
        int ret = ::bind(fd, __addr, __len);
        if (ret < 0) {
            throw NetworkExpcetion("[WARN] bind() failed!");
        }
        return *this;

    };

    Socket &
    listen(int queueLen) {
        int ret = -1;
        if ((ret = ::listen(fd, queueLen)) < 0) {
            throw NetworkExpcetion("[WARN] listen() failed!");
        }
        return *this;
    };

    template<class Buf>
    long
    write(Buf *buffer, size_t len) {
        if (fd < 0) {
            throw std::exception();
        }
        auto ret = ::write(fd, (void *) buffer, len);
        if (ret < 0) {
            fprintf(stderr, "err when write\n");
        }
        return ret;
    };

    auto
    read(void *buffer, size_t len) {
        if (fd < 0) {
            throw std::exception();
        }
        return ::read(fd, buffer, len);
    };

    auto
    read(char *buffer) {
        if (fd < 0) {
            throw std::exception();
        }
        auto readIndex = 0;
        auto nRead = 0;
        auto len = sizeof buffer;
        do {
            nRead += ::read(fd, (char *) buffer + readIndex, len - readIndex);
        } while (nRead > 0);
        return nRead;
    };

    int
    accept(struct sockaddr_in &client_address, socklen_t &len_client_addr) {
        len_client_addr = sizeof(client_address);
        int conn_fd = ::accept(fd, (struct sockaddr *) &client_address, &len_client_addr);
        if (conn_fd < 0) {
            std::cerr << "accept error\n";
            throw std::exception();
        }
        return conn_fd;
    }

    ~Socket() {
        if (fd != -1)
            close(fd);
    }
};

namespace yumira::ipv4 {
    sockaddr_in newSockaddrIn(int port);

    extern std::string IP_ANY;
}
#endif //TINYWEB_SOCKET_H
