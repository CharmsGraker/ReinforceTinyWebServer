//
// Created by nekonoyume on 2022/5/15.
//

#ifndef INGRESS_DPRINTF_H
#define INGRESS_DPRINTF_H

#include <cstdarg>
#include <cstdio>

namespace yumira::debug {
    extern bool debug_flag;

    void
    DPrintf(const char *format, ...);

}

#endif //INGRESS_DPRINTF_H
