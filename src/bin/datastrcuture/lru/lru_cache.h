//
// Created by nekonoyume on 2022/5/11.
//

#ifndef TINYWEB_LRU_CACHE_H
#define TINYWEB_LRU_CACHE_H
#include <vector>
struct lru_cache {
    int capacity_;
public:
    lru_cache(int capacity) {

    }
    bool
    put();
    bool get();
};
#endif //TINYWEB_LRU_CACHE_H
