//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_THREAD_H
#define TINYWEB_THREAD_H

#include <unistd.h>
#include <thread>
#include <boost/noncopyable.hpp>

class Thread : public boost::noncopyable {
public:
    typedef std::thread::id thread_id_t;
public:
    static auto
    getCurrentThread() {
        return std::this_thread::get_id();
    };

    template<typename _Callable, typename... _Args>
    explicit
    Thread(_Callable &&__f, _Args &&... __args):thread_(__f, __args...), threadId_(thread_.get_id()) {

    };

    thread_id_t
    getId() const {
        return threadId_;
    }

    ~Thread();

private:
    std::thread thread_;
    std::thread::id threadId_;
};


#endif //TINYWEB_THREAD_H
