//
// Created by nekonoyume on 2022/4/27.
//

#ifndef TINYWEB_DELAY_QUEUE_H
#define TINYWEB_DELAY_QUEUE_H

#include <boost/noncopyable.hpp>
#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>

template<class E>
class DelayQueue {
public:
    DelayQueue() {};

    DelayQueue(const std::vector<E> &collection);

    ~DelayQueue();

    bool offer(const E &e);
    E& take();
    bool remove(const E& e);
private:
    std::mutex lock;
    std::priority_queue<E, std::vector<E>> q;
    std::thread *leader_thread = nullptr;
    std::condition_variable cond_available;
};

#endif //TINYWEB_DELAY_QUEUE_H
