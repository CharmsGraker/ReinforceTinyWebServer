#ifndef TINYWEB_USERMAIN_H
#define TINYWEB_USERMAIN_H

#include "webserver/current_app.h"
#include "webserver/signal_handler/server_handler.h"
#include "registry/InterceptorRegistry/InterceptorRegistry.h"

class UserMain {
public:
    static int init(void* server= nullptr) {
        UserMain::main(server);
        return errno == 0;
    };

    static int main(void * server_addr);

    ~UserMain()=default;
};


#endif //TINYWEB_USERMAIN_H