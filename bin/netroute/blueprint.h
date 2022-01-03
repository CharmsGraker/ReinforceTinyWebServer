//
// Created by nekonoyume on 2022/1/3.
//

#ifndef TINYWEB_BLUEPRINT_H
#define TINYWEB_BLUEPRINT_H

#include <vector>
#include "router.h"

class Blueprint {
    string _bp_name = nullptr;
    vector<Router *> routerList;

public:
    Blueprint(const char *bp_name) : _bp_name(bp_name) {}

    void
    registRoute(Router *router) {
        routerList.push_back(router);
    }

    string get_bp_name() const {
        return _bp_name;
    }

    Router *
    canDealWith(Request &request) {
        Router *handler = nullptr;
        string requestUrl = request.url;
        auto match_pattern = [](const char *s, const char *pattern) {
            return strncasecmp(s, pattern, strlen(pattern)) == 0;
        };
        // should let bp to pattern url, instead of url to pattern bp,because bp are much shorter to parse
        if (match_pattern(requestUrl.c_str(), get_bp_name().c_str())) {
            printf("matched bp: %s\n", get_bp_name().c_str());
            for (auto &router: routerList) {
                if (match_pattern(requestUrl.c_str(), router->getFullRoute().c_str())) {
                    handler = router;
                    break;
                }
            }
        }
        return handler;
    };

    void
    set_route_prefix() {
        // set prefix for route belongs to this blueprint
        for (auto &route: routerList) {
            route->set_prefix(get_bp_name().c_str());
        }

    }

    void
    set_bp_name(string new_bp_name) {
        if (new_bp_name.empty()) {
            new_bp_name = "/"; // register to root bp
        }
        _bp_name = new_bp_name;
    }

    void
    set_route_prefix(const char *new_bp_name) {
        set_bp_name(new_bp_name);
        set_route_prefix();

    }

    ~Blueprint() {
        for (auto &route: routerList) {
            delete route;
        }
    }
};

#endif //TINYWEB_BLUEPRINT_H
