//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_EVENTLOOP_H
#define TINYWEB_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include "../grakerThread/thread.h"


#include <iostream>
#include <assert.h>
#include <vector>

class Poller;
class Channel;

class EventLoop : public boost::noncopyable {
public:
    EventLoop();

    ~EventLoop();

    void loop();

    void quit();

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortWhileNotInLoop();
        }
    };

    bool isInLoopThread() const {
        return threadId_ == Thread::getCurrentThread();
    }

    // return EventLoop Object on current Thread
    // may be NULL
    static EventLoop *getCurrentEventLoop();
    void
    updateChannel(Channel * channel);
private:
    void abortWhileNotInLoop();

private:
    typedef std::vector<Channel*> channel_list;
    typedef channel_list::iterator channel_list_iter;

    bool looping_;
    bool quit_;

    const typename Thread::thread_id_t threadId_;
    boost::scoped_ptr<Poller> poller_;

    channel_list activeChannels_;
    int kPollTimeMs;
};

#endif //TINYWEB_EVENTLOOP_H
