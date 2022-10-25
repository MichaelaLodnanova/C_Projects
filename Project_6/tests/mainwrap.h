#ifndef MAINWRAP_H
#define MAINWRAP_H

#ifndef WRAP_SYMBOL
#   define WRAP_SYMBOL student_main
#endif

#ifndef WRAP_FILE
#   define WRAP_FILE "../src/main.c"
#endif

#ifndef WRAP_INDIRECT
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
int WRAP_SYMBOL(int argc, char *argv[])
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

#endif // MAINWRAP_H
