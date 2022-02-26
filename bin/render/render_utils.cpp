#include "render_utils.h"


std::string getcwd() {
    char buf[PATH_MAX];
    if (NULL == getcwd(buf, sizeof(buf))) {
        fprintf(stderr, "error %s", strerror(errno));
    }
    return std::string(buf);
}