//
// Created by nekonoyume on 2022/3/4.
//
#include "boost/noncopyable.hpp"

#ifndef TINYWEB_MUTEXLOCKGUARD_H
#define TINYWEB_MUTEXLOCKGUARD_H
class MutexLockGuard:public boost::noncopyable {
    MutexLock& mutex_lock_;
    MutexLockGuard(MutexLock& lock):mutex_lock_(lock) {
        mutex_lock_.lock();
    }
    ~MutexLockGuard() {
        mutex_lock_.unlock();
    }
};
#endif //TINYWEB_MUTEXLOCKGUARD_H
