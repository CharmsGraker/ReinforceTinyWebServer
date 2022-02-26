#include "server_handler.h"

namespace yumira {
    yumira::WebServer<yumira::httpConnType> *current_app;
    yumira::quit_handler quitHandler;
    TemplateBinder<WebServerType> *templateBinder = nullptr;
    std::function<url_t(url_t, parameter_t &)> render_template;

    quit_handler
    registerQuitHanlder(quit_handler handler) {
        printf("trying to register quit handler\n");

        auto old_handler = quitHandler;
        quitHandler = handler;
        signal(SIGINT, handler);
        printf("successfully register quit handler: %d\n", handler);

        return old_handler;
    }


    void
    cleanResHandler(int sig) {
        LOG_WARN("capture signal: %d\n", sig);
        if (current_app) {
            delete current_app;
            LOG_INFO("successfully shutdown server: %d", current_app);
            current_app = nullptr;

        }
        if (templateBinder) {
            delete templateBinder;
            LOG_INFO("successfully delete templateBinder: %d", templateBinder);
            templateBinder = nullptr;

        }
    }

    WebServerType *
    builderServer() {
        current_app = new WebServerType();
        assert(current_app);
        registerQuitHanlder(cleanResHandler);

        templateBinder = new TemplateBinder<WebServerType>(current_app);

        render_template = std::bind(
                &TemplateBinder<WebServerType>::renderTemplateOn<url_t, parameter_t>,
                templateBinder, std::placeholders::_1, std::placeholders::_2);
        return current_app;
    }

}