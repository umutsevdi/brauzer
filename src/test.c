#include "assert.h"
#include "br_net.h"
#include "br_protocols.h"
#include "br_util.h"
#include <string.h>
typedef void (*TestFn)(void);
#define TEST(t)                                                                \
    {                                                                          \
        fputs("[  ] TEST " #t, stdout);                                        \
        fflush(stdout);                                                        \
        test_##t();                                                            \
        fputs("\r[OK] TEST " #t "\n", stdout);                                 \
    }

/******************************************************************************
                                br_util.h
*****************************************************************************/

void test_is_null_terminated()
{
    assert(is_null_terminated("", -1));
    assert(is_null_terminated("str", -1));
    char* c = malloc(3 * sizeof(char));
    c[0] = 1;
    c[1] = 2;
    c[2] = 3;
    c[3] = 4;
    assert(!is_null_terminated(c, 3));
    free(c);
    char k[MAX_URI_LENGTH + 2];
    memset(k, 'a', MAX_URI_LENGTH + 1);
    assert(!is_null_terminated(k, -1));
    assert(!is_null_terminated(NULL, -1));
}

void test_is_ip()
{
    assert(is_ip("192.168.1.10"));
    assert(!is_ip("100"));
    assert(!is_ip("100.100.100.100.100"));
    assert(!is_ip("496.496.496.496"));
    assert(!is_ip("https://some.url.com"));
    assert(!is_ip("some.url.com"));
    assert(!is_ip(""));
    assert(!is_ip(NULL));
}

void test_uri_strip()
{
    char* u1 = NULL;
    char u2[] = "test:test";
    char u3[] = "test_text";
    char u4[] = "test:";
    char u5[] = "";
    char u6[] = "::";
    uri_strip(u1);
    uri_strip(u2);
    uri_strip(u3);
    uri_strip(u4);
    uri_strip(u5);
    uri_strip(u6);
    assert(u1 == NULL);
    assert(!strncmp(u2, "test", 4));
    assert(!strncmp(u3, "test_text", 9));
    assert(!strncmp(u4, "test", 4));
    assert(!strncmp(u5, "", 1));
    assert(!strncmp(u6, "", 1));
}

void test_to_abs_path()
{
    assert(to_abs_path(NULL, "/") == NULL);
    assert(to_abs_path("some.url.com", NULL) == NULL);
    assert(
        !strncmp(to_abs_path("gemini://some.url/", "gemini://some.url/search"),
                 "gemini://some.url/search", 24));
    assert(!strncmp(to_abs_path("https://some.url.com/", "/search"),
                    "https://some.url.com/search", 27));
    assert(!strncmp(to_abs_path("https://some.url.com/", "search"),
                    "https://some.url.com/search", 27));
}

void test_parse_port()
{
    assert(parse_port(NULL) == -1);
    assert(parse_port("some.url.com") == -1);
    assert(parse_port("255.255.255.255") == -1);
    char c[] = "255.255.255.255:24";
    assert(parse_port(c) == 24);
    char d[] = "https://some.url.com:443";
    assert(parse_port(d) == -1);
    char e[] = "some.url.com:25";
    assert(parse_port(e) == 25);
    char f[] = "some.url.com:";
    assert(parse_port(f) == 0);
}

void test_capture_protocol()
{
    int start_addr;
    char uri[] = "https://some.url.com";
    assert(capture_protocol(uri, &start_addr) == BR_PROTOCOL_HTTPS);
    assert(!strcmp(uri + start_addr, "some.url.com"));

    char uri2[] = "gemini://some.url.com:1965/path/to?search=10";
    assert(capture_protocol(uri2, &start_addr) == BR_PROTOCOL_GEMINI);
    assert(start_addr == 9);

    assert(capture_protocol(NULL, &start_addr) == BR_PROTOCOL_UNSUPPORTED);
    assert(capture_protocol(uri2, NULL) == BR_PROTOCOL_GEMINI);
}

/******************************************************************************
                                br_protocols.h
*****************************************************************************/

void test_br_http_response_new()
{
    BrSession s = {0};
    s.req = malloc(1024 * sizeof(char));
    sprintf(s.req, "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    s.req_s = 1024;
    s.protocol = BR_PROTOCOL_HTTPS;
    s.resp = malloc(1024 * sizeof(char));
    sprintf(s.resp, "HTTP/1.1 301 Moved Permanently\r\nHost: "
                    "www.duckduckgo.com\r\n\r\n<html><head><title>301 Moved"
                    "Permanently</title></head>"
                    "<body> <center><h1>301 Moved Permanently</h1></center>"
                    "<hr><center>nginx</center> </body> </html>\r\n");
    BrHttpResponse r = {0};
    assert(br_http_response_new(&s, &r) == BR_PRT_HTTP_OK);
    assert(r.status == 301);
    assert(r.req != NULL && r.body != NULL);
    assert(r.headers != NULL);
    assert(g_hash_table_contains(r.headers, "Host"));
}

void test_br_http_response_new_when_no_headers()
{
    BrSession s = {0};
    s.req = malloc(1024 * sizeof(char));
    sprintf(s.req, "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    s.req_s = 1024;
    s.protocol = BR_PROTOCOL_HTTPS;
    s.resp = malloc(1024 * sizeof(char));
    sprintf(s.resp, "HTTP/1.1 404 Not Found\r\n\r\n<html><head><title>Not Found"
                    "</title></head>"
                    "<body><hr><center>nginx</center> </body> </html>\r\n");
    BrHttpResponse r = {0};
    assert(br_http_response_new(&s, &r) == BR_PRT_HTTP_OK);
    assert(r.status == 404);
    assert(r.req != NULL && r.body != NULL);
    assert(r.headers == NULL);
}

void test_br_http_response_new_when_invalid_headers()
{
    BrSession s = {0};
    s.req = malloc(1024 * sizeof(char));
    sprintf(s.req, "\b\\k\r\nd\\009;0");
    s.req_s = 1024;
    s.protocol = BR_PROTOCOL_HTTPS;
    s.resp = malloc(1024 * sizeof(char));
    sprintf(s.resp, "\b\\k\r\nd\\009;0\r\r\r\n\r\n<html><head><title>Not Found"
                    "</title></head>"
                    "<body><hr><center>nginx</center> </body> </html>\r\n");
    BrHttpResponse r = {0};
    assert(br_http_response_new(&s, &r) == BR_PRT_HTTP_OK);
}

int main(void)
{
    TEST(is_ip);
    TEST(is_null_terminated);
    TEST(uri_strip);
    TEST(to_abs_path);
    TEST(parse_port);
    TEST(capture_protocol);
    TEST(br_http_response_new);
    TEST(br_http_response_new_when_no_headers);
    TEST(br_http_response_new_when_invalid_headers);
}
