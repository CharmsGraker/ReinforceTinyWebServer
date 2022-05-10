//
// Created by nekonoyume on 2022/4/17.
//

#ifndef REDISTEST_TRIETREE_H
#define REDISTEST_TRIETREE_H

#include <vector>
#include <string>
#include <exception>

// only allow alpha and digits
// 62 * 8 =
typedef enum {
    LEAF,
    NOT_LEAF
} trie_node_type;

typedef unsigned long long dirty_marker_type;

template<class T>
struct TrieNode {
private:
    int __size;
    bool need_free;
public:
    trie_node_type state;
    T *data;
    TrieNode **children;

    TrieNode(int size_) : __size(size_), state(NOT_LEAF), children(nullptr), data(nullptr) {};

    ~TrieNode() {
        int pos = 0;
        for (int i = 0; i < __size; ++i) {
            auto cur = *(children + pos);
            if (cur) {
                delete cur;
                cur = nullptr;
            }
        }
    }
};

template<class T>
class TrieTree {
public:
    typedef T data_type;
private:
    TrieNode<data_type> *root;
    int n_child;
private:
    std::pair<TrieNode<data_type> *, int>
    __find(const std::string &keyword) {
        if (keyword.empty())
            return {nullptr, -1};
        TrieNode<data_type> *node = root;
        TrieNode<data_type> *prev = nullptr;
        auto slen = keyword.length();
        int pos = -1;
        for (auto i = 0; i < slen; ++i) {
            pos = getCode(keyword[i]);
            if (!*(node->children + pos)) {
                if (prev)
                    *prev = *node;
                return {prev, -1};
            }
            prev = node;
            node = *(node->children + pos);
        }
        return {prev, pos};
    }

    int getCode(char x) {
        if (std::isdigit(x)) {
            return x - '0';
        } else if (std::isupper(x)) {
            return x - 'A' + 10 + 26;
        } else if (std::islower(x)) {
            return x - 'a' + 10;
        } else {
            throw std::exception();
        }
    }

public:

    TrieTree() : root(new TrieNode<data_type>(n_child)), n_child(62) {

    };


    bool
    insert(const std::string &s, T *data_ = nullptr) {
        bool ret = true;
        auto p = root;
        auto slen = s.length();
        for (auto i = 0; i < slen; ++i) {
            int pos = getCode(s[i]);
            if (!p->children) {
                p->children = new TrieNode<data_type> *[n_child];
            }
            if (!*(p->children + pos)) {
                auto &&next = *(p->children + pos);
                next = new TrieNode<data_type>(n_child);
                next->children = new TrieNode<data_type> *[n_child];
            }
            p = *(p->children + pos);
        }
        p->state = trie_node_type::LEAF;
        if (data_)
            p->data = data_;
        return ret;
    }

    bool contain(const std::string &keyword) {
        auto ret = __find(keyword);
        return ret.second != -1 && (ret.first + ret.second)->state == trie_node_type::LEAF;
    }

    data_type *
    operator[](const std::string &key) {
        auto ret = __find(key);
        auto idx = ret.second;
        if (idx != -1) {
            return (*(ret.first->children + idx))->data;
        }
        return nullptr;
    }

    bool
    remove(const std::string &keyword, data_type *val = nullptr) {
        auto ret = __find(keyword);
        if (ret.second)
            return false;
        auto prev = ret.first;
        // unlink
        prev.children + ret.second = nullptr;
        return true;
    }

    ~TrieTree() {
        delete root;
        root = nullptr;
    }
};

#endif //REDISTEST_TRIETREE_H
