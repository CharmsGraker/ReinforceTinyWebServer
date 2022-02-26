#include "http_conn.h"

#include <mysql/mysql.h>
#include <fstream>
#include "../../config.h"

#include "../netroute/request.h"

//定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";


namespace yumira {
    Locker m_lock;
    std::map<std::string, std::string> userTable;
    Environment *environment = nullptr;
    thread_local Request *request = nullptr;
}

static struct httpHeaderEnum {
    const char *CONNECTION = "Connection:";
    const char *KEEP_ALIVE = "keep-alive";
    const char *CONTENT_LENGTH = "Content-length:";
    const char *HOST = "Host:";
} HTTP_HEADER;

static unsigned long __header_offset = 0;


auto is_route_request = [](const string &url) {
    return 1;
};


void yumira::http_conn::initmysql_result(connection_pool *connPool) {
    //先从连接池中取一个连接
    MYSQL *mysql = nullptr;
    connectionRAII mysqlcon(&mysql, connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user")) {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result)) {
        string temp1(row[0]);
        string temp2(row[1]);
        userTable[temp1] = temp2;
    }
}

//对文件描述符设置非阻塞
int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode) {
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//从内核时间表删除描述符
void removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode) {
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int yumira::http_conn::m_user_count = 0;
int yumira::http_conn::m_epollfd = -1;

//关闭连接，关闭一个连接，客户总量减一
void yumira::http_conn::close_conn(bool real_close) {
    if (real_close && (m_sockfd != -1)) {
        printf("close sockfd %d\n", m_sockfd);
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
        Request::release(request);
    }
}

//初始化连接,外部调用初始化套接字地址
void yumira::http_conn::init(int sockfd, const sockaddr_in &addr, char *root, int triggerMode,
                             string user, string passwd, string sqlname) {
    m_sockfd = sockfd;
    m_address = addr;

    addfd(m_epollfd, sockfd, true, m_TRIGMode);
    ++m_user_count;

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    doc_root = root;
    m_TRIGMode = triggerMode;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();
}

//初始化新接受的连接
//check_state默认为分析请求行状态
void
yumira::http_conn::init() {
    mysql = nullptr;
    bytes_to_send = 0;
    bytes_have_send = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = nullptr;
    m_version = nullptr;
    m_content_length = 0;
    m_host = nullptr;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    m_state = 0;
    timer_flag = 0;
    improv = 0;

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

//从状态机，用于分析出一行内容
//返回值为行的读取状态，有LINE_OK,LINE_BAD,LINE_OPEN
yumira::http_conn::LINE_STATUS
yumira::http_conn::parse_line() {
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx) {
        temp = m_read_buf[m_checked_idx];
        if (temp == '\r') {
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            else if (m_read_buf[m_checked_idx + 1] == '\n') {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if (temp == '\n') {
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r') {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

//循环读取客户数据，直到无数据可读或对方关闭连接
//非阻塞ET工作模式下，需要一次性将数据读完
bool
yumira::http_conn::read_once() {
    if (m_read_idx >= READ_BUFFER_SIZE) {
        return false;
    }
    int bytes_read = 0;

    //LT读取数据
    if (0 == m_TRIGMode) {
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;

        if (bytes_read <= 0) {
            return false;
        }

        return true;
    }
        //ET读数据
    else {
        while (true) {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            } else if (bytes_read == 0) {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}

//解析http请求行，获得请求方法，目标url及http版本号
yumira::http_conn::HTTP_CODE
yumira::http_conn::parse_request_line(char *text) {
    m_url = strpbrk(text, " \t");
    if (!m_url) {
        return BAD_REQUEST;
    }
    *m_url++ = '\0';
    // set to head
    printf("%s\n", m_url);

    char *method = text;
    if (strcasecmp(method, "GET") == 0) {
        printf("GET request arrived\n");
        m_method = GET;
    } else if (strcasecmp(method, "POST") == 0) {
        printf("POST request arrived\n");
        m_method = POST;
        cgi = 1;
    } else
        return BAD_REQUEST;
    m_url += strspn(m_url, " \t");


    if (!(m_version = _getHtmlVersion(m_url))) {
        return BAD_REQUEST;
    }

    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return BAD_REQUEST;

//
//    if (strncasecmp(m_url, "http://", 7) == 0) {
//        m_url += 7;
//        m_url = strchr(m_url, '/');
//    }


//    if (strncasecmp(m_url, "https://", 8) == 0) {
//        m_url += 8;
//        m_url = strchr(m_url, '/');
//    }

//    if (!m_url || m_url[0] != '/')
//        return BAD_REQUEST;

    // register index html here
    //当url为/时，显示判断界面
    if (strlen(m_url) == 1)
        strcat(m_url, INDEX_HTML_FILENAME);
    m_check_state = CHECK_STATE_HEADER;
//    printf("raw_url: %s, m_string=%s\n", m_url, m_string);

    return NO_REQUEST;
}


//解析http请求的一个头部信息
yumira::http_conn::HTTP_CODE
yumira::http_conn::parse_headers(char *text) {
    // has new http request can reach this method,log this will look up http exchange so many message
//    printf("\t%s\n", text);

    if (text[0] == '\0') {
        if (m_content_length != 0) {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    } else if (strncasecmp(text, HTTP_HEADER.CONNECTION,
                           __header_offset = strlen(HTTP_HEADER.CONNECTION)) == 0) {
        text += __header_offset;
        text += strspn(text, " \t");
        if (strcasecmp(text, HTTP_HEADER.KEEP_ALIVE) == 0) {
            m_linger = true;
        }
    } else if (strncasecmp(text, HTTP_HEADER.CONTENT_LENGTH,
                           __header_offset = strlen(HTTP_HEADER.CONTENT_LENGTH)) == 0) {
        text += __header_offset;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    } else if (strncasecmp(text, HTTP_HEADER.HOST,
                           __header_offset = strlen(HTTP_HEADER.HOST)) == 0) {
        text += __header_offset;
        text += strspn(text, " \t");
        m_host = text;
    } else {
        LOG_INFO("ops!unknown header: %s", text);
    }
    return NO_REQUEST;
}

//判断http请求是否被完整读入
yumira::http_conn::HTTP_CODE
yumira::http_conn::parse_content(char *text) {
    if (m_read_idx >= (m_content_length + m_checked_idx)) {
        text[m_content_length] = '\0';

        //POST请求中最后为输入的用户名和密码
        m_string = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

yumira::http_conn::HTTP_CODE
yumira::http_conn::process_read() {
    // real do process http connection

    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = nullptr;

    while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) ||
           ((line_status = parse_line()) == LINE_OK)) {
        text = get_line();
        m_start_line = m_checked_idx;
        LOG_INFO("%s", text);
        switch (m_check_state) {
            case CHECK_STATE_REQUESTLINE: {
                ret = parse_request_line(text);
                printf("out of request line, m_url=%s\n", m_url);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;

                break;
            }
            case CHECK_STATE_HEADER: {
                // the m_url will be set at this case
                ret = parse_headers(text);
                printf("out of header parse, m_url=%s\n", m_url);

                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret == GET_REQUEST) {
                    return do_request();
                }

                break;
            }
            case CHECK_STATE_CONTENT: {
                // m_url has been set when header sent before
                ret = parse_content(text);
                if (ret == GET_REQUEST)
                    return do_request();
                line_status = LINE_OPEN;
                break;
            }
            default:
                return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}


yumira::http_conn::HTTP_CODE
yumira::http_conn::do_request() {
    auto to_fill_route = [](string &url) {
        if (url.find('?') == url.npos) url += '?';
        return url;
    };

    auto resetRealFile = [this](const char *path, size_t size) {
        strncpy(m_real_file, path, size);
    };

    while (true) {
        _clean_request_flag();

        url_t st_url_real; // construct Url
        {
            string url;
            if (m_url) {
                url = m_url;
            }
            if (cgi == 1) {
                string m_queryInfo;
                if (m_string) {
                    m_queryInfo = m_string;
                }
//            printf("m_url: %s,m_string: %s", m_url, m_string);
                // check null, string constructor not support null
                relative_url_path = to_fill_route(url) + m_queryInfo;

            } else {
                relative_url_path = string(url);

            }
        }
//        printf("relative_url: %s\n", relative_url_path.c_str());
        request = Request::makeRequest(request, relative_url_path, m_method, this);

//        printf("[INFO] enter to do_request(), m_url=%s\n", request.route().c_str());

        /*
         * this function deal with http resource get request.
         * usually, first ask for *.html file,
         * then get the elements in such *.html file mentioned */

        retry:
        strcpy(m_real_file, doc_root);

        int len_root_path = strlen(doc_root);

//        printf("m_string: %s, cgi= %d\n", m_string, cgi);
//        printf("m_url: %s\n", m_url);

        try {
            if (!request->isResRequest()) {
                if (_need_remake_request()) continue;

                Router *route_handler;

                // make sure to correct bp name concat for routers
                // traverse blueprint(s) to handle this request

                for (auto &bp: getInterceptors()) {
                    bp->set_prefix();
                    //printf("loop bp:%s\n", bp->get_bp_name().c_str());
                    //printf("%d\n", bp);
                    if ((route_handler = bp->canDealWith(request)) != nullptr) {
                        // the request actual method type won't be check here. it let user to determine that.
                        URL_STATUS ret;
                        ThreadLocal::put<Request>("request", request);

                        if ((ret = route_handler->view(environment)) == URL_STATUS::VIEW_NOT_FOUND) {
                            // do with retry
                            //goto retry;
                        } else if (ret & URL_STATUS::RESOURCE_NOT_FOUND) {

                        }

                        // check status here, so this is a recurrent point
                        if (request->isResRequest()) {
                            // ask for file like html
                            // concat full path pointer to file
                            const char *real_url = request->getParsedUrl().getUrl().c_str();
                            strncpy(m_real_file + len_root_path, real_url, strlen(real_url));

                            // successfully deal request
                            _clean_request_flag();
                        } else {
                            // still a route visit or just redirect, need to remake request
                            _enable_remake_request();
                        }
                        break; /* go pass to state check... */
                    }
                }
                if (_need_remake_request()) {
//                    printf("[INFO] remake request...\n");
                    continue;
                }

            } else /* request for file or element doc */
                strncpy(m_real_file + len_root_path, m_url, FILENAME_LEN - len_root_path - 1);

        } catch (NullException &nullException) {
            nullException.what();
        } catch (exception &e) {
            fprintf(stderr, "[FATAL] do_request() error!\n");
            Request::release(request);
            throw exception();
        }
        // set href file here
        if (request->getParsedUrl().use_template) {
            resetRealFile(request->getParsedUrl().tpl_addr.c_str(), request->getParsedUrl().tpl_addr.size());
        }

        auto noSuchFile = [this]() { return stat(m_real_file, &m_file_stat) < 0; };
        auto noPermissionToAccessFile = [this]() { return !(m_file_stat.st_mode & S_IROTH); };
        auto is_folder = [this]() { return S_ISDIR(m_file_stat.st_mode); };

        // init file here
        if (noSuchFile())
            return NO_RESOURCE;

        if (noPermissionToAccessFile())
            return FORBIDDEN_REQUEST;

        if (is_folder())
            return BAD_REQUEST;

        __map_file_into_cache();
        return FILE_REQUEST;
    }
}

int
yumira::http_conn::__map_file_into_cache() {
    /**
     *  do open file description
     * */
    int fd = open(m_real_file, O_RDONLY);
    if (errno != 0) {
        fprintf(stderr, "error when open %s\n", strerror(errno));
    }
//    printf("[INFO] read file from storage: %s\n", m_real_file);

    // projection file to memory cache
    m_file_address = (char *) mmap(nullptr, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd); // avoid waste
}

void
yumira::http_conn::unmap() {
    if (m_file_address) {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = nullptr;
    }
}

bool
yumira::http_conn::write() {
    int temp = 0;

    if (bytes_to_send == 0) {
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
        init();
        return true;
    }

    while (1) {
        temp = writev(m_sockfd, m_iv, m_iv_count);

        if (temp < 0) {
            if (errno == EAGAIN) {
                modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += temp;
        bytes_to_send -= temp;
        if (bytes_have_send >= m_iv[0].iov_len) {
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        } else {
            m_iv[0].iov_base = m_write_buf + bytes_have_send;
            m_iv[0].iov_len -= bytes_have_send;
        }

        if (bytes_to_send <= 0) {
            unmap();
            modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);

            if (m_linger) {
                init();
                return true;
            } else {
                return false;
            }
        }
    }
}

bool
yumira::http_conn::add_response(const char *format, ...) {
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx)) {
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);

    LOG_INFO("request:%s", m_write_buf);

    return true;
}

bool
yumira::http_conn::add_status_line(int status, const char *title) {
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool
yumira::http_conn::add_headers(int content_len) {
    return add_content_length(content_len) && add_linger() &&
           add_blank_line();
}

bool
yumira::http_conn::add_content_length(int content_len) {
    return add_response("Content-Length:%d\r\n", content_len);
}

bool
yumira::http_conn::add_content_type() {
    return add_response("Content-Type:%s\r\n", "text/html");
}

bool
yumira::http_conn::add_linger() {
    return add_response("Connection:%s\r\n", m_linger ? "keep-alive" : "close");
}

bool
yumira::http_conn::add_blank_line() {
    return add_response("%s", "\r\n");
}

bool
yumira::http_conn::add_content(const char *content) {
    return add_response("%s", content);
}

bool
yumira::http_conn::process_write(yumira::http_conn::HTTP_CODE ret) {
    switch (ret) {
        case INTERNAL_ERROR: {
            add_status_line(500, error_500_title);
            add_headers(strlen(error_500_form));
            if (!add_content(error_500_form))
                return false;
            break;
        }
        case BAD_REQUEST: {
            add_status_line(404, error_404_title);
            add_headers(strlen(error_404_form));
            if (!add_content(error_404_form))
                return false;
            break;
        }
        case FORBIDDEN_REQUEST: {
            add_status_line(403, error_403_title);
            add_headers(strlen(error_403_form));
            if (!add_content(error_403_form))
                return false;
            break;
        }
        case FILE_REQUEST: {
            add_status_line(200, ok_200_title);
            if (m_file_stat.st_size != 0) {
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base = m_write_buf;
                m_iv[0].iov_len = m_write_idx;
                m_iv[1].iov_base = m_file_address;
                m_iv[1].iov_len = m_file_stat.st_size;
                m_iv_count = 2;
                bytes_to_send = m_write_idx + m_file_stat.st_size;
                return true;
            } else {
                const char *ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if (!add_content(ok_string))
                    return false;
            }
        }
        default:
            return false;
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    bytes_to_send = m_write_idx;
    return true;
}

void
yumira::http_conn::process() {
    HTTP_CODE read_ret;
    if ((read_ret = process_read()) == NO_REQUEST) {
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
        return;
    }
    if (!(process_write(read_ret))) {
        close_conn();
    }


    modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
}

void
yumira::http_conn::set_href_url(const string &html_path) {
    return set_href_url((const char *) html_path.c_str());
}

void
yumira::http_conn::set_href_url(const char *html_path) {
    strcpy(m_url, html_path);
    strncpy(m_real_file + strlen(doc_root), m_url, strlen(m_url));
}


yumira::http_conn::~http_conn() {
    // free interceptors elements
    LOG_WARN("deconstruct http_conn at: %d", this);
    for (auto &blueprint: getInterceptors()) {
        if(blueprint) {
            LOG_WARN("trying to deconstruct blueprint %d",blueprint);
            delete blueprint;
        }
        blueprint = nullptr;
    }
}

void close_user_fd(client_data_t *user_data) {
    // remove client fd from epoll moniter
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, nullptr);
    assert(user_data);
    close(user_data->sockfd);
    yumira::http_conn::decrease_active_user(1);
}