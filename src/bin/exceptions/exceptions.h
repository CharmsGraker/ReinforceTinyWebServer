//
// Created by nekonoyume on 2022/4/11.
//

#ifndef REDISTEST_REDISCONNECTFAILEXCEPTION_H
#define REDISTEST_REDISCONNECTFAILEXCEPTION_H

#include <exception>
#include <stdexcept>

class NullPointerException : public std::runtime_error {
public:
    explicit NullPointerException(const std::string &msg) : runtime_error(msg) {};

};

class IOException : public std::exception {

};

class RedisConnectFailException : public std::runtime_error {
public:
    explicit RedisConnectFailException(const char *msg) : runtime_error(msg) {};

    RedisConnectFailException() : RedisConnectFailException("a") {};
};

class UnAuthorizedException : public std::runtime_error {
public:
    explicit UnAuthorizedException(const char *msg) : runtime_error(msg) {};

    UnAuthorizedException() : UnAuthorizedException("UnAuthorizedException") {};
};

class IndexOutOfRangeException: public  std::runtime_error {
public:
    explicit IndexOutOfRangeException(const char *msg) : runtime_error(msg) {};

    IndexOutOfRangeException() : IndexOutOfRangeException("IndexOutOfRange") {};
};

#endif //REDISTEST_REDISCONNECTFAILEXCEPTION_H
