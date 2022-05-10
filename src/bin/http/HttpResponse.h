//
// Created by nekonoyume on 2022/4/10.
//

#ifndef TINYWEB_HTTPRESPONSE_H
#define TINYWEB_HTTPRESPONSE_H

#include <string>
#include <unordered_map>

class HttpResponse {
    std::unordered_map<std::string, std::string> mp;


public:
    auto
    clear() {

        return mp.clear();
    }

    HttpResponse() : mp() {

    }


    std::string
    get(const std::string &key_) {
        return mp[key_];
    }



    bool
    addLocation(const std::string &loc, bool cache_ = false) {
        if (!cache_) {
            auto cache_control_value = get("Cache-Control");
            put("Cache-Control", cache_control_value + "no-cache=" + loc);
        }
        return put("Location", loc);
    }

    std::string
    render();

protected:
    bool put(const std::string &key, const std::string &value) {
        printf("[Response] add Response segment\n");

        auto it = mp.find(key);
        if (it != mp.end()) {
            printf("exist, addResponse done.\n");
            return false;
        } else {
            mp[key] = value;
            printf("addResponse %s=%s\n", key.c_str(), value.c_str());
            return true;
        }
    }

public:
    ~HttpResponse() {

    }
};

#endif //TINYWEB_HTTPRESPONSE_H
