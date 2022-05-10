//
// Created by nekonoyume on 2022/4/11.
//

#ifndef REDISTEST_RUNNABLE_H
#define REDISTEST_RUNNABLE_H

#include "Functor.h"
class Runnable :virtual public Functor {
public:

    virtual void
    run()=0;
};

#endif //REDISTEST_RUNNABLE_H
