#include "epoller.h"
void
regist_fd_func(const int &epollFd, int fd, bool one_shot, int triggerMode) {
    printf("regist fd=%d\n",fd);
    Utils::regist_fd(epollFd, fd, one_shot, triggerMode);
}