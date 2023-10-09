#include "../include/br_net.h"
#include "../include/br_protocols.h"
#include "br_net.h"
#include "br_protocols.h"

int main(int argc, char* argv[])
{
    BrSession c;
    if (br_session_new(&c, argv[1])) {
        return 1;
    }
    printf(BR_SESSION_UNWRAP((&c)));
    br_connect(&c);
    char msg[4096];
    int written =
        c.protocol == BR_PROTOCOL_GEMINI
            ? snprintf(msg, 4096, "gemini://%s/%s\r\n", c.host, argv[2])
            : snprintf(msg, 4096, "GET /%s HTTP/1.1\r\n", argv[2]);
    if (c.protocol != BR_PROTOCOL_GEMINI)
        br_http_set_req_headers(c.host, msg + written, 4096 - written, true);
    PRINT("REQUEST: %s", , msg);
    BR_NET_STATUS status = br_request(&c, msg, strnlen(msg, 4096));
    if (!status)
        return 1;
    char* str;
    size_t bytes = br_resolve(&c, &str, true);
    if (c.protocol != BR_PROTOCOL_GEMINI) {
        BrHttpResponse http_resp;
        BR_PRT_STATUS status = br_http_response_new(&http_resp, str, bytes);
        if (!status)
            return status;
        printf("{%s}\n", http_resp.body);
        br_http_response_destroy(&http_resp);
        PRINT("START POLLING %d", , argc - 3);
        for (int i = 3; i < argc; i++) {
            bool keep = i < (argc - 1);
            int written = snprintf(msg, 4096, "GET /%s HTTP/1.1\r\n", argv[i]);
            br_http_set_req_headers(c.host, msg + written, 4096 - written,
                                    keep);
            PRINT("REQUEST:%s", , msg);
            br_request(&c, msg, strnlen(msg, 4096));
            char* str;
            size_t bytes = br_resolve(&c, &str, keep);
            BrHttpResponse r;
            BR_PRT_STATUS status = br_http_response_new(&r, str, bytes);
            if (!status)
                return status;
            printf(BR_HTTP_RESP_UNWRAP(&r));
            br_http_response_destroy(&r);
        }
    } else {
        BrGemResponse gem_resp;
        BR_PRT_STATUS status = br_gemini_response_new(&gem_resp, str, bytes);
        if (!status)
            return status;
        printf(BR_GEM_RESP_UNWRAP(&gem_resp));
        br_gemini_response_destroy(&gem_resp);
    }
    return 0;
}
