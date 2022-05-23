//
// Created by nekonoyume on 2022/5/18.
//

#ifndef TINYWEB_RESOURCE_RAII_H
#define TINYWEB_RESOURCE_RAII_H

#include <functional>
#include <memory>

using namespace std;

struct res_base;





template<class T>
class raii_guard {
    struct res_base {
        virtual void * get_object_ptr() {
            return nullptr;
        }
        virtual ~res_base() = default;
    };

    template<class M>
    struct obj_owner : res_base {
        typedef M object_type;
        typedef
        std::function<void(object_type *)> deleter_t;
        object_type *object_ptr;
        deleter_t deleter;

        object_type *get_object() {
            return object_ptr;
        }

        obj_owner(M *obj, deleter_t deleter_) : object_ptr(obj), deleter(deleter_) {}

        void * get_object_ptr() {
            printf("child get_object_ptr\n");
            return object_ptr;
        }

        ~obj_owner() override {
            deleter(object_ptr);
        }
    };
public:
    raii_guard(T *obj_ptr, std::function<void(T *)> deleter = [](T *p) {
        delete p;
    }):objectOwner(new obj_owner<T>(obj_ptr,deleter)) {

    };

    raii_guard(const T &obj, std::function<void(T *)> deleter = [](T *p) {
        delete p;
    }):objectOwner(new obj_owner<T>(new T(obj), deleter)) {

    };

    T * get_object() {
        return (T *)(objectOwner->get_object_ptr());
    };

    ~raii_guard() = default;

private:
    unique_ptr<res_base> objectOwner;
};

#endif //TINYWEB_RESOURCE_RAII_H
