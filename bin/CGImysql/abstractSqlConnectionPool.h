#ifndef TINYWEB_ABSTRACTSQLCONNECTIONPOOL_H
#define TINYWEB_ABSTRACTSQLCONNECTIONPOOL_H

class AbstractSqlConnectionPool {
public:
    AbstractSqlConnectionPool() = default;

    virtual ~AbstractSqlConnectionPool() = default;
};

class SqlConnectionPoolBaseBuilder {
public:
    SqlConnectionPoolBaseBuilder() {};

};

using sql_builder = SqlConnectionPoolBaseBuilder;

template<typename SQL>
class SqlConnectionPoolBase : public AbstractSqlConnectionPool {
public:
//    virtual sql_builder builder() = 0;

    static void build(){};

    virtual SQL *getConnection() = 0;                 //获取数据库连接
    virtual bool releaseConnection(SQL *conn) = 0; //释放连接
    virtual void destroyPool() = 0;

    virtual ~SqlConnectionPoolBase() = default;
};

#endif //TINYWEB_ABSTRACTSQLCONNECTIONPOOL_H
