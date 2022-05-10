cmake_minimum_required(VERSION 3.21)
project(redisTest)
set(CMAKE_CXX_FLAGS " -std=c++17 ${CAMKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)
add_executable(test_jwt testing/jwt_test.cpp)
add_executable(test_jwtcpp testing/test_jwtcpp.cpp JwtTokenizer.cpp JwtTokenizer.h)

TARGET_LINK_LIBRARIES(test_jwt ssl crypto)
TARGET_LINK_LIBRARIES(test_jwtcpp ssl crypto)