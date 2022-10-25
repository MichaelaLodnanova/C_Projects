#ifndef LISTS_H
#define LISTS_H

#include <stdint.h>
#include <dirent.h>
#include <linux/limits.h>

struct path_list  {
    char **paths;
    uint32_t used;
    uint32_t allocated;
};

typedef struct import_entry {
    char file_path[PATH_MAX];
    char file_name[PATH_MAX];
    char owner[255];
    char group[255];
    char user_perms[4];
    char group_perms[4];
    char other_perms[4];
    char flags[4];
} import_entry_t;

struct import_list  {
    struct import_entry *entries;
    uint32_t used;
    uint32_t allocated;
};


void create_paths(struct path_list *path_list);
int add_path(struct path_list *path_list, char *str);
void destroy_paths(struct path_list *path_list);

void create_import(struct import_entry *import_entry);
int add_import_entry(struct import_list *import_list, struct import_entry *entry);
void destroy_import(struct import_list *import_list);

#endif
