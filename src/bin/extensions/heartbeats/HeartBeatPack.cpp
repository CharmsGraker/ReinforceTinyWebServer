#include "GrakerHeartBeat.h"

int HeartBeatPack::MAGIC = 0xcafebabb;

int
peekHBPack(int conn_fd) {
    printf("peekHBPack()\n");
    char buf[256];
    int n = recvfrom(conn_fd, (void *) buf, sizeof buf, MSG_DONTWAIT | MSG_PEEK, nullptr, nullptr);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("peekHBPack() recvfrom() nothing\n");

            return 1;
        }
        printf("peekHBPack() recvfrom() return negative\n");
        return -1;
    } else {
        if (*((int *) buf) == HeartBeatPack::MAGIC) {
            printf("peekHBPack() get HeartBeatPack\n");
            return 0;
        }
        printf("peekHBPack() receive new http Client\n");

        return 1;
    }
}