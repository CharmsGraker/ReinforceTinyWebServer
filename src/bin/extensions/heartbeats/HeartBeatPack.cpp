#include "GrakerHeartBeat.h"
#include "../../debug/dprintf.h"

int HeartBeatPack::MAGIC = 0xcafebabb;

using yumira::debug::DPrintf;

int
peekHBPack(int conn_fd) {
    DPrintf("peekHBPack()\n");
    char buf[256];
    int n = recvfrom(conn_fd, (void *) buf, sizeof buf, MSG_DONTWAIT | MSG_PEEK, nullptr, nullptr);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            DPrintf("peekHBPack() recvfrom() nothing\n");

            return 1;
        }
        DPrintf("peekHBPack() recvfrom() return negative\n");
        return -1;
    } else {
        if (*((int *) buf) == HeartBeatPack::MAGIC) {
            DPrintf("peekHBPack() get HeartBeatPack\n");
            return 0;
        }
        DPrintf("peekHBPack() receive new http Client\n");

        return 1;
    }
}