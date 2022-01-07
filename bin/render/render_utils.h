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
#include "../../bin/concurrent/ThreadLocal.h"
#include "../../utils/helpfulUtils.h"

using namespace yumira;
template<class Url,class Parameter>
Url render_template(Url html_url, Parameter &KVMap);


#endif //TINYWEB_RENDER_UTILS_H
