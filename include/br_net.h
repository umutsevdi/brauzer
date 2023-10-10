/******************************************************************************

 * File: include/br_net.h
 *
 * Author: Umut Sevdi
 * Created: 09/10/23
 * Description: Brauzer Network utilities

*****************************************************************************/
#pragma once

#include "br_util.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

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
    BR_ERROR_BROKEN_CONNECTION,
} BR_NET_STATUS;

typedef struct {
    int enabled;
    SSL_CTX* ctx;
    SSL* ssl;
} BrSessionSsl;

typedef struct {
    int sockfd;
    char ip[16];
    int port;
    BR_PROTOCOL protocol;
    BrSessionSsl ssl;
    char* host;
    char* resp;
    size_t resp_s;
} BrSession;

#define BR_SESSION_UNWRAP(r)                                                   \
    "BrSession{host: %s, ip:%s, port: %d, protocol %d,"                        \
    "BrSessionSsl{enabled: %c, SSL: %p, SSL_Ctx: %p},"                         \
    "resp_s: %ld}\n",                                                          \
        (r)->host, (r)->ip, (r)->port, (r)->protocol,                          \
        (r)->ssl.enabled ? 'T' : 'F', (r)->ssl.ssl, (r)->ssl.ctx, (r)->resp_s

/**
 * Parses the given URI and sets up the given BrSession
 * @c - Connection to assign
 * @URI - to parse
 * @return - BR_NET_STATUS_OK if the URI is valid and matches with an IP
 * address.
 *     - If the URI is an IP address, tries to find a host name.
 *     - If the URI contains a port, overrides the default port of the protocol.
 *     - URI must contain a protocol and the protocol must be supported.
 * @errors:
 * - BR_NET_ERROR_INVALID_URI_STRING
 * - BR_PROTOCOL_UNSUPPORTED
 * - BR_NET_ERROR_IP_NOT_FOUND
 */
BR_NET_STATUS br_session_new(BrSession* c, const char* uri);
/**
 * Performs a connection according to the given BrSession specification.
 * @return
 * - BR_NET_STATUS_SSL_ENABLED if the connection is established with the SSL
 * - BR_NET_STATUS_OK if the connection is established without the SSL
 * - Supports all SSL functions supported by openssl
 * @errors:
 * - BR_NET_ERROR_SOCKET_CREATION
 * - BR_NET_ERROR_CONNECTION_FAILED
 * - BR_NET_ERROR_SSL_CONTEXT
 * - BR_NET_ERROR_SSL
 * - BR_NET_ERROR_SSL_CONNECTION
 */
BR_NET_STATUS br_connect(BrSession* c);
/**
 * Sends a request that contains the given buffer and captures the response
 * @c - Session to obtain content
 * @buffer - Buffer to send
 * @buffer_s - Exact size of the string to send
 * @return
 * - BR_NET_STATUS_OK if the request has been sent and a response received
 * @errors:
 * - BR_ERROR_BROKEN_CONNECTION
 */
BR_NET_STATUS br_request(BrSession* c, const char* buffer, size_t buffer_s);

/**
 * Closes the session
 */
void br_close(BrSession* c);
