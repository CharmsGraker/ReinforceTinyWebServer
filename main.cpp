#include "config.h"
#include "bin/UserMain.h"

int main(int argc, char *argv[]) {
    //需要修改的数据库信息,登录名,密码,库名 please config in /conf/server-config.xml


    //命令行解析
    Configure conf;
    conf.gen_conf(argc, argv);

    WebServer server;

    //初始化
    server.loadFromConf(conf);


    //日志
    server.log_write();

    //数据库
    server.sql_pool();

    //线程池
    server.createThreadPool();

    //触发模式
    server.setTrigMode();

    UserMain::setPara(argc, argv);
    UserMain::bindServer(&server);

    UserMain *userMain = UserMain::getInstance();

    int err = userMain->operator()();
    if (!err) {
        //监听
        server.eventListen();

        //运行
        server.eventLoop();
    }

    return 0;
}
