//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_HASHMAP_H
#define TINYWEB_HASHMAP_H

#include "../all_exception.h"

#include <iostream>
#include <map>

template<typename K, typename V>
class HashMap {
    std::map<K, V> container;
    bool throw_exception = false;
public:
    HashMap() : HashMap(false) {};

    HashMap(bool throw_exception) : throw_exception(throw_exception), container() {};

    V &
    operator[](K &&key) {
        // please catch Exception above!!!!!
        if (container.count(key) <= 0) {
            if (throw_exception) throw NotExistKeyException();
            std::cout << "[HashMap<K,V>] visit key: " << key << ", but get null." << std::endl;
        }
        return container.operator[](key);
    }

    V &
    operator[](const K &key) {
        return container.operator[](key);
    }

    bool empty() {
        return container.empty();
    }

    auto&&
    begin() {
        return container.begin();
    }

    auto&&
    end() {
        return container.end();
    }

    auto&&
    find(const K & key) {
        printf("map find\n");
        auto&&  iter = container.find(key);
        std::cout << iter;
        return iter;
    }
};

#endif //TINYWEB_HASHMAP_H