//
// Created by nekonoyume on 2022/1/3.
//

#include "../bin/UserMain.h"

int UserMain::main() {
    http_conn::register_interceptor(new router("/href", [](string url) { return "/h.html"; }));
    return 0;
}
