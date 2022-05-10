#ifndef TINYWEB_REDIS_UTILS_H
#define TINYWEB_REDIS_UTILS_H

#include <hiredis/hiredis.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <cassert>
#include <boost/noncopyable.hpp>

using namespace std;

class InetAddress;

class RedisConnection : public boost::noncopyable {
public:
    typedef std::vector<std::string> replyVector;
    typedef long long int replyInteger;
    typedef std::string replyStr;
protected:
    // return true on error occur, false with normal
    bool
    check_error() {
        if (!_reply)
            return true;
        if (_reply->type == REDIS_REPLY_ERROR) {
            printf("[redis Error] %s\n", _reply->str);
            error_str = _reply->str;
            freeReplyObject(_reply);
            return true;
        };

        return false;
    }

    void
    deal_reply_array(replyVector &array_) {
        // printf("deal_reply_array\n");
        if (check_error()) {
            return;
        }
        if (_reply->type != REDIS_REPLY_ARRAY) {
            freeReplyObject(_reply);
            return;
        }
        auto num_elements = _reply->elements;
        std::cout << "redis vector, num_elements: " << num_elements << endl;
        array_.reserve(num_elements);
        for (int i = 0; i < num_elements; ++i) {
            array_.emplace_back(_reply->element[i]->str);
        }
        freeReplyObject(_reply);
        _reply = nullptr;
    };

    void
    deal_reply_status(replyStr &str) {
        if (check_error())
            return;

        if (_reply->type != REDIS_REPLY_STATUS) {
            return;
        }

        str = this->_reply->str;
        freeReplyObject(this->_reply);
        _reply = nullptr;
    };

    void
    deal_reply_str(replyStr &str) {
        if (check_error())
            return;

        if (_reply->type != REDIS_REPLY_STRING) {
            return;
        }

        str = this->_reply->str;
        freeReplyObject(this->_reply);
        _reply = nullptr;
    };

    void
    deal_reply_integer(replyInteger &integer) {
        if (check_error())
            return;
        if (_reply && _reply->type == REDIS_REPLY_INTEGER) {

            integer = this->_reply->integer;
            freeReplyObject(this->_reply);
            _reply = nullptr;
        }
    };
public:


    RedisConnection() {};

    ~RedisConnection() {
        this->_connect = nullptr;
        this->_reply = nullptr;
    }


