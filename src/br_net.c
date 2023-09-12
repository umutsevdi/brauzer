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
    BrBuffer req;
    BrBuffer resp;
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

int br_read(BrConnection* c, char* buffer, size_t buffer_s)
{
    return c->ssl.enabled ? SSL_read(c->ssl.ssl, buffer, buffer_s)
                          : recv(c->sockfd, buffer, buffer_s, 0);
}

int br_write(BrConnection* c, char* buffer, size_t buffer_s)
{
    return c->ssl.enabled ? SSL_write(c->ssl.ssl, buffer, buffer_s)
                          : send(c->sockfd, buffer, buffer_s, 0);
}

/**
 * sends a message to the given socket and returns whether
 * there have been an error while sending the message
 * @param sockfd socket
 * @param buffer to write on
 * @return whether connection should be kept or not
 *
 */
int gemini_send(BrConnection* c, char* buffer, uint64_t buffer_s)
{
    c->req = strndup(buffer, buffer_s);
    c->req_size = buffer_s;

    int n = c->ssl.enabled ? SSL_write(c->ssl.ssl, c->req, c->req_size)
                           : send(c->sockfd, c->req, c->req_size, 0);
    if (n < 0) {
        close(c->sockfd);
        return WARN(BR_NET_ERROR_SEND);
    }

    int bytes_received;
    char packet[4096];
    while (c->ssl.enabled ? (bytes_received = SSL_read(c->ssl.ssl, packet, sizeof(packet) - 1)) > 0
                          : (bytes_received = recv(c->sockfd, packet,
                                 sizeof(packet) - 1, 0))
                > 0) {
        packet[bytes_received] = '\0';
        printf("[%s]\n", packet);
        char* con_mem = malloc(c->resp_size + bytes_received);
        if (c->resp_size) {
            memcpy(con_mem, c->resp, c->resp_size);
        }
        memcpy(&con_mem[c->resp_size], packet, bytes_received);
        free(c->resp);
        c->resp = con_mem;
        c->resp_size += bytes_received;
    }

    printf("%s->\n%s\n", c->req, c->resp);
    return BR_NET_STATUS_OK;
}

int init_openssl(BrConnection* c)
{
    c->ssl.enabled = 0;
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx) {
        SSL_CTX_free(ctx);
        close(c->sockfd);
        return WARN(BR_NET_ERROR_SSL_CONTEXT);
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(c->sockfd);
        return WARN(BR_NET_ERROR_SSL_CONNECTION);
    }

    SSL_set_fd(ssl, c->sockfd);
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(c->sockfd);
        return WARN(BR_NET_ERROR_SSL_CONNECTION);
    }
    BrConnectionSsl conf = { .enabled = 1, .ssl = ssl, .ctx = ctx };
    c->ssl = conf;
    return BR_NET_STATUS_OK;
}

int gemini_cleanup(BrConnection* c)
{
    if (c->ssl.enabled) {
        SSL_free(c->ssl.ssl);
        SSL_CTX_free(c->ssl.ctx);
    }
    if (c->req != NULL) {
        free(c->req);
    }
    c->req_size = 0;
    if (c->resp != NULL) {
        free(c->req);
    }
    c->resp_size = 0;
    close(c->sockfd);
    return BR_NET_STATUS_OK;
}

int gemini_send_get(BrConnection* c, char* path, uint64_t path_s)
{
    char* message = calloc(sizeof(char), path_s + 6 + strnlen(c->host, 4096));
    snprintf(message, path_s + 6 + strnlen(c->host, 4096), "gemini://%s/%s\r\n", c->host, path);
    printf("REQ:[%s]\n", message);
    return gemini_send(c, message, path_s + 6 + strnlen(c->host, 4096));
}

int gopher_send_get(BrConnection* c, char* path, uint64_t path_s)
{
    char* message = calloc(sizeof(char), path_s + 6 + strnlen(c->host, 4096));
    snprintf(message, path_s + 6 + strnlen(c->host, 4096), "gemini://%s/%s\r\n", c->host, path);
    printf("REQ:[%s]\n", message);
    return gemini_send(c, message, path_s + 6 + strnlen(c->host, 4096));
}

int gemini_connect(char* hostname, BrConnection* c, BR_PROTOCOL type)
{
    char* ip = gemini_util_get_ip(hostname);
    if (ip == NULL) {
        return WARN(BR_NET_ERROR_URI_NOT_FOUND);
    }
    c->host = hostname;
    memcpy(c->ip, ip, sizeof(c->ip));
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return WARN(BR_NET_ERROR_SOCKET_CREATION);
    }

    c->sockfd = sockfd;
    for (int i = 0; i < 16; i++)
        c->ip[i] = ip[i];

    BR_NET_STATUS ssl_ok = !init_openssl(c);
    // Return to regular connection if SSL doesn't work
    if (!ssl_ok) {
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(RequestTypePort[type]);
        address.sin_addr.s_addr = inet_addr(ip);
        if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) != 0) {
            return WARN(BR_NET_ERROR_CONNECTION_FAILED);
        }
    }

    return BR_NET_STATUS_OK;
}
int main(int argc, char* argv[])
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    char* hostname = argv[1];
    BrConnection connection;
    int error = gemini_connect(hostname, &connection, BR_PROTOCOL_GOPHER);
    if (error) {
        exit(error);
    }
    error = gemini_send_get(&connection, argv[2], sizeof(argv[2]));
    if (error) {
        exit(error);
    }
    printf("RESPONSE: %s\n", connection.resp);
    error = gemini_cleanup(&connection);
    if (error) {
        exit(error);
    }

    EVP_cleanup();
    return 0;
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

static int __try_ssl(BrConnection* c);
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

    BrConnectionSsl conf = { .enabled = 1, .ssl = ssl, .ctx = ctx };
    c->ssl = conf;
    return BR_NET_STATUS_OK;
}
