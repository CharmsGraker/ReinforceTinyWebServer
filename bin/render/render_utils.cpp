#include "render_utils.h"
#include "../../utils/helpfulUtils.h"

std::string getcwd() {
    char buf[PATH_MAX];
    if (NULL == getcwd(buf, sizeof(buf))) {
        fprintf(stderr, "error %s", strerror(errno));
    }
    return std::string(buf);
}


auto parse_abs_url = [](const url_t &relative) {
    auto ptr_root_dir = yumira::current_app->configs.get<StringNode *>("template_root")->real_str;
    auto dir = *ptr_root_dir + relative.url;
    cout << "template_root dir: " << *ptr_root_dir << endl;
    cout << "parse template file at: " << dir << endl;
    return dir;
};


template<>
url_t
render_template(url_t html_url, parameter_t &KVMap) {
    cout << "into reander_template: " << html_url.url << endl;
    auto tpl = ctemplate::Template::GetTemplate(parse_abs_url(html_url), ctemplate::DO_NOT_STRIP);
    if (tpl) {
        try {
            printf("[INFO] hit template!\n");
            ctemplate::TemplateDictionary dict("temp_dict");
            std::string output;

            for (auto &KV: KVMap) {
                dict.SetValue(KV.first, KV.second); // the var want expand
                if (KV.first == "user") {

                    cout << "find user\n";
                    ThreadLocal::put("current_user", string(KV.second));
                }
            }

            if (tpl->Expand(&output, &dict)) {


                auto resRoot = current_app->configs.get<StringNode *>("template_relative_path")->real_str;
                auto project_root = current_app->configs.get<StringNode *>("app_root")->real_str;
                std::string rendered_file_abspath =
                        string(*project_root) + "/bin/.tpl/" + string(*resRoot) +
                        ThreadLocal::get("route") + "/" +
                        ThreadLocal::get("current_user") +
                        html_url.url;

                cout << "render template store path: " << rendered_file_abspath << endl;
                auto file_dirname = dirName((char *) rendered_file_abspath.c_str());
                printf("dirname: %s\n", file_dirname.c_str());
                if (mkdir(file_dirname) != 0) {
                    fprintf(stderr, "error when mkdir! %s\n", strerror(errno));
                }

                auto fd = open(rendered_file_abspath.c_str(), O_RDWR | O_CREAT, 0777);
                if (errno != 0) {
                    fprintf(stderr, "error when open! %s\n", strerror(errno));

                }
                write(html_url._fd = fd, output.c_str(), html_url._file_size = output.size());
                close(fd);
                printf("successfully expand KV.\n");
                fprintf(stdout, "%s\n", output.c_str());

                html_url.set_template_ptr(rendered_file_abspath);
            };
        }
        catch (std::exception &e) {
            return html_url;
        }
    }
    return html_url;
}


