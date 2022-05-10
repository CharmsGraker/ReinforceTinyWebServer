
#ifndef TINYWEB_REQUEST_H
#define TINYWEB_REQUEST_H

#include <string>
#include <map>
#include <utility>
#include "../all_exception.h"
#include "../container/hashmap.h"

#include "../extensions/string_utils/string_utils.h"
#include "../http/http_const_declare.h"
#include "urlparser.h"
#include "parsedurl.h"
#include <json/json.h>

using namespace string_util;

using namespace yumira;

namespace yumira {
    class http_conn;
}

class Request {
    std::string _route;
    http_conn *connection;
    Json::Value *json_value;
    parameter_t KV;
private:
    Request() : _route(), connection(nullptr), json_value(nullptr), KV() {

    };

    Request(ParsedUrl url, http_req_method_t _method, http_conn *httpConn) :
            parsedUrl(std::move(url)),
            method(_method),
            connection(httpConn),
            json_value(nullptr),
            KV() {};

    parameter_t &
    get_parsed_param() {
//        printf("get_parsed_param()\n");
        return parsedUrl.getParams();
    }


public:
    http_req_method_t method;

    ParsedUrl parsedUrl;
    std::string result;

public:
    /** url means the href route,not include params */

    static Request *
    Create() {
        return new Request();
    }

    Request &
    setMethod(http_req_method_t method_) {
        method = method_;
        return *this;
    }

    Request &
    setConn(http_conn *conn_) {
        connection = conn_;
        return *this;
    };

    Request &
    setParseUrl(const std::string &raw_url) {
        parsedUrl = UrlParser<ParsedUrl>::parse(raw_url);
        return *this;

    }

    http_conn *
    getConnection() {
        return connection;
    }

    bool
    isResRequest() {
        return parsedUrl.isResource();
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

    void
    setJsonObject(Json::Value &obj) {
        json_value = &obj;
    };

    ParsedUrl &
    getParsedUrl() {
        return parsedUrl;
    }

    static
    void
    Release(Request *req) {
        if (req) {
            delete req;
            req = nullptr;
        }
    }


    std::string
    operator[](const std::string &key_) {
        auto iter = KV.find(key_);
        printf("HashMap find() called\n");
        if (iter != KV.end())
            return iter->second;
        // search in Json
        if (!json_value)
            return {};
        auto val = json_value->get(key_, "null").asString();
        if (val == "null")
            return {};
        return val;
    }
};


#endif //TINYWEB_REQUEST_H