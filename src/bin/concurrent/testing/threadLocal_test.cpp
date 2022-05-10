#include "../ThreadLocal.h"
#include <iostream>

using namespace std;
thread_local int a = 111;

int main() {
    ThreadLocal tl;
    std::string A = "a";
    int var = 2;
    ThreadLocal::put<int>(A, var);
    cout << "tid:" << gettid() << "put a:2" << endl;
    a = ThreadLocal::getAs<int>(A);
    cout << "global a=" << a << endl;
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    auto tid = pthread_create(&thread, &attr, [](void *) -> void * {
        ThreadLocal::put<int>("a", 3);
        cout << "child thread see a=" << a << endl;
        sleep(1);
        cout << "tid: " << gettid() << "get " << ThreadLocal::getAs<int>("a");
    }, 0);
    cout << "tid:" << gettid() << "get a with value: " << ThreadLocal::getAs<int>("a") << endl;
    pthread_join(thread, nullptr);
}