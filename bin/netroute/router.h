
#ifndef TINYWEB_ROUTER_H
#define TINYWEB_ROUTER_H

#include <string>
#include "request.h"

#include "http_request_enum.h"
#include "../concurrent/ThreadLocal.h"


struct unused_http_conn_view {

};
struct use_http_conn_view {

};
using namespace std;

enum URL_STATUS {
    VIEW_NOT_FOUND,
    VIEW_NULL
};

using namespace yumira;

class Router {
private:
    /**
     * suffix may not set when construct the Router,
     * but it will make sure set when process request.
     */
    string _suffix;
    string _prefix;


    char _sep;

    static string __sep() {
        static string sep = ".";
        return sep;
    };

    view_func_raw_t _full_view_f = nullptr;


public:
    //STATUS

    url_t href_url;

    /**
     * view func represent the which view of route should show,
     * it should be a stirng like relative path to the true file you store in config ROOT res dir.*
     * if you want dont need connection info, please specify partial view function to construct the route, instead of given http_conn when invoke constructor .
     * because only if when specify the _full_view_f, then will add url param to request. */

    Router(const string routeName, view_func_raw_t full_f) : _suffix(routeName), _full_view_f(full_f),
                                                             _prefix("") {
    };

    Router(const char *routeName) : Router(string(routeName), nullptr) {};

    /**
     * the connection method state was wrapper in request */
    template<class request_t>
    URL_STATUS view(request_t &request, url_t &out_url) {
        // let user to decide invoke which
        return __view(request, out_url);
    }

    template<class request_t>
    URL_STATUS __view(request_t &request, url_t &out_url) {
        printf("[INFO] into %s view...\n", getFullRoute().c_str());

        // check view func
        if (!_full_view_f) {
            printf("bad route viewer\n");
            return URL_STATUS::VIEW_NULL;
        }
        // set ThreadLocal Object here
        ThreadLocal::put("route", getFullRoute());

        if ((href_url = ((url_t) _full_view_f(&request))).empty()) {
            // make default res url
            href_url = url_t(getFullRoute() + ".html");
        }
        out_url = href_url;
    }


    string getFullRoute() const {
        // concat full abs route
        unsigned long pos = -1;

        if (_prefix.empty()) {
            printf("error when get route name!");
            throw exception();
        }

        if (_prefix == "/") {
            // no need to join "." between "/" and route_name
            return _suffix;
        } else {
            if (_suffix.empty()) {
                return _prefix + "/";
            }
//            cout << "route name: " << _prefix + __sep() + _suffix;
            return _prefix + __sep() + _suffix.substr(1);
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

    void
    set_suffix(const string &suffix) {
        this->_suffix = suffix;
    }
};

#endif //TINYWEB_ROUTER_H
