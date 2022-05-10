//
// Created by nekonoyume on 2022/4/19.
//

#ifndef TESTREGEX_INTERCEPTORREGEXNODE_H
#define TESTREGEX_INTERCEPTORREGEXNODE_H

#include <string>
#include "../../ClassHolder.h"


class RegexNode {
public:
    template<class T>
    explicit RegexNode(const std::string &regex,ClassHolder<T> holder) : regexPattern(regex) {
        class_name = ClassHolder<T>::class_name;
        data = new T();
    };

public:
    std::string regexPattern;
    std::string class_name;
    void *data;
};

#endif //TESTREGEX_INTERCEPTORREGEXNODE_H
