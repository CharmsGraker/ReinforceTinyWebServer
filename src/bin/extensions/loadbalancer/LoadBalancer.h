//
// Created by nekonoyume on 2022/4/10.
//

#ifndef TINYWEB_LOADBALANCER_H
#define TINYWEB_LOADBALANCER_H

#include "../../socket/InetAddress.h"
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>

class LoadBalancer;


class LoadBalancer {
public:
    typedef std::vector<InetAddress> InetAddressList;
    typedef std::map<int, std::string> nodeSortedMap;

    class ConsistentHash {
        static constexpr int p = 16777619;

    public:
        ConsistentHash() {

        }

        static int
        getHash(const std::string &key) {
            int hash = (int) 2166136261L;
            for (int i = 0; i < key.length(); i++) {
//                std::cout<< (int)(key[i])<<std::endl;
                hash = (hash ^ int(key[i])) * p;
            }
            hash += hash << 13;
            hash ^= hash >> 7;
            hash += hash << 3;
            hash ^= hash >> 17;
            hash += hash << 5;
            //如果算出来的值为负数则取其绝对值
            if (hash < 0) {
                hash = abs(hash);
            }
            // printf("%s ,hash=%d\n",key.c_str(),hash);
            return hash;
        }
    };


public:
    LoadBalancer() : LoadBalancer("load-balancer" + std::to_string(LoadBalancer::use_ref_count++)) {}

    LoadBalancer(const std::string &secret) : count_(0), secretKey(secret) {};

    LoadBalancer(const std::vector<std::string> &list) : count_(0) {
        for (auto &&node: list) {
            doAddNode(node);
        }
    }

    bool
    addNode(const std::string &key) {
        printf("invoke addNode()\n");
        return doAddNode(key);
    };

    bool
    delNode(const std::string &key) {
        return doDelNode(key);
    };

    std::string
    select(const std::string &key) {
        printf("invoke select()\n");

        return doSelect(key);
    };

    bool isAlive() {
        return _isAlive();
    }



protected:
    virtual bool _isAlive() {
        return true;
    };
    virtual bool
    doAddNode(const std::string &key) {
        printf("invoke LoadBalancer::doAddNode()\n");
        if (!clusterNodes.empty() && clusterNodes.find(key) != clusterNodes.end()) {
            return false;
        }
        clusterNodes.insert(key);
        auto h = hash_strategy::getHash(key);
        clusterHash[h] = key;
        ++count_;
        return true;
    }

    virtual bool
    doDelNode(const std::string &nodeIdentifier) {
        auto it = clusterNodes.find(nodeIdentifier);
        if (!clusterNodes.empty() && it != clusterNodes.end()) {
            clusterNodes.erase(it);
            --count_;
            return true;
        }
        return false;
    }

    virtual std::string
    doSelect(const std::string &key) {
        if (clusterHash.empty()) {
            printf("empty cluster!\n");
            std::abort();
        };
        auto hash = hash_strategy::getHash(key);
        auto l_bound = clusterHash.lower_bound(hash);
        if (l_bound == clusterHash.end()) {
            return clusterHash.begin()->second;
        } else {
            return l_bound->second;
        }
    }

protected:
    typedef ConsistentHash hash_strategy;
    int count_;
private:
    static int use_ref_count;
    nodeSortedMap clusterHash;
    std::string secretKey;
    std::unordered_set<std::string> clusterNodes;
};

#endif //TINYWEB_LOADBALANCER_H
