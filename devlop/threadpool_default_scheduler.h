
#ifndef TINYWEB_THREADPOOL_DEFAULT_SCHEDULER_H
#define TINYWEB_THREADPOOL_DEFAULT_SCHEDULER_H


#include "../bin//threadpool/threadpool.h"
#include "../bin/http/http_conn.h"



/**
 * @author: CharmsGraker
 * defaultScheduler means use class :
 * http_conn for HTTP Connection Impl,
 * connection_pool for SqlConnection Impl
 * can adapt TaskType for other missions.
 * */
template<typename TaskType>
class ThreadPoolTaskDefaultScheduler : public default_threadpool_task_scheduler_t<TaskType> {
    connection_pool *sql_conn_pool;  //数据库
    int m_actor_model;          //模型切换

private:

public:
    ThreadPoolTaskDefaultScheduler(threadPool<TaskType> *task_pool, int actor_model, connection_pool *sqlConnectionPool)
            : m_actor_model(actor_model) {
        sql_conn_pool = sqlConnectionPool;
        default_threadpool_task_scheduler_t<TaskType>::getParent(this)->setScheduler(
                task_pool);
    }


    void call(http_conn *task);

    ~ThreadPoolTaskDefaultScheduler() {};
};

#endif //TINYWEB_THREADPOOL_DEFAULT_SCHEDULER_H
