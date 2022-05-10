
#include "webserver.h"
#include "../../main/devlop/threadpool_default_scheduler.h"
#include "../extensions/loadbalancer/LoadBalancer.h"
#include "../extensions/loadbalancer/RedisLoadBalancer.h"
#include "../extensions/heartbeats/SocketHeartBeatThread.h"
#include "../extensions/heartbeats/GrakerHeartBeat.h"

#include "../extensions/heartbeats/cluster/Cluster.h"

namespace yumira {
    const char *connIp = nullptr;
    int connPort = -1;
    std::string REDIS_CLUSTER_KEY = "grakerServerLb";
    std::shared_ptr<LoadBalancer> loadBalancer{};

}

/** return remain available Fd */
int
GRAKER_SERVER(Http_handler)(void *) {
    return MAX_FD - http_conn::activeConnectionSize();
};


template<class HttpConn>
yumira::WebServer<HttpConn>::WebServer():f_canAcceptHttp(GRAKER_SERVER(Http_handler)),
                                         f_checkConnFd(nullptr),
                                         f_createIdleThread(GRAKER_SERVER(createIdle)) {
    //http_conn类对象
    httpConnForUsers = new http_conn[MAX_FD];

    //定时器
    users_timer = new client_data_t[MAX_FD];


    //root文件夹路径
    std::string curFileAbsPath = (__FILE__);
    auto idx = curFileAbsPath.rfind("/");
    auto pdir = curFileAbsPath.substr(0, idx + 1);
    auto idx2 = curFileAbsPath.rfind("/", idx - 1);
    auto ppdir = curFileAbsPath.substr(0, idx2 + 1);

//    cout << absPath<<endl;
    m_root = (char *) malloc(pdir.size() + strlen(resourceFolder));
    strcpy(m_root, pdir.c_str());
    strcat(m_root, resourceFolder);


    /** store useful info */
    Node *p_m_root = new StringNode(m_root);
    Node *p_resourceFolder = new StringNode(resourceFolder);
    Node *p_server_abspath = new StringNode(ppdir.substr(0, ppdir.length() - 1));

    server_configs.put("template_root", p_m_root);
    printf("template root=%s\n", m_root);
    server_configs.put("template_relative_path", p_resourceFolder);
    server_configs.put("app_bin", p_server_abspath);
    __show_configs();

}


template<class HttpConn>
void
yumira::WebServer<HttpConn>::init(int port, string user,
                                  string passWord, string databaseName,
                                  int log_write,
                                  int opt_linger,
                                  int trigmode, int sql_num, int thread_num,
                                  int close_log,
                                  int actor_model) {
    m_port = port;
    m_user = user;
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_log_write = log_write;
    SOCKET_OPT_LINGER = opt_linger;
    m_TRIGMode = trigmode;
    m_close_log = close_log;
    actorMode = actor_model;
    yumira::m_service_ = "GrakerWebServer";


}


template<class HttpConn>
void yumira::WebServer<HttpConn>::setTrigMode() {
    //LT + LT
    if (0 == m_TRIGMode) {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 0;
    }
        //LT + ET
    else if (1 == m_TRIGMode) {
        m_LISTENTrigmode = 0;
        m_CONNTrigmode = 1;
    }
        //ET + LT
    else if (2 == m_TRIGMode) {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 0;
    }
        //ET + ET
    else if (3 == m_TRIGMode) {
        m_LISTENTrigmode = 1;
        m_CONNTrigmode = 1;
    }
}




template<class HttpConn>
void yumira::WebServer<HttpConn>::createThreadPool() {
    //线程池
    http_conn_pool = new threadPool<HttpConn*>(m_thread_num);
    auto scheduler = new ThreadPoolTaskDefaultScheduler<HttpConn>(http_conn_pool, actorMode, m_connPool);

}

void
setConnIp(const char *Ip) {
    yumira::connIp = const_cast<char *>(Ip);
};

void
setConnPort(const int port) {
    yumira::connPort = port;
};

