#include "../bin/UserMain.h"


// define your request parser here


const char *request_for_pic(runtime_request_infer_t req) {
    return "/picture.html";
}

const char *requset_for_video(runtime_request_infer_t req) {
    return "/video.html";
}

const char *follow(runtime_request_infer_t req) {
    return "/fans.html";
}



const char *login(runtime_request_infer_t req) {
    if (req->method == POST) {
        //根据标志判断是登录检测还是注册检测
        printf("enter to login post\n");

//        strncpy(conn->m_real_file + len_root_path, m_url_real, conn->FILENAME_LEN - len_root_path - 1);

        //将用户名和密码提取出来
        char name[100], password[100];

        printf("url in login view=%s\n", req->route().c_str());

        strcpy(name, req->args()["user"].c_str());
        strcpy(password, req->args()["password"].c_str());

        extern map<string, string> users;

        printf("[INFO] name=%s,passwd=%s\n", name, password);
        if (users.find(name) != users.end() && users[name] == password) {
            return "/welcome.html";
        } else
            return "/logError.html";
    }
    return "/log.html";
}

const char *
route_register(runtime_request_infer_t req) {
    if (req->method == POST) {
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        auto connect = req->getAdapter();
        extern map<std::string, std::string> users;
        extern Locker m_lock;

        //如果是注册，先检测数据库中是否有重名的
        //没有重名的，进行增加数据
        const char *name = req->args()["user"].c_str();
        const char *passwd = req->args()["password"].c_str();
        printf("[INFO] name=%s,passwd=%s\n", name, passwd);

        if (name!= nullptr && passwd != nullptr && users.find(name) == users.end()) {
            char *sql_insert = (char *) malloc(sizeof(char) * 200);
            sprintf(sql_insert, "INSERT INTO user(username, passwd) VALUES('%s','%s')", name, passwd);

            m_lock.lock();
            int res;
            try {
                res = mysql_query(connect.query(), sql_insert);
                m_lock.unlock();
                if (!res) {
                    users.insert(pair<string, string>(name, passwd));
                    return connect.redirect("/login",POST);
                } else
                    return "/registerError.html";

            } catch (exception &e) {
                m_lock.unlock();
                return "/register.html";
            }
        } else
            return "/registerError.html";
    }
    return "/register.html";
}

int UserMain::main() {

    http_conn::register_interceptor(new Router("/href", [](runtime_request_infer_t req) { return "/h.html"; }));


    // root bp============================================================================
    http_conn::register_interceptor(new Blueprint<Router>("/"), [](Blueprint<Router> *root_bp) {
        root_bp->registRoute(new Router("/register", route_register));
        root_bp->registRoute(new Router("/login", login));
        root_bp->registRoute(new Router("/req_picture", request_for_pic));
        root_bp->registRoute(new Router("/req_video", requset_for_video));
        root_bp->registRoute(new Router("/follow", follow));
    });

    // another bp here==============================================================================
//    http_conn::register_interceptor(new Blueprint("/dash"),[](Blueprint*  root_bp){
//        root_bp ->registRoute(new Router("/req_video",requset_for_video));
//        root_bp ->registRoute(new Router("/follow",follow));
//    });
    return 0;
}
