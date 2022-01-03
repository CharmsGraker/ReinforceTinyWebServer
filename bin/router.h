//
// Created by nekonoyume on 2022/1/3.
//

#ifndef TINYWEB_ROUTER_H
#define TINYWEB_ROUTER_H

#include <algorithm>
#include <string>

using namespace std;


typedef const char *(*__view_func_type)(string);


class router {
private:
    string _route_name;
    string _prefix;
    string after_view_url;
    __view_func_type __view_func = nullptr;


public:
    //STATUS
    enum URL_STATUS {
        VIEW_NOT_FOUND,
        VIEW_NULL
    };

    string href_url;

    router(string routeName, __view_func_type view_f) : _route_name(routeName), __view_func(view_f) {
    };

    router(const char *routeName) : router(routeName, nullptr) {};

    router::URL_STATUS view(string url, string &real_url) {
        printf("[INFO] into %s view...\n", getRoute());

        // make default res url
        href_url = string(getRoute()) + ".html";

        if (!__view_func) {
            printf("bad route viewer\n");
            return URL_STATUS::VIEW_NULL;
        }

        href_url = (string) __view_func((string) url);
        real_url = href_url;
    }


    const char *getRoute() const {
        return _route_name.c_str();
    }
};

#endif //TINYWEB_ROUTER_H
