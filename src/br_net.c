#include "br_net.h"
#include "../include/br_net.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <string.h>

const int RequestTypePort[] = {
    70, 1965, 80, 443
};

typedef struct __BR_SSL_CONFIG {
    int enabled;
    SSL_CTX* ctx;
    SSL* ssl;
} BrConnectionSsl;

typedef struct __BR_CONNECTION {
    int sockfd;
    char ip[16];
    int port;
    BR_PROTOCOL protocol;
    BrConnectionSsl ssl;
    char* resp;
    int resp_s;
    char* host;
} BrConnection;

static BR_NET_STATUS __setup_address(BrConnection* c, const char* uri);
static int __is_ip_address(const char* input);
static void __strip_colon(char* str);
static char* __uri_from_ip(const char* ip);
static char* __ip_from_uri(const char* hostname);

/**
 * Tries to obtain the port from the URI and returns it.
 * If the URI does not contain port, returns -1
 */
static int __parse_port(BR_PROTOCOL protocol, const char* URI);
/**
 * Attempts to connect to the target IP through SSL, returns
 * whether the attempt is successful or not.
 * - If the connection is established with SSL, SSL methods should be used.
 * - Else standard TCP connection methods should be used.
 *  @c - BrConnection to establish
 *  @return BR_NET_STATUS - connection status
 */
static int __try_ssl(BrConnection* c);

/**
 * Reads buffer_s bytes from the socket and writes to the given buffer. Uses SSL
 * if enabled.
 * @c - Socket information
 * @buffer - Buffer to write into
 * @buffer_s - Max buffer
 * @return bytes written
 */
static int __br_read(BrConnection* c, char* buffer, size_t buffer_s);

/**
 * Writes buffer_s bytes to the socket and sends. Returns number of bytes
 * written. Uses SSL if enabled.
 * @c - Socket information
 * @buffer - Buffer to read from
 * @buffer_s - Max buffer
 * @return bytes written
 */
static int __br_write(BrConnection* c, char* buffer, size_t buffer_s);

BrConnection* br_connection_new(BR_PROTOCOL protocol, const char* uri, int ssl_enabled)
{
    if (uri == NULL) {
        WARN(BR_NET_ERROR_INVALID_URI_STRING);
        return NULL;
    }

    BrConnection* c = calloc(sizeof(BrConnection), 1);
    c->protocol = protocol;
    if (ssl_enabled)
        c->ssl.enabled = ssl_enabled > 0;
    c->port = __parse_port(protocol, uri);
    if (__setup_address(c, uri)) {
        free(c);
        return NULL;
    }
    return c;
}

BR_NET_STATUS br_connect(BrConnection* c)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return WARN(BR_NET_ERROR_SOCKET_CREATION);
    }
    c->sockfd = sockfd;
    // Return to regular connection if SSL doesn't work
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(c->port);
    address.sin_addr.s_addr = inet_addr(c->ip);
    if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) != 0) {
        return WARN(BR_NET_ERROR_CONNECTION_FAILED);
    }
    return __try_ssl(c);
}

BR_NET_STATUS br_request(BrConnection* c, const char* buffer, size_t buffer_s)
{
    int attempt = BR_REQUEST_SIZE_ATTEMPT;
    size_t written = 0;
    char packet[4096];
    do {
        memcpy(packet, &buffer[written], 4096);
        int bytes_written = __br_write(c, packet, 4096);
        written += bytes_written;
    } while (written < buffer_s && attempt-- > 0);
    memset(packet, 0, 4096);
    int bytes_received;
    while ((bytes_received = __br_read(c, packet, 4096)) > 0) {
        packet[bytes_received] = '\0';
        char* con_mem = malloc(c->resp_s + bytes_received);
        if (c->resp_s) {
            memcpy(con_mem, c->resp, c->resp_s);
        }
        memcpy(&con_mem[c->resp_s], packet, bytes_received);
        free(c->resp);
        c->resp = con_mem;
        c->resp_s += bytes_received;
    }
    return BR_NET_STATUS_OK;
}

size_t br_resolve(BrConnection* c, char** buffer)
{
    *buffer = c->resp;
    if (c->ssl.enabled) {
        SSL_free(c->ssl.ssl);
        SSL_CTX_free(c->ssl.ctx);
    }
    close(c->sockfd);
    free(c->host);
    return c->resp_s;
}

