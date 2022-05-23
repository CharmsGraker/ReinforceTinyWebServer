#include "InterceptorRegistry.h"

InterceptorRegistry::~InterceptorRegistry() {
    for (auto&&[k, interceptors]: urlRegexMap) {
        for (auto &&item: interceptors) {
            delete item;
        };
    }
}

void InterceptorRegistry::addInterceptor(class_holder *&&holder_, const std::string &urlRegex) {
    auto holder = std::move(holder_);
    auto iter = urlRegexMap.find(urlRegex);
    if (iter == urlRegexMap.end()) {
        urlRegexMap[urlRegex] = vector<HttpInterceptor *>();
        iter = urlRegexMap.find(urlRegex);
    }
    iter->second.push_back((HttpInterceptor *) get_reflector().createByName(holder->getClassName()));
    printf("interceptor size=%d\n", urlRegexMap.size());
    delete holder;
}

InterceptorRegistry interceptorRegistry{};
