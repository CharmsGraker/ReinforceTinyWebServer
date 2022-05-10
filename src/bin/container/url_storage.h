#ifndef TINYWEB_URL_STORAGE_H
#define TINYWEB_URL_STORAGE_H

#include <string.h>
#include <stdio.h>
#include <string>

using namespace std;

class url_storage {
private:

public:
    static class url_storage NULL_URL;

    std::string url;
    std::size_t _file_size;
    int _fd;

public:
//    template<class Url, class Parameter>
//    friend Url render_template(Url html_url, Parameter &KVMap);

    url_storage() : _fd(-1), _file_size(-1) {
//        printf("[url_storage] construct.\n");

    }

    explicit url_storage(const std::string __url) : url(__url) {};

    explicit url_storage(const char *__url) : url(__url) {};

    url_storage(const url_storage &obj) : url(obj.url) {}


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
        return *this;
    }

    bool operator==(const url_storage &o) {
        return (url == o.url);
    }
};

#endif //TINYWEB_URL_STORAGE_H
