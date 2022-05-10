#include "ThreadFunction.h"
#include "../timer/CTimer.h"


int ThreadFunction::ref_count = 0;

ThreadFunction::ThreadFunction() : timeMsSpan(1), running_(false) {
    timer_ = new CTimer("timer-clock" + std::to_string(ref_count++));

};

ThreadFunction::ThreadFunction(const std::string &timerName, int timeSpan) : timeMsSpan(timeSpan), running_(false) {
    timer_ = new CTimer(timerName);

};

std::thread::id
ThreadFunction::getSelfId() const {
    if (!running_)
        return std::this_thread::get_id();
    return timer_->getThread()->get_id();
}

ThreadFunction::~ThreadFunction() {
    delete timer_;
}


void ThreadFunction::registerLoop(int time_span, bool async) {
    if (async)
        timer_->AsyncLoop(time_span, &ThreadFunction::run, this);
    else
        timer_->SyncLoop(time_span, &ThreadFunction::run, this);
}

void ThreadFunction::registerOnce() {
    timer_->AsyncOnce(&ThreadFunction::run, this);
}

void ThreadFunction::join() const {
    return timer_->join();
}