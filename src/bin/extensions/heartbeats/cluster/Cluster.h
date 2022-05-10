//
// Created by nekonoyume on 2022/4/13.
//

#ifndef REDISTEST_CLUSTER_H
#define REDISTEST_CLUSTER_H

#include <string>
#include <mutex>
#include <boost/noncopyable.hpp>
#include "../../redis_utils/redis_utils.h"

class RedisConnection;

class ClusterObject : boost::noncopyable {
public:
    template<class C, class K>
    ClusterObject(C, K) {

    }

    ClusterObject();

    virtual bool
    isAllocKey() = 0;

    virtual
    const std::string
    getMasterKey() = 0;

    virtual
    const std::string
    getClusterKey() = 0;

    virtual
    const InetAddress
    getCenter() = 0;

    virtual void
    setCandidate(bool isCandidate) = 0;

    virtual void
    waitUntilCandidateDone() = 0;



protected:
    std::mutex &
    getMutex() &{
        return cluster_mutex_;
    };
private:
    std::mutex cluster_mutex_;
};


template<class A=InetAddress, class K=std::string, class M=std::string>
class Cluster : public ClusterObject {
    Cluster() = delete;

public:
    typedef A cluster_center_t;
    typedef K cluster_key_t;
    typedef M master_key_t;
public:
    Cluster(const A &, const K &);

    Cluster(const A &, const K &, const M &);

    const K
    getClusterKey() override {
        return clusterKey_;
    };

    const M
    getMasterKey() override {
        return MasterKey_;
    };

    const A
    getCenter() override {
        return clusterCenter_;
    }

    virtual
    bool isAllocKey() override {
        return alloc_cluster_key_;
    };

    void
    setClusterKey(const A &cl_registerIp, const std::string &cl_key) {

        if (alloc_cluster_key_)
            return;
        std::lock_guard<std::mutex> lockGuard(getMutex());
        if (!alloc_cluster_key_) {
            clusterCenter_ = cl_registerIp;
            clusterKey_ = cl_key;
            alloc_cluster_key_ = true;
        }
    }

public:
    void
    waitUntilCandidateDone() {

    };

private:
    A clusterCenter_;
    K clusterKey_;
    M MasterKey_;
protected:
    bool alloc_cluster_key_;
};

template<>
class Cluster<InetAddress, std::string, std::string> : public ClusterObject {
public:
    typedef InetAddress cluster_center_t;
    typedef std::string cluster_key_t;
    typedef std::string master_key_t;
public:
    Cluster(const InetAddress &clusterIp, const std::string &clusterKey) : ClusterObject(clusterIp, clusterKey),
                                                                           clusterCenter_(clusterIp),
                                                                           clusterKey_(clusterKey),
                                                                           masterKey_("Idle-Master"),
                                                                           alloc_cluster_key_(true),
                                                                           candidate_done_(false) {

    }

    const cluster_center_t
    getCenter() override {
        return clusterCenter_;
    }

    bool isAllocKey() override {
        printf("isAllocKey\n");
        return alloc_cluster_key_;
    }

    const master_key_t
    getMasterKey() override {
        return masterKey_;
    }

    const cluster_key_t
    getClusterKey() override {
        return clusterKey_;
    }

    void
    setClusterKey(const cluster_center_t &cl_registerIp, const cluster_key_t &cl_key) {
        if (isAllocKey())
            return;
        std::lock_guard<std::mutex> lockGuard(getMutex());
        if (!isAllocKey()) {
            clusterCenter_ = cl_registerIp;
            clusterKey_ = cl_key;
            alloc_cluster_key_ = true;
        }
    }

    void
    setCandidate(bool state) override {
        std::lock_guard<std::mutex> lockGuard(getMutex());
        candidate_done_ = !state;
        if (candidate_done_) {
            candidate_cond.notify_all();
        }
    };


    void waitUntilCandidateDone() {
        std::unique_lock<std::mutex> uLock(getMutex());
        while (candidate_done_) {
            candidate_cond.wait(uLock);
        };
    };

private:
    cluster_center_t clusterCenter_;
    cluster_key_t clusterKey_;
    master_key_t masterKey_;
    std::condition_variable candidate_cond;
protected:
    bool alloc_cluster_key_;
    bool candidate_done_;
    // current master version
    unsigned long long int version_;
};


#endif //REDISTEST_CLUSTER_H
