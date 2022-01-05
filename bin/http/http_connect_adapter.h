//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_HTTP_CONNECT_ADAPTER_H
#define TINYWEB_HTTP_CONNECT_ADAPTER_H
#include <mysql/mysql.h>
#include "../http/http_const_declare.h"

template<class HTTPConnection>
class HttpConnectionAdapter {
private:
    HTTPConnection *real_conn;
public:
    MYSQL *
    query() {
        return real_conn->mysql;
    }

    auto
    redirect(const char *url,http_req_method_t method) {
        return real_conn->redirect(url,method);
    }
};

#endif //TINYWEB_HTTP_CONNECT_ADAPTER_H
