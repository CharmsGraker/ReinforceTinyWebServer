
#ifndef TINYWEB_THREADPOOL_TASK_SCHEDULER_H
#define TINYWEB_THREADPOOL_TASK_SCHEDULER_H

#include "../task.h"

/**
 * @author: CharmsGraker
 * this class is normal interface for abstractTaskScheduler, avoid child Class do many duplicated work
 * */
template<typename T,
        typename threadPoolClass=threadPool<T *>,
        typename SqlPoolClass=connection_pool>
struct ThreadPoolTaskScheduler : public TaskScheduler {
public:
    typedef T task_type;
    typedef threadPoolClass threadpool_type;
    typedef SqlPoolClass sqlpool_type;
    typedef ThreadPoolTaskScheduler<task_type, threadpool_type, sqlpool_type> type;

    ThreadPoolTaskScheduler() = default;

    explicit ThreadPoolTaskScheduler(threadpool_type *task_pool) {
        setScheduler(task_pool);
    };

    void setScheduler(threadpool_type *pool) {
        pool->setTaskScheduler(this);
    }


    void schedule(Task *task) override {
        return this->operator()((task_type *) (task));
    };

    virtual void operator()(task_type *) {};

    ~ThreadPoolTaskScheduler() {};
};

template<typename task_t>
using default_threadpool_task_scheduler_t = ThreadPoolTaskScheduler<task_t, threadPool<task_t>, connection_pool>;


#endif //TINYWEB_THREADPOOL_TASK_SCHEDULER_H
