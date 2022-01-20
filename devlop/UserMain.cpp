#include "../bin/UserMain.h"
#include "../bin/render/render_utils.h"

// define your request parser here



using namespace yumira;


url_t request_for_pic(runtime_request_infer_t req) {
    return url_t("/picture.html");
}

url_t requset_for_video(runtime_request_infer_t req) {
    return url_t("/video.html");
}

url_t follow(runtime_request_infer_t req) {
    return url_t("/fans.html");
}


url_t
login(runtime_request_infer_t req) {
    if (req->method == POST) {
        //根据标志判断是登录检测还是注册检测
        printf("enter to login post\n");

//        strncpy(conn->m_real_file + len_root_path, m_url_real, conn->FILENAME_LEN - len_root_path - 1);

        //将用户名和密码提取出来
        char name[100], password[100];

        printf("url in /login view=%s\n", req->route().c_str());


        auto KVmap = req->args();
        strcpy(name, KVmap.get("user").c_str());
        strcpy(password, KVmap.get("password").c_str());
        printf("%s",KVmap.get("user").c_str());
        printf("[INFO] name=%s,passwd=%s\n", name, password);
        printf("%d %d", strcmp(name,""), strcmp(password,""));
        if (strlen(name) && strlen(password) && userTable.find(name) != userTable.end() && userTable[name] == password) {
            printf("find user\n");
            return render_template(url_t("/welcome.html"), KVmap);
        } else {
            printf("cant not find user\n");
            return url_t("/logError.html");
        }
    }
    return url_t("/index_module/index.html");
}

url_t
route_register(runtime_request_infer_t req) {
    if (req->method == POST) {
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        auto connect = req->getAdapter();
        m_lock;

        //如果是注册，先检测数据库中是否有重名的
        //没有重名的，进行增加数据
        const char *name = req->args()["user"].c_str();
        const char *passwd = req->args()["password"].c_str();
        printf("[INFO] name=%s,passwd=%s\n", name, passwd);

        if (name != nullptr && passwd != nullptr && userTable.find(name) == userTable.end()) {
            char *sql_insert = (char *) malloc(sizeof(char) * 200);
            sprintf(sql_insert, "INSERT INTO user(username, passwd) VALUES('%s','%s')", name, passwd);
            printf("find user.\n");
            m_lock.lock();
            int res;
            try {
                res = mysql_query(connect.query(), sql_insert);
                m_lock.unlock();
                if (!res) {
                    userTable.insert(pair<string, string>(name, passwd));
                    return connect.redirect("/login", GET);
                } else
                    return url_t("/registerError.html");

            } catch (exception &e) {
                m_lock.unlock();
                return url_t("/register.html");
            }
        } else
            return url_t("/registerError.html");
    }
    return url_t("/register.html");
}

int UserMain::main() {

    http_conn::register_interceptor(new Router("/href", [](runtime_request_infer_t req) { return url_t("/h.html"); }));


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
