#include "ingress.h"


bool yumira::debug::debug_flag = true;

InetAddress redisIP{"127.0.0.1",6379};
int main() {
    yumira::Ingress ingressor(redisIP);
    ingressor.listen(9003);
}