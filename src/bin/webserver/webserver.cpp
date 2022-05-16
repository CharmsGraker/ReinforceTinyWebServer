
#include "webserver.h"
#include "../extensions/loadbalancer/LoadBalancer.h"
#include "../extensions/heartbeats/GrakerHeartBeat.h"
#include "../extensions/heartbeats/cluster/Cluster.h"
#include "../debug/dprintf.h"
#include "../exceptions/exceptions.h"

using namespace yumira::dev;
namespace yumira {
    namespace server_config {
        int S_actorMode;
    }
}
using yumira::debug::DPrintf;

/** return remain available Fd */
int
GRAKER_SERVER(Http_handler)(void *) {
    return httpConfig.MAX_FD - http_conn::activeConnectionSize();
};


template<class HttpConn>
yumira::WebServer<HttpConn>::WebServer():f_canAcceptHttp(GRAKER_SERVER(Http_handler)),
                                         f_checkConnFd(nullptr),
                                         f_createIdleThread(GRAKER_SERVER(createIdle)),
                                         epollEvents(new epoll_event[commonConfig.MAX_EVENT_NUMBER]){
    auto len = httpConfig.MAX_FD;
    httpConnForUsers = new http_conn[len];
    users_timer = new client_data_t[len];

    for(int i=0;i<len;++i) {
        httpConnForUsers[i].complete_callback = [&httpConn = httpConnForUsers[i], &timer = users_timer[i].timer]() {
            DPrintf("callback called()\n");
            if (1 == httpConn.improv) {
                if (1 == httpConn.timer_flag) {
                    // callback
                    try {
                        if (timer) {
                            DPrintf("remove callback\n");
                            timer->removeCallback();
                        }
                    } catch (exception &e) {
                        e.what();
                        DPrintf("%s\n", __FILE__);
                    }
                    httpConn.timer_flag = 0;
                }
                httpConn.improv = 0;
            }
        };
    }
    //root文件夹路径
    std::string curFileAbsPath = std::string(__FILE__);
    auto idx = curFileAbsPath.rfind("/");
    auto pdir = curFileAbsPath.substr(0, idx + 1);
    auto idx2 = curFileAbsPath.rfind("/", idx - 1);
    auto ppdir = curFileAbsPath.substr(0, idx2 + 1);
    printf("resourceFolder=%s\n",templateConfig.resourceFolder.c_str());

    m_root = (char *) malloc(pdir.size() + templateConfig.resourceFolder.length());
    strcpy(m_root, pdir.c_str());
    strcat(m_root, templateConfig.resourceFolder.c_str());


    /** store useful info */
    Node *p_m_root = new StringNode(m_root);
    Node *p_resourceFolder = new StringNode(templateConfig.resourceFolder);
    Node *p_server_abspath = new StringNode(ppdir.substr(0, ppdir.length() - 1));

    server_configs.put("template_root", p_m_root);
    DPrintf("template root=%s\n", m_root);
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
    yumira::server_config::S_actorMode = actor_model;
    actorMode = actor_model;
    commonConfig.ServiceName = "GrakerWebServer";


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
    http_thread_pool = new thread_pool();
//    auto scheduler = new ThreadPoolTaskDefaultScheduler<HttpConn>(http_conn_pool, actorMode, m_connPool);

}



template<class HttpConn>
void yumira::WebServer<HttpConn>::registerEventListen() {
    //线程池
    createThreadPool();
    DPrintf("after createThreadPool\n");
    setTrigMode();
    commonConfig.serverIp = getLocalIP("localhost", 0);

    printf("serverHost: %s \n", commonConfig.serverIp.c_str());


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

    serverProviderMap[commonConfig.ServiceName] = commonConfig.serverIp + ":" + to_string(m_port);
    commonConfig.setConnIp(commonConfig.serverIp);
    commonConfig.setConnPort(m_port);

    utils.init(commonConfig.TIMESLOT);

    //epoll创建内核事件表
//    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    Utils::regist_fd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
    http_conn::m_epollfd = m_epollfd;

    /** create a pair of socket,store in m_pipefd */
    assert((ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd)) != -1);
    Utils::setNonBlocking(m_pipefd[1]);
    // m_pipefd[0] use for send signal to main thread
    Utils::regist_fd(m_epollfd, m_pipefd[0], false, 0);

    Utils::addsig(SIGPIPE, SIG_IGN);
    Utils::addsig(SIGALRM, utils.sig_handler, false);
    Utils::addsig(SIGTERM, utils.sig_handler, false);

    alarm(commonConfig.TIMESLOT);

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
            .build();

    DPrintf("createTimerForUser fd=%d, %s\n",connfd, __FILE__);

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中

    auto &&userTimer = new util_timer;
    userTimer->user_data = &users_timer[connfd];
    userTimer->closeCallBack = close_user_fd;

    time_t cur = time(nullptr);
    userTimer->expire = cur + 3 * commonConfig.TIMESLOT;

    users_timer[connfd].timer = userTimer;
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;

    utils.m_timer_lst.add_timer(userTimer);
    DPrintf("userTimer %s\n", __FILE__);

    userTimer->removeCallback = [this, userTimer = users_timer[connfd].timer, connfd] {
        return yumira::remove_timer(users_timer, utils, userTimer, connfd);
    };
    DPrintf("createTimerForUser done %s\n", __FILE__);

}