static int __parse_port(BR_PROTOCOL protocol, const char* URI)
{
    char* port_str;
    // Check if the given Ip Address
    if ((port_str = strstr(URI, ":")), port_str != NULL) {
        PRINT("%s", , port_str);
        char* endptr = NULL;
        int p = strtol(&port_str[1], &endptr, 10);
        PRINT("%s| %d| %s", , port_str + 1, p, endptr);
        if (*endptr == 0) {
            port_str[0] = 0;
            return p;
        }
    }
    return -1;
}

static int __try_ssl(BrConnection* c)
{
    if (c->protocol == BR_PROTOCOL_HTTP)
        return WARN(BR_NET_ERROR_SSL_DISABLED);
    c->ssl.enabled = 0;
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    c->ssl.ctx = SSL_CTX_new(SSLv23_client_method());
    if (!c->ssl.ctx) {
        return WARN(BR_NET_ERROR_SSL_CONTEXT);
    }

    c->ssl.ssl = SSL_new(c->ssl.ctx);
    if (!c->ssl.ssl) {
        return WARN(BR_NET_ERROR_SSL);
    }

    SSL_set_fd(c->ssl.ssl, c->sockfd);
    int r;
    if ((r = SSL_connect(c->ssl.ssl)) != 1) {
        SSL_get_error(c->ssl.ssl, r);
        ERR_print_errors_fp(stderr);
        return WARN(BR_NET_ERROR_SSL_CONNECTION);
    }
    c->ssl.enabled = 1;
    return BR_NET_STATUS_OK;
}

static int __br_read(BrConnection* c, char* buffer, size_t buffer_s)
{
    return c->ssl.enabled ? SSL_read(c->ssl.ssl, buffer, buffer_s)
                          : recv(c->sockfd, buffer, buffer_s, 0);
}

static int __br_write(BrConnection* c, char* buffer, size_t buffer_s)
{
    return c->ssl.enabled ? SSL_write(c->ssl.ssl, buffer, buffer_s)
                          : send(c->sockfd, buffer, buffer_s, 0);
}

static int __is_ip_address(const char* input)
{
    char* input_d = strdup(input);
    int count = 0;
    char* token = strtok(input_d, ".");
    while (token != NULL) {
        int num = atoi(token);
        if (num < 0 || num > 255) {
            free(input_d);
            return 0;
        }
        token = strtok(NULL, ".");
        count++;
    }
    free(input_d);
    return (count == 4);
}

static void __strip_colon(char* str)
{
    char* pos = strchr(str, ':');
    if (pos != NULL) {
        *pos = '\0';
    }
}

static char* __uri_from_ip(const char* ip)
{
    struct in_addr addr;
    inet_pton(AF_INET, ip, &addr);
    struct hostent* host_entry = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (host_entry == NULL) {
        return NULL;
    }
    return host_entry->h_name;
}

static char* __ip_from_uri(const char* hostname)
{
    struct hostent* host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        return NULL;
    }
    return inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
}

static BR_NET_STATUS __setup_address(BrConnection* c, const char* uri)
{
    // if port can not be obtained from the URI, fallback to the protocols
    int port = __parse_port(c->protocol, uri);
    char* uri_n = strdup(uri);
    if (port != -1) {
        c->port = port;
        __strip_colon(uri_n);
    } else {
        c->port = RequestTypePort[c->protocol];
    }
    if (__is_ip_address(uri)) {
        c->host = __uri_from_ip(uri);
        memcpy(&c->ip, uri, 16);
    } else {
        c->host = uri_n;
        char* ip = __ip_from_uri(c->host);
        if (ip == NULL) {
            free(uri_n);
            return WARN(BR_NET_ERROR_IP_NOT_FOUND);
        }
        memcpy(&c->ip, ip, 16);
    }
    PRINT("%s %s %d", , c->host, c->ip, c->port);
    return BR_NET_STATUS_OK;
}

void get_http_fields(BrConnection* c, char* buffer, size_t buffer_s)
{
    snprintf(buffer, buffer_s, "Host: %s\r\nAccept-Language: en\r\n\r\n", c->host);
}
