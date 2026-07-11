#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_TOKEN_BUF_SIZE 128

void *xmalloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

char *xstrdup(const char *s) {
    char *d = strdup(s);
    if (d == NULL) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
    return d;
}

void free_array_of_strings(char **array) {
    if (array) {
        for (int i = 0; array[i] != NULL; i++) {
            free(array[i]);
        }
        free(array);
    }
}

void token_list_free(TokenList *tl) {
    if (tl) {
        for (size_t i = 0; i < tl->count; i++) {
            free(tl->argv[i]);
        }
        free(tl->argv);
        free(tl->glob_eligible);
        free(tl);
    }
}

static int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\a';
}

static void add_char_to_token(char **token_buffer, size_t *token_pos,
                              size_t *token_size, char c) {
    if (*token_pos >= *token_size - 1) {
        if (*token_size > SIZE_MAX / 2) {
            fprintf(stderr, "allocation error\n");
            exit(EXIT_FAILURE);
        }
        *token_size *= 2;
        *token_buffer = xrealloc(*token_buffer, *token_size);
    }
    (*token_buffer)[(*token_pos)++] = c;
}

static void finalize_token(char ***tokens_ptr, size_t *position, size_t *buffer_size,
                           char **token_buffer, size_t *token_pos, size_t *token_size, int force,
                           int **out_glob_eligible, int glob_eligible) {
    (void)token_size;
    if (*token_pos > 0 || force) {
        char **tokens = *tokens_ptr;

        (*token_buffer)[*token_pos] = '\0';

        if (*position >= *buffer_size) {
            if (*buffer_size > SIZE_MAX / 2) {
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
            *buffer_size *= 2;
            tokens = xrealloc(tokens, *buffer_size * sizeof(char *));
            *tokens_ptr = tokens;

            if (out_glob_eligible != NULL) {
                *out_glob_eligible = xrealloc(*out_glob_eligible,
                                               *buffer_size * sizeof(int));
            }
        }

        tokens[*position] = xstrdup(*token_buffer);

        if (out_glob_eligible != NULL) {
            (*out_glob_eligible)[*position] = glob_eligible;
        }

        (*position)++;
        *token_pos = 0;
    }
}

static void process_character(const char c, const char next_c, ParserState *state,
                              char **token_buffer, size_t *token_pos, size_t *token_size,
                              char ***tokens_ptr, size_t *position, size_t *buffer_size,
                              int *skip_next, int *glob_eligible, int **out_glob_eligible) {
    *skip_next = 0;

    switch (*state) {
    case STATE_NORMAL:
        if (c == '\\') {
            *state = STATE_ESCAPED;
        } else if (c == '\'') {
            *state = STATE_IN_SINGLE_QUOTE;
        } else if (c == '"') {
            *state = STATE_IN_DOUBLE_QUOTE;
        } else if (is_whitespace(c)) {
            finalize_token(tokens_ptr, position, buffer_size, token_buffer, token_pos, token_size,
                           0, out_glob_eligible, *glob_eligible);
            *glob_eligible = 1;
        } else {
            add_char_to_token(token_buffer, token_pos, token_size, c);
        }
        break;

    case STATE_ESCAPED:
        add_char_to_token(token_buffer, token_pos, token_size, c);
        *glob_eligible = 0;
        *state = STATE_NORMAL;
        break;

    case STATE_IN_SINGLE_QUOTE:
        if (c == '\'') {
            *state = STATE_NORMAL;
            if (next_c == '\0' || is_whitespace(next_c)) {
                finalize_token(tokens_ptr, position, buffer_size, token_buffer, token_pos,
                               token_size, 1, out_glob_eligible, *glob_eligible);
                *glob_eligible = 1;
            }
        } else {
            add_char_to_token(token_buffer, token_pos, token_size, c);
            *glob_eligible = 0;
        }
        break;

    case STATE_IN_DOUBLE_QUOTE:
        if (c == '"') {
            *state = STATE_NORMAL;
            if (next_c == '\0' || is_whitespace(next_c)) {
                finalize_token(tokens_ptr, position, buffer_size, token_buffer, token_pos,
                               token_size, 1, out_glob_eligible, *glob_eligible);
                *glob_eligible = 1;
            }
        } else if (c == '\\' && next_c != '\0') {
            if (next_c == '"' || next_c == '\\' || next_c == '$' || next_c == ' ') {
                add_char_to_token(token_buffer, token_pos, token_size, next_c);
                *glob_eligible = 0;
                *skip_next = 1;
            } else {
                add_char_to_token(token_buffer, token_pos, token_size, c);
                *glob_eligible = 0;
            }
        } else {
            add_char_to_token(token_buffer, token_pos, token_size, c);
            *glob_eligible = 0;
        }
        break;
    }
}

TokenList *parse_line_with_quotes(const char *line) {
    size_t buffer_size = TRIGGER_TOK_BUFFER_SIZE;
    size_t position = 0;
    char **tokens = xmalloc(buffer_size * sizeof(char *));

    int *glob_eligible_arr = xmalloc(buffer_size * sizeof(int));

    size_t token_size = INITIAL_TOKEN_BUF_SIZE;
    size_t token_pos = 0;
    char *token_buffer = xmalloc(token_size);

    ParserState state = STATE_NORMAL;
    int glob_eligible = 1;
    size_t i = 0;

    while (line[i] != '\0') {
        int skip_next = 0;
        const char next_char = line[i + 1];

        process_character(line[i], next_char, &state, &token_buffer, &token_pos, &token_size,
                          &tokens, &position, &buffer_size, &skip_next, &glob_eligible,
                          &glob_eligible_arr);

        if (skip_next) {
            i++;
        }
        i++;
    }

    if (state == STATE_IN_SINGLE_QUOTE) {
        fprintf(stderr, "Error: Unclosed single quote\n");
        free(token_buffer);
        tokens[position] = NULL;
        free_array_of_strings(tokens);
        free(glob_eligible_arr);
        return NULL;
    }

    if (state == STATE_IN_DOUBLE_QUOTE) {
        fprintf(stderr, "Error: Unclosed double quote\n");
        free(token_buffer);
        tokens[position] = NULL;
        free_array_of_strings(tokens);
        free(glob_eligible_arr);
        return NULL;
    }

    finalize_token(&tokens, &position, &buffer_size, &token_buffer, &token_pos, &token_size, 0,
                   &glob_eligible_arr, glob_eligible);

    if (position >= buffer_size) {
        if (buffer_size > SIZE_MAX / 2) {
            fprintf(stderr, "allocation error\n");
            exit(EXIT_FAILURE);
        }
        buffer_size *= 2;
        tokens = xrealloc(tokens, buffer_size * sizeof(char *));
    }

    tokens[position] = NULL;

    TokenList *tl = xmalloc(sizeof(TokenList));
    tl->argv = tokens;
    tl->glob_eligible = glob_eligible_arr;
    tl->count = position;

    free(token_buffer);

    return tl;
}
