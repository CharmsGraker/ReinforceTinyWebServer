//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_REQUEST_H
#define TINYWEB_REQUEST_H

#include <string>
#include <map>
#include "../all_exception.h"
#include "../container/hashmap.h"

#include "./../../utils/string_utils.h"
#include "../http/http_const_declare.h"

using namespace string_util;


template<class urlParser, class ConnectionAdapter>
class Request {
    std::string _route;
    typedef decltype(urlParser::parse("just for example to calculate return type.")) parsed_url_t;

    Request(parsed_url_t addr, http_req_method_t _method) :
            pa_addr(addr), method(_method) {};

    Request() {};

    parameter_t &
    get_parsed_param() {
        return pa_addr.KV;
    }

private:
    ConnectionAdapter adapter;
public:
    http_req_method_t method;

    typedef ConnectionAdapter adapter_t;
    parsed_url_t pa_addr;


    /** url means the href route,not include params */

    static Request
    makeRequest(std::string raw_url, http_req_method_t method) {
        printf("[makeRequest]");
        return Request(urlParser::parse(raw_url), method);
    }

    HashMap<std::string, std::string> &args() {
        if (get_parsed_param().empty()) {
            throw NullException("null hashmap!");
        }
        return get_parsed_param();
    }


    std::string
    route() const {
        return pa_addr.path;
    }

    ConnectionAdapter
    getAdapter() {
        return adapter;
    }


};

#endif //TINYWEB_REQUEST_H
