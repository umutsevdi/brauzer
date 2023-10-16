#include "assert.h"
#include "br_util.h"
typedef void (*TestFn)(void);
#define TEST(t)                                                                \
    {                                                                          \
        printf("[  ] TEST %s", #t);                                            \
        t();                                                                   \
        printf("\r[OK] TEST %s\n", #t);                                        \
    }

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
    assert(!is_ip("https://www.google.com"));
    assert(!is_ip("www.google.com"));
    assert(!is_ip(""));
    assert(!is_ip(NULL));
}

int main(void)
{
    TEST(test_is_ip);
    TEST(test_is_null_terminated);
}