//若有数据传输，则将定时器往后延迟3个单位
//并对新的定时器在链表上的位置进行调整
template<class HttpConn>
void
yumira::WebServer<HttpConn>::adjust_timer(util_timer *timer) {
    time_t current_time = time(nullptr);
    timer->expire = current_time + 3 * commonConfig.TIMESLOT;
    utils.m_timer_lst.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

template<class HttpConn>
int
yumira::WebServer<HttpConn>::deal_client_data() {
    /**
     *   this function deal with new connection for user,according to mode set in ListenTriggerMode,
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
            DPrintf("accept error\n");
            return commonConfig.M_READERROR;
        }
        ret = 0;
        if (f_checkConnFd && (ret = f_checkConnFd(conn_fd)) == 0) {
            // ignore other datagram on this socket
            // forward to Idle Thread
            //TODO how to do task in main thread?

            return commonConfig.M_IDLEDATA;
        }
        if (ret < 0) {
            DPrintf("error when checkConnFd()\n");
            std::abort();
        }
        int n_remain = 0;
        assert(f_canAcceptHttp);
        if ((n_remain = f_canAcceptHttp((void *) &client_address)) <= 0) {
            utils.show_error(conn_fd, "Internal server busy");
            DPrintf("Internal server busy\n");

            LOG_ERROR("%s", "Internal server busy.");
            return commonConfig.M_SERVERBUSY;
        }
        // allocate conn_fd to worker
        createTimerForUser(conn_fd, client_address);
        return commonConfig.M_SUCCESS;
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
//        printf("REACTOR_MODE read\n");
        if (timer) {
            adjust_timer(timer);
        }
        LOG_INFO("append read event from sock fd: %d to Http Connection Pool:%d", sockfd, http_thread_pool);
        //若监测到读事件，将该事件放入请求队列
        // 如使用std::bind，记得std::ref
        httpConnForUsers[sockfd].m_sockfd = sockfd;
        http_thread_pool->submit([&conn=httpConnForUsers[sockfd], m_connPool = m_connPool]() {
            conn(m_connPool);
        });
    } else if (PROACTOR_MODE == actorMode) {
        printf("Proactor read\n");
        //proactor
        if (httpConnForUsers[sockfd].read_once()) {
            // let main thread read finished, then submit to work thread do logical work
            LOG_INFO("deal with client(%s)", inet_ntoa(httpConnForUsers[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            http_thread_pool->submit([&, m_connPool = m_connPool]() {
                (httpConnForUsers + sockfd)->operator()(m_connPool);
            });

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
    if (httpConnForUsers[sockfd].write()) {
        LOG_INFO("send data to the client(%s)", inet_ntoa(httpConnForUsers[sockfd].get_address()->sin_addr));
        if (timer) {
            adjust_timer(timer);
        }
    } else {
        timer->removeCallback();
    }
}

template<class HttpConn>
void
yumira::WebServer<HttpConn>::eventLoop() {
    /** the main thread only handler new request event or signal
     * actually process is in worker thread of threadPool has been create before. */
    // start de with event
    stop_server = false;

    // pass current server unique id to idle-thread
    int n_param = 3;
    const char *args_[n_param + 1];
    auto serverIpStr = InetAddress::Serialize({commonConfig.serverIp, m_port});
    auto clusterIpStr = InetAddress::Serialize({idleConfig.REDIS_HOST, idleConfig.REDIS_PORT});
    args_[0] = (char *) &n_param;
    args_[1] = serverIpStr.c_str();
    args_[2] = clusterIpStr.c_str();
    args_[3] = idleConfig.REDIS_CLUSTER_KEY.c_str();

    std::cerr<<"[INFO] server start running...\n";

    while (!stop_server) {

        for(auto&& cb_ier=loop_callbacks.begin();cb_ier != loop_callbacks.end();) {
            try {
                (*cb_ier)();
                ++cb_ier;
            } catch (RedisConnectFailException& e) {
                std::cerr<<"catch exceptions\n";
                e.what();
                cb_ier = loop_callbacks.erase(cb_ier);
            }
        }

        int n_fd;
        // main thread need call epoll_wait to wait for socket which has data to read
        if ((n_fd = epoll_wait(m_epollfd, epollEvents, commonConfig.MAX_EVENT_NUMBER, -1)) < 0
            && errno != EINTR) {
            LOG_ERROR("%s", "epoll failure");
            break;
        }
        bool timeout = false;

        for (int i = 0; i < n_fd; ++i) {
            int sockfd = epollEvents[i].data.fd;
            if (sockfd == m_listenfd) {
                // allocate new connection for user if there are some free http connections.
                auto flag = deal_client_data();
                if (flag < 0) {
                    cout << "flag fail" << endl;
                    continue;
                } else if (flag == commonConfig.M_IDLEDATA) {
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
                deal_with_read(sockfd);
            } else if (epollEvents[i].events & EPOLLOUT) {
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