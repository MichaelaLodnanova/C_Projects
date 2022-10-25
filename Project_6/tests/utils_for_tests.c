#include "utils_for_tests.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int file_to_string(const char *f_name, char **buffer)
{
    FILE *f = fopen(f_name, "rb");
    if (!f) {
        return 1;
    }

    if (fseek(f, 0, SEEK_END)) {
        fclose(f);
        return 1;
    }

    long f_size = ftell(f);
    if (f_size == -1L) {
        fclose(f);
        return 1;
    }

    if (fseek(f, 0, SEEK_SET)) {
        fclose(f);
        return 1;
    }

    *buffer = malloc(f_size + 1);
    if (!(*buffer)) {
        fclose(f);
        return 1;
    }

    fread(*buffer, f_size, 1, f);
    if (ferror(f)) {
        fclose(f);
        free(*buffer);
        return 1;
    }

    fclose(f);

    (*buffer)[f_size] = '\0';

    return 0;
}

int string_to_file(const char *f_name, const char *content)
{
    FILE *f = fopen(f_name, "wb");
    if (!f) {
        return 1;
    }

    size_t size_of_content = strlen(content);
    fwrite(content, 1, size_of_content, f);
    if (ferror(f)) {
        fclose(f);
        return 2;
    }

    fclose(f);
    return 0;
}

const char *_match_seek(const char *cursor)
{
    while (*cursor != '\0' && isspace(*cursor))
        ++cursor;
    return cursor;
}

int match(const char *text, const char *pattern)
{
    assert(text != NULL);
    assert(pattern != NULL);

    const char *next_pattern = NULL;

    while (*text != '\0' && *pattern != '\0') {
        if (!isspace(*pattern) || pattern < next_pattern) {
            if (*text != *pattern)
                return 0;
            ++text, ++pattern;
            continue;
        }

        next_pattern = _match_seek(pattern);
        const char *next_text = _match_seek(text);

        if ((*next_pattern == '?' && next_text - text > 1) || (*next_pattern == '+' && next_text == text)) {
            return 0;
        }

        if (strchr("+?*", *next_pattern) != NULL) {
            pattern = next_pattern + 1;
            text = next_text;
        }
    }

    if (*pattern == '\0' && *_match_seek(text) == '\0')
        return 1;

    return 0;
}
