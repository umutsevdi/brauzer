/******************************************************************************

 * File: include/br_net.h
 *
 * Author: Umut Sevdi
 * Created: 09/10/23
 * Description: Brauzer Network utilities

*****************************************************************************/
#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "br_util.h"
#include <openssl/err.h>
#include <openssl/ssl.h>

#define BR_REQUEST_SIZE_ATTEMPT 10

typedef enum {
    BR_PROTOCOL_GOPHER,
    BR_PROTOCOL_GEMINI,
    BR_PROTOCOL_HTTP,
    BR_PROTOCOL_HTTPS,
    BR_PROTOCOL_UNSUPPORTED
} BR_PROTOCOL;

extern const int RequestTypePort[];
typedef enum {
    BR_NET_STATUS_OK = 0,
    BR_NET_STATUS_SSL_ENABLED = 0,
    BR_NET_ERROR_INVALID_URI_STRING,
    BR_NET_ERROR_INVALID_PROTOCOL,
    BR_NET_ERROR_IP_NOT_FOUND,
    BR_NET_ERROR_URI_NOT_FOUND,
    BR_NET_ERROR_SOCKET_CREATION,
    BR_NET_ERROR_SSL_DISABLED,
    BR_NET_ERROR_SSL,
    BR_NET_ERROR_SSL_CONTEXT,
    BR_NET_ERROR_SSL_CONNECTION,
    BR_NET_ERROR_CONNECTION_FAILED,
    BR_NET_ERROR_SEND

} BR_NET_STATUS;

typedef struct __BR_CONNECTION BrConnection;

/**
 * Creates a socket to connect to target URI. Returns the connection if no error occurs
 */
BrConnection* br_connection_new(const char* uri);
/**
 * Performs a connection according to the given BrConnection,
 * based on given information.
 */
BR_NET_STATUS br_connect(BrConnection* c);
/**
 * Sends a request that contains given buffer and saves it to the given BrConnection
 */
BR_NET_STATUS br_request(BrConnection* c, const char* buffer, size_t buffer_s);
/**
 * Resolves the given BrConnection, closes the socket writes the received data to the
 * buffer. Returns the new size of the buffer.
 * @c - connection to resolve
 * @buffer- buffer to write in to. It will be filled with the stored response
 * @return size of the newly allocated buffer
 */
size_t br_resolve(BrConnection* c, char** buffer);

void get_http_fields(BrConnection* c, char* buffer, size_t buffer_s);

void br_protocol_print(BrConnection* c);
