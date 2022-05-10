//
// Created by nekonoyume on 2022/4/19.
//

#ifndef REDISTEST_INTERCEPTORREGISTRY_H
#define REDISTEST_INTERCEPTORREGISTRY_H

#include "../../singletonFactory/singletonFatory.h"
#include "../../ClassHolder.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

#include "../../Interceptor/HttpInterceptor.h"
#include "InterceptorRegexNode.h"
#include <regex>

class InterceptorRegistry {
public:
    static InterceptorRegistry &
    Get() {
        return SingletonFactory<InterceptorRegistry>::Get();
    };

    template<class In>
    void
    addInterceptor(const std::string &urlPattern);

    std::vector<HttpInterceptor *>
    matchPattern(const std::string &routUrl) {
        printf("invoke matchPattern(urlPattern=%s) %s\n", routUrl.c_str(), __FILE__);
        std::vector<HttpInterceptor *> results;
        std::smatch match_;
        for (auto &&p: urlRegexMap) {
            std::regex regexP(p.first);
            try {
                if (std::regex_match(routUrl, match_, regexP)) {
                    printf("match pattern=%s\n", p.first.c_str());
                    // matchCallBack()
                    for (auto &in: p.second) {
                        results.push_back(in);
                    }
                }
            } catch (exception &e) {
                e.what();
            }
        }
        return results;
    };

    ~InterceptorRegistry();

private:
    // regex::interceptors
    unordered_map<std::string, vector<HttpInterceptor *>> urlRegexMap;
};

template<class In>
void InterceptorRegistry::addInterceptor(const std::string &urlRegex) {
    auto iter = urlRegexMap.find(urlRegex);
    if (iter == urlRegexMap.end()) {
        urlRegexMap[urlRegex] = vector<HttpInterceptor *>();
        iter = urlRegexMap.find(urlRegex);
    }
    iter->second.push_back(new In());
}

#endif //REDISTEST_INTERCEPTORREGISTRY_H
