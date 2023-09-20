#include "../include/br_net.h"
#include "br_net.h"

int main(int argc, char* argv[])
{
    WARN("Allocating");
    BrConnection* c = br_connection_new(BR_PROTOCOL_HTTP, argv[1], 0);
    if (c == NULL) {
        return 0;
    }
    WARN("Connecting");
    br_connect(c);
    WARN("Requesting");
    char msg[255];
    int written = snprintf(msg, 255, "GET /%s HTTP/1.1\r\n", argv[2]);
    get_http_fields(c, msg + written, 255 - written);

    PRINT("REQUEST:%s", , msg);
    br_request(c, msg, strnlen(msg, 255));
    char* str;
    WARN("Resolving");
    size_t bytes = br_resolve(c, &str);
    printf("RESULT[%lu]:%s\n", bytes, str);

    return 0;
}
