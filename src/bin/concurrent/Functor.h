//
// Created by nekonoyume on 2022/4/11.
//

#ifndef REDISTEST_FUNCTOR_H
#define REDISTEST_FUNCTOR_H
#include <functional>
class Functor {
public:
    template<class callable,typename... arguments>
    void run(callable func,arguments... args);
};

#endif //REDISTEST_FUNCTOR_H
