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

RedisLoadBalancer::RedisLoadBalancer(const std::string &host,
                                     int port,
                                     const std::string &lbGroupName) :
        LoadBalancer(lbGroupName),
        r_host(host),
        r_port(port),
        lbGroupName_(lbGroupName),
        redisConn_(nullptr) {
    if (lbGroupName_.empty()) {
        throw NullPointerException("cluster key can not be null!");
    }
    redisConn_ = makeRedisConnection();
    cout << host <<": "<< port << endl;
    if (!redisConn_->Connect(host, port)) {
        std::cerr << redisConn_->showConnError() << std::endl;
        std::cerr << __FILE__ << std::endl;
        alive_ = false;
        throw RedisConnectFailException();
    } else {
        std::cout << "Connect to Redis " << host << ":" << port << " ";
        std::cout << __FILE__ << std::endl;

        alive_ = true;
    }
}

RedisLoadBalancer::RedisLoadBalancer(RedisConnection *connection, const std::string &lbGroupName) : alive_(true),
                                                                                                    redisConn_(
                                                                                                            connection),
                                                                                                    lbGroupName_(
                                                                                                            lbGroupName),
                                                                                                    LoadBalancer(
                                                                                                            lbGroupName) {
    if (lbGroupName_.empty()) {
        throw NullPointerException("cluster key can not be null!");
    }
    if (!redisConn_->isConnected()) {
        std::cerr << redisConn_->showConnError() << std::endl;
        alive_ = false;
    }
}

bool
RedisLoadBalancer::doAddNode(const std::string &key) {
    printf("invoke RedisLoadBalancer::doAddNode()\n");
    assert(redisConn_);
    if (!redisConn_->isConnected() && !redisConn_->Connect(r_host, r_port)) {
        throw RedisConnectFailException();
    }

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
//    cout << hash << endl;
    std::string chosenIpPort;
    auto group = redisConn_->ZrangeByScoreGreaterThan(lbGroupName_, hash);
    if (!group.empty()) {
        chosenIpPort = group[0];
    } else {
        printf("branch end\n");
        auto collection = redisConn_->Zrange(lbGroupName_, 0, 0);
        if (collection.empty()) {
            throw IndexOutOfRangeException();
        }
        chosenIpPort = collection[0];
    }
    return chosenIpPort;
}
