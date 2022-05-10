#include "delay_queue.h"
#include "../../grakerThread/thread.h"
template<class E>
DelayQueue<E>::DelayQueue(const std::vector<E> &c) {
    for (auto &e: c) {
        q.push(e);
    }
}

template<class E>
bool
DelayQueue<E>::offer(const E &e) {
    std::lock_guard<std::mutex> lockGuard(lock);
    q.push(e);
    if (q.top() == e) {
        leader_thread = nullptr;
        cond_available.notify_all();
    }
    return true;
}

template<class E>
E &DelayQueue<E>::take() {
    std::unique_lock<std::mutex> lockGuard(lock);
    for (;;) {
        if (q.empty())
            cond_available.wait(lockGuard);

        E &&first = q.top();
        long delay = first.getDelay();
        if (delay <= 0) {
            q.pop();
            return first;
        }
        if(leader_thread)
            cond_available.wait(lockGuard);
        else {
            auto&& thisThread = Thread::getCurrentThread();

        }

    }
}

