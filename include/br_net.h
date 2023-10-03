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

#include "br_util.h"
#include <openssl/err.h>
#include <openssl/ssl.h>

#define BR_REQUEST_SIZE_ATTEMPT 10

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
 * Creates a socket to connect to the target URI. Returns the connection if there
 * is no error
 */
BrConnection* br_connection_new(const char* uri);
/**
 * Performs a connection according to the given BrConnection specification
 */
BR_NET_STATUS br_connect(BrConnection* c);
/**
 * Sends a request that contains the given buffer and stores the response
 */
BR_NET_STATUS br_request(BrConnection* c, const char* buffer, size_t buffer_s);
/**
 * Resolves the given BrConnection, and writes the received data to the
 * buffer. Returns the new size of the buffer. If keep is set to false,
 * closes the connection
 * @c - connection to resolve
 * @buffer - buffer to write into. It will be filled with the stored response
 * @keep - whether to keep the connection alive or not
 * @return size of the newly allocated buffer
 */
size_t br_resolve(BrConnection* c, char** buffer, bool keep);

/**
 * Appends the given HTTP Connection, fills the required headers
 * @buffer - buffer to append
 * @buffer_s - the size of the buffer
 * @keep - whether to keep the connection alive or not
 */
void get_http_fields(BrConnection* c, char* buffer, size_t buffer_s, bool keep);

void br_protocol_print(BrConnection* c);

BR_PROTOCOL br_protocol(BrConnection* c);
