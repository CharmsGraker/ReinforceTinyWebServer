//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_RENDER_UTILS_H
#define TINYWEB_RENDER_UTILS_H

#include "ctemplate/template.h"

#include "../../concurrent/ThreadLocal.h"
#include "../../container/storage.h"

#include "../string_utils/helpfulUtils.h"

template<class Server>
class TemplateBinder {
    Server *const server;
public:
    TemplateBinder(Server *_server) : server(_server) {
        assert(server != nullptr);

    }

    template<class P>
    std::string
    renderTemplateOn(const std::string &url, P &p);

    std::string
    parse_abs_url(const std::string &);
};

#include <boost/filesystem.hpp>

template<class Server>
std::string
TemplateBinder<Server>::parse_abs_url(const std::string &relative_url) {
    auto ptr_root_dir = server->getConfigs().template get<StringNode *>("template_root")->real_str;
    auto dir = *ptr_root_dir + relative_url;
    cout << "template_root dir: " << *ptr_root_dir << endl;
    cout << "parse template file at: " << dir << endl;
    cout << "is file : " << boost::filesystem::is_regular_file(dir) << endl;
    return dir;
};


/** will call by user, so need get app inside the method */
template<class Server>
template<class Parameters>
std::string
TemplateBinder<Server>::renderTemplateOn(const std::string &html_url, Parameters &KVMap) {

    cout << "into reander_template: " << html_url << endl;
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
                string resRoot = "resources/root"; // server->getConfigs().template get<StringNode *>("template_relative_path")->real_str;
                cout << "resRoot=" << resRoot << endl;
                auto project_bin = server->getConfigs().template get<StringNode *>("app_bin")->real_str;
                std::string rendered_file_abspath =
                        string(*project_bin) + "/.tpl/" + string(resRoot) +
                        *ThreadLocal::getAs<std::string>("route") + "/" +
                        *ThreadLocal::getAs<std::string>("current_user") +
                        html_url;

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
//                ThreadLocal::put("template:htmlMappingFd", fd);
                ThreadLocal::put("template:fileSize", output.size());
                ThreadLocal::put("template:enable", 1);
                write(fd, output.c_str(), output.size());
                close(fd);
                printf("successfully expand KV.\n");
                fprintf(stdout, "%s\n", output.c_str());

                return rendered_file_abspath;
            };
        } catch (std::exception &e) {
            e.what();
            return html_url;
        }
    }
    return html_url;
}


#endif //TINYWEB_RENDER_UTILS_H
