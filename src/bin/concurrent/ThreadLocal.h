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
#include <iostream>
#include <functional>
#include "../debug/dprintf.h"

typedef long tid_t;
#define gettid() syscall(__NR_gettid)

typedef std::map<std::string, void *> local_map_t;

using yumira::debug::DPrintf;

class ThreadLocal {
    static thread_local tid_t tid;
public:
    ThreadLocal() = default;

private:
    static std::map<tid_t, local_map_t *> *getThreadLocalMap() {
        tid_t tid = gettid();
        DPrintf("\t thread %ld enter to getThreadLocalMap()\n", tid);
        static std::map<tid_t, local_map_t *> thread_local_map;
        if (thread_local_map.empty()) {
            DPrintf("\t[INFO] empty ThreadLocal map. Now create\n");
        }

        return &thread_local_map;
    }

    static
    void
    initLocalMap() {
        tid_t tid = gettid();
        DPrintf("current thread: [%ld] init localMap\n", tid);
        (*getThreadLocalMap())[tid] = new local_map_t();
    };
public:

    static local_map_t *
    getLocalMap() {
        if ((*getThreadLocalMap())[gettid()] == nullptr) {
            initLocalMap();
        }
        return (*getThreadLocalMap())[gettid()];
    }

    template<typename T>
    static
    T *getAs(const std::string &key) {
        try {
            void *val = (*getLocalMap())[key];
            tid_t tid = gettid();
            DPrintf("%ld exit ThreadLocal getAs()\n", tid);
            return (T *) (val);
        } catch (std::exception &e) {
            return nullptr;
        }
    }


    template<typename T>
    static void
    put(const std::string &key, T &value) {
        (*getLocalMap())[key] = (void *) &value;
        tid_t tid = gettid();

        DPrintf("%ld exit ThreadLocal put()\n", tid);
    }

    template<typename T>
    static void
    put(const std::string &&key, const T &&value) {
        (*getLocalMap())[key] = (void *) new T(value);
        tid_t tid = gettid();

        DPrintf("%ld exit ThreadLocal put()\n", tid);
    }


    /** dont put a non dynamic memory object in ThreadLocal!
     * */
    static
    tid_t fetchId() {
        return gettid();
    }

    template<typename T>
    static void
    put(const std::string& key, T *value) {
        assert(nullptr != value);
        (*getLocalMap())[key] = (void *) value;
        if (tid == -1) {
            tid = fetchId();
        }
        DPrintf("%ld exit ThreadLocal put()\n", tid);
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
        if (tid != -1) {
            return std::to_string(tid);
        }
        tid = fetchId();
        return std::to_string(tid);
    }

    template<class T>
    static T *GetOrCreate(const std::string &key, std::function<T *()> constructor = []() {
        return new T();
    }) {
        T *ret = nullptr;
        if (!(ret = getAs<T>(key))) {
            try {
                ret = constructor();
                put(key, ret);
            } catch (std::exception &e) {
                e.what();
            }
        };
        return ret;
    }
};


#endif //TINYWEB_THREADLOCAL_H