//
// Created by nekonoyume on 2022/4/6.
//

#ifndef TINYWEB_HTTPDECODER_H
#define TINYWEB_HTTPDECODER_H
#include <string>
#include <cstring>

// parse http message
class HttpDecoder {
public:
    HttpDecoder(char * rawBuffer):rawBuffer_(rawBuffer) {

    }

    // return 0 on success
    int
    getRequestLine(char *text)  {
        auto m_url = strpbrk(text, " \t");
            if (!m_url) {
                return BAD_REQUEST;
            }
            *m_url++ = '\0';
            // set to head
//    printf("%s\n", m_url);

            char *method = text;
            if (strcasecmp(method, "GET") == 0) {
//        printf("GET request arrived\n");
                m_method = GET;
            } else if (strcasecmp(method, "POST") == 0) {
//        printf("POST request arrived\n");
                m_method = POST;
//                cgi = 1;
            } else
                return BAD_REQUEST;
            m_url += strspn(m_url, " \t");


            if (!(m_version = _getHtmlVersion(m_url))) {
                return BAD_REQUEST;
            }

            return NO_REQUEST;

    }


private:
    const char * rawBuffer_;
// status
private:
    static int NO_REQUEST;
    static int BAD_REQUEST;
};
#endif //TINYWEB_HTTPDECODER_H
