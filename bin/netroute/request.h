
#ifndef TINYWEB_REQUEST_H
#define TINYWEB_REQUEST_H

#include <string>
#include <map>
#include <utility>
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

class Request {
    std::string _route;
    http_conn *connection;
    parameter_t KV;

private:
    Request(ParsedUrl url, http_req_method_t _method, http_conn *httpConn) :
            parsedUrl(std::move(url)),
            method(_method),
            connection(httpConn) {};

    parameter_t &
    get_parsed_param() {
//        printf("get_parsed_param()\n");
        return parsedUrl.getParams();
    }

    static Request *
    makeNewRequest(std::string &raw_url, http_req_method_t method, http_conn *httpConn) {
        return new Request(UrlParser<ParsedUrl>::parse(raw_url), method, httpConn);
    }

public:
    http_req_method_t method;

    ParsedUrl parsedUrl;

public:
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

    bool
    isResRequest() {
        return parsedUrl.isResRequest();
    }
    parameter_t &
    getParams() {
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


    ParsedUrl&
    getParsedUrl()  {
        return parsedUrl;
    }

    static
    void
    release(Request *req) {
        if (req) {
            delete req;
            req = nullptr;
        }
    }
};



#endif //TINYWEB_REQUEST_H