//
// Created by nekonoyume on 2022/1/4.
//

#ifndef TINYWEB_REQUEST_H
#define TINYWEB_REQUEST_H

#include <string>
#include <map>
#include "exception"
#include <iostream>

#include "./../../utils/string_utils.h"
#include "../all_exception.h"


using namespace string_util;

#define REQUEST_CAST (Request&)

enum http_req_method_t {
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATH
};

template<typename K, typename V>
class HashMap {
    std::map<K, V> *container;
    bool throw_exception = false;
public:
    HashMap() : HashMap(false) {};

    HashMap(bool throw_exception) : throw_exception(throw_exception), container(new std::map<K, V>()) {};

    V&
    operator[](const K& key) {
        // please catch Exception above!!!!!
        if (container->count(key) <= 0) {
            if (throw_exception) throw NotExistKeyException();
            std::cout << "[HashMap<K,V>] visit key: " << key << ", but get null."<<std::endl;
        }
        return container->operator[](key);
    }
//    V&
//    operator[] (K key) {
//        return container->operator[]()
//    }

    bool empty() {
        return container->empty();
    }
};

class Request {
    HashMap<std::string, std::string> __url_parsed_arg;
    std::string _route;

    Request(const std::string raw_url,
            std::string url_param,
            std::string route,
            http_req_method_t method) :
            method(method), _raw_url(raw_url), _url_param(url_param), _route(route),__url_parsed_arg(false) {
        printf("[INFO] makeRequest: raw_url=%s\n", raw_url.c_str());

    };

    Request() {};

public:
    http_req_method_t method;
    std::string _url_param;
    std::string _raw_url;

    static
    Request
    __new() { return Request("", "", "", (http_req_method_t) 0); };

    /** url means the href route,not include params */


    static Request
    makeRequest(std::string __raw_url) {
        try {
            if (__raw_url.empty()) {
            }
        } catch (NullException &e) {
            e.what();
            throw CanNotMakeRequestException("empty raw url!");
        }

        std::string route, url_param;
        _set_route_and_param(__raw_url, route, url_param);
        printf("[INFO] makeRequest: raw_url=%s\n", __raw_url.c_str());
        return Request(__raw_url, url_param, route, GET);
    }

    static Request
    makeRequest(std::string route, std::string paramStr) {
        try {
            if (route.empty()) {
                throw NullException("");
            }
        } catch (NullException &e) {
            e.what();
            throw CanNotMakeRequestException("empty route!");
        }
        // need to parse
        return Request(route + paramStr, paramStr, route, POST);
    }

    HashMap<std::string, std::string> &args() {
        if (__url_parsed_arg.empty()) {
            fprintf(stderr, "null map!\n");
            throw std::exception();
        }
        return __url_parsed_arg;
    }

    void parse_arg() {
        _param_parser();
    }


    std::string
    route() const {
        if (_route.empty()) {
            return raw_url();
        }
        return _route;
    }

    std::string
    raw_url() const {
        if (_raw_url.empty()) {
            throw NullException("empty raw_url");
        }
        return _raw_url;
    }

private:
    static void
    _set_route_and_param(const std::string raw_url, std::string &route, std::string &url_param) {
        if (raw_url.empty()) {
            throw NullException("empty url in request!\n");
        }
#ifdef DEBUG
        printf("req:_raw_url=%s\n",raw_url.c_str());
#endif

        if (raw_url.find('?') != raw_url.npos) {
            printf("\tinto case '?'\n");
            // to specify need to parse param in url
            auto tmp = split(raw_url, '?');
            int n_split;
#ifdef DEBUG
            for (auto &ele: tmp) {
                printf("ele: %s\n", ele.c_str());
            }
#endif

            // len == 2
            if ((n_split = tmp.size()) == 2) {
                route = tmp[0];
                url_param = tmp[1];
            } else if (n_split == 1) {
                /** because sometime,post a empty form can lead here, so the url parameter is none,
                 * so visit tmp[1] may crash. don't want this happen, so check */
                route = tmp[0];
                url_param = "";
            }

        } else {
#ifdef DEBUG
            printf("\tinto case without ?\n");
            std::cout << "raw_url=" << raw_url << ",after set_param" << std::endl;
#endif

            route = "";
            url_param = "";
        }
    }

    void
    _param_parser() {
        std::string K, V;
        std::vector<std::string> lines;
        try {
#ifdef DEBUG
            printf("raw url in req %s\n", _url_param.c_str());
#endif

            if (lines = split(_url_param, '&'), !lines.empty()) {
                for (auto &ele: lines) {
                    auto KV = split(ele, '=');
                    K = KV[0], V = KV[1];
                    std::cout << K << " " << V << std::endl;
                    __url_parsed_arg[K] = V;

                }
            }
        } catch (std::exception &e) {
            fprintf(stderr, "parse occurred error!\n");
            e.what();
        }
        return;
    }

};

typedef Request *RequestPtr;

#endif //TINYWEB_REQUEST_H
