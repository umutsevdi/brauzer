#include "../include/br_protocols.h"
#include "br_net.h"
#include "br_protocols.h"
#include "br_util.h"
#define MEMMOVE(s, r)                                                          \
    r = s;                                                                     \
    r##_s = s##_s;                                                             \
    s = NULL;                                                                  \
    s##_s = 0

/******************************************************************************
                                  HTTP
*****************************************************************************/

#define BR_HTTP_HEADER_SIZE 1024
#define _BR_VERSION "0.9"
#define H_LANGUAGE "Accept-Language: en-US,en;q=0.9\r\n"
#define H_CONN "Connection: keep-alive\r\n"
#define H_CONN_CLOSE "Connection: close\r\n"
#define H_ACCEPT                                                               \
    "Accept: text/html,application/"                                           \
    "xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n" /**/

/**
 * Parses the HTML body to find subsequent links to pull
 */
BR_PRT_STATUS _get_links(BrHttpResponse* r);

BR_PRT_STATUS br_http_response_new(BrSession* s, BrHttpResponse* h_resp)
{
    MEMMOVE(s->req, h_resp->req);
    char status_code_str[255];
    char* l_begin = s->resp;
    char* l_end = s->resp;
    if ((l_end = strstr(l_begin, "\r\n")) != NULL) {
        float _v;
        if (sscanf(l_begin, "HTTP/%f %d %s\r\n", &_v, &h_resp->status_code,
                   status_code_str)
            != 3) {
            return ERROR(BR_PRT_HTTP_NO_STATUS_CODE);
        }
        l_begin = l_end + 2;
    } else
        return ERROR(BR_PRT_HTTP_NO_STATUS_CODE);
    GHashTable* headers = g_hash_table_new(g_str_hash, g_str_equal);
    while ((l_end = strstr(l_begin, "\r\n")) != NULL) {
        if (l_end == l_begin) {
            h_resp->headers = headers;
            h_resp->body = l_end + 2;
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

static void _http_response_print_kv(gpointer key, gpointer value,
                                    gpointer user_data)
{
    printf("%s:%s\n", (char*)key, (char*)value);
}

void br_http_response_headers_print(GHashTable* t)
{
    g_hash_table_foreach(t, _http_response_print_kv, NULL);
}

static void _http_response_destroy_kv(gpointer key, gpointer value,
                                      gpointer user_data)
{
    free(key);
    free(value);
}

void br_http_response_destroy(BrHttpResponse* r)
{
    g_hash_table_foreach(r->headers, _http_response_destroy_kv, NULL);
    g_hash_table_destroy(r->headers);
    if (r->__full_text != NULL)
        free(r->__full_text);
    memset(r, 0, sizeof(BrHttpResponse));
}

BR_PRT_STATUS _get_links(BrHttpResponse* r) { return 0; }

void br_http_set_req_headers(const char* host, char* buffer, size_t buffer_s,
                             bool keep)
{
    char u_agent[4096];
    char os[1024];
    get_os(os, 1024);
    snprintf(u_agent, sizeof(u_agent),
             "User-Agent: Mozilla/5.0 (%s) Gecko/20100101 Brauzer/%s\r\n", os,
             _BR_VERSION);
    if (keep) {
        snprintf(buffer, buffer_s,
                 "Host: %s\r\n" H_ACCEPT H_LANGUAGE H_CONN "%s\r\n", host,
                 u_agent);
    } else {
        snprintf(buffer, buffer_s,
                 "Host: %s\r\n" H_ACCEPT H_LANGUAGE H_CONN_CLOSE "%s\r\n", host,
                 u_agent);
    }
}
/******************************************************************************
 *                                GEMINI
 *****************************************************************************/

static BR_PRT_STATUS _parse_gem_headers(const char* endptr, BrGemResponse* r);
BR_PRT_STATUS _br_gem_get_data(BrSession* s, BrGemResponse* r);

BR_PRT_STATUS br_gem_response_new(BrSession* s, BrGemResponse* r)
{
    MEMMOVE(s->req, r->req);
    r->__full_text = s->resp;
    r->__full_text_s = s->resp_s;
    r->header = NULL;
    char* l_begin = s->resp;
    char* l_end;
    if ((l_end = strstr(l_begin, "\r\n")) != NULL) {
        BR_PRT_STATUS status = _parse_gem_headers(l_end + 2, r);
        if (status == BR_PRT_GEM_STATUS_REQUEST_BODY) {
            status = br_gem_poll(s, r);
            /*TODO potential infinite loop  */
        }
        return status;
    }
    br_gem_response_destroy(r);
    return ERROR(BR_PRT_GEM_ERROR_INVALID_HEADER);
}

void br_gem_response_destroy(BrGemResponse* r)
{
    if (r->req != NULL)
        free(r->req);
    if (r->__full_text != NULL)
        free(r->__full_text);
    if (r->header != NULL)
        free(r->header);
    memset(r, 0, sizeof(BrGemResponse));
}

static BR_PRT_STATUS _parse_gem_headers(const char* endptr, BrGemResponse* r)
{
    const size_t max_len = endptr - r->__full_text;
    char* str = r->__full_text;

    /* Parse the protocol number */
    char code_arr[3] = {0};
    memcpy(code_arr, str, 2);
    r->status_number = strtol(code_arr, NULL, 10);
    r->status_code = (BR_GEMINI_RESP)r->status_number;

    str += 3;
    /* Parse the remaining value to the header */
    r->header = strndup(str, max_len - 5);
    r->header[max_len - 5] = 0;
    r->body = r->__full_text + max_len;
    r->body_s = r->__full_text_s - max_len;
    return r->__full_text_s == max_len ? BR_PRT_GEM_STATUS_REQUEST_BODY
                                       : BR_PRT_GEM_OK;
}

BR_PRT_STATUS br_gem_poll(BrSession* s, BrGemResponse* r)
{
    char msg[MAX_URI_LENGTH];
    switch (r->status_code) {
    case BR_GEMINI_RESP_SUCCESS:
        if (br_request(s, "", 0))
            return ERROR(BR_PRT_GEM_ERROR_POLL_BODY);
        br_gem_response_new(s, r);
        break;
    case BR_GEMINI_RESP_INPUT:
        memcpy(msg, r->req, r->req_s - 2);
        memcpy(msg + r->req_s - 2, "?dogs\r\n", 7);
        char* uri = strdup(s->__uri);
        br_close(s);
        br_gem_response_destroy(r);
        if (br_session_new(s, uri) || br_connect(s)
            || br_request(s, msg, strlen(msg))) {
            free(uri);
            return ERROR(BR_PRT_GEM_ERROR_INVALID_INPUT);
        }
        free(uri);
        br_gem_response_new(s, r);
        break;
    case BR_GEMINI_RESP_REDIRECT_TEMPORARY:
    case BR_GEMINI_RESP_REDIRECT_PERMANENT:
        if (!r->header)
            return ERROR(BR_PRT_GEM_ERROR_INVALID_HEADER);
        br_close(s);
        br_gem_response_destroy(r);
        if (br_session_new(s, s->__uri) || br_connect(s))
            return ERROR(BR_PRT_GEM_ERROR_REDIRECT);
        const char* req = to_abs_path(s->__uri, r->header);
        if (req == NULL || br_request(s, req, strlen(req)))
            return ERROR(BR_PRT_GEM_ERROR_REDIRECT);
        br_gem_response_new(s, r);
        break;
    default: break;
    }
    return BR_PRT_GEM_OK;
}

/******************************************************************************
                                GOPHER
*****************************************************************************/

void br_gph_response_new(BrSession* s, BrGopherResponse* r)
{
    MEMMOVE(s->req, r->req);
    MEMMOVE(s->resp, r->body);
}

void br_gph_response_destroy(BrGopherResponse* r)
{
    free(r->body);
    free(r->req);
    memset(r, 0, sizeof(BrGopherResponse));
}
