#ifndef BUILTINS_H
#define BUILTINS_H

int trigger_cd(char **args);
int trigger_help(char **args);
int trigger_exit(char **args);
int trigger_num_builtins(void);

extern char *builtin_str[];
extern int (*builtin_func[])(char **);

#endif
