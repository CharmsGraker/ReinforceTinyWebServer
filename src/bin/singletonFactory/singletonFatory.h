//
// Created by nekonoyume on 2022/4/19.
//

#ifndef REDISTEST_SINGLETONFATORY_H
#define REDISTEST_SINGLETONFATORY_H
#include <boost/noncopyable.hpp>

template<class S>
class SingletonFactory:public boost::noncopyable {
    SingletonFactory();

public:
    static
    S &
    Get() {
        static S singleton;
        return singleton;
    };

    ~SingletonFactory() {

    };
};

#endif //REDISTEST_SINGLETONFATORY_H
