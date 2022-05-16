#include <unordered_map>
#include "../bin/UserMain.h"
#include "../bin/extensions/heartbeats/SocketHeartBeatThread.h"
#include "../bin/log/log.h"
namespace yumira {
    std::unordered_map<std::string, std::string> serverProviderMap;
}

using namespace yumira::dev;


int main(int argc, char *argv[]) {

    auto&& server = yumira::builderServer();
    printf("a\n");

    {
        Configure conf{};
        conf.gen_conf(argc, argv);
        conf.init(server);
    }
    LoggerFactory::Get().init(server);
    initIdleThread(server);

    UserMain<WebServerType>::setPara(argc, argv);
    UserMain<WebServerType>::bindServer(server);
    printf("after bindServer\n");

    if (!UserMain<WebServerType>::preProcess()) {
        //监听
        server->registerEventListen();
        printf("after eventListen\n");
        server->eventLoop();
    }
    return 0;
}