    replyStr
    Set(const string &key, const string &value) {
        error_str.clear();

        _reply = (redisReply *) redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());
        replyStr success;
        deal_reply_str(success);
        return success;
    }

    replyInteger
    Setnx(const string &key, const string &value) {
        error_str.clear();

        _reply = (redisReply *) redisCommand(this->_connect, "SETNX %s %s", key.c_str(), value.c_str());
        replyInteger success = 0;
        deal_reply_integer(success);
        return success;
    }

    template<class Integer>
    replyInteger
    Zadd(const std::string &zSetName, const string &member, Integer value) {
        error_str.clear();

        printf("Invoke RedisConnection::Zadd()\n");
        _reply = (redisReply *) redisCommand(this->_connect, "ZADD %s %d %s", zSetName.c_str(), value, member.c_str());
        replyInteger addCnt = 0;
        deal_reply_integer(addCnt);
        return addCnt;
    }

    replyInteger
    Zrem(const std::string &key, const std::string &member) {
        error_str.clear();
        _reply = (redisReply *) redisCommand(this->_connect, "ZREM %s %s", key.c_str(), member.c_str());

        replyInteger removeCnt = 0;
        deal_reply_integer(removeCnt);
        return removeCnt;
    }

    replyVector
    Zrange(const std::string &key, int start, int end = -1, bool withScore = false) {
        /**
         * only return keys
         * */
        error_str.clear();
        printf("invoke ZRANGE\n");
        _reply = (redisReply *) redisCommand(this->_connect, "ZRANGE %s %d %d", key.c_str(), start, end);
        printf("invoke ZRANGE complete\n");
        replyVector array_;
        deal_reply_array(array_);
        return array_;
    }


    replyStr
    Get(string key) {
        error_str.clear();
        this->_reply = (redisReply *) redisCommand(this->_connect, "GET %s", key.c_str());
        std::string str;
        deal_reply_str(str);
        return str;
    }

    template<class Integer=int>
    replyVector
    ZrangeByScoreGreaterThan(const std::string &key, Integer low_bound, bool withScore = true,
                             bool include_low = true) {
        error_str.clear();
        if (withScore) {
            _reply = (redisReply *) redisCommand(this->_connect, "ZRANGEBYSCORE %s %d +inf WITHSCORES", key.c_str(),
                                                 low_bound);

        } else
            _reply = (redisReply *) redisCommand(this->_connect, "ZRANGEBYSCORE %s %d +inf", key.c_str(), low_bound);
        replyVector array_;
        deal_reply_array(array_);
        return array_;
    }

    template<class Integer=int>
    replyVector
    ZrangeByScore(const std::string &key, Integer low, Integer high, bool withScore = false,
                  bool include_low = true, bool include_high = true) {
        assert(low >= high);
        error_str.clear();
        if (withScore) {
            _reply = (redisReply *) redisCommand(this->_connect, "ZRANGEBYSCORE %s %d %d WITHSCORES", key.c_str(),
                                                 low,
                                                 high);

        } else
            _reply = (redisReply *) redisCommand(this->_connect, "ZRANGEBYSCORE %s %d %d", key.c_str(), low, high);
        replyVector array_;
        deal_reply_array(array_);
        return array_;
    }

    replyInteger
    Zrank(const std::string &key, const std::string &member) {
        error_str.clear();

        _reply = (redisReply *) redisCommand(this->_connect, "ZRANK %s %s", key.c_str(), member.c_str());
        replyInteger ret = 0;
        deal_reply_integer(ret);
        return ret;
    }

    static RedisConnection *
    Fetch() {
        return new RedisConnection();
    }

    replyStr
    Watch(int count, const std::string &key) {
        error_str.clear();

        _reply = (redisReply *) redisCommand(this->_connect, "WATCH %s", key.c_str());
        replyStr ret;
        deal_reply_status(ret);
        return ret;
    }

    bool
    CompareAndSet(const std::string &key, const std::string &expect, const std::string &change) {
        std::string currentVal = Get(key);
        if (currentVal != expect) {
            printf("the key(%s) now=%s\n", key.c_str(), currentVal.c_str());
            return false;
        };
        auto status = Watch(1, key.c_str());
        if (status != "OK" || startTransaction() != "OK") {
            printf("watch fail before transaction\n");
            return false;
        }
        auto ret = Set(key, change);
        Exec();
        if (error_str.empty())
            return ret == "OK";
        return false;
    }

    replyStr
    startTransaction(const char *error = nullptr) {
        error_str.clear();
        _reply = (redisReply *) redisCommand(this->_connect, "MULTI");
        if (!_reply) {
            if (error)
                error = error_str.c_str();
            return {};
        }
        replyStr ret;
        deal_reply_status(ret);
        return ret;
    }

    replyVector
    Exec(const char *error = nullptr) {
        _reply = (redisReply *) redisCommand(this->_connect, "EXEC");
        if (_reply) {
            if (error)
                error = error_str.c_str();
            return {};
        }
        replyVector vec;
        deal_reply_array(vec);
        return vec;
    }

public:

    bool Connect(const std::string &host, int port) {
        host_ = host;
        port_ = port;
        _connect = redisConnect(host.c_str(), port);
        return isConnected();
    }

    bool Connect(const InetAddress &inetAddress);

    bool
    tryConnect(int n_times) {
        return tryConnect(host_, port_, n_times);
    }

    bool
    tryConnect(const std::string &host, int port, int n_times) {
        n_times = max(min(n_times, 10), 1);
        while (n_times--) {
            if (Connect(host, port))
                return true;
            usleep(100);
        }
        return isConnected();
    }

    bool
    isConnected() const {
        return _connect && !_connect->err;
    }

    string
    showConnError() const {
        return _connect->errstr;
    }

private:
    redisContext *_connect;
    redisReply *_reply;
    std::string error_str;
    std::string host_;
    int port_;
};

#endif //TINYWEB_REDIS_UTILS_H
