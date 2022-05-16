//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_PARSEDURL_H
#define TINYWEB_PARSEDURL_H

#include <string>
#include "../container/hashmap.h"
#include "../http/http_const_declare.h"
#include "../concurrent/ThreadLocal.h"
#include <unordered_set>
#include <functional>

namespace yumira {
    extern std::unordered_set<std::string> allowFileExtension;

    struct ParsedUrl {
    private:
        int use_template = 0;

    public:
        std::string url;
        std::function<bool(const std::string &)> judge_file_url_handler = [](const std::string &url) -> bool {
            auto idx = url.rfind('.');
            if (idx == url.npos)
                return false;
            auto ext = url.substr(idx + 1);
            return allowFileExtension.find(ext) != allowFileExtension.end();
        };
        std::string path;
        std::string protocol;
        std::string fullpath; // not include the char begin from '?'
        std::string query;
        int port = -1;
        std::string host;
        parameter_t parameters;

        int fd = -1;
        size_t fileSize = -1;
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
        isResource() {
            return judge_file_url_handler(url);
        }

        /** pass reference! */
        ParsedUrl &
        operator=(url_t &&trueUrl) noexcept {
            url = std::move(trueUrl.url);
            fileSize = trueUrl.fileSize();
            DPrintf("in %s\n", __FILE__);
            checkTemplate();
        }

        bool
        useTemplate() {
            return use_template > 0;
        }

    private:

        void
        checkTemplate() noexcept {
            try {
                auto ret = ThreadLocal::getAs<int>("template:enable");
                if (!ret)
                    return;
                use_template = *ret;
                fileSize = *ThreadLocal::getAs<int>("template:fileSize");

            } catch (exception &e) {

            };
        }

    };
}

#endif //TINYWEB_PARSEDURL_H
