//
// Created by nekonoyume on 2022/4/19.
//

#ifndef REDISTEST_HTTPINTERCEPTOR_H
#define REDISTEST_HTTPINTERCEPTOR_H


#include "../netroute/request.h"

class Router;

class HttpInterceptor {
public:

    friend void pushContext(HttpInterceptor *interceptor, Request *context);

    friend void clearnContext(HttpInterceptor *interceptor);

    virtual void invoke() {
        printf("invoke HttpInterceptor\n");
    };

    HttpInterceptor() noexcept;

    virtual ~HttpInterceptor() = default;;

    friend Router;

protected:
    Request *request_context;

};

#endif //REDISTEST_HTTPINTERCEPTOR_H
