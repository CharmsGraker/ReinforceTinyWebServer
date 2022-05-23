//
// Created by nekonoyume on 2022/5/22.
//

#ifndef TINYWEB_REFLECTOR_H
#define TINYWEB_REFLECTOR_H

#include <string>
#include <memory>
#include <map>
#include <mutex>

typedef std::function<void*(void )> constructor_func;

#define REGISTER(CLASS_NAME) \
register_action g_register_action_##CLASS_NAME(#CLASS_NAME, []()\
{\
    return new CLASS_NAME(); \
});

class Reflector {
private:
    std::map<std::string, constructor_func> objectMap;
private:
    void register_class(const std::string &class_name, constructor_func &&generator) {
        objectMap[class_name] = generator;
    }

    Reflector() = default;

public:
    friend class register_action;

    void *
    createByName(const std::string &str) {
        auto iter = objectMap.find(str);
        if(iter == objectMap.end()) {
            printf("non type for %s\n",str.c_str());
            for(auto &t:objectMap) {
                printf("type: %s\n",t.first.c_str());
            }
            return nullptr;
        }
        return iter->second();
    }


    static
    std::shared_ptr<Reflector>& Instance() {
        static std::shared_ptr<Reflector> ptr;
        if (ptr == nullptr) {
            ptr.reset(new Reflector());
        }
        return ptr;
    }

    Reflector(Reflector &) = delete;

    Reflector(Reflector &&) = delete;
};
#define get_reflector() (*Reflector::Instance())

class register_action {
public:
    register_action(const std::string &class_name, constructor_func &&generator) {
        Reflector::Instance()->register_class(class_name, std::forward<constructor_func>(generator));
    }
};

#endif //TINYWEB_REFLECTOR_H