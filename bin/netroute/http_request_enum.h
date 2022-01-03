//
// Created by nekonoyume on 2022/1/3.
//

#ifndef TINYWEB_HTTP_REQUEST_ENUM_H
#define TINYWEB_HTTP_REQUEST_ENUM_H


class Request;
#define REQUEST_CAST (Request&)

typedef Request* RequestPtr;


enum http_req_method_t {
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATH
};

class Request {
public:
    http_req_method_t method;
    std::string url;

    Request(std::string url, http_req_method_t method) : url(url), method(method) {};

    static Request makeRequest(const std::string &url, const http_req_method_t &method) {
        return {url, method};
    }
};

#endif //TINYWEB_HTTP_REQUEST_ENUM_H
