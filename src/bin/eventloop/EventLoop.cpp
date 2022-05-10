#include "EventLoop.h"
#include "../channel__/Channel.h"
#include "../poller/Poller.h"

__thread EventLoop *t_loopInThisThread = 0;

EventLoop::EventLoop() : looping_(false),
                         threadId_(Thread::getCurrentThread()) {
    if (t_loopInThisThread) {
        std::cerr << "another EventLoop has bind on thread: " << threadId_ << "at " << this;
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs,&activeChannels_);
        for (channel_list_iter it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }
    }
    looping_ = false;


}

EventLoop *
EventLoop::getCurrentEventLoop() {
    return t_loopInThisThread;
}
void
EventLoop::quit() {
    quit_ = false;
}

void
EventLoop::updateChannel(Channel *channel) {
    return poller_->updateChannel(channel);
}