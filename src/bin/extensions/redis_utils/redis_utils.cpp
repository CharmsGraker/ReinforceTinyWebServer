#include "redis_utils.h"
#include "../../socket/InetAddress.h"

bool RedisConnection::Connect(const InetAddress &inetAddress) {
    return Connect(inetAddress.getHost(), inetAddress.getPort());
}