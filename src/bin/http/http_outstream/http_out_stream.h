//
// Created by nekonoyume on 2022/4/15.
//

#ifndef HEARTBEATTHREADTEST_HTTP_OUT_STREAM_H
#define HEARTBEATTHREADTEST_HTTP_OUT_STREAM_H

#include <string>
class http_outstream {
public:
    http_outstream &
    operator<<(std::string & kv) {

        return *this;
    }
};

#endif //HEARTBEATTHREADTEST_HTTP_OUT_STREAM_H
