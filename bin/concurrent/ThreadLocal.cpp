//
// Created by nekonoyume on 2022/1/6.
//
#include "ThreadLocal.h"
thread_local local_map_t * local_map = ThreadLocal::getLocalMap();
