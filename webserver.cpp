#include "webserver.h"
#include "devlop/threadpool_default_scheduler.h"

yumira::WebServer::WebServer() {
    //http_conn类对象
    httpConnForUsers = new http_conn[MAX_FD];

    //定时器
    users_timer = new client_data_t[MAX_FD];

    //root文件夹路径
    char server_abspath[200];
    getcwd(server_abspath, 200);

    m_root = (char *) malloc(strlen(server_abspath) + strlen(resourceFolder) + 1);
    strcpy(m_root, server_abspath);
    strcat(m_root, resourceFolder);


    /** store useful info */
    Node *p_m_root = new StringNode(m_root);
    Node *p_resourceFolder = new StringNode(resourceFolder);
    Node *p_server_abspath = new StringNode(server_abspath);

    configs.put("template_root", p_m_root);
    configs.put("template_relative_path", p_resourceFolder);
    configs.put("app_root", p_server_abspath);


    __show_configs();
}

yumira::WebServer::~WebServer() {
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    delete[] httpConnForUsers;
    delete[] users_timer;
    delete http_conn_pool;
    for (auto &ele: configs) {
        delete ele.second;
    }
}

void yumira::WebServer::init(int port, string user,
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
    SERVER_ACTOR_MODE = actor_model;


}

ConfigurePtr yumira::WebServer::bindConf(Configure &conf) {
    // return binded conf poniter
    return this->configObj = &conf;
}

void yumira::WebServer::loadFromConf(Configure &conf) {
    this->parseFromConf(*bindConf(conf));
}

void yumira::WebServer::setTrigMode() {
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

void yumira::WebServer::log_write() {
    if (M_ENABLED_LOG == m_close_log) {
        //初始化日志
        if (1 == m_log_write)
            Logger::getInstance()->init(severLogPath, m_close_log, 2000, 800000, 800);
        else
            Logger::getInstance()->init(severLogPath, m_close_log, 2000, 800000, 0);
    }
}

void yumira::WebServer::sql_pool() {
    //初始化数据库连接池

    m_connPool = connection_pool::builder()
            .url(M_DEFAULT_URL)
            .user(m_user)
            .passWord(m_passWord)
            .dataBaseName(m_databaseName)
            .port(M_DEFAULT_PORT)
            .maxConn(m_sql_num)
            .close_log(m_close_log).build();

    //初始化数据库读取表
    httpConnForUsers->initmysql_result(m_connPool);
}

void yumira::WebServer::createThreadPool() {
    //线程池
    http_conn_pool = new threadPool<http_conn>(m_thread_num);
    auto scheduler = new ThreadPoolTaskDefaultScheduler<http_conn>(http_conn_pool, SERVER_ACTOR_MODE, m_connPool);

}

void yumira::WebServer::registerEventListen() {
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
        throw NetworkExpcetion("bind() failed!");
    }
    if ((ret = listen(m_listenfd, 5)) < 0) {
        throw NetworkExpcetion("listen() failed!");
    }

    utils.init(TIMESLOT);

    //epoll创建内核事件表
//    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.addfd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
    http_conn::m_epollfd = m_epollfd;

    /** create a pair of socket,store in m_pipefd */
    assert((ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd)) != -1);
    utils.setNonBlocking(m_pipefd[1]);
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);

    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

void yumira::WebServer::createTimerForUser(int connfd, struct sockaddr_in client_address) {
    /** create work thread here
     * */
    httpConnForUsers[connfd].init(connfd, client_address, m_root, m_CONNTrigmode, m_close_log, m_user, m_passWord,
                                  m_databaseName);

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中

    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;

    auto *userTimer = new util_timer;
    userTimer->user_data = &users_timer[connfd];
    userTimer->cb_func = callback_func;

    time_t cur = time(nullptr);
    userTimer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = userTimer;
    utils.m_timer_lst.add_timer(userTimer);
}

