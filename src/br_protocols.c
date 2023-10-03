#include "../include/br_protocols.h"
#include "br_protocols.h"

/******************************************************************************
                                  HTTP
*****************************************************************************/

#define BR_HTTP_HEADER_SIZE 1024
#define _BR_VERSION "0.9"
#define H_LANGUAGE "Accept-Language: en-US,en;q=0.9\r\n"
#define H_CONN "Connection: keep-alive\r\n"
#define H_CONN_CLOSE "Connection: close\r\n"
#define H_ACCEPT "Accept: text/html,application/" \
                 "xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
/* Some language servers capture the text above as beginning of a comment,
 * so this text is here*/

/**
 * Parses the given response buffer, sets headers and returns BR_PRT_HTTP_OK
 * if the process is successful
 */
static BR_PRT_STATUS _parse_http_headers(char* resp, size_t resp_s, BrHttpResponse* r);
/**
 * Parses the HTML body to find subsequent links to pull
 */
BR_PRT_STATUS _get_links(BrHttpResponse* r);

BrHttpResponse* br_http_response_new(char* resp, size_t resp_s)
{
    BrHttpResponse* h_resp = calloc(1, sizeof(BrHttpResponse));
    if (_parse_http_headers(resp, resp_s, h_resp)) {
        free(h_resp);
        return NULL;
    }
    return h_resp;
}

static void _http_response_destroy_kv(gpointer key, gpointer value, gpointer user_data)
{
    free(key);
    free(value);
}

void br_http_response_destroy(BrHttpResponse* r)
{
    g_hash_table_foreach(r->headers, _http_response_destroy_kv, NULL);
    g_hash_table_destroy(r->headers);
    free(r->__full_text);
    free(r);
}

static BR_PRT_STATUS _parse_http_headers(char* resp, size_t resp_s, BrHttpResponse* r)
{
    char status_code_str[255];
    char* l_begin = resp;
    char* l_end = resp;
    if ((l_end = strstr(l_begin, "\r\n")) != NULL) {
        float _v;
        if (sscanf(l_begin,
                "HTTP/%f %d %s\r\n", &_v, &r->status_code, status_code_str)
            != 3) {
            return ERROR(BR_PRT_HTTP_NO_STATUS_CODE);
        }
        printf("%s %d\n", status_code_str, r->status_code);
        l_begin = l_end + 2;
    } else
        return ERROR(BR_PRT_HTTP_NO_STATUS_CODE);

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
    return ERROR(BR_PRT_HTTP_INVALID_HEADERS);
}

BR_PRT_STATUS _get_links(BrHttpResponse* r)
{
    return 0;
}

void br_http_set_req_headers(const char* host, char* buffer, size_t buffer_s, bool keep)
{
    char u_agent[4096];

    char os[1024];
    get_os(os, 1024);
    snprintf(u_agent, sizeof(u_agent),
        "User-Agent: Mozilla/5.0 (%s) Gecko/20100101 Brauzer/%s\r\n",
        os, _BR_VERSION);
    if (keep) {
        snprintf(buffer, buffer_s, "Host: %s\r\n" H_ACCEPT H_LANGUAGE H_CONN "%s\r\n",
            host, u_agent);
    } else {
        snprintf(buffer, buffer_s, "Host: %s\r\n" H_ACCEPT H_LANGUAGE H_CONN_CLOSE "%s\r\n", host, u_agent);
    }
}

/******************************************************************************
                                  GEMINI
*****************************************************************************/

static BR_PRT_STATUS _parse_gem_headers(char* resp, size_t resp_s, const char* endptr, BrGemResponse* r);

BrGemResponse* br_gemini_response_new(char* resp, size_t resp_s)
{
    BrGemResponse* h_resp = calloc(1, sizeof(BrGemResponse));
    char* l_begin = resp;
    char* l_end = resp;
    if ((l_end = strstr(l_begin, "\r\n")) != NULL) {
        BR_PRT_STATUS status = _parse_gem_headers(resp, resp_s, l_end + 2, h_resp);
        if (status == BR_PRT_GEMINI_OK)
            return h_resp;
    }
    br_gemini_response_destroy(h_resp);
    WARN(BR_PRT_GEMINI_INVALID_HEADER);
    return NULL;
}

void br_gemini_response_destroy(BrGemResponse* r)
{
    free(r->__full_text);
    free(r->mime);
    free(r);
}

static BR_PRT_STATUS _parse_gem_headers(char* resp, size_t resp_s, const char* endptr, BrGemResponse* r)
{
    const int max_len = endptr - resp;
    char* str = resp;
    int i = 0, parse_idx = 0;
    while (i < max_len) {
        if (resp[i] == ' ') {
            char code_arr[2];
            switch (parse_idx) {
            case 0: /* Parse return code */
                memcpy(code_arr, str, 2);
                r->status_number = strtol(code_arr, NULL, 10);
                r->status_code = (r->status_number / 10) * 10;
                parse_idx++;
                str = resp + i + 1;
                break;
            case 1: /* Parse file name */
                r->mime = strndup(str, i - (str - resp + 1));
                str = resp + i + 1;
                break;
            case 2:
                if (r->status_code != BR_GEMINI_RESP_INPUT) {
                    return ERROR(BR_PRT_GEMINI_NON_INPUT_POST_TEXT);
                }
                r->question = strndup(str, i - (str - resp + 1));
                break;
            }
        }
        i++;
    }
    r->body = resp + i;
    r->__full_text = resp;
    r->__full_text_s = resp_s;
    return BR_PRT_GEMINI_OK;
}
