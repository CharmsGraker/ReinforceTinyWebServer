//
// Created by nekonoyume on 2022/1/6.
//
#include "ThreadLocal.h"
#include <iostream>

using namespace std;

int main() {
    ThreadLocal tl;
    cout << "tid:" << gettid();
    ThreadLocal::put("a", "2");
    cout << "tid:" << gettid() << "put a:2" << endl;
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    auto tid = pthread_create(&thread, &attr, [](void *) -> void * {
        ThreadLocal::put("a", "3");
        sleep(1);
        cout << "tid: " << gettid() << "get " << ThreadLocal::get("a");
    }, 0);
    cout << "tid:" << gettid() << "get a with value: " << ThreadLocal::get("a") << endl;
    pthread_join(thread,nullptr);
}