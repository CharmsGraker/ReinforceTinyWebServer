//
// Created by nekonoyume on 2022/4/11.
//

#ifndef REDISTEST_THREAD_FUNC_H
#define REDISTEST_THREAD_FUNC_H

#include <mutex>
#include <condition_variable>
#include "../../socket/InetAddress.h"
#include <functional>
#include "../../concurrent/ThreadFunction.h"
#include "../loadbalancer/LoadBalancer.h"
#include "../loadbalancer/RedisLoadBalancer.h"
#include "../../config/config.h"
#include "GrakerHeartBeat.h"

class HBWorker;
class HBMaster;
class HBSlave;
class ClusterObject;
using namespace yumira::dev;
namespace yumira {
    namespace idle {
        extern std::shared_ptr<LoadBalancer> loadBalancer;
    }
};
namespace yumira{
    namespace heartbeat {
        void addExcludeAddress(const InetAddress &inetAddress);
    }
}

class SocketHeartBeatThread : public ThreadFunction {
public:
    typedef enum {
        SLAVE,
        MASTER,
        CANDIDATE
    } hb_thread_type;
public:
    SocketHeartBeatThread();

    void
    setClusterCenter(ClusterObject *cluster) {
        cluster_ = cluster;
    };

    void
    setClusterCenter(ClusterObject &cluster) {
        cluster_ = &cluster;
    };

    void
    tryCandidate();

    void run() override;

    ~SocketHeartBeatThread();

    bool reactFd(int);


    int getSocketFd() const;

    std::string
    printState() {
        std::string ret;
        switch (type) {
            case CANDIDATE: {
                ret = "CANDIDATE";
                break;
            }
            case MASTER: {
                ret = "MASTER";
                break;
            }
            case SLAVE : {
                ret = "SLAVE";
                break;
            }
            default: {
                ret = "UNKNOWN";
                break;
            }
        }
        return ret;
    };

    template<typename callable, typename... arguments>
    void setCallBack(callable &&f, arguments &&...  args) {
        std::function<typename std::result_of<callable(arguments...)>::type()>
                task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...)); //绑定任务函数或lambda成function
        slave_callback = std::move(task);

    }

    void setFdOnSlave(int serverFd) {
        setCallBack(&SocketHeartBeatThread::setSocketFd, this, serverFd);
    }
public:

    hb_thread_type type;
protected:

    void
    flushHeartBeatInfo();
    std::function<void()> addExcludeAddressCallback;

    void setSocketFd(int);

protected:
    std::function<void()> slave_callback;

private:
    HBWorker *coordinator_;

    int HEARTBEAT_PORT = 29999;
    int heartBeat_flush_freq = 10;
    bool worker_alive;

private:
    static int N_CONN_RETRY;
    ClusterObject *cluster_;
};

template<template<typename > class  W,typename T>
void initIdleThread(W<T> * server);

template<template<class> class W, class T>
void initIdleThread(class W<T> *server) {
    server->f_checkConnFd = peekHBPack;
    auto cb = [](){
        using yumira::idle::loadBalancer;
        if (!loadBalancer.get()) {
            loadBalancer.reset(
                    new RedisLoadBalancer(idleConfig.REDIS_HOST, idleConfig.REDIS_PORT, idleConfig.REDIS_CLUSTER_KEY));
        }
        InetAddress currentInetAddress(commonConfig.serverIp, commonConfig.serverPort);
        loadBalancer->addNode(currentInetAddress.toString());
    };
    server->addCallbackToLast(cb);
    printf("register callback\n");
}
#endif //REDISTEST_THREAD_FUNC_H
