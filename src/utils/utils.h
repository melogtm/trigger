#ifndef UTILS_H
#define UTILS_H

#define true 1
#define false 0

typedef enum {
    STATE_NORMAL,
    STATE_IN_SINGLE_QUOTE,
    STATE_IN_DOUBLE_QUOTE,
    STATE_ESCAPED
} ParserState;

typedef struct {
    char **tokens;
    int count;
    int buffer_size;
} ParseResult;

void print_array_of_tokens(char** tokens);

int is_whitespace(char c);

void add_char_to_token(char **token_buffer, int *token_pos, int *token_size, char c);

void finalize_token(char ***tokens_ptr, int *position, int *buffer_size,
                    char **token_buffer, int *token_pos, int *token_size, int force);

char** parse_line_with_quotes(const char *line);

#endif

