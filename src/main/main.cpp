#include <unordered_map>
#include "../bin/UserMain.h"
#include "../bin/extensions/heartbeats/SocketHeartBeatThread.h"

using namespace yumira::dev;


int main(int argc, char *argv[]) {
    auto&& server = yumira::builderServer();
    {
        Configure conf{};
        conf.gen_conf(argc, argv);
        conf.init(server);
    }
    LoggerFactory::Get().init(server);
    initIdleThread(server);

    printf("after bindServer\n");

    if (UserMain::init(server) < 0) {
        throw exception();
    }

    server->registerEventListen();
    printf("after eventListen\n");
    server->eventLoop();
    return 0;
}
