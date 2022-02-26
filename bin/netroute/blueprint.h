#ifndef TINYWEB_BLUEPRINT_H
#define TINYWEB_BLUEPRINT_H

#include <vector>
#include "request.h"
#include "router.h"

class Blueprint : public Router {
    vector<Router *> routerList;
public:
    Blueprint(const char *bp_name) : Router(bp_name, bp_name, nullptr) {}

    void
    registerRoute(Router *router) {
        routerList.push_back(router);
    }

    const string &getBluePrintName() {
        return (const string &) this->getBaseName();
    }

    Router *
    canDealWith(Request* req) {
        Router *handler = nullptr;
        string url_route = req->getParsedUrl().path;

        // should let bp to pattern url, instead of url to pattern bp,because bp are much shorter to parse
        if (((Router *) this)->canDealWith(url_route.c_str())) {
            LOG_INFO("matched Blueprint [\"%s\"]\n",getBluePrintName().c_str());
            for (auto &router: routerList) {
                if (router->canDealWith(url_route.c_str())) {
                    handler = router;
                    break;
                }
            }
        }
        return handler;
    };

    void
    set_suffix(const string &suffix) override {
        // set suffix for route belongs to this blueprint
        for (auto &route: routerList) {
            route->set_suffix(suffix);
        }

    }

    void
    set_bp_name(string new_bp_name) {
        if (new_bp_name.empty()) {
            new_bp_name = "/"; // register to root bp
        }
        this->getBaseName() = new_bp_name;
    }

    void
    set_prefix(const char *new_bp_name) override {
        set_bp_name(new_bp_name);
        this->set_prefix();
    }

    void
    set_prefix() {
        // set prefix for route belongs to this blueprint
        for (auto &route: routerList) {
            route->set_prefix(this->getBluePrintName().c_str());
        }

    }
    ~Blueprint() {
        LOG_WARN("deconstruct blueprint: %d", this);
        if(!routerList.empty()) {
            for(auto & router:routerList) {
                delete router;
                router = nullptr;
            }
        }
    }
};

#endif //TINYWEB_BLUEPRINT_H
