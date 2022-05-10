#ifndef TINYWEB_CONFIG_H
#define TINYWEB_CONFIG_H
#include <string>
namespace yumira {
    const int MAX_FD = 65536;           //最大文件描述符
    const int TIMESLOT = 5;             //最小超时单位
    constexpr static int MAX_EVENT_NUMBER = 10000; //最大事件数
    extern const char *resourceFolder;
    using namespace std;
    extern std::string m_service_;
    // use in deal client data
    extern int M_READERROR;
    extern int M_IDLEDATA;
    extern int M_SUCCESS;
    extern int M_SERVERBUSY;
}
//#define CONFIG_PROPERTY
#endif //TINYWEB_CONFIG_H
