//
// Created by nekonoyume on 2021/12/30.
//

#include<libxml/parser.h>
#include <assert.h>

char _tmp_resolved_path[PATH_MAX];

int
openXMlFile(const char *filename, xmlDocPtr *doc_ptr) {
    *doc_ptr = xmlParseFile(filename);
    if (*doc_ptr == NULL) {
        realpath(filename,_tmp_resolved_path);
        fprintf(stderr, "Failed to parse xml file: %s\n", _tmp_resolved_path);

    }
    return 0;
}

int
parse_server_conf_xml(const char *filename) {
    xmlDocPtr doc = NULL;   //xml整个文档的树形结构
    if (openXMlFile(filename, &doc) < 0) {
        realpath(filename,_tmp_resolved_path);

        fprintf(stderr,"err when open xml: %s",_tmp_resolved_path);
    }
    xmlNodePtr cur, root, config_node;  //xml节点
    xmlChar *name, *value;     //phone id


    root = xmlDocGetRootElement(doc);

    // attention: cur means current node like <config></config>
    for (cur = root->children; cur; cur = cur->next) {
#ifdef Y_DEBUG
        printf("%s\n", cur->name);
#endif
        config_node = cur;
        if (0 == xmlStrcasecmp(config_node->name, BAD_CAST "config")) {
            name = xmlGetProp(config_node, BAD_CAST "name");
            value = xmlGetProp(config_node, BAD_CAST "value");
            //printf("name=%s,value=%s\n", (char *) name, (char *) value);
            xmlFree(name);
            xmlFree(value);
        }
    }

    if (doc != NULL) {
        xmlFreeDoc(doc);
    }
    return -1;
}