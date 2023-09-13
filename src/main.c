#include "../include/br_net.h"
#include "br_net.h"

int main(int argc, char* argv[])
{
    WARN("Allocating");
    BrConnection* c = br_connection_new(BR_PROTOCOL_HTTP, argv[1], 0);
    WARN("Connecting");
    br_connect(c);
    WARN("Requesting");
    char msg[255];
    snprintf(msg, 255, "GET /%s HTTP/1.1\r\n\r\n", argv[2]);

    PRINT("REQUEST:%s",,msg);
    br_request(c, msg, strnlen(msg, 255));
    char* str;
    WARN("Resolving");
    size_t bytes = br_resolve(c, &str);
    printf("RESULT:%s\n", str);

    return 0;
}
