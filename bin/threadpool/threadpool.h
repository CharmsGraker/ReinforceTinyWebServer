#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include <map>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "task.h"

template<typename T>
class AbstractPool {
public:
    virtual bool append(T *task) = 0;

    AbstractPool() = default;

    virtual void run() = 0;

    virtual ~AbstractPool() {};
};

template<typename T>
class threadPool : AbstractPool<T> {
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadPool(int thread_number = 8, int max_request = 10000);

    ~threadPool();

    bool append(T *task, int state);

    bool append(T *task);


    std::map<std::string, int> KV;

    void setTaskScheduler(abstractTaskScheduler*scheduler) {
        __scheduler = scheduler;
    };

private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void *worker(void *arg);
    abstractTaskScheduler * __scheduler;


    [[noreturn]] void run();

    int doSchedule(T *task) {
        try {
            __scheduler->call(task);
        } catch (exception &e) {
            e.what();
            return -1;
        }
        return 0;
    };

private:
    int m_thread_number;        //线程池中的线程数
    int m_max_requests;         //请求队列中允许的最大请求数
    pthread_t *m_threads;       //描述线程池的数组，其大小为m_thread_number
    std::list<T *> m_workQueue; //请求队列
    Locker queue_mutex;       //保护请求队列的互斥锁
    Semaphore m_queuestat;            //是否有任务需要处理

};

template<typename T>
threadPool<T>::threadPool(int thread_number, int max_requests):
          m_thread_number(thread_number),
          m_max_requests(max_requests),
          m_threads(nullptr),
          KV(std::map<std::string, int>()) {
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();

    if (!(m_threads = new pthread_t[m_thread_number]))
        throw std::exception();

    for (int i = 0; i < thread_number; ++i) {
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i])) {
            delete[] m_threads;
            throw std::exception();
        }
    }
//    putVar(varName(actor_model),actor_model);
}

template<typename T>
threadPool<T>::~threadPool() {
    delete __scheduler;
    delete[] m_threads;
}

template<typename T>
bool threadPool<T>::append(T *task, int state) {
    if (queue_mutex.lock() && m_workQueue.size() >= m_max_requests) {
        queue_mutex.unlock();
        return false;
    }
    task->m_state = state;
    m_workQueue.push_back(task);
    queue_mutex.unlock();
    m_queuestat.post();
    return true;
}

template<typename T>
bool threadPool<T>::append(T *task) {
    if (queue_mutex.lock() && m_workQueue.size() >= m_max_requests) {
        queue_mutex.unlock();
        return false;
    }
    m_workQueue.push_back(task);
    queue_mutex.unlock();
    m_queuestat.post();
    return true;

}

template<typename T>
void *threadPool<T>::worker(void *arg) {
    auto *pool = (threadPool *) arg;
    pool->run();
    return pool;
}

template<typename T>
[[noreturn]] void threadPool<T>::run() {
    while (true) {
        if (m_queuestat.wait() && !queue_mutex.trylock() && queue_mutex.lock() && m_workQueue.empty()) {
            queue_mutex.unlock();
            continue;
        }
        // poll a request
        T *task = m_workQueue.front();
        m_workQueue.pop_front();
        queue_mutex.unlock();
        if (!task)
            continue;

        // To use Strategy Pattern.
        auto ret = doSchedule(task);
        if (ret) {
            exit(-1);
        }
    }
}

#endif
