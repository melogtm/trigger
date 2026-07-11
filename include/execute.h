#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdbool.h>

typedef struct TokenList TokenList;

typedef struct {
    int status;
    bool should_exit;
} ExecuteResult;

int trigger_launch(char **args);
ExecuteResult trigger_execute(TokenList *tl);

#endif
