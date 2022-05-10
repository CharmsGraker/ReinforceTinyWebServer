#include <unordered_map>
#include "../bin/UserMain.h"

namespace yumira {
    std::string m_service_ = "WebServer";
    std::unordered_map<std::string, std::string> serverProviderMap;
    std::string redis_host_;
    int redis_port_;
}

template<class Conf>
void
__init_redis__(Conf &conf) {
    yumira::redis_host_ = conf.template getPropOf<string>("redis_host");
    yumira::redis_port_ = conf.template getPropOf<int>("redis_port");
}


int main(int argc, char *argv[]) {
    auto server = yumira::builderServer();

    {
        Configure conf{};
        __init_redis__(conf);
        //需要修改的数据库信息,登录名,密码,库名 please config in /conf/server-config.xml

        //命令行解析
        conf.gen_conf(argc, argv);
        conf.init(server);

    }
    server->f_checkConnFd = peekHBPack;
    //初始化
    printf("after conf\n");

    initLogger(server);

    //数据库
    initSqlPool(server);

    printf("after sql pool\n");

    UserMain<WebServerType>::setPara(argc, argv);
    UserMain<WebServerType>::bindServer(server);
    printf("after bindServer\n");

    if (!UserMain<WebServerType>::preProcess()) {
        //监听
        server->registerEventListen();
        printf("after eventListen\n");

        //运行
        try {
            server->eventLoop();
        } catch (exception &e) {
            printf("in catch block \n");
            e.what();
        }
    }
    return 0;
}
