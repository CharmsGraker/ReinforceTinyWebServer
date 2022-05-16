#include <iostream>
#include "config.h"


namespace yumira::dev {
    IdleConfig idleConfig{};
    HttpConfig httpConfig{};
    CommonConfig commonConfig{};
    TemplateConfig templateConfig{};

    void init_CommonConfig() {
        commonConfig.M_READERROR = -1;
        commonConfig.M_SERVERBUSY = -2;
        commonConfig.M_IDLEDATA = 1;
        commonConfig.M_SUCCESS = 0;
    }

    void init_idleConfig() {
        idleConfig.REDIS_CLUSTER_KEY = "grakerWebServerLb";
    }
    void init_TemplateConfig() {
        templateConfig.resourceFolder = "../../resources/root";
        std::cout << "init_TemplateConfig\n";
    }

}





