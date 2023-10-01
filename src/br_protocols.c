#include "../include/br_protocols.h"
#include "br_protocols.h"

#define BR_HTTP_HEADER_SIZE 1024

BR_PRT_STATUS __parse_headers(char* resp, size_t resp_s, BrHttpResponse* r);

void print_kv(gpointer key, gpointer value, gpointer user_data)
{
    printf("{%s:%s}\n", (char*)key, (char*)value);
}
BrHttpResponse* br_http_response_new(char* resp, size_t resp_s)
{
    BrHttpResponse* h_resp = calloc(1, sizeof(BrHttpResponse));
    if (__parse_headers(resp, resp_s, h_resp)) {
        free(h_resp);
        return NULL;
    }
    g_hash_table_foreach(h_resp->headers, print_kv, NULL);
    return h_resp;
}

BR_PRT_STATUS __parse_headers(char* resp, size_t resp_s, BrHttpResponse* r)
{
    char status_code_str[255];
    char* l_begin = resp;
    char* l_end = resp;
    if ((l_end = strstr(l_begin, "\r\n")) != NULL) {
        float _v;
        if (sscanf(l_begin, "HTTP/%f %d %s\r\n", &_v, &r->status_code, status_code_str) != 3) {
            return WARN(BR_PRT_HTTP_NO_STATUS_CODE);
        }
        printf("%s %d\n", status_code_str, r->status_code);
        l_begin = l_end + 2;
    } else
        return WARN(BR_PRT_HTTP_NO_STATUS_CODE);

    GHashTable* headers = g_hash_table_new(g_str_hash, g_str_equal);
    while ((l_end = strstr(l_begin, "\r\n")) != NULL) {
        if (l_end == l_begin) {
            r->headers = headers;
            r->body = l_end + 2;
            r->__full_text = resp;
            r->__full_text_s = resp_s;
            return BR_PRT_HTTP_OK;
        }

        char* k = calloc(BR_HTTP_HEADER_SIZE / 4, sizeof(char));
        char* v = calloc(BR_HTTP_HEADER_SIZE, sizeof(char));
        char* delimeter;
        if ((delimeter = strchr(l_begin, ':')) != NULL) {
            memcpy(k, l_begin, delimeter - l_begin);
            k[delimeter - l_begin] = 0;
            memcpy(v, delimeter + 2, l_end - delimeter - 2);
            v[l_end - delimeter - 2] = 0;
            g_hash_table_insert(headers, k, v);
        } else {
            WARN(BR_PRT_HTTP_INVALID_HEADER);
            free(k);
            free(v);
        }
        l_begin = l_end + 2;
    }
    return WARN(BR_PRT_HTTP_INVALID_HEADERS);
}

static void __http_response_destroy_kv(gpointer key, gpointer value, gpointer user_data)
{
    free(key);
    free(value);
}

void br_http_response_destroy(BrHttpResponse* r)
{
    g_hash_table_foreach(r->headers, __http_response_destroy_kv, NULL);
    g_hash_table_destroy(r->headers);
    free(r->__full_text);
}
