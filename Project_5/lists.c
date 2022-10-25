#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lists.h"


void create_paths(struct path_list *path_list) {
    path_list->allocated = 0;
    path_list->used = 0;
    path_list->paths = NULL;
}

int add_path(struct path_list *path_list, char *str) {
    if (path_list->allocated == path_list->used) {
        char **list = NULL;
        if (path_list->allocated == 0) {
            list = malloc(sizeof(char *) * 8);
            path_list->allocated = 8;
        } else {
            list = realloc(path_list->paths, sizeof(char *) * path_list->allocated * 2);
            path_list->allocated = path_list->allocated * 2;
        }

        if (list == NULL) {
            return 1;
        }
        path_list->paths = list;
    }


    char *str_copy = malloc(sizeof(char) * strlen(str) + 1);

    if (str_copy == NULL) {
        return 1;
    }

    strcpy(str_copy, str);

    path_list->paths[path_list->used] = str_copy;
    path_list->used++;

    return 0;
}

void destroy_paths(struct path_list *path_list) {
    for (size_t index = 0; index < path_list->used; index++) {
        free(path_list->paths[index]);
        path_list->paths[index] = NULL;
    }

    path_list->used = 0;
    free(path_list->paths);
    path_list->allocated = 0;
}

int add_import_entry(struct import_list *import_list, struct import_entry *entry) {
    if (import_list->allocated == import_list->used) {
        struct import_entry *list = NULL;
        if (import_list->allocated == 0) {
            list = malloc(sizeof(char *) * 8);
            import_list->allocated = 8;
        } else {
            list = realloc(import_list->entries, sizeof(char *) * import_list->allocated * 2);
            import_list->allocated = import_list->allocated * 2;
        }

        if (list == NULL) {
            return 1;
        }
        import_list->entries = list;
    }

    struct import_entry new_entry;

    memcpy(&new_entry, entry, sizeof(struct import_entry));

    import_list->entries[import_list->used] = new_entry;
    import_list->used++;
    return 0;
}
