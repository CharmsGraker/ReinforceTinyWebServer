#ifndef TINYWEB_USERMAIN_H
#define TINYWEB_USERMAIN_H

#include "webserver/current_app.h"
#include "webserver/signal_handler/server_handler.h"
#include "registry/InterceptorRegistry/InterceptorRegistry.h"

template<class Server>
class UserMain {
private:
    Locker lock;

    Server* server;

public:
    int _argc;
    char **_argv;

    static
    UserMain *
    getInstance() {
        static UserMain *instance = nullptr;
        if (!instance) {
            instance = new UserMain();
        }
        return instance;
    }

    static
    void
    setPara(const int &argc, char **argv) {
        getInstance()->_argc = argc;
        getInstance()->_argv = argv;
    }

    static
    void
    bindServer(Server* ser) {
        getInstance()->server = ser;
    }

    static int preProcess() {
        getInstance()->main();
        return errno == 0;
    };

    static int main();

    ~UserMain() { if (getInstance())delete getInstance(); }
};


#endif //TINYWEB_USERMAIN_H