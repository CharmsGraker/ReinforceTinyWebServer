cmake_minimum_required(VERSION 3.21)
project(ingress)
set(CMAKE_CXX_FLAGS " -std=c++17 ${CAMKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)
aux_source_directory(../extensions/loadbalancer/ DIR_REDIS_LOADBALANCER)
aux_source_directory(../config DIR_CONFIG)
add_executable(ingressMain ingressApplication.cpp
        ../timer/lst_timer.cpp ../debug/dprintf.cpp
        ../http/http_request/http_request.cpp
        args.cpp
        ingress.cpp
        socket_op.cpp
        ${DIR_CONFIG}
        ${DIR_REDIS_LOADBALANCER}
        ../socket/Socket.cpp
        ../http/HttpResponse.h
        ../http/http_outstream/http_response_header.cpp
        ../string/string.cpp)

find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(ingressMain PRIVATE jsoncpp_object jsoncpp_static JsonCpp::JsonCpp)
target_link_libraries(ingressMain PRIVATE hiredis)
#target_link_libraries(threadsafe_queue_test pthread)
