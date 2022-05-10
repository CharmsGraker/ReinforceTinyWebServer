//
// Created by nekonoyume on 2022/1/6.
//

#ifndef TINYWEB_CURRENT_APP_H
#define TINYWEB_CURRENT_APP_H
#include "webserver.h"

namespace yumira {
    extern yumira::WebServer<yumira::httpConnType>* current_app;

    extern std::function<std::string(std::string,parameter_t&)> render_template;
}

#endif //TINYWEB_CURRENT_APP_H
