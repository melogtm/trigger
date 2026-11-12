#ifndef BUILTINS_H
#define BUILTINS_H

typedef struct {
    const char *name;
    int (*fn)(char **);
} Builtin;

int trigger_cd(char **args);
int trigger_help(char **args);
int trigger_exit(char **args);
int trigger_pwd(char **args);
int trigger_echo(char **args);
int trigger_export(char **args);
int trigger_unset(char **args);

int trigger_num_builtins(void);
const Builtin *find_builtin(const char *name);

#endif
