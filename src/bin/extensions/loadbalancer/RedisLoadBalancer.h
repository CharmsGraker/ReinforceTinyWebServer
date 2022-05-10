//
// Created by nekonoyume on 2022/4/10.
//

#ifndef REDISTEST_REDISLOADBALANCER_H
#define REDISTEST_REDISLOADBALANCER_H

#include "LoadBalancer.h"

namespace yumira {
    extern std::string REDIS_CLUSTER_KEY;

}
class RedisConnection;

class RedisLoadBalancer final : public LoadBalancer {
    RedisConnection *redisConn_;
    std::string lbGroupName_ = yumira::REDIS_CLUSTER_KEY;
    bool alive_;
public:
    RedisLoadBalancer() = delete;

    RedisLoadBalancer(const std::string &host, int port);


    RedisLoadBalancer(RedisConnection *connection);


    ~RedisLoadBalancer();

protected:
    bool
    _isAlive() override {
        return alive_;
    }

    bool
    _connect(const std::string &host, int port);

    RedisConnection *
    makeRedisConnection();

    bool doAddNode(const std::string &) override;

    bool doDelNode(const std::string &) override;

    std::string
    doSelect(const std::string &key) override;
};

#endif //REDISTEST_REDISLOADBALANCER_H
