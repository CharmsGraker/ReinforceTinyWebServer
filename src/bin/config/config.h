#ifndef TINYWEB_CONFIG_H
#define TINYWEB_CONFIG_H
#include <string>

namespace yumira {
    struct Config {

    };

    namespace dev {
        void init_CommonConfig();
        void init_idleConfig();
        void init_TemplateConfig();


        struct CommonConfig {
            std::string serverIp;
            int serverPort;
            const int TIMESLOT = 5;             //最小超时单位
            const int MAX_EVENT_NUMBER = 10000; //最大事件数
            std::string ServiceName;
            const char *severLogPath = "./ServerLog";

            // use in deal client data
            int M_READERROR;
            int M_IDLEDATA;
            int M_SUCCESS;
            int M_SERVERBUSY;

            void setConnIp(const std::string&Ip) {
                serverIp = Ip;
            }
            void setConnPort(const int port) {
                serverPort = port;
            };
            CommonConfig() {
                init_CommonConfig();
            }
        };

        struct HttpConfig: public Config {
            const int MAX_FD = 65536;           //最大文件描述符
            HttpConfig() {

            }
        };
        struct TemplateConfig: public Config {
            std::string resourceFolder;
            TemplateConfig(){
                init_TemplateConfig();
            };
        };
        struct IdleConfig: public Config {
            std::string REDIS_CLUSTER_KEY;
            std::string REDIS_HOST;
            int REDIS_PORT;
            IdleConfig(){
                init_idleConfig();
            };
        };

        extern HttpConfig httpConfig;
        extern TemplateConfig templateConfig;
        extern IdleConfig idleConfig;
        extern CommonConfig commonConfig;


    }

}
//#define CONFIG_PROPERTY
#endif //TINYWEB_CONFIG_H
