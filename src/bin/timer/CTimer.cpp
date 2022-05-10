//
//  CTimer.cpp
//  ZJCrossCpp
//
//  Created by eafy on 2020/10/5.
//  Copyright © 2020 ZJ. All rights reserved.
//
#include "CTimer.h"
#include <future>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>


CTimer::CTimer(const std::string& sTimerName) : m_bExpired(true), m_bTryExpired(false), m_bLoop(false) {
    m_sName = sTimerName;
}

CTimer::~CTimer() {
    m_bTryExpired = true;   //尝试使任务过期
    DeleteThread();
}

bool
CTimer::Start(unsigned int msTime, std::function<void()> task, bool bLoop, bool async) {
    if (!m_bExpired || m_bTryExpired)
        return false;  //任务未过期(即内部仍存在或正在运行任务)
    m_bExpired = false;
    m_bLoop = bLoop;
    m_nCount = 0;

    if (async) {
        // wait last task quit
        std::lock_guard<std::mutex> lockGuard(m_ThreadLock);
        DeleteThread();
        m_Thread = new std::thread([this, msTime, task]() {
            while (!m_bTryExpired) {
                // old thread may be drive out when new task insert this Timer, so it occurs race
                m_ThreadCon.wait_for(m_ThreadLock, std::chrono::milliseconds(msTime));  //休眠
                if (!m_bTryExpired) {
                    task();     //执行任务

                    m_nCount++;
                    if (!m_bLoop) {
                        break;
                    }
                }
            }

            m_bExpired = true;      //任务执行完成(表示已有任务已过期)
            m_bTryExpired = false;  //为了下次再次装载任务
        });
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(msTime));
        if (!m_bTryExpired) {
            task();
        }
        m_bExpired = true;
        m_bTryExpired = false;
    }

    return true;
}

void CTimer::Cancel() {
    if (m_bExpired || m_bTryExpired || !m_Thread) {
        return;
    }

    m_bTryExpired = true;
}

void CTimer::DeleteThread() {
    if (m_Thread) {
        // wait last Thread quit
        m_ThreadCon.notify_all();   //休眠唤醒
        m_Thread->join();           //等待线程退出
        delete m_Thread;
        m_Thread = nullptr;
    }
}

#pragma mark -

uint64_t CTimer::Timestamp() {
    uint64_t msTime = 0;

    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    msTime = ((uint64_t) abstime.tv_sec) * 1000 + ((uint64_t) abstime.tv_nsec) / 1000000;   //需要强制转long long

    return msTime;
}

std::string CTimer::FormatTime(const std::string sFormat) {
    time_t timep;
    time(&timep);

    char tmp[64];
    strftime(tmp, sizeof(tmp), sFormat.c_str(), localtime(&timep));

    return std::string(tmp);
}

struct tm *CTimer::UTCTime(long long secTime) {
    time_t timep;
    if (secTime) {
        timep = secTime;
    } else {
        time(&timep);
    }

    struct tm *data = gmtime(&timep);
    data->tm_year += 1900;
    data->tm_mon += 1;

    return data;
}

int64_t CTimer::UTCTime() {
    int64_t msTime = 0;

#if defined(__APPLE__)  //iOS
    if (__builtin_available(iOS 10.0, *)) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        msTime = ((int64_t)abstime.tv_sec) * 1000 + ((int64_t)abstime.tv_nsec) / 1000000;
    } else {
        struct timeval abstime;
        gettimeofday(&abstime, NULL);
        msTime = ((int64_t)abstime.tv_sec) * 1000 + ((int64_t)abstime.tv_usec) / 1000;
    }
#else
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    msTime = (int64_t) abstime.tv_sec;
#endif

    return msTime;
}

int CTimer::TimeDiffFromGMT() {
    time_t now = time(nullptr);
    struct tm *gmTime = gmtime(&now);
    if (gmTime) {
        return (int) difftime(now, mktime(gmTime));
    }

    return 0;
}