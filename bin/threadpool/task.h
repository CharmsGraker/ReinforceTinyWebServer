//
// Created by nekonoyume on 2022/1/16.
//

#ifndef TINYWEB_TASK_H
#define TINYWEB_TASK_H

class Task {
public:
    Task(){};
    virtual void process()=0;
    virtual ~Task(){};
};

class abstractTaskScheduler {
public:
    abstractTaskScheduler(){};

     virtual void call(Task *task)=0;
     virtual ~abstractTaskScheduler() {};

};

#endif //TINYWEB_TASK_H
