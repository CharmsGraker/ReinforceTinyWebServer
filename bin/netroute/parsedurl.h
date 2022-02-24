//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_PARSEDURL_H
#define TINYWEB_PARSEDURL_H

#include <string>
#include "../container/hashmap.h"
#include "../http/http_const_declare.h"

namespace yumira {
    struct ParsedUrl {
        std::string url;

        std::string path;
        std::string protocol;
        std::string fullpath; // not include the char begin from '?'
        std::string query;
        int port=-1;
        std::string host;
        parameter_t parameters;

        int fd = -1;
        size_t fileSize = -1;
        int use_template = -1;
        std::string tpl_addr;



        std::string &
        getUrl() {
            return url;
        }

        parameter_t &
        getParams() {
            return parameters;
        }

        bool
        isResRequest() {
            ssize_t pos = -1;
            if ((pos = url.rfind('/')) != url.npos) {
                return url.find('.', pos) != url.npos;
            }
            return false;
        }

    };
}

#endif //TINYWEB_PARSEDURL_H
