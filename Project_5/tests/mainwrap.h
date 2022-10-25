#ifndef MAINWRAP_H
#define MAINWRAP_H

#ifndef WRAP_FILE
#   define WRAP_FILE "../main.c"
#endif

#ifdef WRAP_INCLUDE
#   define main __PB071_wrapped_main
#   include WRAP_FILE
#   undef main
#else
int __PB071_wrapped_main(int argc, char **argv);
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exitus.h"

static inline
void *_xmalloc(size_t size)
{
    void *ptr = malloc(size);

    /* This is NOT how we should handle memory allocation failure in clean code.
     * However, tests cannot sanily continue upon this failure anyway, so this
     * is the only thing we can do. */
    if (ptr == NULL) {
        fprintf(stderr, "Fatal test error: Cannot allocate memory\n");
        abort();
    }

    return ptr;
}

static inline
char *_xstrdup(const char *str)
{
    return strcpy(_xmalloc(strlen(str) + 1), str);
}

static inline
int __app_main_2(int argc, const char *argv[])
{
    char **args_copy = _xmalloc((argc + 1) * sizeof(char *));
    for (int argix = 0; argix <= argc; ++argix)
        args_copy[argix] = argv[argix] == NULL ? NULL : _xstrdup(argv[argix]);

    int retval = exitus(__PB071_wrapped_main, argc, args_copy);

    for (int argix = 0; argix <= argc; ++argix)
        free(args_copy[argix]);
    free(args_copy);

    return retval;
}

static inline
int __app_main_1(const char *argv[])
{
    return __app_main_2(exitus_count_args((char **) argv), argv);
}

/* A little bit of magic never hurt anyone. */
#define app_main(...)                                               \
    __app_main_1((const char *[]){ "app", __VA_ARGS__, NULL })

#endif // MAINWRAP_H
