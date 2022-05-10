//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_POLLER_H
#define TINYWEB_POLLER_H

#include <boost/noncopyable.hpp>
#include "../eventloop/EventLoop.h"
#include <map>
class TimeStamp;
class Channel;
struct pollfd;

class Poller : public boost::noncopyable {
public:
    typedef std::vector<Channel *> ChannelList;
    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int,class Channel*> ChannelMap;
    Poller(EventLoop *eventLoop);

    TimeStamp
    poll(int timeOutMs, ChannelList *channelContainer);

    // change our intrested event on 'channel'
    void
    updateChannel(Channel *channel);

    void assertInLoopThread() {
        ownLoop_->assertInLoopThread();
    }

    ~Poller() {

    };
private:
    void
    fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

private:
    EventLoop *ownLoop_;
    PollFdList pollfds_;
    ChannelList Channel;
    ChannelMap channels_;
};

#endif //TINYWEB_POLLER_H
