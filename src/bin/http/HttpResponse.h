//
// Created by nekonoyume on 2022/4/10.
//

#ifndef TINYWEB_HTTPRESPONSE_H
#define TINYWEB_HTTPRESPONSE_H

#include <string>
#include <unordered_map>
#include <cstring>
#include "../string/string.h"
#include "http_outstream/http_response_header.h"


class HttpResponse {
    std::unordered_map<std::string, std::string> mp;
    std::string writeBuffer;
    int code;
public:
    auto
    clear() {

        return mp.clear();
    }

    HttpResponse() : mp() {

    }
    static HttpResponse*
    makeResponse() {
        return new HttpResponse;
    }

    const char *
    bytes() {
        return writeBuffer.c_str();
    };


    std::string
    get(const std::string &key_) {
        return mp[key_];
    }


    bool
    addLocation(const std::string &loc, bool cache_ = false) {
        if (!cache_) {
            auto cache_control_value = get("Cache-Control");
            put("Cache-Control", cache_control_value + "no-cache=" + loc);
        }
        return put("Location", loc);
    }

    template<int c>
    auto
    add_status_line(http_message::response::header<c> responseHeader) {
        return add2Buffer(Format("%s %d %s\r\n", "HTTP/1.1", responseHeader.code(), responseHeader.title()));
    };

    auto
    add_content_length(int content_len) {
        return add2Buffer(Format("Content-Length:%d\r\n", content_len));
    }

    void
    add_content(const char * content) {
        return add2Buffer(content);
    }
    auto
    add_linger(bool keepAlive= true) {
        add2Buffer(Format("Connection:%s\r\n", keepAlive ? "keep-alive" : "close"));
    }
private:
    void
    add2Buffer(const std::string& content) {
        writeBuffer += content;
    }

    std::string
    render301() {
        add_status_line(http_message::response::header<301>());
        add_content_length(strlen(http_message::response::redirect_301_form));
        add_linger();
        add_content(http_message::response::redirect_301_form);

        std::string ans(writeBuffer.begin(),writeBuffer.end());
        writeBuffer.clear();
        return ans;
    }

public:
    std::string
    render() {
        switch (code) {
            case 301: {
                return std::move(render301());
            }
            default: {
                return "404";

            }
        }
    };

protected:
    bool put(const std::string &key, const std::string &value) {
        printf("[Response] add Response segment\n");

        auto it = mp.find(key);
        if (it != mp.end()) {
            printf("exist, addResponse done.\n");
            return false;
        } else {
            mp[key] = value;
            printf("addResponse %s=%s\n", key.c_str(), value.c_str());
            return true;
        }
    }

public:
    ~HttpResponse() {

    }
};

#endif //TINYWEB_HTTPRESPONSE_H
