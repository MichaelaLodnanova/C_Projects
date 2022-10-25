#include "sample_printer.h"

#include <string.h>
#include <stdbool.h>

typedef struct sample_entry
{
    const char *file_name;
    const char *owner;
    const char *group;
    const char *permissions;
    const char *flags;
} sample_entry;

static void print_sample_entry(FILE *file, const sample_entry *entry)
{
    fprintf(file, "# file: %s\n", entry->file_name);
    fprintf(file, "# owner: %s\n", entry->owner);
    fprintf(file, "# group: %s\n", entry->group);

    if (entry->flags != NULL && !strcmp(entry->flags, "---")) {
        fprintf(file, "# flags: %s\n", entry->flags);
    }

    fprintf(file, "user::%3s\n", &entry->permissions[0]);
    fprintf(file, "group::%3s\n", &entry->permissions[3]);
    fprintf(file, "other::%3s\n", &entry->permissions[6]);
}

void print_sample_output(FILE *file)
{
    sample_entry samples[] = {
        { // target directory
                .file_name = ".",
                .owner = "xlogin",
                .group = "student",
                .permissions = "rwxrw-r--",
                .flags = "---" },
        { // subdirectory
                .file_name = "test",
                .owner = "xlogin",
                .group = "student",
                .permissions = "rwxrwx---",
                .flags = "--t" },
        { // a VERY unsafe executable
                .file_name = "test/run",
                .owner = "root",
                .group = "root",
                .permissions = "rwxrwxrwx",
                .flags = "ss-" }
    };

    for (size_t i = 0; i < sizeof(samples) / sizeof(sample_entry); ++i) {
        if (i != 0) {
            // empty line
            putc('\n', file);
        }
        print_sample_entry(file, &samples[i]);
    }
}
