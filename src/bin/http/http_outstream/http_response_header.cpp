#include "http_response_header.h"
namespace http_message {
    namespace response {
        const char *RESPONSE_HEADER_200 = "OK";
        const char *redirect_301_title = "Moved Permanently";
        const char *redirect_301_form = "switch to new Server.\n";
        const char *error_400_title = "Bad Request";
        const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
        const char *error_403_title = "Forbidden";
        const char *error_403_form = "You do not have permission to get file form this server.\n";
        const char *error_404_title = "Not Found";
        const char *error_404_form = "The requested file was not found on this server.\n";
        const char *error_500_title = "Internal Error";
        const char *error_500_form = "There was an unusual problem serving the request file.\n";
    }
    // 200
    template<>
    response::title_type
    response::header<200>::title() {
        return "OK";
    };

// 300
    template<>
    response::title_type
    response::header<301>::title() {
        return "Moved Permanently";
    };

    template<>
    response::title_type
    response::header<303>::title() {
        return "See Other";
    };

// 400
    template<>
    response::title_type
    response::header<400>::title() {
        return "Bad Request";
    };

    template<>
    response::title_type
    response::header<403>::title() {
        return "Not Found";
    };

    template<>
    response::title_type
    response::header<404>::title() {
        return "Not Found";
    };

    template<>
    response::title_type
    response::header<500>::title() {
        return "Internal Error";
    };
}