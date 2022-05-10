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

#include "../threadpool/threadpool.h"
#include "../timer/lst_timer.h"

#include "server_configuration_loader/configure.h"
#include "../config/config.h"
#include "../http/http_const_declare.h"
#include "../http/http_conn.h"

class SocketHeartBeatThread;

template<class A, class K, class M>
class Cluster;

#ifndef GRAKER_SERVER
#define GRAKER_SERVER(x) x

int
GRAKER_SERVER(Http_handler)(void *);

void *
GRAKER_SERVER(createIdle)(int agrc, char *args[]);

#endif

int
peekHBPack(int conn_fd);

namespace yumira {
    constexpr static int M_DISABLED_LOG = 1;
    constexpr static int M_ENABLED_LOG = 0;


    using _Url = url_t;
    using _StorageMap = StorageMap;

    extern std::shared_ptr<LoadBalancer> loadBalancer;

    /** about server mode:
     *      Reactor
     *      Proactor
     * */
    static int REACTOR_MODE = 1;
    static int PROACTOR_MODE = 2;
    extern std::unordered_map<std::string, std::string> serverProviderMap;

    // connection to Redis
    extern string redis_host_;
    extern int redis_port_;

    using httpConnType = http_conn;
    using WebServerType = yumira::WebServer<yumira::httpConnType>;


    void remove_timer(client_data_t *, Utils &utils, util_timer *timer, int sockfd);


    template<class HttpConn=http_conn>
    class WebServer {
        int stop_server = 0;

        std::string M_MYSQL_URL = "localhost";
        const char *m_server_ip_;
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
            delete m_root;
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


        template<class T>
        friend void
        initSqlPool(WebServer<T> *server);

        template<class W>
        friend void
        initLogger(W *);

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

        int (*f_checkConnFd)(int sockFd);

        int (*f_canAcceptHttp)(void *);

        void *(*f_createIdleThread)(int agrc_, char *args_[]);

    private:
        void createTimerForUser(int connfd, struct sockaddr_in client_address);

        void adjust_timer(util_timer *timer);

        // return 0 on success, negative on failure, positive for idle thread
        int
        deal_client_data();

        bool deal_sys_signal(bool &timeout, bool &stop_server);

        void deal_with_read(int sockfd);

        void deal_with_write(int sockfd);

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
        threadPool<HttpConn*> *http_conn_pool;
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

template
class yumira::WebServer<httpConnType>;


#else
#endif
