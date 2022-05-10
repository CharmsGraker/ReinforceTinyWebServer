cmake_minimum_required(VERSION 3.21)
project(HeartBeatThreadTest)
set(CMAKE_CXX_FLAGS " -std=c++14 ${CAMKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 14)

aux_source_directory(. DIR_LIB_SRCS)
aux_source_directory(../../socket DIR_SOCKET_SRCS)
aux_source_directory(../../concurrent DIR_CONCURRENT_SRCS)
aux_source_directory(../../timer DIR_TIMER_SRCS)
aux_source_directory(../redis_utils DIR_REDIS_UTILS_SRCS)
add_executable(test_hb testing/test_hb_main.cpp ${DIR_LIB_SRCS} ${DIR_SOCKET_SRCS} ../../concurrent/ThreadFunction.cpp ${DIR_TIMER_SRCS}
        ../redis_utils/redis_utils.cpp)
add_executable(test_hb_slaves testing/new_hb_slave.cpp ${DIR_LIB_SRCS} ${DIR_SOCKET_SRCS} ../../concurrent/ThreadFunction.cpp ${DIR_TIMER_SRCS}
        ../redis_utils/redis_utils.cpp)
add_executable(idle_exec_main exec_cpp/idle_exec_main.cpp ${DIR_SOCKET_SRCS} ${DIR_LIB_SRCS} ${DIR_REDIS_UTILS_SRCS} ${DIR_CONCURRENT_SRCS} ${DIR_TIMER_SRCS})
target_link_libraries(test_hb hiredis)
target_link_libraries(test_hb pthread)

target_link_libraries(test_hb_slaves hiredis)
target_link_libraries(test_hb_slaves pthread)

target_link_libraries(idle_exec_main hiredis)
target_link_libraries(idle_exec_main pthread)
target_link_libraries(idle_exec_main boost_system boost_filesystem)
