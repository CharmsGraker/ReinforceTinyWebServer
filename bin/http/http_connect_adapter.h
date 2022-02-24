//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_HTTP_CONNECT_ADAPTER_H
#define TINYWEB_HTTP_CONNECT_ADAPTER_H

#include <mysql/mysql.h>
#include "../http/http_const_declare.h"

using namespace yumira;


template<class HTTPConnection>
class HttpConnectionAdapter {
private:
    HTTPConnection *real_conn;

    bool
    redirect(const char *url, url_t &redirect_url, http_req_method_t method) {
        if (!real_conn) {
            return false;
        }
        redirect_url = url_t(real_conn->redirect(url, method));
        return true;
    }

public:

    explicit HttpConnectionAdapter(HTTPConnection *connection) : real_conn(connection) {};

    MYSQL *
    query() {
        if (real_conn)
            return real_conn->mysql;
        return nullptr;
    }


    url_t
    redirect(const char *url, http_req_method_t method) {
        url_t redirect_url;
        if (redirect(url, redirect_url, method)) {
            return redirect_url;
        }
        return url_t::NULL_URL;
    }
};

#endif //TINYWEB_HTTP_CONNECT_ADAPTER_H
