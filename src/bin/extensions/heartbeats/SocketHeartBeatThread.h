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

class HBWorker;
class HBMaster;
class HBSlave;
class ClusterObject;

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

#endif //REDISTEST_THREAD_FUNC_H
