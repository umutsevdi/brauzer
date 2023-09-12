/******************************************************************************

 * File: include/br_util.h
 *
 * Author: Umut Sevdi
 * Created: 09/11/23
 * Description: Common functions and data structures for Brauzer

*****************************************************************************/
#include <stddef.h>
#ifndef BR_UTIL

typedef struct {
    size_t size;
    char* data;
} BrBuffer;

#define MAXLINE 256
#define WARN(CODE) (printf(#CODE "\r\n"), CODE)

#endif // !BR_UTIL
