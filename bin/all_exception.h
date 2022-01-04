//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_ALL_EXCEPTION_H
#define TINYWEB_ALL_EXCEPTION_H

#include "exception"
#include <string>
#include <iostream>

class CanNotMakeRequestException:public std::runtime_error {
public:
    CanNotMakeRequestException(std::string msg): runtime_error("[CanNotMakeRequestException] "+msg) {

    }
};

class NullException:public std::runtime_error {
public:
    NullException(std::string msg): runtime_error("[NullException]"+msg) {

    }
};

class NotExistKeyException:public std::runtime_error {
public:
    NotExistKeyException(): runtime_error("[NotExistKeyException]") {

    }
    NotExistKeyException(std::string msg): runtime_error("[NotExistKeyException]"+msg) {

    }
};

#endif //TINYWEB_ALL_EXCEPTION_H
