//
// Created by nekonoyume on 2022/1/3.
//

#ifndef TINYWEB_USERMAIN_H
#define TINYWEB_USERMAIN_H

#include "../webserver.h"

class UserMain {
private:
    Locker lock;

    UserMain() {};

    ServerPtr server;


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
    bindServer(yumira::WebServer *ser) {
        getInstance()->server = ser;
    }

    int operator()() {
        getInstance()->main();
        return errno == 0;
    };

    static int main();

    ~UserMain() { if (getInstance())delete getInstance(); }
};


#endif //TINYWEB_USERMAIN_H
