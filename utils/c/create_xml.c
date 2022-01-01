//
// Created by nekonoyume on 2021/12/30.
//
#include<stdio.h>
#include<libxml/tree.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int createConfigRootNode(xmlDocPtr *doc, xmlDocPtr *ptr) {
    //Define document pointer
    *doc = xmlNewDoc(BAD_CAST "1.0");

    *ptr = (xmlDocPtr) xmlNewNode(NULL, BAD_CAST "configs"); // set root_node
    xmlDocSetRootElement(*doc, (xmlNodePtr) *ptr);

    return 0;
}

int addConfigNode(xmlNodePtr node_configs, char *name, char *value) {
    //Create child nodes directly in the root node
    xmlNodePtr node = xmlNewTextChild(node_configs, NULL, BAD_CAST "config", BAD_CAST "");

    xmlNewProp(node, BAD_CAST "name", BAD_CAST name);
    xmlNewProp(node, BAD_CAST "value", BAD_CAST value);
    return 0;
}

int create_default_config_xml(const char *filename) {
    xmlDocPtr config_root;
    xmlDocPtr doc;

    //Define node pointer
    createConfigRootNode(&doc, &config_root);

    //Set the root element of the document

    //Create child nodes directly in the root node
    addConfigNode(config_root, "port", "9007");
    addConfigNode(config_root, "db_name", "lightdb");
    addConfigNode(config_root, "db_user", "root");
    addConfigNode(config_root, "db_passwd", "root");

    //Dump an XML document to a file
    char __saved_xml_fname[128] = {0};

    //check extension
    char *__ext = filename + strlen(filename) - 4;

    snprintf(__saved_xml_fname, 128, "%s%s", filename, strcmp(".xml",__ext) != 0 ? ".xml" : "");


    int nRel = xmlSaveFile(__saved_xml_fname, doc);
    if (nRel >= 0) {
        //Free up all the structures used by a document,tree included
        xmlFreeDoc(doc);
        return 0;
    }

    // occur error,check errno
    char *err_msg = strerror(errno);
    fprintf(stderr, "[ERROR] errno=%d: %s\n", errno, err_msg);
    return -1;
}

int
create_config_xml(const char *filename) {
    xmlDocPtr config_root;
    xmlDocPtr doc;

    //Define node pointer
    createConfigRootNode(&doc, &config_root);

    //Set the root element of the document

    //Create child nodes directly in the root node


//
//    xmlNodePtr config2 = xmlNewTextChild(root_node, NULL, BAD_CAST "newnode2", BAD_CAST "");
//
//    //Create a new node
//    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "node2");
//    //Create a new text node
//    xmlNodePtr content = xmlNewText(BAD_CAST "NODE CONTENT");
//    //Add a new node to parent
//    xmlAddChild(root_node, node);
//    xmlAddChild(node, content);
//    //Create a new property carried by a node
//    xmlNewProp(node, BAD_CAST "attribute", BAD_CAST "yes");

//    //Create a son and grandson node element
//    node = xmlNewNode(NULL, BAD_CAST "son");
//    xmlAddChild(root_node, node);
//    xmlNodePtr grandson = xmlNewNode(NULL, BAD_CAST "grandson");
//    xmlAddChild(node, grandson);
//    xmlAddChild(grandson, xmlNewText(BAD_CAST "THis is a grandson node"));

    //Dump an XML document to a file
    char __saved_xml_fname[128] = {0};

    //check extension
    char *__ext = filename + strlen(filename) - 4;

    snprintf(__saved_xml_fname, 128, "%s%s", filename, __ext != ".xml" ? ".xml" : "");


    int nRel = xmlSaveFile(__saved_xml_fname, doc);
    if (nRel >= 0) {
        //Free up all the structures used by a document,tree included
        xmlFreeDoc(doc);
        return 0;
    }

    // occur error,check errno
    char *err_msg = strerror(errno);
    fprintf(stderr, "[ERROR] errno=%d: %s\n", errno, err_msg);
    return -1;
}

