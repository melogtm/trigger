#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>

#define TRIGGER_TOK_BUFFER_SIZE 64

typedef struct TokenList {
    char **argv;
    int *glob_eligible;
    size_t count;
} TokenList;

typedef enum {
    STATE_NORMAL,
    STATE_IN_SINGLE_QUOTE,
    STATE_IN_DOUBLE_QUOTE,
    STATE_ESCAPED
} ParserState;

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);

void free_array_of_strings(char **array);
void token_list_free(TokenList *tl);

TokenList *parse_line_with_quotes(const char *line);

#endif
