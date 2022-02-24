#ifndef TINYWEB_URL_STORAGE_H
#define TINYWEB_URL_STORAGE_H

#include <string.h>
#include <stdio.h>
#include <string>
using namespace std;

class url_storage {
private:
    template<class Url, class Parameter>
    friend Url render_template(Url html_url, Parameter &KVMap);

    std::string tpl_addr;
    std::size_t _file_size;
    int _fd;

public:
    std::string url;
    static class url_storage NULL_URL;

    url_storage() :  _fd(-1), _file_size(-1) {
//        printf("[url_storage] construct.\n");

    }

    explicit url_storage(const std::string __url) : url(__url), tpl_addr("") {};
    explicit url_storage(const char * __url) : url(__url), tpl_addr("") {};
    url_storage(const url_storage &obj) : url(obj.url), tpl_addr(obj.tpl_addr) {}

    void
    set_template_ptr(const std::string &byte_ptr) {
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

    std::size_t length() const {
        return url.length();
    }

    bool isResRequest() const {
        return url.find('.') != url.npos;
    };

    const char *
    which() const {
        return url.c_str();
    }

    std::string template_addr() {
        if (useTemplate()) {
            return tpl_addr;
        }
    };

    bool
    empty() const {
        return url.empty();
    }


    const std::size_t
    fileSize() const {
        return _file_size;
    }

    const int
    fd() const {
        return _fd;
    }

    url_storage &
    operator=(const url_storage obj) {
        url = obj.url;
        tpl_addr = obj.tpl_addr;
        return *this;
    }

    bool operator==(const url_storage &o) {
        return (url == o.url) && (tpl_addr == o.tpl_addr);
    }
};

#endif //TINYWEB_URL_STORAGE_H
