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
#include <openssl/ssl.h>

#define BR_REQUEST_SIZE_ATTEMPT 10

typedef enum {
    BR_PROTOCOL_GOPHER,
    BR_PROTOCOL_GEMINI,
    BR_PROTOCOL_HTTP,
    BR_PROTOCOL_HTTPS
} BR_PROTOCOL;

extern const int RequestTypePort[];
typedef enum {
    BR_NET_STATUS_OK,
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
BrConnection* br_connection_new(BR_PROTOCOL protocol, const char* uri, int ssl_enabled);
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
