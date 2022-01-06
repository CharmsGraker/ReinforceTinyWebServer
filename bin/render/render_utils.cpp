#include "render_utils.h"



std::string getcwd() {
    char buf[PATH_MAX];
    if (NULL == getcwd(buf, sizeof(buf))) {
        fprintf(stderr, "error %s", strerror(errno));
    }
    return std::string(buf);
}


auto parse_abs_url = [](url_t relative) {
    extern std::map<std::string, std::string> configs;
    auto root_dir = yumira::current_app->configs.get<string>("template_root");
    return root_dir + "/" + relative.url;
};

url_t
render_template(url_t html_url, parameter_t &KVMap) {
    auto tpl = ctemplate::Template::GetTemplate(parse_abs_url(html_url), ctemplate::DO_NOT_STRIP);
    if (tpl) {
        try {
            ctemplate::TemplateDictionary dict("temp_dict");
            std::string output;

            for (auto &KV: KVMap) {
                dict.SetValue(KV.first, KV.second); // the var want expand
            }

//            if (tpl->Expand(&output, &dict)) {
//                printf("successfully expand KV.\n");
//                fprintf(stdout, "%s\n", output.c_str());
//                auto resRoot = current_app.configs.get<const char *>("template_root");
//
//                string rendered_file_abspath = string(resRoot) + "/" + current_app.route.getRoute();
//
//                auto fd = open(rendered_file_abspath.c_str(), O_RDWR);
//                write(fd, output.c_str(), sizeof(output.c_str()));
//                html_url.set_template_ptr(const_cast<char *>(rendered_file_abspath));
//            };
        }
        catch (std::exception &e) {
            return html_url;
        }
    }
    return html_url;
}


