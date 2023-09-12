#ifdef X__SHOULD_NOT_BE_DEFINED

#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if 1
#include <openssl/ssl.h>
#endif

#define MAXLINE 256
#define WARN(CODE) (printf(#CODE "\r\n"), CODE)

typedef enum {
    REQUEST_TYPE_GOPHER,
    REQUEST_TYPE_HTTP,
    REQUEST_TYPE_HTTPS,
    REQUEST_TYPE_GEMINI,
} BR_REQUEST_TYPE;

const int RequestTypePort[] = {
    70, 80, 443, 1965
};

const char* RequestTypePrefix[] = {
    "%s", // A path that starts with
};

typedef enum {
    GEMINI_OK,
    GEMINI_ERROR_URI_NOT_FOUND,
    GEMINI_ERROR_SOCKET_CREATION,
    GEMINI_ERROR_SSL,
    GEMINI_SSL_CONTEXT_ERROR,
    GEMINI_SSL_CONNECTION_ERROR,
    GEMINI_ERROR_CONNECTION_FAILED,
    GEMINI_ERROR_SEND

} GEMINI_STATUS;

typedef struct {
    int enabled;
    SSL_CTX* ctx;
    SSL* ssl;
} GeminiSslConfiguration;

typedef struct {
    int sockfd;
    char ip[16];
    char* req;
    uint64_t req_size;
    char* resp;
    uint64_t resp_size;
    char* host;
    GeminiSslConfiguration ssl;
} GeminiConnection;

/**
 * gets the IP address of the given host name
 * @param String representation of the local name ex: pi.local, fedora
 * @return IP address as string ex: 192.168.1.1
 *
 */
char* gemini_util_get_ip(char* hostname)
{
    struct hostent* host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        return NULL;
    }
    return inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
}

/**
 * sends a message to the given socket and returns whether
 * there have been an error while sending the message
 * @param sockfd socket
 * @param buffer to write on
 * @return whether connection should be kept or not
 *
 */
int gemini_send(GeminiConnection* c, char* buffer, uint64_t buffer_s)
{
    c->req = strndup(buffer, buffer_s);
    c->req_size = buffer_s;

    int n = c->ssl.enabled ? SSL_write(c->ssl.ssl, c->req, c->req_size)
                           : send(c->sockfd, c->req, c->req_size, 0);
    if (n < 0) {
        close(c->sockfd);
        return WARN(GEMINI_ERROR_SEND);
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
    return WARN(GEMINI_OK);
}

int init_openssl(GeminiConnection* c)
{
    c->ssl.enabled = 0;
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx) {
        SSL_CTX_free(ctx);
        close(c->sockfd);
        return WARN(GEMINI_SSL_CONTEXT_ERROR);
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(c->sockfd);
        return WARN(GEMINI_SSL_CONNECTION_ERROR);
    }

    SSL_set_fd(ssl, c->sockfd);
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(c->sockfd);
        return WARN(GEMINI_SSL_CONNECTION_ERROR);
    }
    GeminiSslConfiguration conf = { .enabled = 1, .ssl = ssl, .ctx = ctx };
    c->ssl = conf;
    return WARN(GEMINI_OK);
}

int gemini_cleanup(GeminiConnection* c)
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
    return GEMINI_OK;
}

int gemini_send_get(GeminiConnection* c, char* path, uint64_t path_s)
{
    char* message = calloc(sizeof(char), path_s + 6 + strnlen(c->host, 4096));
    snprintf(message, path_s + 6 + strnlen(c->host, 4096), "gemini://%s/%s\r\n", c->host, path);
    printf("REQ:[%s]\n", message);
    return gemini_send(c, message, path_s + 6 + strnlen(c->host, 4096));
}

int gopher_send_get(GeminiConnection* c, char* path, uint64_t path_s)
{
    char* message = calloc(sizeof(char), path_s + 6 + strnlen(c->host, 4096));
    snprintf(message, path_s + 6 + strnlen(c->host, 4096), "gemini://%s/%s\r\n", c->host, path);
    printf("REQ:[%s]\n", message);
    return gemini_send(c, message, path_s + 6 + strnlen(c->host, 4096));
}

int gemini_connect(char* hostname, GeminiConnection* c, BR_REQUEST_TYPE type)
{
    char* ip = gemini_util_get_ip(hostname);
    if (ip == NULL) {
        return WARN(GEMINI_ERROR_URI_NOT_FOUND);
    }
    c->host = hostname;
    memcpy(c->ip, ip, sizeof(c->ip));
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return WARN(GEMINI_ERROR_SOCKET_CREATION);
    }

    c->sockfd = sockfd;
    for (int i = 0; i < 16; i++)
        c->ip[i] = ip[i];

    GEMINI_STATUS ssl_ok = !init_openssl(c);
    // Return to regular connection if SSL doesn't work
    if (!ssl_ok) {
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(RequestTypePort[type]);
        address.sin_addr.s_addr = inet_addr(ip);
        if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) != 0) {
            return WARN(GEMINI_ERROR_CONNECTION_FAILED);
        }
    }

    return WARN(GEMINI_OK);
}
int main(int argc, char* argv[])
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    char* hostname = argv[1];
    GeminiConnection connection;
    int error = gemini_connect(hostname, &connection, REQUEST_TYPE_GOPHER);
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
#endif
