#include "string.h"
#include <stdio.h>
#include <cstdarg>
std::string Format(const char * format,...) {
    char tmp_[256]={};
    va_list args;
    va_start(args,format);
    vsprintf(tmp_,format,args);
    va_end(args);
//    printf("%s\n",tmp_);
    return tmp_;
}