#ifndef BR_NET
#define BR_NET
/******************************************************************************

 * File: include/br_net.h
 *
 * Author: Umut Sevdi
 * Created: 09/10/23
 * Description: Brauzer Network utilities

*****************************************************************************/
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
    BR_PROTOCOL_HTTP,
    BR_PROTOCOL_GEMINI,
} BR_PROTOCOL;

const int RequestTypePort[] = {
    70, 80, 443, 1965
};

const char* BrRequestTypePrefix[] = {
    "%s", // A path that starts with
};

typedef enum {
    BR_NET_STATUS_OK,
    BR_NET_ERROR_URI_NOT_FOUND,
    BR_NET_ERROR_SOCKET_CREATION,
    BR_NET_ERROR_SSL,
    BR_NET_ERROR_SSL_CONTEXT,
    BR_NET_ERROR_SSL_CONNECTION,
    BR_NET_ERROR_CONNECTION_FAILED,
    BR_NET_ERROR_SEND

} BR_NET_STATUS;

typedef struct __BR_SSL_CONFIG BrConnectionSsl;

typedef struct __BR_CONNECTION BrConnection;

/**
 * gets the IP address of the given host name
 * @hostname - String representation of the local name ex: pi.local, fedora
 * @return IP address as string ex: 192.168.1.1
 */
char* br_ip_from_uri(const char* hostname);
/**
 * Creates a socket to connect to target URI. Returns the connection if no error occurs
 */
BrConnection* br_connnection_new(BR_PROTOCOL protocol, const char* uri, int ssl_enabled);
/**
 * Performs a connection according to the given BrConnection,
 * based on given informations.
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
size_t br_resolve(BrConnection* c, char* buffer);

#endif
