//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_URLPARSER_H
#define TINYWEB_URLPARSER_H

#include "../../utils/string_utils.h"
#include <cstring>
#include <strings.h>


using namespace string_util;

using namespace yumira;

template<class U>
class UrlParser {
public:
    /** to implements of parse a raw url */
    static U
    parse(const std::string &raw_url) {
        U parsed;
        doParse(raw_url, parsed);
        return parsed;
    }

protected:
    static void
    doParse(const std::string &url_in, U &outParsedUrl) {
        const char *p = url_in.c_str();
        static std::string HTTP_PROTOCOL = "http://";
        static std::string HTTPS_PROTOCOL = "https://";

        // parse protocol
        if (strncasecmp(url_in.c_str(), HTTP_PROTOCOL.c_str(), 7) == 0) {
            p += HTTP_PROTOCOL.size();
            outParsedUrl.protocol = HTTP_PROTOCOL;
            p = strchr(p, '/'); // point to route
        } else if (strncasecmp(url_in.c_str(), HTTPS_PROTOCOL.c_str(), 8) == 0) {
            p += HTTPS_PROTOCOL.size();
            outParsedUrl.protocol = HTTPS_PROTOCOL;

            p = strchr(p, '/');
        }

        if (!p || p[0] != '/') {
            // bad request
            fprintf(stderr, "[WARN] bad request!");
            exit(1);
        }

        std::string K, V;
        outParsedUrl.url = url_in; // full url
        outParsedUrl.fullpath = p; // full url

        size_t pos;
        if ((pos = url_in.find('?', 0)) != url_in.npos) {
            outParsedUrl.fullpath = url_in.substr(0, pos + 1); // get full domain and route, include ?
            outParsedUrl.query = url_in.substr(pos + 1);
//            printf("query: %s\n", addr_out.query.c_str());
        }

        auto safe_parse_param = [&K, &V, &outParsedUrl](U &addr) {
            for (auto &line: split(addr.query, '&')) {
                // line will be like ["user=graker","passwd=123",]
                auto KV = split(line, '=');
                if (KV.size() == 2) {
                    K = KV[0], V = KV[1];
                } else if (KV.size() == 1) {
                    K = KV[0], V = "";
                }
                // assignment
                outParsedUrl.parameters[K] = V;
            }
            return 0;
        };

        // analyse query
        if (!outParsedUrl.query.empty()) {
            if (outParsedUrl.query.find('&') == outParsedUrl.query.npos || safe_parse_param(outParsedUrl)) {
                // given error param, dont parse param to avoid disaster
                fprintf(stderr, "bad url param! Ignored.\n");
            }

        }

        // fullpath be like /domain:port/route
        if (!outParsedUrl.fullpath.empty() &&
            (pos = outParsedUrl.fullpath.find('/')) != outParsedUrl.fullpath.npos) {
            outParsedUrl.path = outParsedUrl.fullpath.substr(pos);
            outParsedUrl.host = outParsedUrl.fullpath.substr(0, pos);
        }

        // fullpath be like /domain:port

        //check port if given
        if ((pos = outParsedUrl.fullpath.find(':')) != outParsedUrl.fullpath.npos) {
            outParsedUrl.port = atoi(outParsedUrl.fullpath.substr(pos + 1).c_str());
            outParsedUrl.host = outParsedUrl.fullpath.substr(0, pos); //shrink host range
        }
    }

};


#endif //TINYWEB_URLPARSER_H