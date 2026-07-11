#ifndef INPUT_H
#define INPUT_H

char *trigger_read_line(void);
char **trigger_split_line(const char *line);
char **trigger_split_line_ex(const char *line, int **out_glob_eligible);

#endif
