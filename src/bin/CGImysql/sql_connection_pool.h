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
#include "../all_exception.h"

using namespace std;

class SqlConnectionPoolBaseBuilder {
public:
    SqlConnectionPoolBaseBuilder() {};

};

using sql_builder = SqlConnectionPoolBaseBuilder;

class connection_pool {
    /** connection for database(MySQL) Pool */
public:
    //获取数据库连接
    MYSQL *getConnection();

    template<class M>
    bool releaseConnection(M *conn); //释放连接
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
            if (!out) {
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

        connection_pool *build() {
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
    std::list<MYSQL *> connList; //连接池
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
    struct base {
        template<class M>
        M *getConn();

        virtual ~base() = default;;
    };

    template<class M>
    struct conn_impl : base {
        M *conRAII;
        typedef M conn_type;
        connection_pool *pool;

        M* getConn() {
           return conRAII;
        }
        conn_impl(M *con, connection_pool *pool_) : conRAII(con), pool(pool_) {}

        ~conn_impl() override {
            pool->releaseConnection<M>(conRAII);
        }
    };

    std::unique_ptr<base> connImpl;
public:
    template<class M>
    connectionRAII(M **con, connection_pool *pool):connImpl(new conn_impl{pool->getConnection(), pool}) {
        *con = ((conn_impl<M>*)(connImpl.get()))->getConn();
    };

    ~connectionRAII() = default;

private:

};
//释放当前使用的连接
template<class M>
bool connection_pool::releaseConnection(M *con) {
    if (nullptr == con)
        return false;
    lock.lock();
    try {

        connList.push_back(con);
        ++m_FreeConn, --m_CurConn;

        lock.unlock();

        reserve.post();
    } catch (exception &e) {
        lock.unlock();
        e.what();
    }
    return true;
}
#endif
