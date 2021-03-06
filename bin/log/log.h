#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

using namespace std;

class Logger {
public:
    //C++11以后,使用局部变量懒汉不用加锁
    static Logger *getInstance() {
        static Logger instance;
        return &instance;
    }

    static void *flush_log_thread(void *args) {
        Logger::getInstance()->async_write_log();
    }

    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000,
              int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush();

    static void
    Flush() {
        Logger *logger = getInstance();
        if(logger->m_close_log)
            return;
        logger->flush();
    }

    static void
    WriteLog(int level, const char *format, ...) {

        Logger *logger = getInstance();
        if(logger->m_close_log)
            return;
//            printf("into Logger\n");
        va_list args;
        logger->write_log(level, format, args);

    }

private:
    Logger();

    virtual ~Logger();

    void *async_write_log() {
        string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log)) {
            lock_write.lock();
            fputs(single_log.c_str(), m_fp);
            lock_write.unlock();
        }
    }

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int M_LOG_BUFFER_SIZE; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char *m_buf;
    block_queue<string> *m_log_queue; //阻塞队列
    bool m_is_async;                  //是否同步标志位
    Locker lock_write;
    int m_close_log = 1; //关闭日志
};

#define LOG_DEBUG(format, ...) {Logger::WriteLog(0, format, ##__VA_ARGS__); Logger::Flush();}
#define LOG_INFO(format, ...)  {Logger::WriteLog(1, format, ##__VA_ARGS__); Logger::Flush();}
#define LOG_WARN(format, ...)  {Logger::WriteLog(2, format, ##__VA_ARGS__); Logger::Flush();}
#define LOG_ERROR(format, ...)  {Logger::WriteLog(3, format, ##__VA_ARGS__); Logger::Flush();}

#endif
