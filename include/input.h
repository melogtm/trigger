#ifndef INPUT_H
#define INPUT_H

#define TRIGGER_TOK_BUFFER_SIZE 64
#define TRIGGER_AUTOMATIC_BUFFER_SIZE 0
#define GETLINE_REACHED_END_OF_FILE_OR_ERROR -1
#define TRIGGER_TOK_DELIM " \t\r\n\a"

char *trigger_read_line(void);
char **trigger_split_line(char *line);

#endif

