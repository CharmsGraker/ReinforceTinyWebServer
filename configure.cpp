#include "configure.h"
#include "utils/c/parse_xml.h"
#include <cassert>
#include <libxml/parser.h>
#include <getopt.h>
#include <cstring>

class IOException : exception {

};

Configure::Configure() {
    assert(_init_store_container() >= 0);
    setDefaultProp();
    __verbose = 1;

}

int
Configure::_init_store_container() {
    tab = new unordered_map<string, string>();
    return 0;
}

typedef std::pair<std::string, std::string> PAIR_CC;

void
Configure::setDefaultProp() {
    assert(tab != nullptr);

    //端口号,默认9006
    setProp("port", "9006");

    //日志写入方式，默认同步
    setProp("logWrite", "0");

    //触发组合模式,默认listenfd LT + connfd LT
    setProp("triggerMode", "0"); //TRIGMode = 0;

    //listenfd触发模式，默认LT
    setProp("listenTriggerMode", "0");


    //connfd触发模式，默认LT
    setProp("connectfdTriggerMode", "0");

    //优雅关闭链接，默认不使用
    setProp("lingerOption", "0");

    //数据库连接池数量,默认8
    setProp("sqlConPoolSize", "8");

    //线程池内的线程数量,默认8
    setProp("httpConnPoolSize", "8");

    //关闭日志,默认不关闭
    setProp("disableLogger", "0");

    //并发模型,默认是proactor
    setProp("concurrentActor", "0");
};

class NullPointerException : runtime_error {
public:
    explicit NullPointerException(char *msg) : runtime_error(msg) {};

};

template<typename T>
void checkNull(T x) {
    if (x == NULL || !x) {
        throw NullPointerException((char *) "[WARN] checkNull Failed!");
    }
}

string Configure::getProp(const char *propName) {
    try {
        checkNull(tab);

        bool flg_empty = false;
        bool flg_countLess = false;
        // printf("%ld", tab->size());

        FLUSH_STDOUT();
        if (tab->empty() || tab->count(propName) <= 0) {
            fprintf(stderr, "error in Config::getProp(), propName=%s\n", propName);
            if (flg_countLess) fprintf(stderr, "%s\n", "count <= 0 ");
            throw IOException();
        }
        return (string) (*tab)[propName];

    } catch (NullPointerException &e) {
        fprintf(stderr, "Config tab is null!\n");
    }
}

template<>
int
Configure::getPropOf<int>(const char *propName) {
    return atoi(getProp(propName).c_str());
}

int Configure::setProp(const char *attr, const char *val) {
    try {
        checkNull(tab);
        (*tab)[attr] = val;
        return 0;
    } catch (NullPointerException &e) {
        return -1;
    }
}

int Configure::loadConfigFromXml(const char *file_path) {
    xmlDocPtr doc = nullptr;   //xml整个文档的树形结构
    if (openXMlFile(file_path, &doc) < 0) {
        printf("err when open %s", file_path);
    }

    xmlNodePtr cur, root, config_node;  //xml节点
    xmlChar *name, *value;     //phone id


    root = xmlDocGetRootElement(doc);

    // attention: cur means current node like <config></config>
    for (cur = root->children; cur; cur = cur->next) {

        config_node = cur;
        if (0 == xmlStrcasecmp(config_node->name, BAD_CAST "config")) {
            name = xmlGetProp(config_node, BAD_CAST "name");
            value = xmlGetProp(config_node, BAD_CAST "value");

            setProp(reinterpret_cast<const char *>(name), reinterpret_cast<const char *>(value));
            //printf("name=%s,value=%s\n", (char *) name, (char *) value);
            xmlFree(name);
            xmlFree(value);
        }
    }

    if (doc != nullptr) {
        xmlFreeDoc(doc);
    }
    return -1;


}


void Configure::parse_arg(int argc, char *argv[]) {
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 'p': {
                setProp("port", optarg);
                break;
            }
            case 'l': {
                setProp("logWrite", optarg);
                break;
            }
            case 'm': {
                setProp("triggerMode", optarg);
                break;
            }
            case 'o': {
                OPT_LINGER = atoi(optarg);
                break;
            }
            case 's': {
                sql_num = atoi(optarg);
                break;
            }
            case 't': {
                thread_num = atoi(optarg);
                break;
            }
            case 'c': {
                close_log = atoi(optarg);
                break;
            }
            case 'a': {
                actor_model = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
}

void Configure::gen_conf(int argc, char *argv[]) {
    // pwd top workspace
    loadConfigFromXml("./conf/server-config.xml");

    printf("[INFO] successfully load conf from xml...\n");
    parse_arg(argc, argv);
    if (__verbose) {
        printf("[INFO] all conf:\n");
        for (auto const &ele: *tab) {
            printf("\t%s: %s\n", ele.first.c_str(), ele.second.c_str());
        }
    }

}
