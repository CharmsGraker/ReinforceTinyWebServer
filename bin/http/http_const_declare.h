//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_HTTP_CONST_DECLARE_H
#define TINYWEB_HTTP_CONST_DECLARE_H

#include "../container/hashmap.h"

typedef HashMap <std::string, std::string> parameter_t;

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


#endif //TINYWEB_HTTP_CONST_DECLARE_H