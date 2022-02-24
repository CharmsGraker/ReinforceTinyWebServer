
#ifndef TINYWEB_REQUEST_H
#define TINYWEB_REQUEST_H

#include <string>
#include <map>
#include "../all_exception.h"
#include "../container/hashmap.h"

#include "./../../utils/string_utils.h"
#include "../http/http_const_declare.h"
#include "../netroute/urlparser.h"
#include "../netroute/parsedurl.h"

using namespace string_util;

using namespace yumira;

namespace yumira {
    class http_conn;
}
//template<class urlParser, class ConnectionAdapter>
class Request {
    std::string _route;


    Request(ParsedUrl addr, http_req_method_t _method, http_conn *httpConn) :
            pa_addr(addr), method(_method), connection(httpConn) {};

    Request() {};

    parameter_t &
    get_parsed_param() {
//        printf("get_parsed_param()\n");
        return pa_addr.KV;
    }

    http_conn *connection;

    static Request *
    makeNewRequest(std::string &raw_url, http_req_method_t method, http_conn *httpConn) {
        return new Request(UrlParser<ParsedUrl>::parse(raw_url), method, httpConn);
    }

public:
    http_req_method_t method;

    ParsedUrl pa_addr;


    /** url means the href route,not include params */



    static Request *
    makeRequest(Request *request, std::string &raw_url, http_req_method_t method, http_conn *httpConn) {
        printf("[makeRequest]");
        if (!request) {
            return makeNewRequest(raw_url, method, httpConn);
        } else {
            return new(request)Request(UrlParser<ParsedUrl>::parse(raw_url), method, httpConn);
        }
    }

    http_conn *
    getConnection() {
        return connection;
    }

    parameter_t &args() {
        try {
            printf("into args()\n");
            std::cout << get_parsed_param().empty() << std::endl;
            if (get_parsed_param().empty()) {
                throw NullException("null hashmap!");
            }
            return get_parsed_param();
        } catch (NullException &e) {
            e.what();
            exit(1);
        }
    }


    std::string
    route() const {
        return pa_addr.path;
    }

    static
    void
    release(Request *req) {
        delete req;
        req = nullptr;
    }
};

#endif //TINYWEB_REQUEST_H