#include "Socket.h"
namespace yumira::ipv4 {
    sockaddr_in newSockaddrIn(int port) {
        struct sockaddr_in address;
        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        return address;
    }

    std::string IP_ANY = "0.0.0.0";

}