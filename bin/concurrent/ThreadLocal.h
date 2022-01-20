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


typedef long tid_t;
#define gettid() syscall(__NR_gettid)

typedef std::map<std::string, std::string> local_map_t;

class ThreadLocal {
public:
    ThreadLocal() = default;

    static const int THREADLOCAL_FREE = 0;
    static const int THREADLOCAL_LOCKED = 1;

private:

    static std::map<tid_t, local_map_t *> *getThreadLocalMap() {
        tid_t tid = gettid();
        printf("\t thread %ld enter to getThreadLocalMap()\n", tid);
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
        printf("current thread: [%ld] init localMap\n", tid);
        (*getThreadLocalMap())[tid] = new local_map_t();
    };

    static local_map_t *
    __current_thread_map() {
        if ((*getThreadLocalMap())[gettid()] == nullptr) {
            __init__current_thread_map();
        }
        return (*getThreadLocalMap())[gettid()];
    }


public:

    static std::string
    get(const std::string &key) {
        auto val = (*__current_thread_map())[key];
        tid_t tid = gettid();
        printf("%ld exit ThreadLocal get()\n", tid);
        return val;
    }

    static void
    put(std::string key, std::string value) {
        (*__current_thread_map())[key] = value;
        tid_t tid = gettid();

        printf("%ld exit ThreadLocal put()\n", tid);

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