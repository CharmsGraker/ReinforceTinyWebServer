#include "HttpInterceptor.h"
HttpInterceptor::HttpInterceptor() noexcept : request_context(nullptr)  {
    printf("construct HttpInterceptor\n");
}