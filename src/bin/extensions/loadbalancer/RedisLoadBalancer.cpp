#include "RedisLoadBalancer.h"
#include "../redis_utils/redis_utils.h"
#include "../../exceptions/exceptions.h"


RedisConnection *
RedisLoadBalancer::makeRedisConnection() {
    return new RedisConnection();
}

RedisLoadBalancer::~RedisLoadBalancer() {
    delete redisConn_;
    redisConn_ = nullptr;
}

RedisLoadBalancer::RedisLoadBalancer(const std::string &host, int port) : LoadBalancer(), redisConn_(nullptr) {
    redisConn_ = makeRedisConnection();
    if (!redisConn_->Connect(host, port)) {
        std::cerr << redisConn_->showConnError();
        alive_ = false;
    } else {
        std::cerr << "Connect to Redis " << host << ":" << port << " ";
        alive_ = true;
    }
    std::cerr << __FILE__ << std::endl;
}

RedisLoadBalancer::RedisLoadBalancer(RedisConnection *connection) : alive_(true),
                                                                    redisConn_(connection),
                                                                    LoadBalancer(lbGroupName_) {
    if (!redisConn_->isConnected()) {
        std::cerr << redisConn_->showConnError() << std::endl;
        alive_ = false;
    }
}

bool
RedisLoadBalancer::doAddNode(const std::string &key) {
    printf("invoke RedisLoadBalancer::doAddNode()\n");
    assert(redisConn_);
    if (!redisConn_->isConnected() && !redisConn_->tryConnect(3))
        throw RedisConnectFailException();

    auto hash = hash_strategy::getHash(key);
    auto addCnt = redisConn_->Zadd(lbGroupName_, key, hash);
    count_ += addCnt;
    return addCnt > 0;
}

bool RedisLoadBalancer::doDelNode(const std::string &nodeIdentifier) {
    if (!redisConn_->isConnected() && !redisConn_->tryConnect(3))
        throw RedisConnectFailException();
    auto removeCnt = redisConn_->Zrem(lbGroupName_, nodeIdentifier);
    count_ += removeCnt;
    return removeCnt > 0;
}

std::string
RedisLoadBalancer::doSelect(const std::string &key) {
    if (!redisConn_->isConnected() && !redisConn_->tryConnect(3))
        throw RedisConnectFailException();
    auto hash = hash_strategy::getHash(key);
    cout << hash << endl;
    std::string chosenIpPort;
    auto group = redisConn_->ZrangeByScoreGreaterThan(lbGroupName_, hash);
    if (!group.empty()) {
        chosenIpPort = group[0];
    } else {
        printf("branch end\n");
        chosenIpPort = redisConn_->Zrange(lbGroupName_, 0, 0)[0];
    }
    return chosenIpPort;
}
