#include <string>
#include <stdint.h>
#include "openssl/hmac.h"
#include "openssl/evp.h"     //使用openssl
#include "../Base64Encoder.h"
#include<algorithm>
#include<string>
#include<iostream>
#include <cstring>

int main(int argc, char **argv) {

    //注意密钥
    std::string secret_key = "ee394b990568d08a";
    // playload
    const char str[] = "eyJhbGciOiJIUzUxMiJ9.eyJzdWIiOiJRTVVMVVhBQSIsInNjb3BlIjpbIlJPTEVfQ0xJRU5UIl0sImlzcyI6Imh0dHA6Ly9za3lsaWdodC5jb20uaGsiLCJpZCI6ImZmNTdjOWUzLWI4YTItNGVkYi05MDUzLTc1MzMxZTg4ZjQ4YSIsImV4cCI6MTU1MzA1MTI5NSwiaWF0IjoxNTUzMDQ3Njk1fQ";

    HMAC_CTX *mdctx = HMAC_CTX_new();

    const EVP_MD *evpmd = NULL;
    // header
    evpmd = EVP_sha512();     //算法

    HMAC_CTX_reset(mdctx);

    HMAC_Init_ex(mdctx, secret_key.c_str(), secret_key.size(), evpmd, NULL);

    unsigned char *outdate = new unsigned char(EVP_MAX_MD_SIZE);

    unsigned int size = 0;

    HMAC_Update(mdctx, (const unsigned char *) str, strlen(str));

    HMAC_Final(mdctx, outdate, &size);

    HMAC_CTX_free(mdctx);

    for (int i = 0; i < size; i++) {
        printf("%x", (unsigned int) outdate[i]);
    }

    printf("\n");

    //注意：此时得到的outdate并非是token中的sign，还需要继续处理，如下：

    auto encoded = Base64Encode(outdate, size);   //使用Base64Encode

    printf("encode: %s\n", encoded.c_str());

    std::string signature = encoded;

    // for using in url
    replace(signature.begin(), signature.end(), '+', '-');
    replace(signature.begin(), signature.end(), '/', '_');

    std::string::iterator iter;
    // remove '='
    for (iter = signature.begin(); iter != signature.end(); iter++)
        if (*iter == '=') {
            signature.erase(iter);
            iter--;
        }
///
    printf("[signature]: %s\n", signature.c_str());

    int cmpRet = 0;
    cmpRet = strncmp("Em8HJJM2vCK2bqYD5qw-Czxz__hYuOw-DiDHYZPxmzH5clYIAFJ9WUgciihdbps8Fmm88gspYFoHqYRz8X5BfA",
                       signature.c_str(), signature.size());
    //字符串比较相等，鉴权也就通过了，签名OK
    printf("l_success: %d\n", cmpRet);

    return 0;
}