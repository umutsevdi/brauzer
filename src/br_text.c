#include "br_txt.h"
/**
 * Moves inside the given NULL terminated string until a non-space character
 * is found, returning its address */
char* _gtxt_space_trim(char* line);
bool _gtxt_parse(BrGemtextItem* text, char* line, bool is_pre);

typedef enum {
    _GTXSMB_LINK,
    _GTXSMB_PRE,
    _GTXSMB_LIST,
    _GTXSMB_QUOTE,
    _GTXSMB_H3,
    _GTXSMB_H2,
    _GTXSMB_H,
} _GTXSMB;
static const char* gtxt_strs[] = {"=>", "```", "*", ">", "###", "##", "#"};

void br_gemtext_new(BrGemtext* t, char* buffer, size_t buffer_s)
{
    size_t l_count = 0;
    for (size_t i = 0; i < buffer_s; i++)
        if (buffer[i] == '\n')
            l_count++;
    t->line_s = l_count;
    t->line = calloc(l_count, sizeof(BrGemtextItem));
    char* l;
    size_t i = 0;
    bool is_pre = false;
    while (buffer != NULL) {
        l = strsep(&buffer, "\n");
        is_pre = _gtxt_parse((t->line + i), l, is_pre);
        i++;
    }
}

void br_gemtext_print(BrGemtext* t)
{
    printf("BrGemtext{line_s:%ld, line: [\n", t->line_s);
    for (size_t i = 0; i < t->line_s; i++) {
        printf(BrGemtextItem_unwrap(&(t->line[i])));
    }
    printf("]\n");
}

void br_gemtext_free(BrGemtext* t)
{
    free(t->line);
    t->line_s = 0;
    t->line = 0;
}

bool _gtxt_parse(BrGemtextItem* text, char* line, bool is_pre)
{
    // if is_pre ignore all formatting and continue until PRE is cancelled
    if (is_pre) {
        text->type = BR_GEMTXT_PRE;
        text->line = line;
        if (!strncmp(line, gtxt_strs[_GTXSMB_PRE], 3)) {
            text->line += 3;
            return false;
        }
        return true;
    }
    // if pre is not defined check if pre toggle is beginning
    if (!strncmp(line, gtxt_strs[_GTXSMB_PRE], 3)) {
        text->type = BR_GEMTXT_PRE;
        text->line = line + 3;
        return true;
    }
    // otherwise check for rest of the types
    else if (!strncmp(line, gtxt_strs[_GTXSMB_LINK], 2)) {
        text->type = BR_GEMTXT_LINK;
        text->URI = _gtxt_space_trim(line + 2);
        text->line = strchr(text->URI, ' ');
        if (text->line != NULL) {
            *text->line = 0;
            text->line++;
        }
    } else if (!strncmp(line, gtxt_strs[_GTXSMB_QUOTE], 1)) {
        text->type = BR_GEMTXT_QUOTE;
        text->line = _gtxt_space_trim(line + 1);
    } else if (!strncmp(line, gtxt_strs[_GTXSMB_LIST], 1)) {
        text->type = BR_GEMTXT_LIST;
        text->line = _gtxt_space_trim(line + 1);
    } else if (!strncmp(line, gtxt_strs[_GTXSMB_H3], 3)) {
        text->type = BR_GEMTXT_H3;
        text->line = _gtxt_space_trim(line + 3);
    } else if (!strncmp(line, gtxt_strs[_GTXSMB_H2], 2)) {
        text->type = BR_GEMTXT_H2;
        text->line = _gtxt_space_trim(line + 2);
    } else if (!strncmp(line, gtxt_strs[_GTXSMB_H], 1)) {
        text->type = BR_GEMTXT_H;
        text->line = _gtxt_space_trim(line + 1);
    } else {
        text->type = BR_GEMTXT_TXT;
        text->line = _gtxt_space_trim(line);
    }
    return false;
}

char* _gtxt_space_trim(char* line)
{
    char* c = line;
    while ((*c == ' ' || *c == '\t') && *c != 0 && *c != '\n')
        c++;
    return c;
}

void br_gprtext_new(BrGprtext* t, char* buffer, size_t buffer_s)
{
    size_t l_count = 0;
    for (size_t i = 0; i < buffer_s; i++)
        if (buffer[i] == '\n')
            l_count++;
    t->line_s = l_count;
    t->line = calloc(l_count, sizeof(BrGprtextItem));
    char* l;
    size_t i = 0;
    while (buffer != NULL) {
        l = strsep(&buffer, "\n");
        t->line[i].type = (BR_GPRTXT)l[0];
        switch (l[0]) {
        case BR_GPRTXT_ERROR:
        case BR_GPRTXT_INFO: t->line[i].line = l + 1; break;
        default:
            t->line[i].line = _gtxt_space_trim(l + 1);
            t->line[i].URI = strchr(t->line[i].line, '\t');
            if (t->line[i].URI != NULL) {
                *t->line[i].URI = 0;
                t->line[i].URI++;
            }
        }
        i++;
    }
}

void br_gprtext_print(BrGprtext* t)
{
    printf("BrGophertext{line_s:%ld, line: [\n", t->line_s);
    for (size_t i = 0; i < t->line_s; i++) {
        printf(BrGprtextItem_unwrap(&(t->line[i])));
    }
    printf("]\n");
}

void br_gprtext_free(BrGprtext* t)
{
    free(t->line);
    t->line_s = 0;
    t->line = 0;
}
