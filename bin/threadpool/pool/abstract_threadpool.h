//
// Created by nekonoyume on 2022/1/19.
//

#ifndef TINYWEB_ABSTRACT_THREADPOOL_H
#define TINYWEB_ABSTRACT_THREADPOOL_H
template<typename T>
class AbstractThreadPool {
public:
    virtual bool append(T *task) = 0;

    AbstractThreadPool() = default;

    virtual void run() = 0;

    virtual ~AbstractThreadPool() {};
};
#endif //TINYWEB_ABSTRACT_THREADPOOL_H
