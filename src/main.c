#include "../include/br_net.h"
#include "../include/br_protocols.h"
#include "br_net.h"
#include "br_protocols.h"

int main(int argc, char* argv[])
{
    BrConnection* c = br_connection_new(argv[1]);
    if (c == NULL) {
        return 0;
    }
    br_protocol_print(c);
    br_connect(c);
    char msg[4096];
    int written = br_protocol(c) == BR_PROTOCOL_GEMINI
        ? snprintf(msg, 255, "gemini://%s/%s\r\n", argv[1], argv[2])
        : snprintf(msg, 4096, "GET /%s HTTP/1.1\r\n", argv[2]);

    if (br_protocol(c) != BR_PROTOCOL_GEMINI)
        get_http_fields(c, msg + written, 4096 - written, true);
    PRINT("REQUEST:%s", , msg);
    br_request(c, msg, strnlen(msg, 4096));
    char* str;
    size_t bytes = br_resolve(c, &str, true);
   // printf("RESULT[%lu]:%s\n", bytes, str);
    BrHttpResponse* http_resp = br_http_response_new(str, bytes);
    br_http_response_destroy(http_resp);
//    free(str);
    printf("AFTER_FULL_TEXT: %s", str);
    PRINT("START POLLING %d", , argc - 3);
    for (int i = 3; i < argc; i++) {
        bool keep = i < (argc - 1);
        PRINT("%d %d %d", , i, argc, keep);
        int written = snprintf(msg, 4096, "GET /%s HTTP/1.1\r\n", argv[i]);
        get_http_fields(c, msg + written, 4096 - written, keep);
        PRINT("REQUEST:%s", , msg);
        br_request(c, msg, strnlen(msg, 4096));
        char* str;
        size_t bytes = br_resolve(c, &str, keep);
        BrHttpResponse * r = br_http_response_new(str, bytes);
        printf("RESULT[%lu]:%s\n", bytes, str);
        br_http_response_destroy(r);
//        free(str);
    }
    return 0;
}
