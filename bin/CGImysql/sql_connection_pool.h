#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"
#include "abstractSqlConnectionPool.h"
#include "../../bin/all_exception.h"
using namespace std;


class connection_pool : SqlConnectionPoolBase<MYSQL> {
    /** connection for database(MySQL) Pool */
public:
    MYSQL *getConnection() override;              //获取数据库连接
    bool releaseConnection(MYSQL *conn) override; //释放连接
    int getFreeConn() const;                     //获取连接

    //Singleton pattern
    static connection_pool *getInstance();

//    void
//    init(const string &url, const string &User, const string &PassWord, const string &DataBaseName, const int &port,
//         const int &MaxConn, const int &close_log);
    void
    init();

    class Builder {
    private:
        connection_pool *outer;
    public:
        Builder(connection_pool *out) {
            if(!out) {
                throw NullException("connection_pool is Null!");
            }
            outer = out;
        };

        Builder &url(const string &url) {
            outer->m_url = url;
            return *this;
        }

        Builder &user(const string &User) {
            outer->m_User = User;
            return *this;
        }

        Builder &passWord(const string &PassWord) {
            outer->m_PassWord = PassWord;
            return *this;
        }

        Builder &dataBaseName(const string &DataBaseName) {
            outer->m_DatabaseName = DataBaseName;
            return *this;
        }

        Builder &port(const int &port) {
            outer->m_Port = port;
            return *this;
        }

        Builder &maxConn(const int &maxConn) {
            outer->m_MaxConn = maxConn;
            return *this;
        }

        Builder &close_log(const int &close_log) {
            outer->m_close_log = close_log;
            return *this;
        }

        connection_pool* build() {
            outer->init();
            return outer;
        };
    };

    static Builder &builder() {
        static Builder __builder = Builder(getInstance());
        return __builder;
    }


private:
    connection_pool();

    ~connection_pool();

    void destroyPool(); // clean all connection when trigger deconstruct
    void MYSQL_OCCUR_ERROR(MYSQL *connect) const;

private:
    int m_MaxConn;  //最大连接数
    int m_CurConn;  //当前已使用的连接数
    int m_FreeConn; //当前空闲的连接数
    Locker lock;
    list<MYSQL *> connList; //连接池
    Semaphore reserve; // > 0 means there has free connection to use


public:
    string m_url;           //主机地址
    int m_Port;             //数据库端口号
    string m_User;         //登陆数据库用户名
    string m_PassWord;     //登陆数据库密码
    string m_DatabaseName; //使用数据库名
    int m_close_log;        //日志开关
};

class connectionRAII {

public:
    connectionRAII(MYSQL **con, connection_pool *connPool);

    ~connectionRAII();

private:
    MYSQL *conRAII;
    connection_pool *poolRAII;
};

#endif
