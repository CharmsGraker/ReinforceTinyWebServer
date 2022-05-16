//
// Created by nekonoyume on 2022/5/15.
//
#include "dprintf.h"

namespace yumira::debug {
    void
    DPrintf(const char *format, ...) {
        if (yumira::debug::debug_flag) {
            va_list args;
            va_start(args, format);
            vprintf(format, args);
            va_end(args);
        }
    }
}