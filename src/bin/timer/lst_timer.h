#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include <functional>

class util_timer;

struct st_client_data {
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};


typedef st_client_data client_data_t;

struct util_timer {
    time_t expire;

    util_timer() : prev(nullptr), next(nullptr),removeCallback(nullptr),closeCallBack(nullptr) {}

    std::function<void(st_client_data*)> closeCallBack;

    st_client_data *user_data;
    util_timer *prev;
    util_timer *next;
    std::function<void()> removeCallback;
};

class sort_timer_lst {
public:
    sort_timer_lst();

    ~sort_timer_lst();

    void add_timer(util_timer *timer);

    void adjust_timer(util_timer *timer);

    void remove(util_timer *timer);

    void tick();

private:
    void add_timer(util_timer *timer, util_timer *lst_head);

    util_timer *head;
    util_timer *tail;
};

class Utils {
public:
    Utils() {};

    ~Utils() = default;

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setNonBlocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void regist_fd(const int& epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;
};

void close_user_fd(client_data_t *user_data);

#endif