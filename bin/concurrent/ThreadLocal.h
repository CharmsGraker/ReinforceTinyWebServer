//
// Created by nekonoyume on 2022/1/6.
//

#ifndef TINYWEB_THREADLOCAL_H
#define TINYWEB_THREADLOCAL_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <map>
#include <string>
#include <sys/wait.h>
#include "../lock/locker.h"


typedef long tid_t;
#define gettid() syscall(__NR_gettid)

typedef std::map<std::string, std::string> local_map_t;

class ThreadLocal {
public:
    ThreadLocal() = default;

    static const int THREADLOCAL_FREE = 0;
    static const int THREADLOCAL_LOCKED = 1;

private:
    typedef std::map<tid_t, Locker *> ACL_t;

    static
    ACL_t *
    busy_visit_table() {
        static ACL_t busy_visit;
        return &busy_visit;
    }

    static std::map<tid_t, local_map_t *> *getThreadLocalMap() {
        printf("\tenter to getThreadLocalMap()\n");
        static std::map<tid_t, local_map_t *> thread_local_map;
        if (thread_local_map.empty()) {
            printf("\t[INFO] empty ThreadLocal map. Now create\n");
        }

        return &thread_local_map;
    }

    static
    void
    __init__current_thread_map() {
        tid_t tid = gettid();
        printf("current thread: %ld\n", tid);
        (*getThreadLocalMap())[tid] = new local_map_t();
    };

    static local_map_t *
    __current_thread_map() {
        if((*getThreadLocalMap())[gettid()] == nullptr) {
            __init__current_thread_map();
        }
        return (*getThreadLocalMap())[gettid()];
    }




public:

    static std::string
    get(const std::string &key) {
        return (*__current_thread_map())[key];
    }

    static void
    put(std::string key, std::string value) {
        (*__current_thread_map())[key] = value;
    }

    static void
    put(const char *key, std::string &value) {
        return put(std::string(key), value);
    }

    ~ThreadLocal() {
        for (auto localmapKV: *getThreadLocalMap()) {
            delete localmapKV.second;
        }
        //dont do this! because is a static member not allocate at runtime
//        delete getThreadLocalMap();
    }


    static const std::string getId() {
        /** will return a string like tid */
        return std::to_string(gettid());
    }
};

#endif //TINYWEB_THREADLOCAL_H