template<class HttpConn>
void yumira::WebServer<HttpConn>::registerEventListen() {
    //线程池
    createThreadPool();
    printf("after createThreadPool\n");
    setTrigMode();
    m_server_ip_ = getLocalIP("localhost", 0);

    printf("serverHost: %s \n", m_server_ip_);


    //网络编程基础步骤
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        fprintf(stderr, "[WARN] create socket failed!\n%s\n", strerror(errno));
    }

    /** gracefully close connection.
     *  means when close a TCP connection,will send the residual data to client */
    if (0 == SOCKET_OPT_LINGER) {
        struct linger tmp = {0, 1}; // { on/off linger, risidual time }
        /**
         * l_onoff been not zero, l_linger > 0:
         *      close action determined by 2 conditions:
         *          1) if there exist data ?
         *          2) if the socket is block?
         *              for block socket, close will wait a period of time set in "l_linger" .
         * */
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    } else if (1 == SOCKET_OPT_LINGER) {
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    // force recycling use local address 5.11
    constexpr int REUSE = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &REUSE, sizeof(REUSE));

    if ((ret = bind(m_listenfd, (struct sockaddr *) &address, sizeof(address))) < 0) {
        throw NetworkExpcetion("[WARN] bind() failed!");
    }
    if ((ret = listen(m_listenfd, 5)) < 0) {
        throw NetworkExpcetion("[WARN] listen() failed!");
    }

    serverProviderMap[::m_service_] = string(m_server_ip_) + ":" + to_string(m_port);
    setConnIp(m_server_ip_);
    setConnPort(m_port);

    utils.init(TIMESLOT);
    printf("before create Idle\n");


    //epoll创建内核事件表
//    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.regist_fd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
    http_conn::m_epollfd = m_epollfd;

    /** create a pair of socket,store in m_pipefd */
    assert((ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd)) != -1);
    utils.setNonBlocking(m_pipefd[1]);
    // m_pipefd[0] use for send signal to main thread
    utils.regist_fd(m_epollfd, m_pipefd[0], false, 0);

    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

void
yumira::remove_timer(client_data_t *users_timer, Utils &utils, util_timer *timer, int sock_fd) {
    if (timer) {
        close_user_fd(&users_timer[sock_fd]);
        utils.m_timer_lst.remove(timer);
        LOG_INFO("close fd %d", users_timer[sock_fd].sockfd);
    }
}

template<class HttpConn>
void
yumira::WebServer<HttpConn>::createTimerForUser(int connfd, struct sockaddr_in client_address) {
    /** create work thread here
     * */
    httpConnForUsers[connfd].builder()
            .sockFd(connfd)
            .address(client_address)
            .docRoot(m_root)
            .triggerMode(m_CONNTrigmode)
            .sqlDataBaseUser(m_user)
            .sqlDataBasePassword(m_passWord)
            .sqlDataBaseName(m_databaseName)
            .loadBalancer(loadBalancer).build();

    printf("createTimerForUser %s\n", __FILE__);

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中

    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;

    auto &&userTimer = new util_timer;
    userTimer->user_data = &users_timer[connfd];
    userTimer->closeCallBack = close_user_fd;

    time_t cur = time(nullptr);
    userTimer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = userTimer;
    utils.m_timer_lst.add_timer(userTimer);
    printf("userTimer %s\n", __FILE__);

    userTimer->removeCallback = [this, userTimer = users_timer[connfd].timer, connfd] {
        return yumira::remove_timer(users_timer, utils, userTimer, connfd);
    };
    printf("createTimerForUser done %s\n", __FILE__);

}

