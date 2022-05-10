#ifndef TINYWEB_SERVER_HANDLER_H
#define TINYWEB_SERVER_HANDLER_H
#include "../webserver.h"
#include "../handler.h"
#include "../../extensions/render/render_utils.h"

namespace yumira {
    extern quit_handler quitHandler;


    quit_handler
    registerQuitHanlder(quit_handler  handler);

    void cleanResHandler(int sig);
    WebServerType*
    builderServer();

    void
    releaseServer();
}
#endif //TINYWEB_SERVER_HANDLER_H
