//
// Created by nekonoyume on 2022/1/3.
//

#ifndef TINYWEB_STRING_UTILS_H
#define TINYWEB_STRING_UTILS_H


#include <vector>
#include <string>
#include <sstream>

namespace string_util {
    static void _split(const std::string &s, char delim, std::vector<std::string> &elems);

    std::vector<std::string> split(const std::string &s, char delimiter);
}


#endif //TINYWEB_STRING_UTILS_H
