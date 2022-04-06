//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_POLLER_H
#define TINYWEB_POLLER_H

#include <boost/noncopyable.hpp>
#include "../eventloop/EventLoop.h"
class TimeStamp;
class Channel;
struct pollfd;

class Poller : public boost::noncopyable {
public:
    typedef std::vector<Channel *> ChannelList;

    Poller(EventLoop *eventLoop);

    template<class ChannelContainer>
    TimeStamp poll(int timeOutMs, ChannelContainer *channelContainer);

    // change our intrested event on 'channel'
    void
    updateChannel(Channel *channel);

    void assertInLoopThread() {
        ownLoop_->assertInLoopThread();
    }

    ~Poller() {

    };

private:
    EventLoop *ownLoop_;
    ChannelList Channel;
};

#endif //TINYWEB_POLLER_H
