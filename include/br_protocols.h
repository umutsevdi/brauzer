/******************************************************************************

 * File: include/br_protocols.h
 * Author: Umut Sevdi
 * Created: 09/25/23
 * Description: Includes protocol specific functions

*****************************************************************************/
#pragma once

#include "../include/br_util.h"
#include "br_net.h"
#include "br_util.h"
#include <glib.h>
typedef enum {
    BR_PRT_HTTP_OK = 0,
    BR_PRT_HTTP_NO_STATUS_CODE,
    BR_PRT_HTTP_INVALID_HEADER,
    BR_PRT_HTTP_INVALID_HEADERS,
    BR_PRT_HTTP_ERROR_INVALID_PROTOCOL,
    BR_PRT_HTTP_ERROR_IP_NOT_FOUND,
    BR_PRT_HTTP_ERROR_URI_NOT_FOUND,
    BR_PRT_HTTP_ERROR_SOCKET_CREATION,
    BR_PRT_HTTP_ERROR_SSL_DISABLED,
    BR_PRT_HTTP_ERROR_SSL,
    BR_PRT_HTTP_ERROR_SSL_CONTEXT,
    BR_PRT_HTTP_ERROR_SSL_CONNECTION,
    BR_PRT_HTTP_ERROR_CONNECTION_FAILED,
    BR_PRT_HTTP_ERROR_SEND,
    BR_PRT_GEMINI_OK,
    BR_PRT_GEMINI_INVALID_HEADER,
    BR_PRT_GEMINI_NON_INPUT_POST_TEXT,
} BR_PRT_STATUS;

/******************************************************************************
                                  HTTP
*****************************************************************************/

typedef enum {
    BR_PRT_HTTP_GET,
    BR_PRT_HTTP_POST,
    BR_PRT_HTTP_PUT,
    BR_PRT_HTTP_DELETE
} BR_PRT_HTTP_TYPES;

typedef struct {
    int status_code;
    GHashTable* headers;
    char* body;
    char* __full_text;
    size_t __full_text_s;
} BrHttpResponse;

#define BR_HTTP_RESP_UNWRAP(r)                             \
    "BrHttpResponse{status:%d, body: %s, full_size: %ld}", \
        (r)->status_code,                                  \
        (r)->body,                                         \
        (r)->__full_text_s

/**
 * Parses the data from the connection and converts it into a HttpResponse
 */
BR_PRT_STATUS br_http_response_new(BrHttpResponse* h_resp, char* resp, size_t resp_s);
void br_http_response_destroy(BrHttpResponse* r);

/**
 * Appends the given HTTP Connection, fills the required headers
 * @buffer - buffer to append
 * @buffer_s - the size of the buffer
 * @keep - whether to keep the connection alive or not
 */
void br_http_set_req_headers(const char* host, char* buffer, size_t buffer_s, bool keep);

/******************************************************************************
                                  GEMINI
*****************************************************************************/

typedef enum {
    BR_GEMINI_RESP_INPUT = 10,
    BR_GEMINI_RESP_OK = 20,
    BR_GEMINI_RESP_REDIRECT = 30,
    BR_GEMINI_RESP_FAIL_TMP = 40,
    BR_GEMINI_RESP_FAIL_PERMA = 50,
    BR_GEMINI_RESP_CERT_REQ = 60,

} BR_GEMINI_RESP;

typedef struct {
    int status_number;
    BR_GEMINI_RESP status_code;
    char* mime;
    char* question;
    char* body;
    char* __full_text;
    size_t __full_text_s;
} BrGemResponse;
#define BR_GEM_RESP_UNWRAP(r)                                                \
    "BrGemResponse{status:%d,mime: %s,query: %s, body: %s, full_size: %ld}", \
        (r)->status_number,                                                  \
        (r)->mime,                                                           \
        (r)->question,                                                       \
        (r)->body,                                                           \
        (r)->__full_text_s

BR_PRT_STATUS br_gemini_response_new(BrGemResponse* gem_r, char* resp, size_t resp_s);
void br_gemini_response_destroy(BrGemResponse* r);
