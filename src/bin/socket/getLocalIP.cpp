#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

char *
getLocalIP(const char *host, int idx) {
    auto host_ent = gethostbyname(host);
    return inet_ntoa(*(struct in_addr *) (host_ent->h_addr_list[idx]));
}
