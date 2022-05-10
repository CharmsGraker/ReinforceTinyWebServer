//
// Created by nekonoyume on 2022/4/19.
//

#ifndef TINYWEB_CLASSHOLDER_H
#define TINYWEB_CLASSHOLDER_H

#include <string>
#include <map>

struct BaseClassHolder {

};

extern std::map<std::string, BaseClassHolder *> registerMap;

template<typename T>
struct ClassHolder;

template<typename T>
void
registerType(const std::string &className) {
    registerMap[className] = new ClassHolder<T>();

}

template<typename T>
void
unloadType(const std::string &className) {
    auto iter = registerMap.find(className);
    if (iter != registerMap.end()) {
        delete iter->second;
        registerMap.erase(iter);
    }
}

template<typename T>
struct ClassHolder : public BaseClassHolder {
    typedef T class_type;

    ClassHolder(const std::string &className) : class_name(className) {
        registerType<T>(className);
    }

    explicit operator T *() {
    }

    std::string class_name;
};


#endif //TINYWEB_CLASSHOLDER_H
