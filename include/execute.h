#ifndef EXECUTE_H
#define EXECUTE_H

#define CHILD_PROCESS_EXITED 0
#define EXEC_RETURNED_FAILURE (-1)

int trigger_launch(char **args);
int trigger_execute(char **args);

#endif

