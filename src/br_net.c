#include "br_net.h"
#include "../include/br_net.h"

#define PACKET_SIZE 4096

const int RequestTypePort[] = {70, 1965, 80, 443};

static BR_NET_STATUS _setup_address(BrSession* c, const char* uri, int proto_idx);
/**
 * Attempts to connect to the target IP through SSL, returns
 * whether the attempt is successful or not.
 * - If the connection is established with SSL, SSL methods should be used.
 * - Else standard TCP connection methods should be used.
 *  @c - BrConnection to establish
 *  @return BR_NET_STATUS - connection status
 */
static int _try_ssl(BrSession* c);

/**
 * Reads buffer_s bytes from the socket and writes to the given buffer. Uses SSL
 * if enabled.
 * @c - Socket information
 * @buffer - Buffer to write into
 * @buffer_s - Max buffer
 * @return bytes written
 */
static ssize_t _br_read(BrSession* c, char* buffer, int buffer_s);

/**
 * Writes buffer_s bytes to the socket and sends. Returns number of bytes
 * written. Uses SSL if enabled.
 * @c - Socket information
 * @buffer - Buffer to read from
 * @buffer_s - Max buffer
 * @return bytes written
 */
static ssize_t _br_write(BrSession* c, char* buffer, int buffer_s);

BR_NET_STATUS br_session_new(BrSession* c, const char* uri)
{
    if (uri == NULL || !is_null_terminated(uri)) {
        return ERROR(BR_NET_ERROR_INVALID_URI_STRING);
    }
    int start_addr = 0;
    c->protocol = capture_protocol(uri, &start_addr);
    if (c->protocol == BR_PROTOCOL_UNSUPPORTED) {
        return ERROR(BR_NET_ERROR_INVALID_PROTOCOL);
    }
    return _setup_address(c, uri, start_addr);
}

BR_NET_STATUS br_connect(BrSession* c)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return ERROR(BR_NET_ERROR_SOCKET_CREATION);
    }
    c->sockfd = sockfd;
    // Return to regular connection if SSL doesn't work
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(c->port);
    address.sin_addr.s_addr = inet_addr(c->ip);
    if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) != 0) {
        return ERROR(BR_NET_ERROR_CONNECTION_FAILED);
    }
    BR_NET_STATUS ssl_status = _try_ssl(c);
    return ssl_status != BR_NET_ERROR_SSL_DISABLED
               ? ssl_status
               : BR_NET_STATUS_OK;
}

BR_NET_STATUS br_request(BrSession* c, const char* buffer, size_t buffer_s)
{
    int attempt = BR_REQUEST_SIZE_ATTEMPT;
    size_t total_w = 0;
    int frame_w;
    char frame[PACKET_SIZE];
    do {
        frame_w = buffer_s < PACKET_SIZE ? (int)buffer_s : PACKET_SIZE;
        memcpy(frame, &buffer[total_w], frame_w);
        ssize_t bytes_w = _br_write(c, frame, frame_w);
        if (bytes_w == -1)
            return ERROR(BR_ERROR_BROKEN_CONNECTION);
        total_w += bytes_w;
    } while (total_w < buffer_s && attempt-- > 0);
    memset(frame, 0, 4096);
    ssize_t frame_r;
    bool eof = false;
    while (!eof && (frame_r = _br_read(c, frame, PACKET_SIZE)) > 0) {
        if (frame_r < PACKET_SIZE) {
            eof = true;
        }
        frame[frame_r] = '\0';
        char* total_resp = malloc(c->resp_s + frame_r);
        if (c->resp_s) {
            memcpy(total_resp, c->resp, c->resp_s);
        }
        memcpy(&total_resp[c->resp_s], frame, frame_r);
        free(c->resp);
        c->resp = total_resp;
        c->resp_s += frame_r;
    }
    if (total_w == 0 || c->resp_s == 0)
        return ERROR(BR_ERROR_BROKEN_CONNECTION);
    return BR_NET_STATUS_OK;
}

size_t br_resolve(BrSession* c, char** buffer, bool keep)
{
    if (buffer != NULL) {
        *buffer = c->resp;
    }
    if (!keep) {
        if (c->ssl.enabled) {
            SSL_free(c->ssl.ssl);
            SSL_CTX_free(c->ssl.ctx);
        }
        close(c->sockfd);
        free(c->host);
    }
    return c->resp_s;
}

static int _try_ssl(BrSession* c)
{
    c->ssl.enabled = 0;
    if (c->protocol == BR_PROTOCOL_HTTP)
        return ERROR(BR_NET_ERROR_SSL_DISABLED);
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    c->ssl.ctx = SSL_CTX_new(SSLv23_client_method());
    if (!c->ssl.ctx) {
        return ERROR(BR_NET_ERROR_SSL_CONTEXT);
    }

    c->ssl.ssl = SSL_new(c->ssl.ctx);
    if (!c->ssl.ssl) {
        return ERROR(BR_NET_ERROR_SSL);
    }

    SSL_set_fd(c->ssl.ssl, c->sockfd);
    int r;
    if ((r = SSL_connect(c->ssl.ssl)) != 1) {
        SSL_get_error(c->ssl.ssl, r);
        ERR_print_errors_fp(stderr);
        return ERROR(BR_NET_ERROR_SSL_CONNECTION);
    }
    c->ssl.enabled = 1;
    return ERROR(BR_NET_STATUS_SSL_ENABLED);
}

static ssize_t _br_read(BrSession* c, char* buffer, int buffer_s)
{
    return c->ssl.enabled ? SSL_read(c->ssl.ssl, buffer, buffer_s)
                          : recv(c->sockfd, buffer, buffer_s, 0);
}

static ssize_t _br_write(BrSession* c, char* buffer, int buffer_s)
{
    return c->ssl.enabled ? SSL_write(c->ssl.ssl, buffer, buffer_s)
                          : send(c->sockfd, buffer, buffer_s, 0);
}

static BR_NET_STATUS _setup_address(BrSession* c, const char* uri, int proto_idx)
{
    // if port can not be obtained from the URI, fallback to the protocols
    const char* uri_trimmed = uri + proto_idx;
    int port = parse_port(uri_trimmed);
    char* uri_n = strdup(uri_trimmed);
    if (port != -1) {
        c->port = port;
        uri_strip(uri_n);
    } else {
        c->port = RequestTypePort[c->protocol];
    }
    if (is_ip(uri)) {
        c->host = uri_from(uri_n);
        memcpy(&c->ip, uri, 16);
    } else {
        c->host = uri_n;
        char* ip = ip_from(c->host);
        if (ip == NULL) {
            free(uri_n);
            return ERROR(BR_NET_ERROR_IP_NOT_FOUND);
        }
        memcpy(&c->ip, ip, 16);
    }
    return BR_NET_STATUS_OK;
}
