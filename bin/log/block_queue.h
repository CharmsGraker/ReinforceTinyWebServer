#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <mutex>
#include <condition_variable>
#include "../lock/locker.h"

using namespace std;

template<class T>
class block_queue {
private:
    //判断队列是否满了
    bool isFull() {
        if (m_size >= m_max_size) return true;
        return false;
    }

    //判断队列是否为空
    bool isEmpty() {
        if (0 == m_size) {
            return true;
        }
        return false;
    }

public:
    bool full() {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        return isFull();
    }

    explicit block_queue(int max_size = 1000) {
        if (max_size <= 0) {
            throw runtime_error("[block_queue] size can not be negative");
        }

        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    void clear() {
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    ~block_queue() {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        if (m_array != nullptr)
            delete[] m_array;
    }


    //返回队首元素
    bool
    front(T &value) {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        if (0 == m_size) {
            return false;
        }
        value = m_array[m_front];
        return true;
    }

    //返回队尾元素
    bool back(T &value) {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        if (0 == m_size) {
            return false;
        }
        value = m_array[m_back];
        return true;
    }

    int size() {
        int tmp = 0;
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        tmp = m_size;
        return tmp;
    }

    int max_size() {
        int tmp = 0;

        std::lock_guard<std::mutex> lockGuard(m_mutex);
        tmp = m_max_size;

        return tmp;
    }

    //往队列添加元素，需要将所有使用队列的线程先唤醒
    //当有元素push进队列,相当于生产者生产了一个元素
    //若当前没有线程等待条件变量,则唤醒无意义
    bool push(const T &item) {
        std::unique_lock<std::mutex> uniqueLock(m_mutex);
        while (isFull()) {
            onFull.wait(uniqueLock);
        }

        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        m_size++;
        onEmpty.notify_one();
        return true;
    }

    //pop时,如果当前队列没有元素,将会等待条件变量
    bool pop(T &item) {
        std::unique_lock<std::mutex> uniqueLock(m_mutex);
        while (isEmpty()) {
            onEmpty.wait(uniqueLock);
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        onFull.notify_all();
        return true;
    }

    //增加了超时处理
    bool pop(T &item, int ms_timeout) {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, nullptr);
        std::unique_lock<std::mutex> uniqueLock(m_mutex);
        bool waiting = false;
        while (isEmpty()) {
            if (waiting)
                return false;
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            onEmpty.wait(uniqueLock, t);
            waiting = true;
        };


        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        onFull.notify_all();
        return true;
    }

private:
    std::mutex m_mutex;
    std::condition_variable onFull;
    std::condition_variable onEmpty;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#endif
