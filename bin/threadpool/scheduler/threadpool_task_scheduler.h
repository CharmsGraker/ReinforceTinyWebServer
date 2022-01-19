
#ifndef TINYWEB_THREADPOOL_TASK_SCHEDULER_H
#define TINYWEB_THREADPOOL_TASK_SCHEDULER_H
#include "../../threadpool/task.h"

/**
 * @author: CharmsGraker
 * this class is normal interface for abstractTaskScheduler, avoid child Class do many duplicated work
 * */
template<typename TaskType,
        typename threadPoolClass,
        typename SqlPoolClass>
class ThreadPoolTaskScheduler : public abstractTaskScheduler {
public:
    ThreadPoolTaskScheduler() = default;

    void setScheduler(threadPoolClass *pool) {
        pool->setTaskScheduler(this);
    }

    void call(Task *task) override {
        return call((TaskType *) task);
    };

    static ThreadPoolTaskScheduler<TaskType,threadPoolClass,SqlPoolClass>* getParent(ThreadPoolTaskScheduler * ptr) {
        return (ThreadPoolTaskScheduler<TaskType,threadPoolClass,SqlPoolClass>*)(ptr);
    }

    virtual void call(TaskType *task) {};

    ~ThreadPoolTaskScheduler() {};
};

template<typename task_t>
using default_threadpool_task_scheduler_t = ThreadPoolTaskScheduler<task_t, threadPool<task_t>, connection_pool>;


#endif //TINYWEB_THREADPOOL_TASK_SCHEDULER_H
