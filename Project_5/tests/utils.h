#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#include <sys/types.h>

struct fs_entry {
    const char *name;
    mode_t mode;
};

enum test_env_opt {
    TEOPT_NONE =       0,
    TEOPT_CHDIR = 1 << 0,
    TEOPT_INSPECT = 1 << 1,
};

struct env_data;

/* Macros usually help us hide some inconvenient stuff, like type of the
 * second argument (we already know how to initialize it) */

/* ‹test_env_setup(envd, options, list...) will set up the temporary test
 * structure and will create files according to the description in ‹list...›.
 * See sample test for explanation. */
#define test_env_setup(ENVD, OPTIONS, ...)                      \
    test_env_setup2(ENVD, OPTIONS, (struct fs_entry []){ __VA_ARGS__, { 0 }})

bool test_env_setup2(struct env_data **envdata, enum test_env_opt options,
        const struct fs_entry entries[]);

/* Destroy all resources and tear down the test structure. */
bool test_env_teardown(struct env_data *envdata);

/* Resolve relative ‹FN› path in the working directory of the program and store
 * it into «array» ‹PATH›. */
#define test_env_path(ENVD, PATH, FN)                           \
    test_env_path4(ENVD, sizeof(PATH), PATH, FN)

bool test_env_path4(const struct env_data *envdata, size_t path_size,
        char path[path_size], const char *filename);

/* Same as above, but copy the file first into a temporary file, resolve
 * ‹{USER}› and ‹{GROUP}› marks and store path to that temporary file
 * in ‹PATH› argument. */
#define test_data_file(ENVD, PATH, FN)                          \
    test_data_file4(ENVD, sizeof(PATH), PATH, FN)

bool test_data_file4(struct env_data *envdata, size_t path_size,
        char path[path_size], const char *filename);

/* Return the absolute path of an object ‹FN› inside the test structure. */
#define test_tmpdir_path(ENVD, PATH, FN)                        \
    test_tmpdir_path4(ENVD, sizeof(PATH), PATH, FN)

bool test_tmpdir_path4(struct env_data *envdata, size_t path_size,
        char path[path_size], const char *filename);

/* Returns the temporary directory name. */
const char *test_env_dir(const struct env_data *envdata);
/* Returns the full path to working directory where the program was started. */
const char *test_env_oldwd(const struct env_data *envdata);

bool compare_permfiles(const char *fn1, const char *fn2);

#define verify_perms(...)                                       \
    verify_perms1((struct fs_entry []){ __VA_ARGS__, { 0 }})

bool verify_perms1(const struct fs_entry entries[]);

#endif // UTILS_H
