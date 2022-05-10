#include "ustdnet.h"
#include <strings.h>

struct addrinfo *
host_serv(const char *host, const char *serv, int family, int sock_type) {
    int n;
    struct addrinfo hints, *res;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = family;
    hints.ai_socktype = sock_type;
    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        return NULL;
    return res;
};