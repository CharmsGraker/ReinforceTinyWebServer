//
// Created by nekonoyume on 2022/1/6.
//

#ifndef TINYWEB_URL_STORAGE_H
#define TINYWEB_URL_STORAGE_H
#include <string>
struct url_storage {
private:
    char *tpl_addr = nullptr;
public:
    std::string url;

    url_storage() : url(""), tpl_addr(nullptr) {
        printf("[url_storage] construct.\n");

    }

    explicit url_storage(std::string url) : url(url), tpl_addr(nullptr) {

    }

    void
    set_template_ptr(char *byte_ptr) {
        if (!byte_ptr) {
            fprintf(stderr, "bad set template ptr!");
            return;
        }
        tpl_addr = byte_ptr;
    }

    bool
    useTemplate() const {
        return tpl_addr != nullptr;
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

    char *template_addr() {
        if (useTemplate()) {
            return tpl_addr;
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

    url_storage &
    operator=(url_storage obj) {
        url = obj.url;
        tpl_addr = obj.tpl_addr;
        return *this;
    }
};
#endif //TINYWEB_URL_STORAGE_H
