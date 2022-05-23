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
#include "../../reflect/Reflector.h"
#include <regex>

#define interceptor_registry() (interceptorRegistry);

class InterceptorRegistry {
public:
    static InterceptorRegistry &
    Get() {
        return SingletonFactory<InterceptorRegistry>::Get();
    };

    void
    addInterceptor(class_holder * &&holder, const std::string &urlPattern);

    std::vector<HttpInterceptor *>
    matchPattern(const std::string &routUrl) {
        printf("invoke matchPattern(urlPattern=%s) %s\n", routUrl.c_str(), __FILE__);
        std::vector<HttpInterceptor *> results;
        std::smatch match_;
        printf("urlRegexMap size=%d\n",urlRegexMap.size());
        for (auto &&p: urlRegexMap) {
            printf("pattern=%s\n",p.first.c_str());
            std::regex regexP(p.first);
            try {
                if (std::regex_match(routUrl, match_, regexP)) {
                    printf("match pattern=%s\n", p.first.c_str());
                    // matchCallBack()
                    for (auto &in: p.second) {
                        results.push_back(in);
                    }
                    printf("result size=%d\n",results.size());
                    return results;
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

extern InterceptorRegistry interceptorRegistry;

#endif //REDISTEST_INTERCEPTORREGISTRY_H
