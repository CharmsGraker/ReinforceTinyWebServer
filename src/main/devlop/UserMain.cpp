#include "../../bin/UserMain.h"
#include "interceptor/JWTInterceptor.h"
using namespace yumira;


std::string
request_for_pic() {
    return "/picture.html";
}

std::string
request_for_video() {
    return "/video.html";
}

std::string
follow() {
    return "/fans.html";
}


std::string
login() {
    cout << "enter to login" << endl;
    if (request->method == POST) {
        //根据标志判断是登录检测还是注册检测
        printf("enter to login post\n");

        //将用户名和密码提取出来
        char name[100], password[100];

//        printf("url in /login view=%s\n", req->route().c_str());

        auto KVmap = request->getParams();
        strcpy(name, KVmap["user"].c_str());
        strcpy(password, KVmap["password"].c_str());
//        printf("%s",KVmap["user"].c_str());
//        printf("[INFO] name=%s,passwd=%s\n", name, password);
        if (strlen(name) && strlen(password) && userTable.find(name) != userTable.end() &&
            userTable[name] == password) {
//            printf("find user\n");
            return render_template("/welcome.html", KVmap);
        } else {
            printf("cant not find user\n");
            return "/logError.html";
        }
    }
    return "/index_module/index.html";
}

std::string
route_register() {
    if (request->method == POST) {
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        auto *conn = request->getConnection();

        //如果是注册，先检测数据库中是否有重名的
        //没有重名的，进行增加数据
        const char *name = request->getParams()["user"].c_str();
        const char *passwd = request->getParams()["password"].c_str();
//        printf("[INFO] name=%s,passwd=%s\n", name, passwd);

        if (name != nullptr && passwd != nullptr && userTable.find(name) == userTable.end()) {
            char *sql_insert = (char *) malloc(sizeof(char) * 200);
            sprintf(sql_insert, "INSERT INTO user(username, passwd) VALUES('%s','%s')", name, passwd);
            m_lock.lock();
            int res;
            try {
                res = mysql_query(conn->mysql, sql_insert);
                m_lock.unlock();
                if (!res) {
                    userTable.insert(pair<string, string>(name, passwd));
                    return conn->redirect("/login", GET);
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

template<>
int UserMain<WebServerType>::main() {

    http_conn::registerInterceptor(new Router("/href", []() { return "/h.html"; }));


    // root bp============================================================================
    http_conn::registerInterceptor(new Blueprint("/"), [](Blueprint *root_bp) {
        root_bp->registerRoute(new Router("/register", route_register));
        root_bp->registerRoute(new Router("/login", login));
        root_bp->registerRoute(new Router("/req_picture", request_for_pic));
        root_bp->registerRoute(new Router("/req_video", request_for_video));
        root_bp->registerRoute(new Router("/follow", follow));
    });
    InterceptorRegistry::Get().addInterceptor<JwtInterceptor>("/*");

    // another bp here==============================================================================
//    http_conn::register_interceptor(new Blueprint("/dash"),[](Blueprint*  root_bp){
//        root_bp ->registRoute(new Router("/req_video",requset_for_video));
//        root_bp ->registRoute(new Router("/follow",follow));
//    });
    return 0;
}
