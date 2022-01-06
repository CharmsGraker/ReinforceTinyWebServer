//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_PARSEDURL_H
#define TINYWEB_PARSEDURL_H

#include <string>
#include "../container/hashmap.h"

namespace yumira {
    struct ParsedUrl {

        std::string path;
        std::string protocol;
        std::string url;
        std::string fullpath; // not include the char begin from '?'
        std::string query;
        std::string port;
        std::string host;


        parameter_t KV;

    };
}

#endif //TINYWEB_PARSEDURL_H
