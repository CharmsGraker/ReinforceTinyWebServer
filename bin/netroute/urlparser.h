//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_URLPARSER_H
#define TINYWEB_URLPARSER_H

#include "../../utils/string_utils.h"
#include <cstring>
#include <strings.h>

using namespace string_util;




template<class U>
class UrlParser {
public:
    /** to implements of parse a raw url */
    static U
    parse(const std::string &raw_url) {
        U parsed;
        parsed_url(raw_url, parsed);
        return parsed;
    }

    static void
    parsed_url(const std::string &url_in, U &addr_out) {
        const char *p = url_in.c_str();
        static std::string HTTP_PROTOCOL = "http://";
        static std::string HTTPS_PROTOCOL = "https://";

        // parse protocol
        if (strncasecmp(url_in.c_str(), HTTP_PROTOCOL.c_str(), 7) == 0) {
            p += HTTP_PROTOCOL.size();
            addr_out.protocol = HTTP_PROTOCOL;
            p = strchr(p, '/'); // point to route
        } else if (strncasecmp(url_in.c_str(), HTTPS_PROTOCOL.c_str(), 8) == 0) {
            p += HTTPS_PROTOCOL.size();
            addr_out.protocol = HTTPS_PROTOCOL;

            p = strchr(p, '/');
        }


        if (!p || p[0] != '/') {
            // bad request
            fprintf(stderr, "[WARN] bad request!");
            exit(1);
        }

        std::string K, V;
        addr_out.url = url_in; // full url
        addr_out.fullpath = p; // full url

        size_t pos;
        if ((pos = url_in.find('?', 0)) != url_in.npos) {
            addr_out.fullpath = url_in.substr(0, pos + 1); // get full domain and route, include ?
            addr_out.query = url_in.substr(pos + 1);
//            printf("query: %s\n", addr_out.query.c_str());
        }

        auto safe_parse_param = [&K, &V](U &addr) {
            for (auto &line: split(addr.query, '&')) {
                // line will be like ["user=graker","passwd=123",]
                auto KV = split(line, '=');
                if (KV.size() == 2) {
                    K = KV[0], V = KV[1];
                } else if (KV.size() == 1) {
                    K = KV[0], V = "";
                }
                // assignment
                addr.KV[K] = V;
            }
            return 0;
        };

        // analyse query
        if (!addr_out.query.empty()) {
            if (addr_out.query.find('&') == addr_out.query.npos || safe_parse_param(addr_out)) {
                // given error param, dont parse param to avoid disaster
                fprintf(stderr, "bad url param! Ignored.\n");
            }

        }

        // fullpath be like /domain:port/route
        if (!addr_out.fullpath.empty() &&
            (pos = addr_out.fullpath.find('/')) != addr_out.fullpath.npos) {
            addr_out.path = addr_out.fullpath.substr(pos);
            addr_out.host = addr_out.fullpath.substr(0, pos);
        }

        // fullpath be like /domain:port

        //check port if given
        if ((pos = addr_out.fullpath.find(':')) != addr_out.fullpath.npos) {
            addr_out.port = addr_out.fullpath.substr(pos + 1);
            addr_out.host = addr_out.fullpath.substr(0, pos); //shrink host range
        }
    }

};


#endif //TINYWEB_URLPARSER_H