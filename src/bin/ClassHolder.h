//
// Created by nekonoyume on 2022/4/19.
//

#ifndef TINYWEB_CLASSHOLDER_H
#define TINYWEB_CLASSHOLDER_H

#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <cstring>

class ClassRegistry;
struct class_holder {
    virtual const char *
    getClassName()=0;
};

template<typename T>
struct ClassHolder;

template<typename T>
struct ClassHolder : public class_holder {
    T *p = nullptr;
    typedef T class_type;

    const char * getClassName() override {
        auto class_name = typeid(class_type).name();
        return std::find_if(class_name,class_name + strlen(class_name),[](char c){return isalpha(c);});
    }
    ClassHolder() {
    }

    explicit operator T *() {
    }
};


#endif //TINYWEB_CLASSHOLDER_H
