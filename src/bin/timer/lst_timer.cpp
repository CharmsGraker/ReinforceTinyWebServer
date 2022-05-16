#include "lst_timer.h"
#include "../http/http_conn.h"

sort_timer_lst::sort_timer_lst() {
    head = nullptr;
    tail = nullptr;
}

sort_timer_lst::~sort_timer_lst() {
    util_timer *tmp = head;
    while (tmp) {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_lst::add_timer(util_timer *timer) {
    if (!timer) {
        return;
    }
    if (!head) {
        head = tail = timer;
        return;
    }
    if (timer->expire < head->expire) {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    add_timer(timer, head);
}

void sort_timer_lst::adjust_timer(util_timer *timer) {
    if (!timer) {
        return;
    }
    util_timer *tmp = timer->next;
    if (!tmp || (timer->expire < tmp->expire)) {
        return;
    }
    if (timer == head) {
        head = head->next;
        head->prev = nullptr;
        timer->next = nullptr;
        add_timer(timer, head);
    } else {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}

void sort_timer_lst::remove(util_timer *timer) {
    if (!timer) {
        return;
    }
    if ((timer == head) && (timer == tail)) {
        delete timer;
        head = nullptr;
        tail = nullptr;
        return;
    }
    if (timer == head) {
        head = head->next;
        head->prev = nullptr;
        delete timer;
        return;
    }
    if (timer == tail) {
        tail = tail->prev;
        tail->next = nullptr;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

void sort_timer_lst::tick() {
    // timer will be sort, so when we find a node not expired, then stop
    if (!head) {
        return;
    }

    time_t current_time = time(nullptr);
    util_timer *cur_timer = head;
    while (cur_timer) {
        if (current_time < cur_timer->expire) {
            break;
        }
        // callback and remove, reset the head to cur->next
        cur_timer->closeCallBack(cur_timer->user_data);
        if (cur_timer->next) {
            head = cur_timer->next;
            head->prev = nullptr;
        }
        delete cur_timer;
        cur_timer = head;
    }
}

void sort_timer_lst::add_timer(util_timer *timer, util_timer *lst_head) {
    util_timer *prev = lst_head;
    util_timer *tmp = prev->next;
    while (tmp) {
        if (timer->expire < tmp->expire) {
            prev->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if (!tmp) {
        prev->next = timer;
        timer->prev = prev;
        timer->next = nullptr;
        tail = timer;
    }
}

void Utils::init(int timeslot) {
    m_TIMESLOT = timeslot;
}

//对文件描述符设置非阻塞
int Utils::setNonBlocking(int fd) {
    /**
     * @return old fd option
     * */
    int old_option = fcntl(fd, F_GETFL);
    // only valid for device or net file
    int new_option = old_option | O_NONBLOCK;
    // F_SETFL: Set file status flags
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::regist_fd(const int& epoll_fd, int fd, bool one_shot, int TRIGMode) {
    if(epoll_fd < 0 || fd < 0)
        throw std::exception();
    epoll_event event{};
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

//信号处理函数
void Utils::sig_handler(int signal) {
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = signal;
    // send to pipe, will receive msg at contrary pipe
    send(u_pipefd[1], (char *) &msg, 1, 0);
    errno = save_errno;
}

//设置信号函数
void Utils::addsig(int sig, void(sig_handler)(int), bool restart) {
    /**
     * register handler for sig provide in param "sig"
     * @param sig:
     * @param handler: which function want be invoke when sig arrive
     * */

    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::timer_handler() {
    m_timer_lst.tick();
    alarm(m_TIMESLOT);
}

void Utils::show_error(int conn_fd, const char *infoMsg) {
    send(conn_fd, infoMsg, strlen(infoMsg), 0);
    close(conn_fd);
}

int *Utils::u_pipefd = nullptr;
int Utils::u_epollfd = 0;

class Utils;