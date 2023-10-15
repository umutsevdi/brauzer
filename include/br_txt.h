/******************************************************************************

 * File: src/br_text.c
 *
 * Author: Umut Sevdi
 * Created: 10/15/23
 * Description: A text processing header for various formats

*****************************************************************************/
#pragma once
#include "br_util.h"

typedef enum {
    BR_GEMTXT_TXT,
    BR_GEMTXT_LINK,
    BR_GEMTXT_H,
    BR_GEMTXT_H2,
    BR_GEMTXT_H3,
    BR_GEMTXT_LIST,
    BR_GEMTXT_QUOTE,
    BR_GEMTXT_PRE,
} BR_GEMTXT;
#define BR_GEMTXT_S(s)                                                         \
    s == BR_GEMTXT_TXT     ? "TXT"                                             \
    : s == BR_GEMTXT_LINK  ? "LNK"                                             \
    : s == BR_GEMTXT_H     ? "H1"                                              \
    : s == BR_GEMTXT_H2    ? "H2"                                              \
    : s == BR_GEMTXT_H3    ? "H3"                                              \
    : s == BR_GEMTXT_LIST  ? "LST"                                             \
    : s == BR_GEMTXT_QUOTE ? "QT"                                              \
    : s == BR_GEMTXT_PRE   ? "PRE"                                             \
                           : "NULL"

typedef struct {
    BR_GEMTXT type;
    char* line;
    char* URI;
} BrGemtextItem;
#define BrGemtextItem_unwrap(r)                                                \
    "type:%4s\tURI:%-30s\tline:%s\n", BR_GEMTXT_S((r)->type),                  \
        (r)->URI != NULL ? (r)->URI : "", (r)->line

typedef struct {
    BrGemtextItem* line;
    size_t line_s;
} BrGemtext;

/** Parses given buffer to a valid Gemtext document */
void br_gemtext_new(BrGemtext* t, char* buffer, size_t buffer_s);
void br_gemtext_print(BrGemtext* t);
void br_gemtext_free(BrGemtext* t);
