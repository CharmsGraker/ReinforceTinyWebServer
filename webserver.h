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

#include "bin/threadpool/threadpool.h"
#include "bin/http/http_conn.h"
#include "bin/timer/lst_timer.h"

#include "configure.h"
#include "config.h"

namespace yumira {
    constexpr static int M_DISABLED_LOG = 1;
    constexpr static int M_ENABLED_LOG = 0;

    /** about server mode:
     *      Reactor
     *      Proactor
     * */
    static int REACTOR_MODE = 1;
    static int PROACTOR_MODE = 2;


    typedef Configure *ConfigurePtr;

    class WebServer {
        int stop_server = 0;
        ConfigurePtr configObj = nullptr;

        std::string M_DEFAULT_URL = "localhost";
        int M_DEFAULT_PORT = 3306;
        const char *severLogPath = "./ServerLog";

    public:
        WebServer();

        ~WebServer();

        void init(int port, string dbUser, string dbPasswd, string databaseName,
                  int log_write, int opt_linger, int triggerMode, int sql_num,
                  int thread_num, int close_log, int actor_model);


        ConfigurePtr bindConf(Configure &conf);

        void loadFromConf(Configure &conf);

        void createThreadPool();

        void sql_pool();

        void log_write();

        void setTrigMode();

        void registerEventListen();

        void eventLoop();

    private:
        void createTimerForUser(int connfd, struct sockaddr_in client_address);

        void adjust_timer(util_timer *timer);

        void remove_timer(util_timer *timer, int sockfd);

        bool deal_client_data();

        bool deal_sys_signal(bool &timeout, bool &stop_server);

        void deal_with_read(int sockfd);

        void deal_with_write(int sockfd);

        void parseFromConf(Configure &conf);

        void add_res_path(string res_path);

    public:
        //基础
        int m_port;
        char *m_root;
        int m_log_write;
        int m_close_log;
        int actorMode;

        int m_pipefd[2];
        int m_epollfd;
        http_conn *httpConnForUsers; // connection for http

        //数据库相关
        connection_pool *m_connPool;
        string m_user;         //登陆数据库用户名
        string m_passWord;     //登陆数据库密码
        string m_databaseName; //使用数据库名
        int m_sql_num;

        //线程池相关
        threadPool<http_conn> *http_conn_pool;
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
        storage_t configs;

    private:
        void
        __show_configs() {
            fprintf(stdout, "server configs: {\n");
            for (auto &config: configs) {
                fprintf(stdout, "\t[%s],", config.first.c_str());
            }
            fprintf(stdout, "\n}\n");
        }
    };
}

namespace yumira {
    extern yumira::yumira_server_t *current_app;
}
#else
#endif
