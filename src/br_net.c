#include "br_net.h"
#include "../include/br_net.h"
#include "br_util.h"

#define PACKET_SIZE 4096

const int RequestTypePort[] = {
    70, 1965, 80, 443
};

static BR_NET_STATUS __setup_address(BrConnection* c, const char* uri, int proto_idx);
static int __is_ip_address(const char* input);
static void __strip_colon(char* str);
static char* __uri_from_ip(const char* ip);
static char* __ip_from_uri(const char* hostname);

/**
 * Tries to obtain the protocol from the URI, returns the protocol name
 * while setting up the index to where protocol definition ends.
 *
 * @URI - to parse
 * @start_addr - address to set where URI begins
 * @returns - protocol type BR_PROTOCOL, BR_PROTOCOL_UNSUPPORTED,
 * if the protocol is not recognized
 *
 * example:
 * __capture_protocol("https://www.google.com", &addr); -> BR_PROTOCOL_HTTPS
 * addr will be 9, because that's where the domain starts('w')
 */
static BR_PROTOCOL __capture_protocol(const char* uri, int* start_addr);
/**
 * Tries to obtain the port from the URI and returns it.
 * If the URI does not contain port, returns -1
 */
static int __parse_port(const char* URI);
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

BrConnection* br_connection_new(const char* uri)
{
    if (uri == NULL) {
        WARN(BR_NET_ERROR_INVALID_URI_STRING);
        return NULL;
    }

    BrConnection* c = calloc(sizeof(BrConnection), 1);
    int start_addr = 0;
    c->protocol = __capture_protocol(uri, &start_addr);
    if (c->protocol == BR_PROTOCOL_UNSUPPORTED) {
        WARN(BR_PROTOCOL_UNSUPPORTED);
        goto br_connection_new_error;
    }
    if (__setup_address(c, uri, start_addr)) {
        goto br_connection_new_error;
    }
    return c;
br_connection_new_error:
    free(c);
    return NULL;
}

BR_NET_STATUS br_connect(BrConnection* c)
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
    return __try_ssl(c);
}

BR_NET_STATUS br_request(BrConnection* c, const char* buffer, size_t buffer_s)
{
    int attempt = BR_REQUEST_SIZE_ATTEMPT;
    size_t total_w = 0;
    size_t frame_w;
    char frame[PACKET_SIZE];
    do {
        frame_w = buffer_s < PACKET_SIZE ? buffer_s : PACKET_SIZE;
        memcpy(frame, &buffer[total_w], frame_w);
        int bytes_w = __br_write(c, frame, frame_w);
        total_w += bytes_w;
    } while (total_w < buffer_s && attempt-- > 0);
    memset(frame, 0, 4096);
    int frame_r = 0;
    bool eof = false;
    while (!eof && (frame_r = __br_read(c, frame, PACKET_SIZE)) > 0) {
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
    return BR_NET_STATUS_OK;
}

size_t br_resolve(BrConnection* c, char** buffer, bool keep)
{
    *buffer = c->resp;
    size_t buffer_s = c->resp_s;
    c->resp = NULL;
    c->resp_s = 0;
    if (!keep) {
        if (c->ssl.enabled) {
            SSL_free(c->ssl.ssl);
            SSL_CTX_free(c->ssl.ctx);
        }
        close(c->sockfd);
        free(c->host);
    }
    return buffer_s;
}
void br_protocol_print(BrConnection* c)
{
    printf("{host:%s,ip: %s, port: %d, protocol: %d, ssl_enabled: %d}\n",
        c->host, c->ip, c->port, c->protocol, c->ssl.enabled);
}

static int __parse_port(const char* URI)
{
    char* port_str;
    // Check if the given Ip Address
    if ((port_str = strstr(URI, ":")), port_str != NULL) {
        char* endptr = NULL;
        int p = strtol(&port_str[1], &endptr, 10);
        if (*endptr == 0) {
            port_str[0] = 0;
            return p;
        }
    }
    return -1;
}

static int __try_ssl(BrConnection* c)
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

static BR_NET_STATUS __setup_address(BrConnection* c, const char* uri, int proto_idx)
{
    // if port can not be obtained from the URI, fallback to the protocols
    const char* uri_trimmed = uri + proto_idx;
    int port = __parse_port(uri_trimmed);
    char* uri_n = strdup(uri_trimmed);
    if (port != -1) {
        c->port = port;
        __strip_colon(uri_n);
    } else {
        c->port = RequestTypePort[c->protocol];
    }
    if (__is_ip_address(uri)) {
        c->host = __uri_from_ip(uri_n);
        memcpy(&c->ip, uri, 16);
    } else {
        c->host = uri_n;
        char* ip = __ip_from_uri(c->host);
        if (ip == NULL) {
            free(uri_n);
            return ERROR(BR_NET_ERROR_IP_NOT_FOUND);
        }
        memcpy(&c->ip, ip, 16);
    }
    return BR_NET_STATUS_OK;
}

static BR_PROTOCOL __capture_protocol(const char* uri, int* start_addr)
{
    if (!strncmp("gemini", uri, 6)) {
        // gemini://
        *start_addr = 9;
        return BR_PROTOCOL_GEMINI;
    } else if (!strncmp("https", uri, 5)) {
        // https://
        *start_addr = 8;
        return BR_PROTOCOL_HTTPS;
    } else if (!strncmp("http", uri, 4)) {
        *start_addr = 7;
        return BR_PROTOCOL_HTTP;
    } else if (!strncmp("gopher", uri, 6)) {
        *start_addr = 9;
        return BR_PROTOCOL_GOPHER;
    }
    return BR_PROTOCOL_UNSUPPORTED;
}
