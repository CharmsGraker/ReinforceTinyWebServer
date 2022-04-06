#include "channel.h"
#include <poll.h>
#include <iostream>
#include "../eventloop/EventLoop.h"

const int Channel::K_NONE_EVENT = 0;
const int Channel::K_READ_EVENT = POLLIN | POLLPRI;
const int Channel::K_WRITE_EVENT = POLLOUT;
const int Channel::EMPTY_STATE = 0;

Channel::Channel(EventLoop *evloop, int monitorFd) : evLoop_(evloop),
                                                     fd_(monitorFd),
                                                     events_(K_NONE_EVENT),
                                                     revents_(K_NONE_EVENT),
                                                     stateBits(EMPTY_STATE) {

}
void Channel::handleEvent() {
    if(revents_ & POLLNVAL) {
        std::cerr<<"Channel::handleEvent() POLLNVAL";
    }
}

void Channel::__updateEvents() {
    evLoop_->updateChannel(this);
}