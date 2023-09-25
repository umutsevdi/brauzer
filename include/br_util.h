/******************************************************************************

 * File: include/br_util.h
 *
 * Author: Umut Sevdi
 * Created: 09/11/23
 * Description: Common functions and data structures for Brauzer

*****************************************************************************/
#pragma once
#include <stddef.h>
#include <stdio.h>

#define MAXLINE 256
#define PRINT(FMT, ARGS...) printf(__FILE__ #FMT "\r\n" ARGS)
#define WARN(CODE) (fprintf(stderr, __FILE__ ":%d " #CODE "\r\n", __LINE__), CODE)

extern const int RequestTypePort[];
