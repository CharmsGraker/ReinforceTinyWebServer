#include "GrakerHeartBeat.h"

SocketConn::SocketConn() : sockFd(-1), dt_heart_(0), observe_(false) {};

SocketConn::SocketConn(int Fd) : sockFd(Fd), dt_heart_(0), observe_(false) {
    printf("create SocketConn fd=%d\n", sockFd);
}

void
SocketConn::resetExpire() {
    printf("resetExpire() socket fd=%d\n", sockFd);
    dt_heart_ = 0;
    observe_ = false;
}

bool
SocketConn::isTimeOut(const time_t &dt) {
    dt_heart_ += dt;
    if (dt_heart_ >= CLIENT_SPAN_SEC) {
        printf("client with fd=%d timeout\n hb_time=%ld", sockFd, dt_heart_);

        return true;
    } else
        return false;
}

int
SocketConn::getSockFd() {
    return sockFd;
}

time_t SocketConn::CLIENT_SPAN_SEC = 3000;