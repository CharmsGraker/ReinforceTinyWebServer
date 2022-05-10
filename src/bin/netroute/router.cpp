#include "router.h"
#include "../Interceptor/HttpInterceptor.h"
#include "../registry/InterceptorRegistry/InterceptorRegistry.h"

vector<HttpInterceptor *>
Router::getRouteInterceptors() {
    return InterceptorRegistry::Get().matchPattern(getFullRoute());
}

void pushContext(HttpInterceptor *interceptor, Request *context) {
    interceptor->request_context = context;
}

void clearnContext(HttpInterceptor *interceptor) {
    interceptor->request_context = nullptr;
}

URL_STATUS Router::__view(void *environment) {
    printf("[INFO] into %s view...\n", getFullRoute().c_str());

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
        try {
            pushContext(interceptor, request);
            printf("invoke interceptor\n");
            interceptor->invoke();
            clearnContext(interceptor);

        } catch (std::exception &e) {
            e.what();
            return URL_STATUS::FORBIDDEN;
        }

    }, std::placeholders::_1);

    for (auto &&interceptor_handler: getRouteInterceptors()) {
        safeCall(interceptor_handler);
        interceptor_handler->invoke();
    }
    auto resString = view_handler();
    if (resString.rfind('.') != resString.npos) {
        printf("file request in %s\n", __FILE__);
        request->parsedUrl = url_t(resString);
        return VALID_URL;
    } else {
        // return raw str
        printf("raw request in %s\n", __FILE__);

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