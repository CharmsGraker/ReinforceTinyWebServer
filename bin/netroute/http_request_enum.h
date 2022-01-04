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

/** to avoid infinite loop import,
 * please declare all class here */

class Request;
class http_conn;

using namespace string_util;


typedef std::unordered_map<std::string,std::string> inputArgs;

/**
 * please be careful for this typedef, because it may lead to recurrent include.
 * */
typedef const char *(*__view_func_raw_t)(Request *, http_conn* httpConn);

typedef const char *(*__view_func_partial_t)(Request *);





#endif //TINYWEB_HTTP_REQUEST_ENUM_H
