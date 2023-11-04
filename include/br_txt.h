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
    "|type:%5s|\tURI:%-30s|\t%s\n", BR_GEMTXT_S((r)->type),                  \
        (r)->URI != NULL ? (r)->URI : "", (r)->line

typedef struct {
    BrGemtextItem* line;
    size_t line_s;
} BrGemtext;

/** Parses given buffer to a valid Gemtext document */
void br_gemtext_new(BrGemtext* t, char* buffer, size_t buffer_s);
void br_gemtext_print(BrGemtext* t);
void br_gemtext_free(BrGemtext* t);

typedef enum {
    BR_GPRTXT_TXT = '0', // Regular text
    BR_GPRTXT_LINK = '1',// Link
    BR_GPRTXT_ERROR = '3',
    BR_GPRTXT_HQX = '4',
    BR_GPRTXT_DOS = '5',
    BR_GPRTXT_QUERY = '7',
    BR_GPRTXT_BIN_GEN = '9',
    BR_GPRTXT_GIF = 'g',
    BR_GPRTXT_IMG = 'I',
    BR_GPRTXT_WAV = 's',
    BR_GPRTXT_INFO = 'i',
    BR_GPRTXT_DOC = 'd',
    BR_GPRTXT_HTML = 'h',
    BR_GPRTXT_PNG = 'p',
    BR_GPRTXT_PDF = 'P',
    BR_GPRTXT_MOV = ';',
    BR_GPRTXT_BMP = ':',
    BR_GPRTXT_XML = 'x',
} BR_GPRTXT;
#define BR_GPRTXT_S(s)                                                         \
    s == BR_GPRTXT_TXT       ? "TXT"                                           \
    : s == BR_GPRTXT_LINK    ? "LNK"                                           \
    : s == BR_GPRTXT_QUERY   ? "QRY"                                           \
    : s == BR_GPRTXT_HQX     ? "HQX"                                           \
    : s == BR_GPRTXT_DOS     ? "DOS"                                           \
    : s == BR_GPRTXT_BIN_GEN ? "GNR"                                           \
    : s == BR_GPRTXT_GIF     ? "GIF"                                           \
    : s == BR_GPRTXT_IMG     ? "IMG"                                           \
    : s == BR_GPRTXT_WAV     ? "WAV"                                           \
    : s == BR_GPRTXT_ERROR   ? "ERR"                                           \
    : s == BR_GPRTXT_INFO    ? "INF"                                           \
    : s == BR_GPRTXT_DOC     ? "DOC"                                           \
    : s == BR_GPRTXT_HTML    ? "HTML"                                          \
    : s == BR_GPRTXT_PNG     ? "PNG"                                           \
    : s == BR_GPRTXT_PDF     ? "PDF"                                           \
    : s == BR_GPRTXT_MOV     ? "MOV"                                           \
    : s == BR_GPRTXT_BMP     ? "BMP"                                           \
    : s == BR_GPRTXT_XML     ? "XML"                                           \
                             : "NULL"

typedef struct {
    BR_GPRTXT type;
    char* line;
    char* URI;
} BrGprtextItem;

typedef struct {
    BrGprtextItem* line;
    size_t line_s;
} BrGprtext;

#define BrGprtextItem_unwrap(r)                                                \
    "type: %5s\tURI: %30s\t%s\n", BR_GPRTXT_S((r)->type),                  \
        (r)->URI != NULL ? (r)->URI : "", (r)->line

void br_gprtext_new(BrGprtext* t, char* buffer, size_t buffer_s);
void br_gprtext_print(BrGprtext* t);
void br_gprtext_free(BrGprtext* t);
