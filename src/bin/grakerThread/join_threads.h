//
// Created by nekonoyume on 2022/5/10.
//

#ifndef TINYWEB_JOIN_THREADS_H
#define TINYWEB_JOIN_THREADS_H
#include <vector>
#include <thread>
class join_threads {
    std::vector<std::thread>& threads;
public:
    join_threads(std::vector<std::thread>& threads_):threads(threads_) {

    }
    ~join_threads() {
        for(auto && t:threads) {
            if(t.joinable()) {
                t.join();
            }
        }
    }
};
#endif //TINYWEB_JOIN_THREADS_H
