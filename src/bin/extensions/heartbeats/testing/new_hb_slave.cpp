#include <iostream>
#include "../SocketHeartBeatThread.h"
#include "../../redis_utils/redis_utils.h"
#include "../cluster/Cluster.h"
#include <vector>
#include <cstring>
#include "../../../timer/CTimer.h"

int LISTENQ = 10;



static int PORT = 9007;
const InetAddress redisAddress = {"localhost", 6379};
const std::string clkey = "grakerWebServerLb";

void registerNode(const InetAddress &nodeIp) {
    RedisConnection conn;
    conn.Connect(redisAddress);
    auto ret = conn.Zadd(clkey, nodeIp.toString(), 2);
    if (!ret) {
        printf("add node to Redis failed\n");
    } else {
        printf("add node to Redis success\n");

    }
    return;
}

void
test_redis() {
    RedisConnection conn;
    conn.Connect(redisAddress);
    auto vec = conn.Zrange(clkey, 0, -1);
    for (auto &&ele: vec) {
        cout << ele << endl;
    }
}

vector<int> slavePorts{9007, 9008, 9009};
std::string master_key = "Idle-Master";
typedef void(SocketHeartBeatThread::*f)(int);

void
test_hb() {
    Cluster<> cluster(redisAddress, clkey);

    vector<int> slaveSockFdVec;
    auto n_slaves = slavePorts.size();
    vector<int> time_(n_slaves, 50);


    for (int i = 0; i < n_slaves; ++i) {
        slaveSockFdVec.emplace_back(BuildSocket(slavePorts[i]));

        printf("set slave sockFd=%d\n", slaveSockFdVec[i]);
    }

    for (auto &&slavePort: slavePorts)
        registerNode({"127.0.0.1", slavePort});
    SocketHeartBeatThread masterThread;
    masterThread.setTimeSpan(20);
    masterThread.setClusterCenter(cluster);

    // init slave sock done
    std::vector<SocketHeartBeatThread> hb_threads(n_slaves);

    for (int i = 0; i < n_slaves; ++i) {
        auto &&thread = hb_threads[i];
        thread.setClusterCenter(cluster);
        thread.setTimeSpan(time_[i]);
    }

    // start thread
    std::cout << "Main Thread: " << std::this_thread::get_id() << std::endl;
    masterThread.start();
    sleep(1);
    for (int i = 0; i < hb_threads.size(); ++i) {
        auto &&thread = hb_threads[i];
        thread.setFdOnSlave(slaveSockFdVec[i]);
        thread.start();
    }

    // wait candidate done.
//    cluster.waitUntilCandidateDone();

    // avoid Main Thread exit
    std::cerr << "main thread wait for exit\n";
    getchar();

}

int main() {
    test_hb();
//    thread.join();

}