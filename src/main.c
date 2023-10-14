#include "../include/br_net.h"
#include "../include/br_protocols.h"
#include "br_protocols.h"
#include "br_util.h"
#include <stdio.h>

static void _set_ssl()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}

int run(const char* uri, const char* page)
{
    BrSession c = {0};
    if (br_session_new(&c, uri)) {
        return 1;
    }
    printf(BR_SESSION_UNWRAP(&c));
    br_connect(&c);
    char msg[4096] = {};
    int written;
    switch (c.protocol) {
    case BR_PROTOCOL_HTTP:
    case BR_PROTOCOL_HTTPS:
        written = snprintf(msg, 4096, "GET %s HTTP/1.1\r\n", page);
        br_http_set_req_headers(c.host, msg + written, 4096 - written, true);
        break;
    case BR_PROTOCOL_GEMINI:
        written = snprintf(msg, 4096, "gemini://%s%s\r\n", c.host, page);
        break;
    case BR_PROTOCOL_GOPHER:
        written = snprintf(msg, 4096, "%s\r\n", page);
        break;
    default: return ERROR(BR_PROTOCOL_UNSUPPORTED);
    }

    printf(">> %s\n", msg);
    BR_NET_STATUS status = br_request(&c, msg, strnlen(msg, 4096));
    if (status)
        return 1;

    switch (c.protocol) {
    case BR_PROTOCOL_HTTP:
    case BR_PROTOCOL_HTTPS: {
        BrHttpResponse http_resp;
        BR_PRT_STATUS status = br_http_response_new(&c, &http_resp);
        if (status)
            return status;
        br_http_response_headers_print(http_resp.headers);
        printf(BR_HTTP_RESP_UNWRAP(&http_resp));
        br_http_response_destroy(&http_resp);
        break;
    }
    case BR_PROTOCOL_GEMINI: {
        BrGemResponse gem_resp = {0};
        BR_PRT_STATUS status = br_gem_response_new(&c, &gem_resp);
        if (status)
            return 1;
        printf(BR_GEM_RESP_UNWRAP(&gem_resp));
        br_gem_response_destroy(&gem_resp);
        break;
    }
    default: printf("RECEIVED: %s\n", c.resp);
    }
    br_close(&c);
    return 0;
}

int main(int argc, char* argv[])
{
    _set_ssl();
    if (argc > 1) {
        for (int i = 2; i < argc; i++)
            if (run(argv[1], argv[i]))
                WARN(ERROR_OCCURED);
        return 0;
    }
    char msg[MAX_URI_LENGTH] = {0};
    while (strncmp(msg, "q", 1) && strncmp(msg, "quit", 4)) {
        printf("> ");
        fgets(msg, 4096, stdin);
        if (!is_null_terminated(msg))
            msg[MAX_URI_LENGTH - 1] = 0;
        char* c;
        if ((c = strchr(msg, ' ')) != NULL) {
            *c = 0;
            char* k;
            if ((k = strchr(c + 1, '\n')) != NULL)
                *k = ' ';
            if (run(msg, c + 1))
                WARN(ERROR_OCCURED);
        }
    }

    return 0;
}
