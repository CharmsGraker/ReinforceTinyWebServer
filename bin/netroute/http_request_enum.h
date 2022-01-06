//
// Created by nekonoyume on 2022/1/3.
//

#ifndef TINYWEB_HTTP_REQUEST_ENUM_H
#define TINYWEB_HTTP_REQUEST_ENUM_H

#include <algorithm>
#include <string>
#include <iostream>
#include <unordered_map>

#include "../../utils/string_utils.h"
#include "../http/http_connect_adapter.h"
#include "urlparser.h"
#include "request.h"

/** to avoid infinite loop import,
 * please declare all class here */

class http_conn;

using namespace string_util;


namespace yumira {

    typedef http_conn default_connection_t;
    typedef HttpConnectionAdapter<default_connection_t> default_connection_adapter;
    typedef UrlParser default_urlparser_t;
    typedef Request<default_urlparser_t, default_connection_adapter> *default_request_infer_t;
    typedef std::unordered_map<std::string, std::string> url_param_container_t;

    typedef url_t (*view_func_raw_t)(default_request_infer_t);


#ifndef CONFIG_H
#ifndef CONNECTION_TYPE
#define CONNECTION_TYPE default_connection_t;
    typedef default_connection_t runtime_connection_t;
#else
    typedef CONNECTION_TYPE runtime_connection_t;
#endif

#ifndef CONNECTION_ADAPTER_TYPE
#define CONNECTION_ADAPTER_TYPE default_connection_adapter;
    typedef default_connection_adapter runtime_connection_adapter_t;
#else
    typedef CONNECTION_ADAPTER_TYPE runtime_connection_adapter_t;
#endif

#ifndef CONNECTION_URLPARSER_TYPE
    typedef default_urlparser_t runtime_urlparser_t;
#else
    typedef CONNECTION_URLPARSER_TYPE runtime_urlparser_t;
#endif

#ifndef CONNECTION_URLPARSER_TYPE
    typedef default_request_infer_t runtime_request_infer_t;
#else
    typedef CONNECTION_URLPARSER_TYPE runtime_request_infer_t;
#endif
#else
#endif

}
#endif //TINYWEB_HTTP_REQUEST_ENUM_H
