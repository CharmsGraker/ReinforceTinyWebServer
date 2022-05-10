
#ifndef TINYWEB_THREADPOOL_DEFAULT_SCHEDULER_H
#define TINYWEB_THREADPOOL_DEFAULT_SCHEDULER_H


#include "../../bin/threadpool/threadpool.h"
#include "../../bin/http/http_conn.h"


/**
 * @author: CharmsGraker
 * defaultScheduler means use class :
 * http_conn for HTTP Connection Impl,
 * connection_pool for SqlConnection Impl
 * can adapt TaskType for other missions.
 * */
template<typename TaskType>
class ThreadPoolTaskDefaultScheduler : public ThreadPoolTaskScheduler<TaskType> {
public:
    typedef TaskType task_type;
    typedef typename ThreadPoolTaskScheduler<task_type>::threadpool_type threadpool_type;
    typedef typename ThreadPoolTaskScheduler<task_type>::sqlpool_type sqlpool_type;
    typedef typename ThreadPoolTaskScheduler<task_type>::type parent_type;

private:
    sqlpool_type *sql_conn_pool;  //数据库
    int m_actor_model;          //模型切换
public:

    ThreadPoolTaskDefaultScheduler(threadpool_type *task_pool, int actor_model, sqlpool_type *sqlConnectionPool)
            : parent_type(task_pool),m_actor_model(actor_model),sql_conn_pool(sqlConnectionPool) {

    }

    void operator()(http_conn *task) override;


    ~ThreadPoolTaskDefaultScheduler() {};
};

#endif //TINYWEB_THREADPOOL_DEFAULT_SCHEDULER_H
