#include "nepeta.h"
#include "utils/utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static bool nepeta_enabled = false;

bool trigger_nepeta_is_enabled(void) { return nepeta_enabled; }

void trigger_nepeta_set_enabled(bool enabled) { nepeta_enabled = enabled; }

void trigger_nepeta_init_from_env(void) {
    const char *val = getenv("TRIGGER_NEPETA");
    if (val != NULL && val[0] != '\0' && strcmp(val, "0") != 0) {
        nepeta_enabled = true;
    }
}

const char *trigger_nepeta_prompt(void) {
    if (nepeta_enabled) {
        return ":33 < ";
    }
    return "> ";
}

typedef struct {
    const char *from;
    const char *to;
} PunEntry;

static const PunEntry pun_dict[] = {
    {"for", "fur"},
    {"perfect", "purrfect"},
    {"great", "clawesome"},
    {"awesome", "pawsome"},
    {"friend", "furriend"},
    {"now", "meow"},
    {"help", "halp"},
};

static const size_t pun_count = sizeof(pun_dict) / sizeof(pun_dict[0]);

static int alpha_run_len(const char *s) {
    int n = 0;
    while (s[n] != '\0' && isalpha((unsigned char) s[n])) {
        n++;
    }
    return n;
}

static bool alpha_run_equals(const char *s, int slen, const char *dict_word) {
    size_t dwlen = strlen(dict_word);
    if ((size_t) slen != dwlen) {
        return false;
    }
    for (size_t i = 0; i < dwlen; i++) {
        if (tolower((unsigned char) s[i]) != tolower((unsigned char) dict_word[i])) {
            return false;
        }
    }
    return true;
}

static const char *find_pun(const char *s, int slen) {
    for (size_t i = 0; i < pun_count; i++) {
        if (alpha_run_equals(s, slen, pun_dict[i].from)) {
            return pun_dict[i].to;
        }
    }
    return NULL;
}

static void append_char(char **buf, size_t *cap, size_t *len, char c) {
    if (*len + 1 >= *cap) {
        *cap = *cap == 0 ? 64 : *cap * 2;
        *buf = xrealloc(*buf, *cap);
    }
    (*buf)[*len] = c;
    (*len)++;
}

static void append_str(char **buf, size_t *cap, size_t *len, const char *s) {
    for (const char *p = s; *p != '\0'; p++) {
        append_char(buf, cap, len, *p);
    }
}

char *trigger_nepeta_translate(const char *plain_text) {
    if (plain_text == NULL) {
        char *empty = xmalloc(1);
        empty[0] = '\0';
        return empty;
    }

    char *result = NULL;
    size_t cap = 0;
    size_t len = 0;

    const char *p = plain_text;

    while (*p != '\0') {
        if (isalpha((unsigned char) *p)) {
            int run = alpha_run_len(p);
            const char *pun = find_pun(p, run);

            if (pun != NULL) {
                char first = p[0];
                if (isupper((unsigned char) first)) {
                    append_char(&result, &cap, &len, (char) toupper((unsigned char) pun[0]));
                    append_str(&result, &cap, &len, pun + 1);
                } else {
                    append_char(&result, &cap, &len, (char) tolower((unsigned char) pun[0]));
                    append_str(&result, &cap, &len, pun + 1);
                }
            } else {
                for (int i = 0; i < run; i++) {
                    append_char(&result, &cap, &len, p[i]);
                }
            }
            p += run;
        } else {
            append_char(&result, &cap, &len, *p);
            p++;
        }
    }

    append_char(&result, &cap, &len, '\0');
    return result;
}

void trigger_nepeta_say(FILE *stream, const char *plain_text) {
    if (!nepeta_enabled) {
        fputs(plain_text, stream);
        return;
    }

    char *translated = trigger_nepeta_translate(plain_text);
    fprintf(stream, ":33 < %s", translated);
    free(translated);
}

static const char *quotes[] = {
    ":33 < *ac scratches at the terminal prompt*",
    ":33 < purractice makes purrfect! hehe",
    ":33 < *pounces on the enter key* rawr!",
    ":33 < this shell is the cat's pajamas!",
    ":33 < *curls up on the keyboard* warm~",
    ":33 < fur the glory of the hunt!",
    ":33 < *chases the cursor with big kitty eyes*",
    ":33 < all your base are belong to Nepeta!",
    ":33 < *batbatbat* at the command line",
    ":33 < meow is the time fur action!",
};

static const size_t quote_count = sizeof(quotes) / sizeof(quotes[0]);

const char *trigger_nepeta_random_quote(void) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned) time(NULL));
        seeded = 1;
    }
    return quotes[rand() % quote_count];
}

void trigger_nepeta_print_banner(void) {
    if (!nepeta_enabled) {
        return;
    }

    printf(":33 < *Nepeta Leijon has entered the terminal* ");
    printf("purrfect mode activated, %s, fur great justice!\n", trigger_nepeta_random_quote());
}
