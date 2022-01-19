#include "bin/UserMain.h"

namespace yumira {
    yumira::yumira_server_t* current_app;
}

int main(int argc, char *argv[]) {
    //需要修改的数据库信息,登录名,密码,库名 please config in /conf/server-config.xml


    //命令行解析
    Configure conf;
    conf.gen_conf(argc, argv);

    yumira::WebServer server;
    current_app = &server;


    //初始化
    server.loadFromConf(conf);
    printf("after conf\n");


    //日志
    server.log_write();
    printf("after log_write\n");

    //数据库
    server.sql_pool();

    printf("after sql pool\n");
    //线程池
    server.createThreadPool();
    printf("after createThreadPool\n");

    //触发模式
    server.setTrigMode();

    UserMain::setPara(argc, argv);
    UserMain::bindServer(&server);
    printf("after bindServer\n");

    UserMain *userMain = UserMain::getInstance();

    if (!(*userMain)()) {
        //监听
        server.registerEventListen();
        printf("after eventListen\n");

        //运行
        server.eventLoop();

    }

    return 0;
}
