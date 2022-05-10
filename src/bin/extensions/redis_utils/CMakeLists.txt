cmake_minimum_required(VERSION 3.21)
project(redisTest)
set(CMAKE_CXX_FLAGS " -std=c++17 ${CAMKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)
aux_source_directory(../loadbalancer DIR_LOADBALANCER_SRCS)
aux_source_directory(. DIR_CURRENT_SRCS)
add_executable(test_redis_client testing/test_redis.cpp ${DIR_CURRENT_SRCS}  ${DIR_LOADBALANCER_SRCS})
target_link_libraries(test_redis_client hiredis)


