/******************************************************************************

 * File: include/br_protocols.h
 *
 * Author: Umut Sevdi
 * Created: 09/25/23
 * Description: Includes protocol specific functions

*****************************************************************************/
#pragma once
#include <stdlib.h>
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
