#pragma once
#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cassert>
#include <sys/epoll.h>

#include "../threadpool/threadpool.h"
#include "../timer/lst_timer.h"

#include "server_configuration_loader/configure.h"
#include "../config/config.h"
#include "../http/http_const_declare.h"
#include "../http/http_conn.h"
#include "../threadpool/g_threadpool.h"

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

using namespace yumira::dev;

namespace yumira {
    constexpr static int M_DISABLED_LOG = 1;
    constexpr static int M_ENABLED_LOG = 0;

    using _StorageMap = StorageMap;


    /** about server mode:
     *      Reactor
     *      Proactor
     * */
    static int REACTOR_MODE = 1;
    static int PROACTOR_MODE = 2;
    extern std::unordered_map<std::string, std::string> serverProviderMap;

    // connection to Redis

    using httpConnType = http_conn;
    using WebServerType = yumira::WebServer<yumira::httpConnType>;


    void remove_timer(client_data_t&, Utils &utils, util_timer *timer, int epollFd= -1);

    class web_server {
    public:
        void stop();
        virtual ~web_server()=default;
    };

    template<class HttpConn=http_conn>
    class WebServer: public web_server {
        int stop_server = 0;

        std::string M_MYSQL_URL = "localhost";
        int M_DEFAULT_PORT = 3306;
        std::vector<std::function<void(void)>> loop_callbacks;

    public:
        void
        addCallbackToLast(std::function<void(void)> f) {
            loop_callbacks.push_back(f);
        };

        WebServer();

        ~WebServer() {
            LOG_WARN("deconstruct server: %d", this);
            close(m_listenfd);
            delete m_epoller;
            close(m_pipefd[1]);
            close(m_pipefd[0]);
            if (httpConnForUsers)
                delete[] httpConnForUsers;
            httpConnForUsers = nullptr;
            if (users_timer_meta)
                delete[] users_timer_meta;
            users_timer_meta = nullptr;
            if (http_thread_pool)
                delete http_thread_pool;
            http_thread_pool = nullptr;
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
        createThreadPool() {
            http_thread_pool = new thread_pool();
        };


        template<class T>
        friend void
        initSqlPool(WebServer<T> *server) {
            printf("initSqlPool\n");
            server->m_connPool = connection_pool::builder()
                    .url(server->M_MYSQL_URL)
                    .user(server->m_user)
                    .passWord(server->m_passWord)
                    .dataBaseName(server->m_databaseName)
                    .port(server->M_DEFAULT_PORT)
                    .maxConn(server->m_sql_num)
                    .close_log(server->m_close_log).build();

            //初始化数据库读取表
            server->httpConnForUsers->initmysql_result(server->m_connPool);
        };

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
        Epoller* m_epoller;
        HttpConn *httpConnForUsers; // connection for http

        //数据库相关
        connection_pool *m_connPool;
        string m_user;         //登陆数据库用户名
        string m_passWord;     //登陆数据库密码
        string m_databaseName; //使用数据库名
        int m_sql_num;

        //线程池相关
        thread_pool *http_thread_pool;
        int m_thread_num;

        //epoll_event相关
        epoll_event *epollEvents;

        int m_listenfd;
        int SOCKET_OPT_LINGER;
        int m_TRIGMode;
        int m_LISTENTrigmode;
        int m_CONNTrigmode;

        //定时器相关
        client_data_t *users_timer_meta;
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
