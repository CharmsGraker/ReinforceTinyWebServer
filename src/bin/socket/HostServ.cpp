#include "ustdnet.h"
#include "InetAddress.h"

struct addrinfo *
HostServ(const char *host, const char *service) {
    return host_serv(host, service, 0, SOCK_STREAM);
}

struct addrinfo *
HostServ(const InetAddress &inetAddress) {
    return HostServ(inetAddress.getHost().c_str(),
                    std::to_string(inetAddress.getPort()).c_str());
}

struct addrinfo *
HostServ(const std::string &inetString) {
    auto it = inetString.find(":");
    if (it == inetString.npos)
        return nullptr;
    auto host = inetString.substr(0, it);
    auto port = inetString.substr(it + 1);
    printf("parse host=%s, port=%s\n",host.c_str(),port.c_str());
    return HostServ(host.c_str(), port.c_str());
}