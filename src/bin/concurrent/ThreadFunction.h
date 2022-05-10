//
// Created by nekonoyume on 2022/4/11.
//

#ifndef REDISTEST_THREADFUNCTION_H
#define REDISTEST_THREADFUNCTION_H

#include "Runnable.h"
#include <string>
#include <thread>
#include <iostream>

class CTimer;

class ThreadFunction : public Runnable {
    typedef std::function<void(void)> func_t;
public:
    std::thread::id
    getSelfId() const;

    ThreadFunction(const std::string &, int timeSpan = 100);

    ThreadFunction();

    func_t
    setRunFunction(func_t &&f) {
        run_f = f;
    }

    void run() {
        if (run_f)
            run_f();
        else {
            // empty method
        }
    }

    void
    join() const;
    int
    setTimeSpan(int timeMs) {
        auto old_t = timeMsSpan;
        timeMsSpan = timeMs;
        return old_t;
    }

    void start() {
        if (!running_) {
            running_ = true;
            registerLoop(timeMsSpan);
        }
    };

    void
    registerLoop(int time_span, bool async = true);

    void
    registerOnce();


    ~ThreadFunction();

protected:
    CTimer *timer_;
private:
    static int ref_count;
    func_t run_f;
    int timeMsSpan;
    bool running_;
};

#endif //REDISTEST_THREADFUNCTION_H
