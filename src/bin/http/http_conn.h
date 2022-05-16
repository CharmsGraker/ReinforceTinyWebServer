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
#include <utility>
#include <vector>
#include <json/json.h>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../netroute/http_request_enum.h"
#include "../netroute/router.h"
#include "../netroute/blueprint.h"
#include "../http/http_connect_adapter.h"
#include "../threadpool/task.h"
#include "../netroute/urlparser.h"
#include "environment.h"
#include "http_outstream/http_response_header.h"
#include "../debug/dprintf.h"

class HttpResponse;

class LoadBalancer;

using yumira::debug::DPrintf;


using namespace yumira;
namespace yumira {

    namespace server_config {
        extern int S_actorMode;
    }

    class http_conn : public Task {
    public:
        static const int FILENAME_LEN = 256;
        static const int READ_BUFFER_SIZE = 2048;
        static const int WRITE_BUFFER_SIZE = 1024;
        const char *INDEX_HTML_FILENAME = "judge.html";
        std::function<void(void)> complete_callback;

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
            RAW_REQUEST,
            INTERNAL_ERROR,
            PERMANENTLY_REDIRECT,
            CLOSED_CONNECTION
        };
        enum LINE_STATUS {
            LINE_OK = 0,
            LINE_BAD,
            LINE_OPEN
        };

        /** constructor and destructor */
    public:
        http_conn() {}

        ~http_conn();

        // builder
    public:
        class Builder {
        private:
            http_conn *outer;
        public:
            Builder(http_conn *out) {
                if (!out) {
                    throw NullException("http_conn is Null!");
                }
                outer = out;
            };

            Builder &sockFd(int _sockfd) {
                DPrintf("build sockFd\n");
                outer->m_sockfd = _sockfd;
                return *this;
            }

            Builder &address(const sockaddr_in &addr) {
                outer->m_address = addr;
                return *this;
            }

            //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
            Builder &docRoot(char *doc_root_) {
                outer->doc_root = doc_root_;
                return *this;
            }

            Builder &sqlDataBaseUser(const string &database_user) {
                strcpy(outer->sql_user, database_user.c_str());
                return *this;
            }

            Builder &sqlDataBasePassword(const string &database_passwd) {
                strcpy(outer->sql_passwd, database_passwd.c_str());
                return *this;
            }

            Builder &sqlDataBaseName(const string &DataBaseName) {
                outer->sql_database_name = std::move(DataBaseName);
//                printf("sql_database_name: %s\n", outer->sql_database_name.c_str());
                return *this;
            }

            Builder &triggerMode(const int &triggerMode) {
                outer->m_TRIGMode = triggerMode;
                return *this;
            }

            http_conn *build() {
                outer->init();
                return outer;
            };

        };

        Builder &
        builder() {
            DPrintf("enter into http builder\n");
            if (!builder_)
                builder_ = new Builder(this);
            return *builder_;

        }

    public:
        void
        init();

        static void
        setConnIp(const char *Ip);

        static void
        setConnPort(const int port);

        void close_conn(bool real_close = true);

        void process();

        bool read_once();

        bool write();

        void handleRead(int fd, std::function<void()> callback = nullptr);

        void handleWrite(int fd);

        sockaddr_in *get_address() {
            return &m_address;
        }

        void initmysql_result(connection_pool *connPool);

        int timer_flag;
        int improv;

        static void registerInterceptor(const Blueprint &bp) {
            getInterceptors().push_back((Blueprint *) &bp);
        }

        static void
        decrease_active_user(int cnt) {
            m_user_count -= cnt;
        }

        static int
        activeConnectionSize() {
            return m_user_count;
        }

        void
        operator()(connection_pool *sql_conn_pool) {
            m_state = 0;
//            if (1 == yumira::server_config::S_actorMode) {
//                switch (m_state) {
//                    case 0: {
//                        if (read_once()) {
//                            connectionRAII mysqlcon(&mysql, sql_conn_pool);
//                            process();
//                            m_state = 1;
//                        } else {
//                            timer_flag = 1;
//                        }
//                        break;
//                    }
//                    case 1: {
//                        if (write()) {
//                            m_state = 0;
//                        } else {
//                            timer_flag = 1;
//                        }
//                        break;
//                    }
//                }
//                improv = 1;
//
//            } else {
//                connectionRAII mysqlConnect(&mysql, sql_conn_pool);
//                process();
//            }
            if (read_once()) {
                connectionRAII mysqlcon(&mysql, sql_conn_pool);
                process();
            } else {
                timer_flag = 1;
            }
            m_state = 1;

            DPrintf("[] done fd=%d ,task,%s\n",m_sockfd, __FILE__);

//            if (complete_callback) {
//                printf("try exec http_conn callback ,%s\n", __FILE__);
//                complete_callback();
////                complete_callback = nullptr;
//            } else {
//                printf("empty callback, %s\n",__FILE__);
//            }
        }

