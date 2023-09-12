#include "br_net.h"

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

/**
 * Tries to obtain the port from the URI and returns it.
 * If the URI does not contain port, it returns the protocol's port
 * @protocol - protocol to get fallback port
 * @URI - URI to parse
 *
 * @return port value
 */
static int __br_parse_port(BR_PROTOCOL protocol, const char* URI);
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

char* br_ip_from_uri(const char* hostname)
{
    struct hostent* host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        return NULL;
    }
    return inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
}

BrConnection* br_connnection_new(BR_PROTOCOL protocol, const char* uri, int ssl_enabled)
{
    BrConnection* c = calloc(sizeof(BrConnection), 1);
    c->protocol = protocol;
    if (ssl_enabled)
        c->ssl.enabled = ssl_enabled > 0;
    c->port = __br_parse_port(protocol, uri);
    const char* ip = br_ip_from_uri(uri);
    memcpy(c->ip, ip, sizeof(c->ip));
    return c;
}

BR_NET_STATUS br_connect(BrConnection* c)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return WARN(BR_NET_ERROR_SOCKET_CREATION);
    }
    c->sockfd = sockfd;
    BR_NET_STATUS ssl_ok = !__try_ssl(c);
    // Return to regular connection if SSL doesn't work
    if (!ssl_ok) {
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = c->port;
        address.sin_addr.s_addr = inet_addr(c->ip);
        if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) != 0) {
            return WARN(BR_NET_ERROR_CONNECTION_FAILED);
        }
    }
    return BR_NET_STATUS_OK;
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

size_t br_resolve(BrConnection* c, char* buffer)
{
    buffer = c->resp;
    if (c->ssl.enabled) {
        SSL_free(c->ssl.ssl);
        SSL_CTX_free(c->ssl.ctx);
    }
    close(c->sockfd);
    return c->resp_s;
}

static int __br_parse_port(BR_PROTOCOL protocol, const char* URI)
{
    char* port_str;
    // Check if the given Ip Address
    if ((port_str = strstr(URI, ":")), port_str != NULL) {
        char* endptr = NULL;
        int p = strtol(port_str, &endptr, 10);
        if (*endptr != '\0') {
            return p;
        }
    }
    return RequestTypePort[protocol];
}

static int __try_ssl(BrConnection* c)
{
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

    if (SSL_connect(c->ssl.ssl) != 1) {
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
