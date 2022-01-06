//
// Created by nekonoyume on 2022/1/2.
//

#ifndef TINYWEB_HELPFULUTILS_H
#define TINYWEB_HELPFULUTILS_H
#include <string>
#include <auto_ptr.h>
#include <memory>

class path;
typedef path* pathPtr;

class path {
private:
    std::string real_path;
public:


};

size_t GetFileSize(const std::string &file_name);
#define varName(x) #x
#define cat(x,y) x##y

using namespace std;


#endif //TINYWEB_HELPFULUTILS_H