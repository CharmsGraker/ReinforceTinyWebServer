//
// Created by nekonoyume on 2022/1/2.
//

#include "helpfulUtils.h"
#include "sys/errno.h"
#include <string>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

using namespace std;


size_t GetFileSize(const std::string &file_name) {
    FILE *fp = fopen(file_name.c_str(), "r");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fclose(fp);
    return size; //单位是：byte
}


FILE *createFile(const char *filepath, bool not_exist) {
    auto dir_n = dirname((char *) filepath);
    cout << dir_n << endl;
    if (!check_folder(dir_n) && not_exist) {
        mkdir(dir_n);
    }
    return fopen(filepath, "w+");
}

int mkdir(const char *multi_dir) {
    char cmd[PATH_MAX];
    sprintf(cmd, "mkdir -p %s", multi_dir);
    system(cmd);
}

int mkdir(std::string multi_dir) {
    return mkdir(multi_dir.c_str());
}

bool check_folder(const char *folder_path) {
    return opendir(folder_path) == nullptr;
}

std::string dirName(const std::string& path) {
    size_t pos;
    if ((pos = path.rfind(std::string(basename((char *) path.c_str())))) != path.npos) {
        return path.substr(0, pos);
    }
}