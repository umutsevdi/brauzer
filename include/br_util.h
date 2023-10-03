/******************************************************************************

 * File: include/br_util.h
 *
 * Author: Umut Sevdi
 * Created: 09/11/23
 * Description: Common functions and data structures for Brauzer

*****************************************************************************/
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 256
#define PRINT(FMT, ARGS...) printf(__FILE__ #FMT "\r\n" ARGS)
#define WARN(CODE)                                                             \
    do {                                                                       \
        fprintf(stderr, __FILE__ "#%s():%d " #CODE "\r\n", __func__, __LINE__); \
    } while (0)
#define ERROR(CODE) (fprintf(stderr, __FILE__ "#%s():%d " #CODE "\r\n", __func__, __LINE__), CODE)

typedef enum BR_NET_PROTOCOL {
    BR_PROTOCOL_GOPHER,
    BR_PROTOCOL_GEMINI,
    BR_PROTOCOL_HTTP,
    BR_PROTOCOL_HTTPS,
    BR_PROTOCOL_UNSUPPORTED
} BR_PROTOCOL;

#define BR_LINK_SIZE 1024
/**
 * A URI that exists within a protocol
 * to poll the follow-up page
 */
typedef struct __BR_UTIL_URI {
    BR_PROTOCOL protocol;
    bool is_local;
    char URI[BR_LINK_SIZE];
    size_t URI_s;
    struct __BR_UTIL_URI* next;
} BrUri;

/**
 * Writes the browser agent information
 * to the given buffer
 */
void get_os(char* os, size_t os_s);
