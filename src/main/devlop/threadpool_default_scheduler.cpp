#include "threadpool_default_scheduler.h"

// specialize template method
template<>
void
ThreadPoolTaskDefaultScheduler<http_conn>::operator()(http_conn *taskPtr)  {
    if (!taskPtr)
        return;
    auto&& task = *taskPtr;
    printf("scheduler invoke\n");
    if (1 == this->m_actor_model) {
        if (0 == task.m_state) {
            if (task.read_once()) {
                task.improv = 1;
                connectionRAII mysqlcon(&task.mysql, this->sql_conn_pool);
                task.process();
            } else {
                task.improv = 1;
                task.timer_flag = 1;
            }
        } else {
            if (task.write()) {
                task.improv = 1;
            } else {
                task.improv = 1;
                task.timer_flag = 1;
            }
        }
    } else {
        connectionRAII mysqlConnect(&task.mysql, this->sql_conn_pool);
        task.process();
    }
}


