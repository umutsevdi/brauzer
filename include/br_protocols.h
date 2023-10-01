/******************************************************************************

 * File: include/br_protocols.h
 * Author: Umut Sevdi
 * Created: 09/25/23
 * Description: Includes protocol specific functions

*****************************************************************************/
#pragma once
#include "br_util.h"
#include <glib.h>
#include <stdlib.h>
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
    BR_PRT_HTTP_ERROR_SEND

} BR_PRT_STATUS;
typedef enum {
    BR_PRT_HTTP_GET,
    BR_PRT_HTTP_POST,
    BR_PRT_HTTP_PUT,
    BR_PRT_HTTP_DELETE
} BR_PRT_HTTP_TYPES;

typedef enum {
    BR_PRT_PRT_GET,
    BR_PRT_PRT_POST,
    BR_PRT_PRT_PUT,
    BR_PRT_PRT_DELETE
} BR_PRT_GEMINI_TYPES;

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
    const char* meta;
    char* buffer;
    size_t buffer_s;
    size_t buffer_bytes;
} BrGemResponse;

typedef struct {
    int status_code;
    GHashTable* headers;
    char* body;
    char* __full_text;
    size_t __full_text_s;
} BrHttpResponse;

/**
 * Parses given HTTP response
 */
BrHttpResponse* br_http_response_new(char* resp, size_t resp_s);
void br_http_response_destroy(BrHttpResponse* r);
