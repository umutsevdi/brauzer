#include "../include/br_net.h"
#include "br_net.h"

int main(int argc, char* argv[])
{
    BrConnection* c = br_connection_new(argv[1]);
    if (c == NULL) {
        return 0;
    }
    br_protocol_print(c);
    br_connect(c);
    char msg[255];
    int written = snprintf(msg, 255, "GET /%s HTTP/1.1\r\n", argv[2]);
    //        snprintf(msg, 255, "gemini://%s/%s\r\n", argv[1], argv[2]);

    get_http_fields(c, msg + written, 255 - written);
    PRINT("REQUEST:%s", , msg);
    br_request(c, msg, strnlen(msg, 255));
    char* str;
    size_t bytes = br_resolve(c, &str);
    printf("RESULT[%lu]:%s\n", bytes, str);

    return 0;
}
