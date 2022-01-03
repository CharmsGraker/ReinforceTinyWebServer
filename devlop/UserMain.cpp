//
// Created by nekonoyume on 2022/1/3.
//

#include "../bin/UserMain.h"

const char * request_for_pic(RequestPtr req) {
    return "/picture.html";
}

const char * requset_for_video(RequestPtr req) {
    return "/video.html";
}

const char * follow(RequestPtr req) {
    return "/fans.html";
}

int UserMain::main() {
    http_conn::register_interceptor(new Router("/href", [](RequestPtr req) { return "/h.html"; }));


    // root bp============================================================================
    auto root_bp = new Blueprint("/");


    http_conn::register_interceptor(root_bp,[](Blueprint* root_bp){
        root_bp->registRoute(new Router("/register", [](RequestPtr req) { return "/register.html"; }));
        root_bp->registRoute(new Router("/login", [](RequestPtr req) { return "/log.html"; }));
        root_bp ->registRoute(new Router("/req_picture",request_for_pic));
        root_bp ->registRoute(new Router("/req_video",requset_for_video));
        root_bp ->registRoute(new Router("/follow",follow));
    });

    // another bp here==============================================================================
    return 0;
}
