
#ifndef TINYWEB_ROUTER_H
#define TINYWEB_ROUTER_H

#include <string>
#include "request.h"

#include "http_request_enum.h"

struct unused_http_conn_view {

};
struct use_http_conn_view {

};
using namespace std;

enum URL_STATUS {
    VIEW_NOT_FOUND,
    VIEW_NULL
};


template<typename httpConn>
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

    __view_func_partial_t _partial_view_f = nullptr;
    __view_func_raw_t _full_view_f = nullptr;

    enum view_t {
        PARTIAL,
        FULL
    };

    view_t func_type;

public:
    //STATUS

    string href_url;

    /**
     * view func represent the which view of route should show,
     * it should be a stirng like relative path to the true file you store in config ROOT res dir.*
     * if you want dont need connection info, please specify partial view function to construct the route, instead of given http_conn when invoke constructor .
     * because only if when specify the _full_view_f, then will add url param to request. */
    Router(const string routeName, __view_func_partial_t partial_f) : _suffix(routeName), _partial_view_f(partial_f),
                                                                      _prefix(""), func_type(view_t::PARTIAL) {
    };

    Router(const string routeName, __view_func_raw_t full_f) : _suffix(routeName), _full_view_f(full_f),
                                                               _prefix(""), func_type(view_t::FULL) {
    };

    Router(const char *routeName) : Router(string(routeName), nullptr) {};

    /**
     * the connection method state was wrapper in request */
    URL_STATUS view(Request &request, string &out_url, httpConn *conn) {
        // let user to decide invoke which
        return __view(request, out_url, (func_type == view_t::FULL) ? conn : nullptr);
    }

    URL_STATUS __view(Request &request, string &out_url, httpConn *conn) {
        printf("[INFO] into %s view...\n", getFullRoute().c_str());

        // check view func
        if (!_partial_view_f && !_full_view_f) {
            printf("bad route viewer\n");
            return URL_STATUS::VIEW_NULL;
        }

        if (!conn)
            href_url = (string) _partial_view_f(&request);
        else {
            request.parse_arg();
            href_url = (string) _full_view_f(&request, conn);

        }

        if (href_url.empty()) {
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
