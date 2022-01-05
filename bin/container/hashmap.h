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
    std::map<K, V> *container;
    bool throw_exception = false;
public:
    HashMap() : HashMap(false) {};

    HashMap(bool throw_exception) : throw_exception(throw_exception), container(new std::map<K, V>()) {};

    V &
    operator[](const K &key) {
        // please catch Exception above!!!!!
        if (container->count(key) <= 0) {
            if (throw_exception) throw NotExistKeyException();
            std::cout << "[HashMap<K,V>] visit key: " << key << ", but get null." << std::endl;
        }
        return container->operator[](key);
    }
//    V&
//    operator[] (K key) {
//        return container->operator[]()
//    }

    bool empty() {
        return container->empty();
    }
};
#endif //TINYWEB_HASHMAP_H