//若有数据传输，则将定时器往后延迟3个单位
//并对新的定时器在链表上的位置进行调整
void yumira::WebServer::adjust_timer(util_timer *timer) {
    time_t current_time = time(nullptr);
    timer->expire = current_time + 3 * TIMESLOT;
    utils.m_timer_lst.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

void yumira::WebServer::deal_timer(util_timer *timer, int sock_fd) {
    if (timer) {
        timer->cb_func(&users_timer[sock_fd]);
        utils.m_timer_lst.del_timer(timer);
        LOG_INFO("close fd %d", users_timer[sock_fd].sockfd);
    }
}

bool yumira::WebServer::dealclinetdata() {
    /**
     *   this function deal with new connection for user,accoding to mode set in ListenTriggerMode,
     *   will be little different when deal new connection
     *
     *   @attention ERROR: when now more available fd for use.(not exceed than MAX_FD set in WebServer.h )
     *    */
    auto acceptAndCreateTimer = [this]() {
        struct sockaddr_in client_address;

        socklen_t len_client_addr = sizeof(client_address);
        int connfd = accept(m_listenfd, (struct sockaddr *) &client_address, &len_client_addr);
        if (connfd < 0) {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (http_conn::activeConnectionSize() >= MAX_FD) {
            utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy.");
            return false;
        }
        createTimerForUser(connfd, client_address);
        return true;
    };

    if (0 == m_LISTENTrigmode) {
        return acceptAndCreateTimer();
    } else {
        while (!acceptAndCreateTimer()) {}
    }
    return true;
}

bool yumira::WebServer::deal_sys_signal(bool &timeout, bool &stopServer) {
    ssize_t ret = 0;
    int sig;
    char buf_signals[1024];
    ret = recv(m_pipefd[0], buf_signals, sizeof(buf_signals), 0);
    if (ret == -1) {
        return false;
    } else if (ret == 0) {
        return false;
    } else {
        for (int i = 0; i < ret; ++i) {
            switch (buf_signals[i]) {
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

void yumira::WebServer::deal_with_read(int sockfd) {
    /** do EPOLL_IN event here */
    util_timer *timer = users_timer[sockfd].timer;

    //reactor
    if (SERVER_REACTOR_MODE == SERVER_ACTOR_MODE) {
        if (timer) {
            adjust_timer(timer);
        }

        //若监测到读事件，将该事件放入请求队列
        http_conn_pool->append(httpConnForUsers + sockfd, 0);

        while (true) {
            if (1 == httpConnForUsers[sockfd].improv) {
                if (1 == httpConnForUsers[sockfd].timer_flag) {
                    deal_timer(timer, sockfd);
                    httpConnForUsers[sockfd].timer_flag = 0;
                }
                httpConnForUsers[sockfd].improv = 0;
                break;
            }
        }
    } else {
        //proactor
        if (httpConnForUsers[sockfd].read_once()) { // let main thread read finished, then submit to work thread do logical work
            LOG_INFO("deal with the client(%s)", inet_ntoa(httpConnForUsers[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            http_conn_pool->append(httpConnForUsers + sockfd);

            if (timer) {
                adjust_timer(timer);
            }
        } else {
            deal_timer(timer, sockfd);
        }
    }
}

void yumira::WebServer::deal_with_write(int sockfd) {
    util_timer *timer = users_timer[sockfd].timer;
    //reactor
    if (SERVER_REACTOR_MODE == SERVER_ACTOR_MODE) {
        if (timer) {
            adjust_timer(timer);
        }
        // reactor means worker thread actually do the request, so put thre quest to threadPool workQueue
        http_conn_pool->append(httpConnForUsers + sockfd, 1);

        while (true) {
            if (1 == httpConnForUsers[sockfd].improv) {
                if (1 == httpConnForUsers[sockfd].timer_flag) {
                    deal_timer(timer, sockfd);
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
            deal_timer(timer, sockfd);
        }
    }
}

void yumira::WebServer::eventLoop() {
    /** the main thread only handler new request event or signal
     * actuall process is in worker thread of threadPool has been create before. */
    // start de with event
    bool timeout = false;
    stop_server = false;

    fprintf(stdout, "\n[INFO] server successfully running...\n");

    while (!stop_server) {
        int n_fd;
        // main thread need call epoll_wait to wait for socket which has data to read
        if ((n_fd = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1)) < 0
            && errno != EINTR) {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        // detect read events, handle the events to worker threads
        for (int i = 0; i < n_fd; ++i) {
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_listenfd) {
                // allocate new connection for user if there are some free http connections.
                bool flag = dealclinetdata();
                if (!flag)
                    continue;
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //服务器端关闭连接，移除对应的定时器
                deal_timer(users_timer[sockfd].timer, sockfd);
            } else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN)) {
                //处理信号

                if (!deal_sys_signal(timeout, reinterpret_cast<bool &>(stop_server)))
                    LOG_ERROR("%s", "deal client data failure");
            } else if (events[i].events & EPOLLIN) {
                // a read event has been arrived. let worker thread handle it
                //处理客户连接上接收到的数据
                deal_with_read(sockfd);
            } else if (events[i].events & EPOLLOUT) {
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

void
yumira::WebServer::parseFromConf(Configure &conf) {
    this->init(atoi(conf.getProp("port").c_str()),
               conf.getProp("db_user"),
               conf.getProp("db_passwd"),
               conf.getProp("db_name"),
               atoi(conf.getProp("logWrite").c_str()),
               atoi(conf.getProp("lingerOption").c_str()),
               atoi(conf.getProp("triggerMode").c_str()),
               atoi(conf.getProp("sqlConPoolSize").c_str()),
               atoi(conf.getProp("httpConnPoolSize").c_str()),
               atoi(conf.getProp("disableLogger").c_str()),
               atoi(conf.getProp("concurrentActor").c_str()));
}

