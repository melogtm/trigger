#ifndef EXECUTE_H
#define EXECUTE_H

#include "utils/utils.h"
#include <stdbool.h>

typedef struct {
    int status;
    bool should_exit;
} ExecuteResult;

int trigger_launch(char **args);
ExecuteResult trigger_execute(TokenList *tl);

#endif
