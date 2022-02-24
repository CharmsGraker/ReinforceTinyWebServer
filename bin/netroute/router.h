
#ifndef TINYWEB_ROUTER_H
#define TINYWEB_ROUTER_H

#include <string>
#include "request.h"
#include <assert.h>
#include "http_request_enum.h"
#include "../concurrent/ThreadLocal.h"


using namespace std;

enum URL_STATUS {
    VIEW_NOT_FOUND = 0,
    VIEW_NULL = 1,
    VALID_URL = 2,
    RESOURCE_NOT_FOUND = 4,
    DISCONNECT = 6,
};

using namespace yumira;

namespace yumira {
    extern thread_local Request* request;
}

class Router {
private:
    /**
     * suffix may not set when construct the Router,
     * but it will make sure set when process request.
     */
    string _suffix;
    string _prefix;
    string fullName;

    char _sep;

    static std::string __seperator()  {
        return ".";
    };

    viewType view_handler = nullptr;

private:
    URL_STATUS __view(url_t &out_url) {
        printf("[INFO] into %s view...\n", getFullRoute().c_str());

        // check view func
        if (!view_handler) {
            fprintf(stderr, "bad route viewer\n");
            return URL_STATUS::VIEW_NULL;
        }
        // set ThreadLocal Object here
        ThreadLocal::put<string>("route", getFullRoute());
        // push request to view_handler
        request = &ThreadLocal::get<Request>("request");
        cout<<request->route()<<endl;
        assert(request);

        href_url = (url_t) view_handler();
        cout<<href_url.url<<endl;
        if (href_url == url_t::NULL_URL) {
            return URL_STATUS::DISCONNECT;
        } else if (href_url.empty()) {
            // make default resource url
            href_url = url_t(getFullRoute() + ".html");
            return URL_STATUS::RESOURCE_NOT_FOUND;
        }
        out_url = href_url;
        return URL_STATUS::VALID_URL;
    }

public:
    url_t href_url;

    /**
     * view func represent the which view of route should show,
     * it should be a stirng like relative path to the true file you store in config ROOT res dir.*
     * if you want dont need connection info, please specify partial view function to construct the route, instead of given http_conn when invoke constructor .
     * because only if when specify the _full_view_f, then will add url param to request. */

    Router(const string &routeName,
           viewType full_f) :
            _suffix(routeName),
            view_handler(full_f),
            _prefix("") {
        assert(view_handler);
    };

    Router(const string &routeName,
           const string &fullName) :
            _suffix(routeName),
            view_handler(nullptr),
            _prefix(""),
            fullName(fullName) {
    };

    explicit Router(const char *routeName) : Router(routeName, nullptr) {};


    bool canDealWith(const char *url) {
        auto match_pattern = [](const char *s, const char *pattern)->bool {
            return strncasecmp(s, pattern, strlen(pattern)) == 0;
        };
        return match_pattern(url, getFullRoute().c_str());
    }


    /**
     * the connection method state was wrapper in request */
    URL_STATUS view(url_t &out_url) {
        // let user to decide invoke which
        return __view(out_url);
    }

    string getFullRoute() {
        // concat full abs route
        if (!fullName.empty())
            return fullName;

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
            fullName = (_prefix + __seperator() + _suffix.substr(1));
            return fullName;
        }
    }

    string &getBaseName() {
        return _suffix;
    }

    virtual void
    set_prefix(const char *prefix) {
        this->_prefix = string(prefix);
    }

    void
    set_prefix(const string &prefix) {
        this->_prefix = prefix;
    }

    virtual void
    set_suffix(const string &suffix) {
        this->_suffix = suffix;
    }
};

#endif //TINYWEB_ROUTER_H
