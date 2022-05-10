//
// Created by nekonoyume on 2022/1/16.
//

#ifndef TINYWEB_TASK_H
#define TINYWEB_TASK_H

class Task {
public:
    Task()= default;
    virtual void
    process()=0;
    virtual ~Task()= default;
};

class TaskScheduler:boost::noncopyable {
public:
    TaskScheduler()=default;

     virtual void schedule(Task* task) =0;

     virtual ~TaskScheduler() = default;

};

#endif //TINYWEB_TASK_H
