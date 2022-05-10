#include "ustdnet.h"
#include <cstring>
int BuildSocket(int port) {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
        std::abort();

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    // accept all
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    auto ret = bind(listenFd, (const struct sockaddr *) &server_addr, sizeof server_addr);
    if (ret < 0)
        std::abort();
    ret = listen(listenFd, LISTENQ);
    if (ret < 0)
        std::abort();

    return listenFd;
}