//
// Created by nekonoyume on 2022/1/3.
//

#ifndef TINYWEB_ROUTER_H
#define TINYWEB_ROUTER_H

#include <algorithm>
#include <string>
#include <iostream>
#include "http_request_enum.h"


using namespace std;


typedef const char *(*__view_func_type)(Request*);


class Router {
private:
    /**
     * suffix may not set when construct the Router,
     * but it will make sure set when process request.
     */
    string _suffix;
    string _prefix;

    static string __sep() {
        static string sep = ".";
        return sep;
    };
    string after_view_url;
    __view_func_type __view_func = nullptr;
    string ROOT = "";

public:
    //STATUS
    enum URL_STATUS {
        VIEW_NOT_FOUND,
        VIEW_NULL
    };

    string href_url;

    /**
     * view func represent the which view of route should show,
     * it should be a stirng like relative path to the true file you store in config ROOT res dir.*/
    Router(const string routeName, __view_func_type view_f) : _suffix(routeName), __view_func(view_f), _prefix("") {
    };

    Router(const char *routeName) : Router(string(routeName), nullptr) {};

    Router::URL_STATUS view(Request &request, string &out_url) {
        printf("[INFO] into %s view...\n", getFullRoute().c_str());


        if (!__view_func) {
            printf("bad route viewer\n");
            return URL_STATUS::VIEW_NULL;
        }

        href_url = (string) __view_func(&request);
        if (href_url == "") {
            // make default res url
            href_url = string(getFullRoute()) + ".html";
        }

        out_url = href_url;
    }


    string getFullRoute() const {
        // concat full abs route
        unsigned long pos = -1;

        if (_prefix.empty()) {
            printf("error when get route name!");
            throw exception();
        } else if (_prefix == "/") {
            // no need to join "." between "/" and route_name
            pos = _suffix.find('/');
            return _prefix + _suffix.substr(pos != _suffix.npos ? pos + 1 : 0);
        } else {
            return _prefix + __sep() + _suffix.substr(pos != _suffix.npos ? pos + 1 : 0);
        }
    }

    string getBaseName() const {
        return _suffix;
    }

    void
    set_prefix(const char *prefix) {
        this->_prefix = string(prefix);
    }

    void
    set_prefix(const string &prefix) {
        this->_prefix = prefix;
    }
};

#endif //TINYWEB_ROUTER_H
