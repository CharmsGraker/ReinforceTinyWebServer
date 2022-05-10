//
// Created by nekonoyume on 2022/1/2.
//

#ifndef TINYWEB_HELPFULUTILS_H
#define TINYWEB_HELPFULUTILS_H

#include <string>
#include <memory>
#include "fcntl.h"


#include <stdlib.h>
#include <string>
#include <iostream>
#include <climits>
#include <memory>
#include <linux/limits.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

class path;

typedef path *pathPtr;

class path {
private:
    std::string real_path;
public:


};

size_t GetFileSize(const std::string &file_name);

#define varName(x) #x
#define cat(x, y) x##y

using namespace std;

FILE *createFile(const char *filepath, bool not_exist = true);

std::string dirName(const std::string& path);
int mkdir(const char *multi_dir);
int mkdir(std::string multi_dir);


bool check_folder(const char * folder_path);

#endif //TINYWEB_HELPFULUTILS_H