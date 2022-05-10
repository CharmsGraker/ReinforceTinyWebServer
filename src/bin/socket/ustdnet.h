#ifndef REDISTEST_USTDNET_H
#define REDISTEST_USTDNET_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string>
#define LISTENQ 10
class InetAddress;

extern "C" {struct addrinfo *
host_serv(const char *, const char *, int, int);

}

char *
getLocalIP(const char *host, int idx);

struct addrinfo *
HostServ(const char *host, const char *service);

struct addrinfo *
HostServ(const InetAddress &inetAddress);

struct addrinfo *
HostServ(const std::string& inetString);

int BuildSocket(int port);

#endif //REDISTEST_USTDNET_H
