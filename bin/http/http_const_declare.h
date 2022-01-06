//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_HTTP_CONST_DECLARE_H
#define TINYWEB_HTTP_CONST_DECLARE_H

#include "../container/hashmap.h"
#include "../lock/locker.h"
#include "../container/storage.h"
#include "../container/url_storage.h"

namespace yumira {
    class WebServer;

    typedef url_storage url_t;
    typedef StorageMap storage_t;
    typedef std::map<std::string, std::string> parameter_t;
    typedef WebServer *ServerPtr;
    typedef WebServer yumira_server_t;

    extern Locker m_lock;
    extern std::map<std::string, std::string> userTable;

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
};
#endif //TINYWEB_HTTP_CONST_DECLARE_H
