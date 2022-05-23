//
// Created by nekonoyume on 2022/4/19.
//

#ifndef REDISTEST_JWTINTERCEPTOR_H
#define REDISTEST_JWTINTERCEPTOR_H

#include "../../../bin/Interceptor/HttpInterceptor.h"
#include "../../../bin/extensions/security/JwtTokenizer.h"
#include "../../../bin/exceptions/exceptions.h"

class otherHttp: public HttpInterceptor {

};
class JwtInterceptor : public HttpInterceptor {
    std::string issuer_;
    std::string secretKey;
public:
    JwtInterceptor()=default;
    void invoke() override {
        printf("invoke JWTInterceptor()\n");
        if(!request_context)
            return;
        try {
            auto token = request_context->operator[]("token");
            printf("token=%s\n",token.c_str());
            if (token.empty()){
                printf("empty token, return\n");
                return;
            }
            JwtTokenizer jwtTokenizer;

            if (!jwtTokenizer.verifier().secret_key(secretKey).issuer(issuer_).verify(token)) {
                throw UnAuthorizedException();
            }
        } catch (exception e) {
            e.what();
        }
    }
};

REGISTER(JwtInterceptor);
#endif //REDISTEST_JWTINTERCEPTOR_H
