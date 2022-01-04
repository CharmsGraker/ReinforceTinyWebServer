//
// Created by nekonoyume on 2022/1/3.
//

#include "string_utils.h"
namespace string_util {
    static void _split(const std::string &s, char delim, std::vector<std::string> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
    }

    std::vector<std::string> split(const std::string &s, char delimiter) {
        std::vector<std::string> elems;
        _split(s, delimiter, elems);
        return elems;
    }
}