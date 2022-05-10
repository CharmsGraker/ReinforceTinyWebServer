#include "../redis_utils.h"
#include "../../loadbalancer/RedisLoadBalancer.h"

std::string redisHost = "localhost";
int redisPort = 6379;
std::string yumira::REDIS_CLUSTER_KEY = "grakerWebServerLb";

void test_loadbalancer() {
    ////    RedisConnection *redis_conn = new RedisConnection();
//
//    if (!redis_conn->Connect(host, port))
//        std::abort();
//    redis_conn->Set("a2", "1");


    RedisLoadBalancer redisLoadBalancer(redisHost, redisPort);
    cout << redisLoadBalancer.addNode("127.0.0.1:9007") << endl;
    cout << redisLoadBalancer.addNode("127.0.0.1:1907") << endl;

    cout << redisLoadBalancer.addNode("127.0.0.1:19007") << endl;
    auto selected = redisLoadBalancer.select("1314183");
    std::cout << "select: " << selected << endl;

    cout << redisLoadBalancer.delNode("127.0.0.1:19007") << endl;
    selected = redisLoadBalancer.select("/");
    std::cout << "select: " << selected << std::endl;
};

void test_redis_cas() {
    RedisConnection connection;
    connection.Connect(redisHost, redisPort);
    connection.CompareAndSet("a", "1", "2");
}

int main() {
    test_redis_cas();
}