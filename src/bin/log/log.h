#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"
#include "../timer/CTimer.h"
#include <boost/noncopyable.hpp>
#include "../singletonFactory/singletonFatory.h"

using namespace std;

template<class S>
class Stream {
public:
    std::function<S&(void)>open();
    template<class Log>
    void
    write(const Log &log);


    template<class Log>
    friend Stream &
    operator<<(Stream &s, const Log &log);

};


class Logger : public boost::noncopyable {
    CTimer m_timer_;
protected:
    int m_close_log = 1;

    std::function<void(const std::string &)>
            doWrite = [this](const std::string &log_str) -> void {
        fputs(log_str.c_str(), m_fp);
    };

public:
    void doFlush();

    void *flush_log_thread(void *args) {
        string single_log;

        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log)) {
            std::lock_guard<std::mutex> lockGuard(lock_write);
            doWrite(single_log);
        }
    }

    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000,
              int max_queue_size = 10);


    Logger();

    virtual ~Logger();

    void write_log(int level, const char *format, ...);

private:

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int M_LOG_BUFFER_SIZE; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char *m_buf;
    block_queue<std::string> *m_log_queue; //阻塞队列
    std::mutex lock_write;
};

class LoggerFactory : public SingletonFactory<Logger> {
public:
    static void
    Write(int level, const char *format, ...) {
        //            printf("into Logger\n");
        va_list args;
        LoggerFactory::Get().write_log(level, format, args);
    }

    static void
    Flush() {
        LoggerFactory::Get().doFlush();
    }
};

#define LOG_DEBUG(format, ...) {Logger::Write(0, format, ##__VA_ARGS__); Logger::Flush();}
#define LOG_INFO(format, ...)  {LoggerFactory::Write(1, format, ##__VA_ARGS__); LoggerFactory::Flush();}
#define LOG_WARN(format, ...)  {LoggerFactory::Write(2, format, ##__VA_ARGS__); LoggerFactory::Flush();}
#define LOG_ERROR(format, ...)  {LoggerFactory::Write(3, format, ##__VA_ARGS__); LoggerFactory::Flush();}

namespace yumira {

    template<class Server>
    void
    initLogger(Server *server) {
        if (0 == server->m_close_log) {
            //初始化日志
            LoggerFactory::Get().init(server->severLogPath, server->m_close_log, 2000, 800000, 200);
        }
    }
}

#endif
