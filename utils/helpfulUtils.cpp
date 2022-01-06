//
// Created by nekonoyume on 2022/1/2.
//

#include "helpfulUtils.h"
#include <stdlib.h>
#include <string>
#include <cassert>

using namespace std;


size_t GetFileSize(const std::string &file_name) {
    FILE *fp = fopen(file_name.c_str(), "r");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fclose(fp);
    return size; //单位是：byte
}

int
main() {

}