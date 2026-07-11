#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_array_of_strings(char **array) {
    if (array) {
        for (int i = 0; array[i] != NULL; i++) {
            free(array[i]);
        }
        free(array);
    }
}

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

void finalize_token(char ***tokens_ptr, int *position, int *buffer_size, char **token_buffer,
                    int *token_pos, int *token_size, int force, int **out_glob_eligible,
                    int glob_eligible) {
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

            if (out_glob_eligible != NULL) {
                int *new_glob = realloc(*out_glob_eligible, *buffer_size * sizeof(int));

                if (!new_glob) {
                    fprintf(stderr, "allocation error\n");
                    exit(EXIT_FAILURE);
                }

                *out_glob_eligible = new_glob;
            }
        }

        tokens[*position] = strdup(*token_buffer);

        if (out_glob_eligible != NULL) {
            (*out_glob_eligible)[*position] = glob_eligible;
        }

        (*position)++;

        *token_pos = 0;
    }
}

static void process_character(const char c, const char next_c, ParserState *state,
                              char **token_buffer, int *token_pos, int *token_size,
                              char ***tokens_ptr, int *position, int *buffer_size, int *skip_next,
                              int *glob_eligible, int **out_glob_eligible) {
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
                           false, out_glob_eligible, *glob_eligible);
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
                               token_size, true, out_glob_eligible, *glob_eligible);
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
                               token_size, true, out_glob_eligible, *glob_eligible);
                *glob_eligible = 1;
            }
        } else if (c == '\\' && next_c != '\0') {
            if (next_c == '"' || next_c == '\\' || next_c == '$' || next_c == ' ') {
                add_char_to_token(token_buffer, token_pos, token_size, next_c);
                *glob_eligible = 0;
                *skip_next = true;
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

char **parse_line_with_quotes(const char *line, int **out_glob_eligible) {

    int buffer_size = TRIGGER_TOK_BUFFER_SIZE;
    int position = 0;
    char **tokens = malloc(buffer_size * sizeof(char *));

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    int *glob_eligible_arr = NULL;

    if (out_glob_eligible != NULL) {
        glob_eligible_arr = malloc(buffer_size * sizeof(int));

        if (!glob_eligible_arr) {
            fprintf(stderr, "allocation error\n");
            exit(EXIT_FAILURE);
        }

        *out_glob_eligible = glob_eligible_arr;
    }

    int token_size = 128;
    int token_pos = 0;
    char *token_buffer = malloc(token_size);

    if (!token_buffer) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    ParserState state = STATE_NORMAL;
    int glob_eligible = 1;
    int **pc_out_glob = (out_glob_eligible != NULL) ? &glob_eligible_arr : NULL;
    int i = 0;

    while (line[i] != '\0') {
        int skip_next = false;
        const char next_char = line[i + 1];

        process_character(line[i], next_char, &state, &token_buffer, &token_pos, &token_size,
                          &tokens, &position, &buffer_size, &skip_next, &glob_eligible,
                          pc_out_glob);

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

        if (out_glob_eligible != NULL) {
            *out_glob_eligible = NULL;
        }

        return NULL;
    }

    if (state == STATE_IN_DOUBLE_QUOTE) {
        fprintf(stderr, "Error: Unclosed double quote\n");
        free(token_buffer);
        tokens[position] = NULL;
        free_array_of_strings(tokens);
        free(glob_eligible_arr);

        if (out_glob_eligible != NULL) {
            *out_glob_eligible = NULL;
        }

        return NULL;
    }

    finalize_token(&tokens, &position, &buffer_size, &token_buffer, &token_pos, &token_size, false,
                   pc_out_glob, glob_eligible);

    if (position >= buffer_size) {
        buffer_size += TRIGGER_TOK_BUFFER_SIZE;
        char **new_tokens = realloc(tokens, buffer_size * sizeof(char *));

        if (!new_tokens) {
            fprintf(stderr, "allocation error\n");
            exit(EXIT_FAILURE);
        }

        tokens = new_tokens;
    }

    tokens[position] = NULL;

    if (out_glob_eligible != NULL) {
        *out_glob_eligible = glob_eligible_arr;
    } else {
        free(glob_eligible_arr);
    }

    free(token_buffer);

    return tokens;
}
