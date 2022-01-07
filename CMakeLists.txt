cmake_minimum_required(VERSION 3.21)
project(tinyWeb)
set(CMAKE_CXX_FLAGS " -std=c++17 ${CAMKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)

set(INC_DIR /usr/include/mysql/ /usr/local/include/ctemplate/)
set(LINK_DIR /usr/lib/x86_64-linux-gnu/)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(mysqlclient)
link_libraries(ctemplate)

#  libxml lib support

include_directories(/usr/include/libxml2/)

add_executable(tinyWeb main.cpp bin/timer/lst_timer.cpp bin/http/http_conn.cpp bin/log/log.cpp bin/CGImysql/sql_connection_pool.cpp webserver.cpp configure.h configure.cpp devlop/UserMain.cpp bin/http/http_connect_adapter.h bin/http/http_const_declare.h config.h current_app.h bin/container/url_storage.h bin/concurrent/ThreadLocal.cpp bin/concurrent/ThreadLocal.h)
add_executable(parse_xml utils/c/test_parse.c utils/c/create_xml.c utils/c/parse_xml.c  utils/c/parse_xml.h utils/c/create_xml.h  bin/netroute/router.h devlop/UserMain.cpp bin/UserMain.h bin/netroute/blueprint.h bin/netroute/http_request_enum.h utils/string_utils.cpp utils/string_utils.h  bin/netroute/request.h bin/all_exception.h bin/netroute/urlparser.h bin/container/hashmap.h bin/netroute/parsedurl.h bin/CGImysql/sql_query_adapter.h bin/container/storage.h )

add_executable(test_ctemplate bin/render/render_utils.cpp bin/render/render_utils.h bin/render/test_render_func.cpp)
add_executable(concurrent bin/concurrent/test_threadLocal.cpp bin/concurrent/ThreadLocal.cpp )
add_executable(test_util utils/test_utils.cpp  utils/helpfulUtils.cpp utils/helpfulUtils.cpp)

target_link_libraries(parse_xml xml2)

target_link_libraries(tinyWeb xml2)
target_link_libraries(tinyWeb pthread)
target_link_libraries(tinyWeb mysqlclient)

# for template testing
target_link_libraries(test_ctemplate ctemplate pthread)


target_link_libraries(concurrent pthread)