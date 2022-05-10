//
// Created by nekonoyume on 2022/1/20.
//

#ifndef TINYWEB_LOGINFORM_H
#define TINYWEB_LOGINFORM_H

#include <iostream>
struct Form {

};

struct loginForm: public Form{
    loginForm() {

    }
    std::string username;
    std::string password;

};
#endif //TINYWEB_LOGINFORM_H
