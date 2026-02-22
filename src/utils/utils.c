#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define TRIGGER_TOK_BUFFER_SIZE 64

int is_whitespace(const char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\a';
}

void add_char_to_token(char **token_buffer, int *token_pos, int *token_size, const char c) {
    if (*token_pos >= *token_size - 1) {
        *token_size *= 2;
        char *new_buffer = realloc(*token_buffer, *token_size);

        if (!new_buffer) {
            fprintf(stderr, "allocation error\n");
            exit(EXIT_FAILURE);
        }

        *token_buffer = new_buffer;
    }
    (*token_buffer)[(*token_pos)++] = c;
}

void finalize_token(char ***tokens_ptr, int *position, int *buffer_size,
                    char **token_buffer, int *token_pos, int *token_size, int force) {
    if (*token_pos > 0 || force) {
        char **tokens = *tokens_ptr;

        (*token_buffer)[*token_pos] = '\0';

        if (*position >= *buffer_size) {
            *buffer_size += TRIGGER_TOK_BUFFER_SIZE;
            char **new_tokens = realloc(tokens, *buffer_size * sizeof(char *));

            if (!new_tokens) {
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }

            tokens = new_tokens;
            *tokens_ptr = tokens;
        }

        tokens[*position] = strdup(*token_buffer);
        (*position)++;

        *token_pos = 0;
    }
}

static void process_character(const char c, const char next_c, ParserState *state,
                             char **token_buffer, int *token_pos, int *token_size,
                             char ***tokens_ptr, int *position, int *buffer_size,
                             int *skip_next) {
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
                finalize_token(tokens_ptr, position, buffer_size,
                               token_buffer, token_pos, token_size, false);
            } else {
                add_char_to_token(token_buffer, token_pos, token_size, c);
            }
            break;

        case STATE_ESCAPED:
            add_char_to_token(token_buffer, token_pos, token_size, c);
            *state = STATE_NORMAL;
            break;

        case STATE_IN_SINGLE_QUOTE:
            if (c == '\'') {
                *state = STATE_NORMAL;
                if (next_c == '\0' || is_whitespace(next_c)) {
                    finalize_token(tokens_ptr, position, buffer_size,
                                 token_buffer, token_pos, token_size, true);
                }
            } else {
                add_char_to_token(token_buffer, token_pos, token_size, c);
            }
            break;

        case STATE_IN_DOUBLE_QUOTE:
            if (c == '"') {
                *state = STATE_NORMAL;
                if (next_c == '\0' || is_whitespace(next_c)) {
                    finalize_token(tokens_ptr, position, buffer_size,
                                 token_buffer, token_pos, token_size, true);
                }
            } else if (c == '\\' && next_c != '\0') {
                if (next_c == '"' || next_c == '\\' || next_c == '$' || next_c == ' ') {
                    add_char_to_token(token_buffer, token_pos, token_size, next_c);
                    *skip_next = true;
                } else {
                    add_char_to_token(token_buffer, token_pos, token_size, c);
                }
            } else {
                add_char_to_token(token_buffer, token_pos, token_size, c);
            }
            break;
    }
}

char** parse_line_with_quotes(const char *line) {

    int buffer_size = TRIGGER_TOK_BUFFER_SIZE;
    int position = 0;
    char **tokens = malloc(buffer_size * sizeof(char *));

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    int token_size = 128;
    int token_pos = 0;
    char *token_buffer = malloc(token_size);

    if (!token_buffer) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    ParserState state = STATE_NORMAL;
    int i = 0;

    while (line[i] != '\0') {
        int skip_next = false;
        const char next_char = line[i + 1];

        process_character(line[i], next_char, &state,
                         &token_buffer, &token_pos, &token_size,
                         &tokens, &position, &buffer_size,
                         &skip_next);

        if (skip_next) {
            i++;
        }
        i++;
    }

    if (state == STATE_IN_SINGLE_QUOTE) {
        fprintf(stderr, "Error: Unclosed single quote\n");
        free(token_buffer);
        free(tokens);
        return NULL;
    }

    if (state == STATE_IN_DOUBLE_QUOTE) {
        fprintf(stderr, "Error: Unclosed double quote\n");
        free(token_buffer);
        free(tokens);
        return NULL;
    }

    finalize_token(&tokens, &position, &buffer_size,
                   &token_buffer, &token_pos, &token_size, false);

    tokens[position] = NULL;

    free(token_buffer);

    return tokens;
}

