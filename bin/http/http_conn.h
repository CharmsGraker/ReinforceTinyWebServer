#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

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
#include <map>
#include <vector>



#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"
#include "../netroute/http_request_enum.h"
#include "../netroute/router.h"
#include "../netroute/blueprint.h"
#include "../http/http_connect_adapter.h"


class http_conn {
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    typedef http_conn thisConnectionType;

    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };


public:
    http_conn() {}

    ~http_conn();

public:
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);

    void close_conn(bool real_close = true);

    void process();

    bool read_once();

    bool write();

    sockaddr_in *get_address() {
        return &m_address;
    }

    void initmysql_result(connection_pool *connPool);

    int timer_flag;
    int improv;

    static void register_interceptor(const Blueprint<Router> &bp) {
        get_interceptors()->push_back((Blueprint<Router> *) &bp);
    }

    static void register_interceptor(Router *routePtr) {
        // user want straight register route,so construct a blueprint
        auto *new_bp = new Blueprint<Router>(routePtr->getBaseName().c_str());

        new_bp->registRoute(routePtr);
//    printf("new bp name: %s", new_bp->get_bp_name().c_str());
//    printf("%d\n", new_bp);
        register_interceptor(new_bp);
    }

    static void register_interceptor(Blueprint<Router> *bpPtr) {
        get_interceptors()->push_back(bpPtr);
    }

    typedef void(*Code)(Blueprint<Router> *new_bp);

    static void register_interceptor(Blueprint<Router> *bpPtr, Code your_code_here) {
        try {
            your_code_here(bpPtr);
        } catch (exception &e) {
            fprintf(stderr, "[FATAL] code block occurred mistake!");
            e.what();
            delete bpPtr;
        }
        get_interceptors()->push_back(bpPtr);
    }

    static
    vector<Blueprint<Router> *> *get_interceptors() {
        static vector<Blueprint<Router> *> interceptors; // all instance must hold same interceptor
        return &interceptors;
    };

    void set_href_url(const char *html_path);

    void set_href_url(const string html_path);

    const char *
    url_for(http_conn *conn, string routeName) {
        if (routeName.find('.') == routeName.npos) {

            return routeName.c_str();
        }
    }

    void
    _enable_remake_request() {
        remakeRequest = true;
    }

    void
    _clean_request_flag() {
        remakeRequest = false;
    }


    const char *redirect(string route_to_send, http_req_method_t method) {
        // set state for href
        return m_method = method,
                m_url = const_cast<char *>(route_to_send.c_str());
    }

    MYSQL const*
    get_mysql_query()const {
        return mysql;
    }


private:
    void init();

    HTTP_CODE process_read();

    bool process_write(HTTP_CODE ret);

    HTTP_CODE parse_request_line(char *text);

    HTTP_CODE parse_headers(char *text);

    HTTP_CODE parse_content(char *text);

    HTTP_CODE do_request();

    char *get_line() { return m_read_buf + m_start_line; };

    LINE_STATUS parse_line();

    void unmap();

    bool add_response(const char *format, ...);

    bool add_content(const char *content);

    bool add_status_line(int status, const char *title);

    bool add_headers(int content_length);

    bool add_content_type();

    bool add_content_length(int content_length);

    bool add_linger();

    bool add_blank_line();

    char *_getHtmlVersion(const char *m_url) {
        char *html_ver = (char *) (strpbrk(m_url, " \t"));
        if (!html_ver) return nullptr;
        *html_ver++ = '\0';
        html_ver += strspn(html_ver, " \t");
        printf("version: %s\n", html_ver);

        return html_ver;
    };

    bool
    _need_remake_request() const {
        return this->remakeRequest;
    }


public:
    static int m_epollfd;
    static int m_user_count;
    int m_state;  //读为0, 写为1
    friend class HttpConnectionAdapter<http_conn>;
    MYSQL *mysql; // real query store in http_conn

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx;
    int m_checked_idx;
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    CHECK_STATE m_check_state;
    http_req_method_t m_method;
    // the truely use path when open the file or res on server
    char *m_url;
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;
    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2];
    int m_iv_count;
    int cgi;        //是否启用的POST
    //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];


    char m_real_file[FILENAME_LEN];
    char *doc_root;
    char *m_string;
    bool remakeRequest = false; // reloop do_request method, but dont broke the http connection
    string relative_url_path;
    string HTTP_ROOT = "/";
};


#endif