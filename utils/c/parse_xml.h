//
// Created by nekonoyume on 2021/12/30.
//

#ifndef TINYWEB_PARSE_XML_H
#define TINYWEB_PARSE_XML_H

#include<libxml/parser.h>

int
parse_server_conf_xml(const char *filename);

int
openXMlFile(const char *filename, xmlDocPtr *doc_ptr);
#endif //TINYWEB_PARSE_XML_H
