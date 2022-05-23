#include "router.h"
#include "../Interceptor/HttpInterceptor.h"
#include "../registry/InterceptorRegistry/InterceptorRegistry.h"

vector<HttpInterceptor *>
Router::getRouteInterceptors() {
    return interceptorRegistry.matchPattern(getFullRoute());
}

void pushContext(HttpInterceptor *interceptor, Request *context) {
    printf("pushContext to interceptor\n");
    interceptor->request_context = context;
}

void clearnContext(HttpInterceptor *interceptor) {
    interceptor->request_context = nullptr;
}

URL_STATUS
Router::__view(void *environment) {
    printf("[INFO] into %s __view...\n", getFullRoute().c_str());

    // check view func
    if (!view_handler) {
        fprintf(stderr, "bad route viewer\n");
        return URL_STATUS::VIEW_NULL;
    }
    // set ThreadLocal Object here
    ThreadLocal::put("route", getFullRoute());
    // push request to view_handler
    request = ThreadLocal::getAs<Request>("request");


    // interceptors
    auto safeCall = std::bind([](HttpInterceptor *interceptor) {


    }, std::placeholders::_1);

    for (auto &&handler: getRouteInterceptors()) {
        printf("invoke interceptor\n");
        try {
            printf("interceptor=%d\n",handler);
            if(handler) {
                pushContext(handler, request);
                handler->invoke();
                clearnContext(handler);
            }

        } catch (std::exception &e) {
            e.what();
            return URL_STATUS::FORBIDDEN;
        }
    }
    auto resString = view_handler();
    if (resString.rfind('.') != resString.npos) {
        DPrintf("file request: %s in %s\n",resString.c_str(), __FILE__);
        request->parsedUrl = url_t(resString);
        return VALID_URL;
    } else {
        // return raw str
        DPrintf("raw request in %s\n", __FILE__);

        request->result = resString;
    }
    if (href_url == url_t::NULL_URL) {
        return DISCONNECT;
    } else if (href_url.empty()) {
        // make default resource url
        href_url = url_t(getFullRoute() + ".html");
        return RESOURCE_NOT_FOUND;
    }
    return VALID_URL;
}