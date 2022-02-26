//
// Created by nekonoyume on 2022/1/5.
//

#ifndef TINYWEB_RENDER_UTILS_H
#define TINYWEB_RENDER_UTILS_H

#include "ctemplate/template.h"

#include "../concurrent/ThreadLocal.h"
#include "../container/storage.h"

#include "../../utils/helpfulUtils.h"

template<class Server>
class TemplateBinder {
    Server * const server;
public:
    TemplateBinder(Server *_server) : server(_server) {
        assert(server!= nullptr);

    }

    template<class U, class P>
    U
    renderTemplateOn(U u, P &p);

    template<class Url>
    std::string
    parse_abs_url(const Url &);
};


template<class Server>
template<class Url>
std::string
TemplateBinder<Server>::parse_abs_url(const Url &relative) {
    printf("server=%d\n",server);
    auto ptr_root_dir = server->getConfigs().template get<StringNode *>("template_root")->real_str;
    auto dir = *ptr_root_dir + relative.url;
    cout << "template_root dir: " << *ptr_root_dir << endl;
    cout << "parse template file at: " << dir << endl;
    return dir;
};


/** will call by user, so need get app inside the method */
template<class Server>
template<class Url, class Parameters>
Url
TemplateBinder<Server>::renderTemplateOn(Url html_url, Parameters &KVMap) {

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


                auto resRoot = server->getConfigs().template get<StringNode *>("template_relative_path")->real_str;
                auto project_root = server->getConfigs().template get<StringNode *>("app_root")->real_str;
                std::string rendered_file_abspath =
                        string(*project_root) + "/bin/.tpl/" + string(*resRoot) +
                        ThreadLocal::getAs<std::string>("route") + "/" +
                        ThreadLocal::getAs<std::string>("current_user") +
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


#endif //TINYWEB_RENDER_UTILS_H
