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

#define MAX_URI_LENGTH 4096
#define MAXLINE 256
#define PRINT(FMT, ARGS...) printf(__FILE__ #FMT "\r\n" ARGS)
#define WARN(CODE)                                                             \
    do {                                                                       \
        fprintf(stderr, __FILE__ "#%s():%d " #CODE "\r\n", __func__,           \
                __LINE__);                                                     \
    } while (0)

#define ERROR(CODE)                                                            \
    (fprintf(stderr, __FILE__ "#%s():%d " #CODE "\r\n", __func__, __LINE__),   \
     CODE)

typedef enum BR_NET_PROTOCOL {
    BR_PROTOCOL_GOPHER,
    BR_PROTOCOL_GEMINI,
    BR_PROTOCOL_HTTP,
    BR_PROTOCOL_HTTPS,
    BR_PROTOCOL_UNSUPPORTED
} BR_PROTOCOL;
#define BR_PROTOCOL_S(p)                                                       \
    (p == BR_PROTOCOL_HTTP     ? "HTTP"                                        \
     : p == BR_PROTOCOL_HTTPS  ? "HTTPS"                                       \
     : p == BR_PROTOCOL_GEMINI ? "GEMINI"                                      \
     : p == BR_PROTOCOL_GOPHER ? "GOPHER"                                      \
                               : "UNDEFINED")

/* Returns whether there is a NULL character within the first MAX_URI_LENGTH
 * bytes or not */
bool is_null_terminated(const char* str);
/* Returns true if given string is a valid IPv4 address */
bool is_ip(const char* input);
/* Removes all characters starting from the first ':' */
void uri_strip(char* str);
/* Tries to obtain the port from the URI and returns it.
 * If the URI does not contain a port, returns -1 */
int parse_port(const char* URI);
/**
 * Tries to obtain the protocol from the URI, returns the protocol name
 * while setting up the index to where protocol definition ends.
 *
 * @URI - to parse
 * @start_addr - address to set where URI begins
 * @returns - protocol type BR_PROTOCOL, BR_PROTOCOL_UNSUPPORTED,
 * if the protocol is not recognized
 *
 * example:
 * capture_protocol("https://www.google.com", &addr); -> BR_PROTOCOL_HTTPS
 * addr will be 9, because that's where the domain starts('w')
 */
BR_PROTOCOL capture_protocol(const char* uri, int* start_addr);
/* Returns an absolute path to request. From the URI and the request path.
 * Returned string is a null terminated static char array */
const char* to_abs_path(const char* uri, const char* request_path);
/* Obtains a valid host name from given IP address */
char* uri_from(const char* ip);
/* Obtains a valid IP address from given URI */
char* ip_from(const char* uri);
/* Writes the browser agent information to the given buffer */
void get_os(char* os, size_t os_s);
