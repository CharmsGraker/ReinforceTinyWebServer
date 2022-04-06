#pragma once
#ifndef WEBSERVER_H
#define WEBSERVER_H
#define PROJECT_NAME "TINY_WEB"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <assert.h>

#include "bin/threadpool/threadpool.h"
#include "bin/timer/lst_timer.h"

#include "configure.h"
#include "config.h"
#include "bin/http/http_const_declare.h"
#include "bin/http/http_conn.h"

namespace yumira {
    constexpr static int M_DISABLED_LOG = 1;
    constexpr static int M_ENABLED_LOG = 0;

    typedef void(*quit_handler)(int);

    using _Url = url_t;
    using _StorageMap = StorageMap;

    /** about server mode:
     *      Reactor
     *      Proactor
     * */
    static int REACTOR_MODE = 1;
    static int PROACTOR_MODE = 2;

    using httpConnType = http_conn;
    using WebServerType = yumira::WebServer<yumira::httpConnType>;

    template<class HttpConn=http_conn>
    class WebServer {
        int stop_server = 0;

        std::string M_DEFAULT_URL = "localhost";
        int M_DEFAULT_PORT = 3306;
        const char *severLogPath = "./ServerLog";

    public:
        WebServer();

        ~WebServer() {
            LOG_WARN("deconstruct server: %d", this);
            close(m_epollfd);
            close(m_listenfd);
            close(m_pipefd[1]);
            close(m_pipefd[0]);
            if (httpConnForUsers)
                delete[] httpConnForUsers;
            httpConnForUsers = nullptr;
            if (users_timer)
                delete[] users_timer;
            users_timer = nullptr;
            if (http_conn_pool)
                delete http_conn_pool;
            http_conn_pool = nullptr;

        }

        void
        init(int port,
             string dbUser,
             string dbPasswd,
             string databaseName,
             int log_write, int opt_linger, int triggerMode, int sql_num,
             int thread_num, int close_log, int actor_model);


        template<class Configuration=Configure>
        void
        loadFromConf(Configuration &conf);

        void
        createThreadPool();

        void
        sql_pool();

        void
        init_log_write();

        void setTrigMode();

        void registerEventListen();

        void eventLoop();

        static void
        shutdown(WebServer *server) {
            LOG_WARN("server %d trying to shutdown", server);
            delete server;
            LOG_WARN("server has been shutdown");
        }

        storage_t &
        getConfigs() {
            return server_configs;
        }

    private:
        void createTimerForUser(int connfd, struct sockaddr_in client_address);

        void adjust_timer(util_timer *timer);

        void remove_timer(util_timer *timer, int sockfd);

        bool deal_client_data();

        bool deal_sys_signal(bool &timeout, bool &stop_server);

        void deal_with_read(int sockfd);

        void deal_with_write(int sockfd);

        template<class Configuration=Configure>
        void parseFromConf(Configuration &conf);

        void add_res_path(string res_path);


    public:
        //基础
        int m_port;
        char *m_root;
        int m_log_write;
        int m_close_log = 0;
        int actorMode;

        int m_pipefd[2];
        int m_epollfd;
        HttpConn *httpConnForUsers; // connection for http

        //数据库相关
        connection_pool *m_connPool;
        string m_user;         //登陆数据库用户名
        string m_passWord;     //登陆数据库密码
        string m_databaseName; //使用数据库名
        int m_sql_num;

        //线程池相关
        threadPool<HttpConn> *http_conn_pool;
        int m_thread_num;

        //epoll_event相关
        epoll_event epollEvents[MAX_EVENT_NUMBER];

        int m_listenfd;
        int SOCKET_OPT_LINGER;
        int m_TRIGMode;
        int m_LISTENTrigmode;
        int m_CONNTrigmode;

        //定时器相关
        client_data_t *users_timer;
        Utils utils;
        storage_t server_configs;

    private:
        void
        __show_configs() {
            fprintf(stdout, "server configs: {\n");
            for (auto &config: server_configs) {
                fprintf(stdout, "\t[%s],", config.first.c_str());
            }
            fprintf(stdout, "\n}\n");
        }
    };
}

template<class HttpConn>
template<class Configuration>
void
yumira::WebServer<HttpConn>::loadFromConf(Configuration &conf) {
    parseFromConf(conf);
}

template<class HttpConn>
template<class Configuration>
void
yumira::WebServer<HttpConn>::parseFromConf(Configuration &conf) {
    this->init(conf.template getPropOf<int>("port"),
               conf.template getPropOf<string>("db_user"),
               conf.template getPropOf<string>("db_passwd"),
               conf.template getPropOf<string>("db_name"),
               conf.template getPropOf<int>("logWrite"),
               conf.template getPropOf<int>("lingerOption"),
               conf.template getPropOf<int>("triggerMode"),
               conf.template getPropOf<int>("sqlConPoolSize"),
               conf.template getPropOf<int>("httpConnPoolSize"),
               conf.template getPropOf<int>("disableLogger"),
               conf.template getPropOf<int>("concurrentActor"));
}

template
class yumira::WebServer<httpConnType>;

#else
#endif
