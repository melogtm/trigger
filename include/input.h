#ifndef INPUT_H
#define INPUT_H

#define TRIGGER_TOK_BUFFER_SIZE 64
#define TRIGGER_TOK_DELIM " \t\r\n\a"

// Input reading and parsing functions
char *trigger_read_line(void);
char **trigger_split_line(char *line);

#endif // INPUT_H

