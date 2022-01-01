//
// Created by nekonoyume on 2021/12/30.
//

#include <linux/limits.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "parse_xml.h"
#include "create_xml.h"

int main(int argc, char **argv) {
    char buf[PATH_MAX];
    if (NULL == getcwd(buf, sizeof(buf))) {
        fprintf(stderr, "error %s", strerror(errno));
    }
    printf("pwd: %s\n", buf);

    char *filename = "./../conf/server-config.xml";
    create_default_config_xml(filename);
    return 0;
}