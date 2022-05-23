//
// Created by nekonoyume on 2022/5/23.
//

#ifndef TINYWEB_EPOLLER_H
#define TINYWEB_EPOLLER_H

#include <sys/epoll.h>
#include "../timer/lst_timer.h"
#include <boost/noncopyable.hpp>

void
regist_fd_func(const int &epollFd, int fd, bool one_shot, int triggerMode);

struct Epoller {
    int epollFd;
public:
    Epoller(int size) {
        epollFd = epoll_create(size);
        if (epollFd < 0) {
            throw std::exception();
        }
        regist_fd = std::bind(regist_fd_func, epollFd, std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3);
    };

    int wait(struct epoll_event *events,
             int maxevents, int timeout) {
        if(epollFd < 0) {
            throw std::exception();
        }
        return epoll_wait(epollFd, events, maxevents, timeout);
    }

    std::function<void(int, bool, int)> regist_fd;

    void ctl_fd(int fd, int event_type, int TRIGMode) {
        Utils::modfd(epollFd, fd, event_type, TRIGMode);
    };

    void removefd(int fd) {
        printf("remove fd=%d from epoll\n",fd);
        Utils::removefd(epollFd, fd);
    }

    int
    getEpollFd() const {
        if(!regist_fd) {
            return -10;
        }
        return epollFd;
    }

    Epoller(Epoller &e) {
        epollFd = e.epollFd;
        regist_fd = nullptr;
    }

    Epoller(Epoller &&e) : epollFd(std::move(e.epollFd)), regist_fd(nullptr) {
    }

    Epoller &operator=(Epoller &e) {
        epollFd = e.epollFd;
        regist_fd = nullptr;
    }

    Epoller &operator=(Epoller &&e) {
        epollFd = std::move(e.epollFd);
        regist_fd = nullptr;
    }

    ~Epoller() {
        ::close(epollFd);
    }
};

#endif //TINYWEB_EPOLLER_H
