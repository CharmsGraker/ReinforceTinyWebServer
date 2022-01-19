#ifndef TINYWEB_CONFIG_H
#define TINYWEB_CONFIG_H
namespace yumira {
    const int MAX_FD = 65536;           //最大文件描述符
    const int TIMESLOT = 5;             //最小超时单位
    constexpr int MAX_EVENT_NUMBER = 10000; //最大事件数
    extern const char *resourceFolder;

}
//#define CONFIG_PROPERTY
#endif //TINYWEB_CONFIG_H
