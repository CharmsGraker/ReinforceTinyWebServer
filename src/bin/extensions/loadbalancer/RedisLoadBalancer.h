//
// Created by nekonoyume on 2022/4/10.
//

#ifndef REDISTEST_REDISLOADBALANCER_H
#define REDISTEST_REDISLOADBALANCER_H

#include "LoadBalancer.h"

class RedisConnection;

class RedisLoadBalancer final : public LoadBalancer {
    std::string r_host = "";
    int r_port = -1;
    RedisConnection *redisConn_;
    std::string lbGroupName_;
    bool alive_;
public:
    RedisLoadBalancer(const std::string &host, int port, const std::string &lbGroupName);

    RedisLoadBalancer(const InetAddress &address,
                      const std::string &lbGroupName = "") : RedisLoadBalancer(
            address.getHost(),
            address.getPort(),
            lbGroupName) {
    };

    RedisLoadBalancer(RedisConnection *connection, const std::string &lbGroupName = "");

    RedisLoadBalancer() = delete;

    RedisLoadBalancer(RedisLoadBalancer &) = delete;

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
