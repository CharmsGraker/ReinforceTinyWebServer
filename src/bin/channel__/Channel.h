//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_CHANNEL_H
#define TINYWEB_CHANNEL_H
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

// forward declaration
class EventLoop;

class Channel : public boost::noncopyable {
public:
    typedef boost::function<void()> event_callback_t;

    Channel(EventLoop *eloop, int monitorFd);

    int fd() const {
        return fd_;
    };

    int events() const {
        return events_;
    };

    void set_revents(int revents) {
        revents_ = revents;
    }

    bool emptyEvents() const {
        return events_ == K_NONE_EVENT;
    }

    void enableRead() {
        events_ |= K_READ_EVENT;
        __updateEvents();
    };

    int
    setState(int new_state) {
        auto old_state = stateBits;
        stateBits = new_state;
        return old_state;
    }

    void
    handleEvent();


    EventLoop *getEventLoop() {
        return evLoop_;
    };


    void setReadCallback(const event_callback_t &callback) {
        readCallback_ = callback;
    };

    void setErrorCallback(const event_callback_t &callback) {
        errorCallback_ = callback;
    };

    void setWriteCallback(const event_callback_t &callback) {
        writeCallback_ = callback;
    };

    ~Channel();

private:
    void
    __updateEvents();

private:
    const static int K_NONE_EVENT;
    const static int K_READ_EVENT;
    const static int K_WRITE_EVENT;

    const static int EMPTY_STATE;

    int fd_;
    int stateBits;
    int events_;
    int revents_;
    EventLoop *evLoop_;
    event_callback_t readCallback_;
    event_callback_t writeCallback_;
    event_callback_t errorCallback_;

};

#endif //TINYWEB_CHANNEL_H