//若有数据传输，则将定时器往后延迟3个单位
//并对新的定时器在链表上的位置进行调整
template<class HttpConn>
void
yumira::WebServer<HttpConn>::adjust_timer(util_timer *timer) {
    time_t current_time = time(nullptr);
    timer->expire = current_time + 3 * TIMESLOT;
    utils.m_timer_lst.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

template<class HttpConn>
int
yumira::WebServer<HttpConn>::deal_client_data() {
    /**
     *   this function deal with new connection for user,accoding to mode set in ListenTriggerMode,
     *   will be little different when deal new connection
     *
     *   @attention ERROR: when now more available fd for use.(not exceed than MAX_FD set in WebServer.h )
     *    */
    auto acceptAndCreateTimer = [this]() {
        int ret = 0;

        struct sockaddr_in client_address;

        socklen_t len_client_addr = sizeof(client_address);
        int conn_fd = accept(m_listenfd, (struct sockaddr *) &client_address, &len_client_addr);
        if (conn_fd < 0) {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            printf("accept error\n");
            return M_READERROR;
        }
        ret = 0;
        if (f_checkConnFd && (ret = f_checkConnFd(conn_fd)) == 0) {
            // ignore other datagram on this socket
            // forward to Idle Thread
            //TODO how to do task in main thread?

            return M_IDLEDATA;
        }
        if (ret < 0) {
            printf("error when checkConnFd()\n");
            std::abort();
        }
        int n_remain = 0;
        assert(f_canAcceptHttp);
        if ((n_remain = f_canAcceptHttp((void *) &client_address)) <= 0) {
            utils.show_error(conn_fd, "Internal server busy");
            printf("Internal server busy\n");

            LOG_ERROR("%s", "Internal server busy.");
            return M_SERVERBUSY;
        }
        // allocate conn_fd to worker
        createTimerForUser(conn_fd, client_address);
        return M_SUCCESS;
    };

    if (0 == m_LISTENTrigmode) {
        return acceptAndCreateTimer();
    } else {
        while (!acceptAndCreateTimer()) {}
    }
    return true;
}

/**
 * @param timeout: if timeout will be set true
 * @param stopServer: output param
 * */
template<class HttpConn>
bool
yumira::WebServer<HttpConn>::deal_sys_signal(bool &timeout, bool &stopServer) {
    char signal_buffer[1024];
    ssize_t ret = recv(m_pipefd[0], signal_buffer, sizeof(signal_buffer), 0);
//    cout << ret << endl;
    if (ret == -1) {
        return false;
    } else if (ret == 0) {
        return false;
    } else {
        for (int i = 0; i < ret; ++i) {
            switch (signal_buffer[i]) {
                case SIGALRM: {
                    timeout = true;
                    break;
                }
                case SIGTERM: {
                    stopServer = true;

                    break;
                }
            }
        }
    }
    return true;
}

template<class HttpConn>
void
yumira::WebServer<HttpConn>::deal_with_read(int sockfd) {
    /** do EPOLL_IN event here */
    util_timer *timer = users_timer[sockfd].timer;

    //reactor
    if (REACTOR_MODE == actorMode) {
        if (timer) {
            adjust_timer(timer);
        }

        LOG_INFO("append read event from sock fd: %d to Http Connection Pool:%d", sockfd, http_conn_pool);
        //若监测到读事件，将该事件放入请求队列
        http_conn_pool->append(httpConnForUsers + sockfd, 0);

        // process down the http_request
        while (true) {
            if (1 == httpConnForUsers[sockfd].improv) {
                if (1 == httpConnForUsers[sockfd].timer_flag) {
                    // callback
                    try {
                        timer->removeCallback();
                    } catch (exception &e) {
                        e.what();
                        printf("%s\n", __FILE__);
                    }
                    httpConnForUsers[sockfd].timer_flag = 0;
                }
                httpConnForUsers[sockfd].improv = 0;
                break;
            }
        }
    } else if (PROACTOR_MODE == actorMode) {
        //proactor
        if (httpConnForUsers[sockfd].read_once()) {
            // let main thread read finished, then submit to work thread do logical work
            LOG_INFO("deal with client(%s)", inet_ntoa(httpConnForUsers[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            http_conn_pool->append(httpConnForUsers + sockfd);

            if (timer) {
                adjust_timer(timer);
            }
        } else {
            timer->removeCallback();
        }
    } else {
        LOG_WARN("Unsupport ActorMode(%d)", actorMode)
        throw exception();
    }
}

template<class HttpConn>
void
yumira::WebServer<HttpConn>::deal_with_write(int sockfd) {
    util_timer *timer = users_timer[sockfd].timer;
    //reactor
    if (REACTOR_MODE == actorMode) {
        if (timer) {
            adjust_timer(timer);
        }
        // reactor means worker thread actually do the request, so put the quest to threadPool workQueue
        LOG_INFO("append write event from sock fd: %d to Http Connection Pool:%d", sockfd, http_conn_pool);
        http_conn_pool->append(httpConnForUsers + sockfd, 1);

        while (true) {
            if (1 == httpConnForUsers[sockfd].improv) {
                // remove timer
                if (1 == httpConnForUsers[sockfd].timer_flag) {
                    timer->removeCallback();
                    httpConnForUsers[sockfd].timer_flag = 0;
                }
                httpConnForUsers[sockfd].improv = 0;
                break;
            }
        }
    } else {
        //PROACTOR mode
        if (httpConnForUsers[sockfd].write()) {
            LOG_INFO("send data to the client(%s)", inet_ntoa(httpConnForUsers[sockfd].get_address()->sin_addr));

            if (timer) {
                adjust_timer(timer);
            }
        } else {
            timer->removeCallback();
        }
    }
}

template<class HttpConn>
void
yumira::WebServer<HttpConn>::eventLoop() {
    /** the main thread only handler new request event or signal
     * actually process is in worker thread of threadPool has been create before. */
    // start de with event
    bool timeout = false;
    stop_server = false;

    // pass current server unique id to idle-thread
    int n_param = 3;
    const char *args_[n_param + 1];
    auto serverIpStr = InetAddress::Serialize({m_server_ip_, m_port});
    auto clusterIpStr = InetAddress::Serialize({redis_host_, redis_port_});
    std::string clusterKey = "grakerWebServerLb";
    args_[0] = (char *) &n_param;
    args_[1] = serverIpStr.c_str();
    args_[2] = clusterIpStr.c_str();
    args_[3] = clusterKey.c_str();

    if (f_createIdleThread && f_createIdleThread(1, (char **) args_)) {
        printf("abort when create idleThread\n");
        std::exit(-1);
    }
    fprintf(stdout, "\n[INFO] server start running...\n");

    if (!loadBalancer.get()) {
        loadBalancer.reset(new RedisLoadBalancer(redis_host_, redis_port_));
    }
    while (!stop_server) {
        int n_fd;
        // main thread need call epoll_wait to wait for socket which has data to read
        if ((n_fd = epoll_wait(m_epollfd, epollEvents, MAX_EVENT_NUMBER, -1)) < 0
            && errno != EINTR) {
            LOG_ERROR("%s", "epoll failure");
            break;
        }
//        cluster.registeToCenter(InetAddress{m_server_ip_,m_port});

        // detect read events, handle the events to worker threads
        for (int i = 0; i < n_fd; ++i) {
            int sockfd = epollEvents[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_listenfd) {
                // allocate new connection for user if there are some free http connections.
                auto flag = deal_client_data();
                if (flag < 0) {
                    cout << "flag fail" << endl;
                    continue;
                } else if (flag == M_IDLEDATA) {
                    // TODO do something
                }
            } else if (epollEvents[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //服务器端关闭连接，移除对应的定时器
                users_timer[sockfd].timer->removeCallback();
            } else if ((sockfd == m_pipefd[0]) && (epollEvents[i].events & EPOLLIN)) {
                //处理信号
                if (!deal_sys_signal(timeout, reinterpret_cast<bool &>(stop_server))) LOG_ERROR("%s",
                                                                                                "deal client data failure");
            } else if (epollEvents[i].events & EPOLLIN) {
                // a read event has been arrived. let worker thread handle it
                //处理客户连接上接收到的数据
                deal_with_read(sockfd);
            } else if (epollEvents[i].events & EPOLLOUT) {
                // a write event has been arrived.
                deal_with_write(sockfd);
            }
        }
        if (timeout) {
            utils.timer_handler();

            LOG_INFO("%s", "timer out");
            timeout = false;
        }
    }
}

template<>
void yumira::initSqlPool(WebServer<http_conn> *server) {
    //初始化数据库连接池
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
}