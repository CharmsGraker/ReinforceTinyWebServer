#include "bin/UserMain.h"


int main(int argc, char *argv[]) {
    //需要修改的数据库信息,登录名,密码,库名 please config in /conf/server-config.xml

    //命令行解析
    Configure conf;
    conf.gen_conf(argc, argv);

    auto server = yumira::builderServer();

    //初始化
    server->loadFromConf(conf);
    printf("after conf\n");

    server->init_log_write();

    //数据库
    server->sql_pool();

    printf("after sql pool\n");
    //线程池
    server->createThreadPool();
    printf("after createThreadPool\n");

    //触发模式
    server->setTrigMode();

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
        } catch (exception & e) {
            printf("in catch block \n");
            e.what();
        }
    }
    return 0;
}
