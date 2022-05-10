//
// Created by nekonoyume on 2022/1/6.
//

#ifndef TINYWEB_STORAGE_H
#define TINYWEB_STORAGE_H

#include <string>
#include <utility>
#include <vector>
#include <map>


class Node {
public:
    Node() = default;

    virtual ~Node(){};
};

class StringNode : public Node {
public:
    explicit StringNode(std::string str) {
        real_str = new std::string(std::move(str));
    }

    ~StringNode() override {
        delete real_str;
    }

    std::string *real_str;
};


class StorageMap {
    std::map<std::string, Node *> *container;
public:
    StorageMap() : container(new std::map<std::string, Node *>()) {};


    template<class T>
    int put(const std::string &key, T *ele) {
        (*container)[key] = ((Node *) ele);
        return 0;
    }

    template<class T>
    T &get(const std::string &key) {
        return (T &) (*container)[key];
    }

    auto
    clear() {
        return container->clear();
    }

    auto begin() {
        return container->begin();
    }

    auto end() {
        return container->end();
    }

    auto empty() {
        return container->empty();
    }

    ~StorageMap() {
        for (auto &ele: *container) {
            delete ele.second;
        }
        delete container;
    }

};

#endif //TINYWEB_STORAGE_H
