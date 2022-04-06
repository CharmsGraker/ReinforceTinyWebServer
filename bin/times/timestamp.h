//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_TIMESTAMP_H
#define TINYWEB_TIMESTAMP_H

#include <time.h>

class TimeStamp {
public:
    TimeStamp() : time_st(time(nullptr)) {};

    static TimeStamp now() {
        return {};
    };

    ~TimeStamp() {};
private:
    time_t time_st;
};

#endif //TINYWEB_TIMESTAMP_H
