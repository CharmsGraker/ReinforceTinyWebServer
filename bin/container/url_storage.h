//
// Created by nekonoyume on 2022/1/6.
//

#ifndef TINYWEB_URL_STORAGE_H
#define TINYWEB_URL_STORAGE_H

#include <string.h>
#include "../http/http_const_declare.h"


struct url_storage {
private:
    template<class Url, class Parameter>
    friend Url render_template(Url html_url, Parameter &KVMap);

    std::string tpl_addr;
    size_t _file_size;
    int _fd;

public:
    std::string url;

    url_storage() : url(""), tpl_addr("") {
        printf("[url_storage] construct.\n");

    }

    explicit url_storage(std::string url) : url(url), tpl_addr("") {

    }

    void
    set_template_ptr(const std::string& byte_ptr) {
        if (byte_ptr.empty()) {
            fprintf(stderr, "bad set template ptr!");
            return;
        }
        tpl_addr = byte_ptr;
    }

    bool
    useTemplate() const {
        return !tpl_addr.empty();
    }

    size_t length() const {
        return url.length();
    }

    bool isResRequest() {
        return url.find('.') != url.npos;
    };

    const char *
    which() {
        return url.c_str();
    }

    std::string template_addr() {
        if (useTemplate()) {
            return  tpl_addr;
        }
    };

    bool
    empty() {
        return url.empty();
    }

    url_storage(url_storage &obj) : url(obj.url), tpl_addr(obj.tpl_addr) {

    }

    url_storage(url_storage &&obj) noexcept: url(std::move(obj.url)), tpl_addr(obj.tpl_addr) {

    }

    const size_t
    fileSize() const {
        return _file_size;
    }

    const int
    fd() const {
        return _fd;
    }

    url_storage &
    operator=(url_storage obj) {
        url = obj.url;
        tpl_addr = obj.tpl_addr;
        return *this;
    }
};

#endif //TINYWEB_URL_STORAGE_H
