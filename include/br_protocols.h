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
    BR_PRT_GEM_OK = 0,
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
    BR_PRT_GEM_STATUS_REQUEST_BODY,
    BR_PRT_GEM_ERROR_INVALID_HEADER,
    BR_PRT_GEM_ERROR_POLL_BODY,
    BR_PRT_GEM_ERROR_INVALID_INPUT,
    BR_PRT_GEM_ERROR_REDIRECT,

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
    char* req;
    size_t req_s;
    int status_code;
    GHashTable* headers;
    char* body;
    char* __full_text;
    size_t __full_text_s;
} BrHttpResponse;

#define BR_HTTP_RESP_UNWRAP(r)                                                 \
    "BrHttpResponse[status:%d, full_size: %ld]\n%s\n", (r)->status_code,       \
        (r)->__full_text_s, (r)->body

/**
 * Parses the data from the connection and converts it into a HttpResponse
 */
BR_PRT_STATUS br_http_response_new(BrSession* s, BrHttpResponse* h_resp);
void br_http_response_destroy(BrHttpResponse* r);

/**
 * Appends the given HTTP Connection, fills the required headers
 * @buffer - buffer to append
 * @buffer_s - the size of the buffer
 * @keep - whether to keep the connection alive or not
 */
void br_http_set_req_headers(const char* host, char* buffer, size_t buffer_s,
                             bool keep);

/******************************************************************************
                                  GEMINI
*****************************************************************************/

typedef enum {
    BR_GEMINI_RESP_INPUT = 10,
    BR_GEMINI_RESP_SUCCESS = 20,
    BR_GEMINI_RESP_REDIRECT_TEMPORARY = 30,
    BR_GEMINI_RESP_REDIRECT_PERMANENT = 31,
    BR_GEMINI_RESP_TEMPORARY_FAILURE = 40,
    BR_GEMINI_RESP_SERVER_UNAVAILABLE = 41,
    BR_GEMINI_RESP_CGI_ERROR = 42,
    BR_GEMINI_RESP_PROXY_ERROR = 43,
    BR_GEMINI_RESP_SLOW_DOWN = 44,
    BR_GEMINI_RESP_PERMANENT_FAILURE = 50,
    BR_GEMINI_RESP_NOT_FOUND = 51,
    BR_GEMINI_RESP_GONE = 52,
    BR_GEMINI_RESP_PROXY_REQUEST_REFUSED = 53,
    BR_GEMINI_RESP_BAD_REQUEST = 59,
    BR_GEMINI_RESP_CLIENT_CERTIFICATE_REQUIRED = 60,
    BR_GEMINI_RESP_CERTIFICATE_NOT_AUTHORISED = 61,
    BR_GEMINI_RESP_CERTIFICATE_NOT_VALID = 62,
    BR_GEMINI_RESP_CERTIFICATE_REQUIRED = 63,
    BR_GEMINI_RESP_SENSITIVE_INPUT = 70,
    BR_GEMINI_RESP_SENSITIVE_INPUT_WITHOUT_TLS = 71,
    BR_GEMINI_RESP_SENSITIVE_INPUT_FORM = 72,
    BR_GEMINI_RESP_SENSITIVE_INPUT_FORM_WITHOUT_TLS = 73
} BR_GEMINI_RESP;

typedef struct {
    char* req;
    size_t req_s;
    int status_number;
    BR_GEMINI_RESP status_code;
    char* header;
    char* body;
    size_t body_s;
    char* __full_text;
    size_t __full_text_s;
} BrGemResponse;

#define BR_GEM_RESP_UNWRAP(r)                                                  \
    "BrGemResponse[status:%d,header: %s,full_size: %ld]\n %s\n",               \
        (r)->status_number, (r)->header, (r)->__full_text_s, (r)->body

BR_PRT_STATUS br_gem_response_new(BrSession* s, BrGemResponse* gem_r);
BR_PRT_STATUS br_gem_poll(BrSession* s, BrGemResponse* r);
void br_gem_response_destroy(BrGemResponse* r);

/******************************************************************************
                                GOPHER
*****************************************************************************/
