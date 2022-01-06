//
// Created by nekonoyume on 2022/1/6.
//

#ifndef TINYWEB_STORAGE_H
#define TINYWEB_STORAGE_H

#include <string>
#include <vector>
#include <map>

#define STORAGE_BADCAST (void *)

class StorageMap {
    std::map<std::string, void *> container;
public:
    StorageMap() : container() {};


    template<class T>
    int put(const std::string &key, T ele) {
        container[key] = STORAGE_BADCAST ele;
        return 0;
    }

    template<class T>
    T &get(const std::string &key) {
        return (T &) container[key];
    }

    void
    clear() {
        return container.clear();
    }
};

#endif //TINYWEB_STORAGE_H
