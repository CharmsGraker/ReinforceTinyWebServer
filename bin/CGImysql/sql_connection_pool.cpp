#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

connection_pool::connection_pool() {
    m_CurConn = 0;
    m_FreeConn = 0;
}

connection_pool *connection_pool::GetInstance() {
    static connection_pool connPool;
    return &connPool;
}

void
connection_pool::MYSQL_OCCUR_ERROR(MYSQL *connect) const {
    if (connect == nullptr) {
        LOG_ERROR("MySQL Error");
        exit(1);
    }
};

//构造初始化
void
connection_pool::init(const string &url, const string &userTabName, const string &PassWord,
                      const string &DBName, const int &port, const int &MaxConn, const int &close_log) {
    m_url = url;
    m_Port = port;
    m_User = userTabName;
    m_PassWord = PassWord;
    m_DatabaseName = DBName;
    m_close_log = close_log;

    for (int i = 0; i < MaxConn; ++i) {
        MYSQL *con = mysql_init(nullptr);

        MYSQL_OCCUR_ERROR(con);
        con = mysql_real_connect(con, url.c_str(), userTabName.c_str(),
                                 PassWord.c_str(), DBName.c_str(), port, NULL, 0);
        MYSQL_OCCUR_ERROR(con);
        connList.push_back(con);
        ++m_FreeConn;
    }

    reserve = Semaphore(m_FreeConn); // ree resoure add to semaphore

    m_MaxConn = m_FreeConn;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection() {
    MYSQL *con = nullptr;

    if (!connList.empty()) {
        reserve.wait(); // wait for semaphore > 0
        lock.lock();
        try {
            con = connList.front();
            connList.pop_front();

            --m_FreeConn;
            ++m_CurConn;
            lock.unlock();

        } catch (exception &e) {
            lock.unlock();
            e.what();

        }
    }
    return con;
}

//释放当前使用的连接
bool connection_pool::ReleaseConnection(MYSQL *con) {
    if (nullptr == con)
        return false;
    lock.lock();
    try {

        connList.push_back(con);
        ++m_FreeConn;
        --m_CurConn;

        lock.unlock();

        reserve.post();
    } catch (exception &e) {
        lock.unlock();
        e.what();
    }
    return true;
}

//销毁数据库连接池
void connection_pool::DestroyPool() {
    lock.lock();
    if (!connList.empty()) {
        MYSQL *con;
        for (auto it = connList.begin();/*list<MYSQL *>::iterator*/
             it != connList.end();
             ++it) {
            con = *it;
            mysql_close(con);
        }
        bool cleanState = false;
        while (!cleanState) {
            m_CurConn = 0;
            m_FreeConn = 0;
            connList.clear();
            cleanState = true;
        }
    }
    lock.unlock();
}

//当前空闲的连接数
int connection_pool::GetFreeConn() {
    return m_FreeConn;
}

connection_pool::~connection_pool() {
    DestroyPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool) {
    *SQL = connPool->GetConnection();

    conRAII = *SQL;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII() {
    poolRAII->ReleaseConnection(conRAII);
}