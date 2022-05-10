//
// Created by nekonoyume on 2022/4/15.
//

#ifndef HEARTBEATTHREADTEST_HTTP_RESPONSE_HEADER_H
#define HEARTBEATTHREADTEST_HTTP_RESPONSE_HEADER_H

#include <string>

namespace http_message {
    namespace response {
        //定义http响应的一些状态信息
        extern const char *RESPONSE_HEADER_200;
        extern const char *redirect_301_title;
        extern const char *redirect_301_form;
        extern const char *error_400_title;
        extern const char *error_400_form;
        extern const char *error_403_title;
        extern const char *error_403_form;
        extern const char *error_404_title;
        extern const char *error_404_form;
        extern const char *error_500_title;
        extern const char *error_500_form;

        typedef const char *title_type;
        typedef const char *description_type;

        template<int N>
        class header {

            int response_code_ = N;
        public:
            header() = default;

            int
            code() {
                return response_code_;
            }


            title_type
            title();

            description_type
            description();
        };


    }
}
#endif //HEARTBEATTHREADTEST_HTTP_RESPONSE_HEADER_H