        static void registerInterceptor(Router *routePtr) {
            /**
             *  if user want to register Router as a Interceptor,
             *  then need create a Blueprint for it,and set Router as "/"
             *  for implement this, to set Route suffix
             * */

            auto *new_bp = new Blueprint(routePtr->getBaseName().c_str());

            // first ensure regist this Router
            new_bp->registerRoute(routePtr);

            // clear all route suffix
            new_bp->set_suffix("");
#ifdef DEBUG
            printf("new bp name: %s", new_bp->get_bp_name().c_str());
        printf("%d\n", new_bp);
        cout << "routePtr: " << routePtr->getBaseName() << endl;

#endif
            registerInterceptor(new_bp);
        }

        static void registerInterceptor(Blueprint *bpPtr) {
            getInterceptors().push_back(bpPtr);
        }

        typedef void(*Code)(Blueprint *new_bp);

        static void registerInterceptor(Blueprint *bpPtr, Code your_code_here) {
            try {
                your_code_here(bpPtr);
                getInterceptors().push_back(bpPtr);

            } catch (exception &e) {
                fprintf(stderr, "[FATAL] code block occurred mistake!");
                e.what();
                delete bpPtr;
            }
        }

        static
        vector<Blueprint *> &getInterceptors() {
            static vector<Blueprint *> interceptors; // all instance must hold same interceptor
            return interceptors;
        };

        void set_href_url(const char *html_path);

        void set_href_url(const string &html_path);

        url_t
        url_for(http_conn *conn, const std::string &routeName) {
            if (routeName.find('.') == routeName.npos) {

                return url_t(routeName);
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


        std::string
        redirect(std::string route_to_send, http_req_method_t method) {
            // set state for href
            if ((m_method = method) == POST) {
                cgi = 1;
            } else {
                cgi = 0;
            }
            m_string = nullptr;

            auto st_url = url_t(route_to_send);
            m_url = (char *) st_url.url.c_str();
            return m_url;
        }

        MYSQL const *
        get_mysql_query() const {
            return mysql;
        }


    private:
        void __init__();

        HTTP_CODE process_read();

        bool process_write(HTTP_CODE ret);

        HTTP_CODE parse_request_line(char *text);

        HTTP_CODE parse_headers(char *text);

        HTTP_CODE parse_content(char *text);

        HTTP_CODE do_request();

        char *get_line() { return m_read_buf + m_start_line; };

        LINE_STATUS parse_line();

        void unmap();

        // http response
        bool add_response(const char *format, ...);

        bool add_content(const char *content);

        bool add_status_line(int status, const char *title);

        template<int code>
        bool
        add_status_line(http_message::response::header<code> responseHeader);

        bool add_headers(int content_length);

        bool add_content_type();

        bool add_content_length(int content_length);

        bool add_linger();

        bool add_blank_line();

        bool add_location(char *uri);

        char *_getHttpProtocol(char *m_url) {
            char *http_ver = (char *) (strpbrk(m_url, " \t"));
            if (!http_ver) return nullptr;
            *http_ver++ = '\0';
            http_ver += strspn(http_ver, " \t");
//        printf("version: %s\n", http_ver);

            return http_ver;
        };

        bool
        _need_remake_request() const {
            return this->remakeRequest;
        }

        int
        __map_file_into_cache();

        //static method init
    private:
        static void initLoadBalancer();

    public:
        static int m_epollfd;
        int m_state;  //读为0, 写为1
        int m_sockfd;

        friend class HttpConnectionAdapter<http_conn>;

        MYSQL *mysql; // real query store in http_conn

    private:
        Builder *builder_;
        static int m_user_count; // active user connection
        HttpResponse *http_response;

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

        char sql_user[100];
        char sql_passwd[100];
        std::string sql_database_name;


        char m_real_file[FILENAME_LEN];
        char *doc_root;
        char *m_string;
        bool remakeRequest = false; // re loop do_request method, but dont broke the http connection
        string relative_url_path;

        Json::Value *m_json_obj;
        std::string m_content_type;
    };
}


#endif
