#ifndef NEPETA_H
#define NEPETA_H

#include <stdbool.h>
#include <stdio.h>

bool trigger_nepeta_is_enabled(void);
void trigger_nepeta_set_enabled(bool enabled);
void trigger_nepeta_init_from_env(void);
const char *trigger_nepeta_prompt(void);
void trigger_nepeta_print_banner(void);
char *trigger_nepeta_translate(const char *plain_text);
void trigger_nepeta_say(FILE *stream, const char *plain_text);
const char *trigger_nepeta_random_quote(void);

#endif
