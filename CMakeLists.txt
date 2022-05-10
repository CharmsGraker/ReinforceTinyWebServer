cmake_minimum_required(VERSION 3.21)
project(tinyWeb)
set(CMAKE_CXX_FLAGS " -std=c++17 ${CAMKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)

set(INC_DIRS /usr/include/mysql/ /usr/local/include/ctemplate/)
set(LINK_DIRS /usr/lib/x86_64-linux-gnu/)

include_directories(${INC_DIRS})
link_directories(${LINK_DIRS})
link_libraries(mysqlclient)
link_libraries(ctemplate)
link_libraries(pthread)
#  libxml lib support

include_directories(/usr/include/libxml2/)
link_libraries(xml2)

aux_source_directory(. DIR_CUR_SRCS)
aux_source_directory(src/bin/log DIR_LOG_SRCS)
aux_source_directory(src/bin/timer DIR_TIMER_SRCS)
aux_source_directory(src/bin/http DIR_HTTP_SRCS)
aux_source_directory(src/bin/http/http_outstream DIR_HTTP_OUTSTREAM_SRCS)
aux_source_directory(src/bin/extensions/xml_utils DIR_XML_UTILS_SRCS)
aux_source_directory(src/bin/extensions/loadbalancer DIR_LOADBALANCER_SRCS)
aux_source_directory(src/bin/extensions/string_utils DIR_STRING_UTILS_SRCS)
aux_source_directory(src/bin/socket DIR_SOCKET_SRCS)
aux_source_directory(src/bin/extensions/heartbeats DIR_HEARTBEATS_SRCS)
aux_source_directory(src/bin/extensions/redis_utils DIR_REDIS_UTILS_SRCS)
aux_source_directory(src/bin/webserver DIR_WEBSERVER_SRCS)
aux_source_directory(src/bin/webserver/signal_handler DIR_SIGNAL_HANDLER_SRCS)
aux_source_directory(src/bin/webserver/server_configuration_loader DIR_SERVER_CONFIGURATION_LOADER_SRCS)
aux_source_directory(src/bin/netroute DIR_NET_ROUTE_SRCS)
aux_source_directory(src/bin/threadpool/scheduler DIR_THREADPOOL_SCHEDULER_SRCS)
aux_source_directory(src/bin/threadpool DIR_THREADPOOL_SRCS)
aux_source_directory(src/main/devlop/ DIR_DEVELOP_SRCS)
aux_source_directory(src/bin/config DIR_WEBSERVER_CONFIG_SRCS)
aux_source_directory(src/bin/ByteBuffer DIR_BYTEBUFFER_SRCS)
aux_source_directory(src/bin/extensions/render DIR_RENDER_SRCS)
aux_source_directory(src/bin/container DIR_CONTAINER_SRCS)
aux_source_directory(src/bin/concurrent DIR_CONCURRENT_SRCS)
aux_source_directory(src/bin/CGImysql DIR_CGIMYSQL_SRCS)
aux_source_directory(src/bin/extensions/security/ DIR_SECURITY_SRCS)
aux_source_directory(src/bin/Interceptor/ DIR_INTERCEPTORS_SRCS)
aux_source_directory(src/bin/registry/InterceptorRegistry DIR_INTERCEPTOR_REGISTRY_SRCS)
add_executable(tinyWebMain src/main/main.cpp ${DIR_CUR_SRCS} ${DIR_TIMER_SRCS} ${DIR_HTTP_OUTSTREAM_SRCS}
        ${DIR_WEBSERVER_SRCS}
        ${DIR_THREADPOOL_SCHEDULER_SRCS}
        ${DIR_THREADPOOL_SRCS}
        src/bin/ClassHolder.h
        ${DIR_SECURITY_SRCS}
        ${DIR_INTERCEPTORS_SRCS}
        ${DIR_NET_ROUTE_SRCS}
        ${DIR_DEVELOP_SRCS}
        ${DIR_INTERCEPTOR_REGISTRY_SRCS}
        ${DIR_WEBSERVER_CONFIG_SRCS}
        ${DIR_SOCKET_SRCS}
        ${DIR_RENDER_SRCS}
        ${DIR_SERVER_CONFIGURATION_LOADER_SRCS}
        ${DIR_HEARTBEATS_SRCS}
        ${DIR_LOADBALANCER_SRCS}
        ${DIR_STRING_UTILS_SRCS}
        ${DIR_HTTP_SRCS}
        ${DIR_SIGNAL_HANDLER_SRCS}
        ${DIR_BYTEBUFFER_SRCS}
        ${DIR_LOG_SRCS}
        ${DIR_REDIS_UTILS_SRCS}
        ${DIR_XML_UTILS_SRCS}
        ${DIR_CONTAINER_SRCS}
        ${DIR_CGIMYSQL_SRCS}
        ${DIR_CONCURRENT_SRCS})


find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(tinyWebMain PRIVATE jsoncpp_object jsoncpp_static JsonCpp::JsonCpp)
target_link_libraries(tinyWebMain PRIVATE hiredis)
target_link_libraries(tinyWebMain PRIVATE boost_system)
target_link_libraries(tinyWebMain PRIVATE crypto)

target_link_libraries(tinyWebMain PRIVATE boost_filesystem)