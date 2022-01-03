cmake_minimum_required(VERSION 3.21)
project(tinyWeb)
set(CMAKE_CXX_FLAGS " -std=c++17 ${CAMKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)

set(INC_DIR /usr/include/mysql/)
set(LINK_DIR /usr/lib/x86_64-linux-gnu/)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(mysqlclient)

#  libxml lib support

include_directories(/usr/include/libxml2/)

add_executable(tinyWeb main.cpp ./timer/lst_timer.cpp ./http/http_conn.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp  webserver.cpp configure.h configure.cpp devlop/UserMain.cpp)
add_executable(parse_xml utils/c/test_parse.c utils/c/create_xml.c utils/c/parse_xml.c  utils/c/parse_xml.h utils/c/create_xml.h router/router.cpp bin/netroute/router.h devlop/UserMain.cpp bin/UserMain.h bin/netroute/blueprint.h bin/netroute/http_request_enum.h)
add_executable(testUtils utils/helpfulUtils.cpp utils/helpfulUtils.h)

target_link_libraries(tinyWeb xml2)
target_link_libraries(parse_xml xml2)

target_link_libraries(tinyWeb pthread)
target_link_libraries(tinyWeb mysqlclient)