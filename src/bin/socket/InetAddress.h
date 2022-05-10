//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_INETADDRESS_H
#define TINYWEB_INETADDRESS_H

#include <string>
#include "ustdnet.h"

class InetAddress {
    std::string host;
    int port;
public:
    InetAddress(const char *host_ = "") : InetAddress(host_, 0) {};

    InetAddress(const char *_host, int _port) : host(_host), port(_port) {

    };

    InetAddress(const std::string &_host, int _port) : host(_host), port(_port) {

    };

    std::string
    getHost() const {
        return host;
    }

    int
    getPort() const {
        return port;
    }

    std::string
    toString() const {
        return Serialize(*this);
    }

    static
    std::string
    Serialize(const InetAddress& inetAddress) {
        return inetAddress.host + ":" + std::to_string(inetAddress.port);
    }

    bool
    operator==(const InetAddress &address) {
        return port == address.port && host == address.host;
    }

    bool operator==(const std::string& str) {
        auto idx = str.find(":");

        return str.substr(0, idx) == host && port == stoi(str.substr(idx + 1));
    }

    bool
    operator!=(const InetAddress &address) {
        return !operator==(address);
    }

    bool
    empty() {
        return host.empty();
    }

    bool
    isEqual(std::string &str) {
        return this->operator==(str);
    }

    static InetAddress
    GetLocalAddress() {

    }

    static std::string
    GetLocalHost(int idx = 0) {
        return getLocalIP("localhost", idx);
    };

    static InetAddress
    Deserialize(const std::string &str) {
        printf("deserialize: %s\n",str.c_str());
        auto idx = str.find(":");
        auto host = str.substr(0, idx);
        auto port = stoi(str.substr(idx + 1));
        return {host, port};
    }
};

template<>
class std::hash<InetAddress> {
public:

    size_t operator()(const InetAddress &obj) const {
        auto string_hasher = std::hash<std::string>();
        auto int_hasher = std::hash<int>();

        auto h1 = string_hasher(obj.getHost());
        auto h2 = int_hasher(obj.getPort());
        return h1 ^ h2;
    };


};

#endif //TINYWEB_INETADDRESS_H
