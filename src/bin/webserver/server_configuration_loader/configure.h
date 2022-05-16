#ifndef TINYWEB_CONFIGURE_H
#define TINYWEB_CONFIGURE_H

#include <unordered_map>
#include "../../config/config.h"

using namespace yumira::dev;
#define FLUSH_STDOUT(); (fflush(stdout));

using namespace std;

namespace yumira {
    class http_conn;

}

class Configure {
private:
    unordered_map<string, string> *tab = nullptr;
    int __verbose;
public:
    Configure();

    ~Configure() { delete tab; };

    void setDefaultProp();

    int _init_store_container();

    void parse_arg(int argc, char *argv[]);

    void gen_conf(int argc, char *argv[]);

    int loadConfigFromXml(const char *file_path);

    int setProp(const char *attr, const char *val);


    std::string getProp(const char *propName);

    template<typename T>
    T
    getPropOf(const char *propName) {
        return (const T) getProp(propName);
    }

    //端口号
    int PORT;

    //日志写入方式
    int LOGWrite;

    //触发组合模式
    int TRIGMode;

    //listenfd触发模式
    int LISTENTrigmode;

    //connfd触发模式
    int CONNTrigmode;

    //优雅关闭链接
    int OPT_LINGER;

    //数据库连接池数量
    int sql_num;

    //线程池内的线程数量
    int thread_num;

    //是否关闭日志
    int close_log;

    //并发模型选择
    int actor_model;
public:
    template<template<class T>
            class Server>
    void
    init(Server<yumira::http_conn> *server) {
        server->init(getPropOf<int>("port"),
                     getPropOf<string>("db_user"),
                     getPropOf<string>("db_passwd"),
                     getPropOf<string>("db_name"),
                     getPropOf<int>("logWrite"),
                     getPropOf<int>("lingerOption"),
                     getPropOf<int>("triggerMode"),
                     getPropOf<int>("sqlConPoolSize"),
                     getPropOf<int>("httpConnPoolSize"),
                     getPropOf<int>("disableLogger"),
                     getPropOf<int>("concurrentActor"));
        idleConfig.REDIS_HOST = getPropOf<string>("redis_host");
        idleConfig.REDIS_PORT = getPropOf<int>("redis_port");
        initSqlPool(server);
    }
};

/** to explict specialization */
template<>
int
Configure::getPropOf<int>(const char *propName);


#endif //TINYWEB_CONFIGURE_H
