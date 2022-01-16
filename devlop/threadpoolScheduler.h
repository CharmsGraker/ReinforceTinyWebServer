
#ifndef TINYWEB_THREADPOOLSCHEDULER_H
#define TINYWEB_THREADPOOLSCHEDULER_H

#include "../bin/threadpool/threadpool.h"
#include "../bin/http/http_conn.h"
#include "../bin/threadpool/task.h"

class taskScheduler : public abstractTaskScheduler {
    connection_pool *sql_conn_pool;  //数据库
    int m_actor_model;          //模型切换

private:

public:
    taskScheduler(threadPool<http_conn> *task_pool, int actor_model, connection_pool *sqlConnectionPool) :
            m_actor_model(actor_model),
            sql_conn_pool(sqlConnectionPool) {

        setScheduler(task_pool);
    }

    void setScheduler(threadPool<http_conn> *pool) {
        pool->setTaskScheduler(this);
    }

    void call(Task *task) override {
        return __call((http_conn *) task);
    }

    void __call(http_conn *task) ;
    ~taskScheduler(){};
};

#endif //TINYWEB_THREADPOOLSCHEDULER_H
