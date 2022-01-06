//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_RENDER_UTILS_H
#define TINYWEB_RENDER_UTILS_H

#include "ctemplate/template.h"
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

#include "../../current_app.h"
#include "../UserMain.h"

using namespace yumira;
url_t render_template(url_t html_url, parameter_t &KVMap);

#endif //TINYWEB_RENDER_UTILS_H
