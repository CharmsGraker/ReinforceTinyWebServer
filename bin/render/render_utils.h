//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_RENDER_UTILS_H
#define TINYWEB_RENDER_UTILS_H

#include "ctemplate/template.h"


#include "../../current_app.h"


using namespace yumira;
template<class Url,class Parameter>
Url render_template(Url html_url, Parameter &KVMap);


#endif //TINYWEB_RENDER_UTILS_H
