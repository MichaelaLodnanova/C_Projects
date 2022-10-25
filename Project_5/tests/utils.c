#include "utils.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <err.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

struct env_data {
    enum test_env_opt options;
    char dtemp[NAME_MAX];
    char oldwd[PATH_MAX];

    char data_file[PATH_MAX];
};

static
bool _setup_tmpdir(struct env_data *envdata)
{
    strncpy(envdata->dtemp, "pb071-hw05.tmp.XXXXXX", sizeof(envdata->dtemp));
    if (mkdtemp(envdata->dtemp) == NULL) {
        warn("mkdtemp");
        return false;
    }

    if (getcwd(envdata->oldwd, sizeof(envdata->oldwd)) == NULL) {
        warn("getcwd");
        return false;
    }

    return true;
}

static
bool _setup_files(const struct env_data *envdata, const struct fs_entry entries[])
{
    mode_t oldmask = umask(0000);

    for (const struct fs_entry *entry = entries; entry->name != NULL; ++entry) {
        // Entry must have a non-empty path.
        assert(strlen(entry->name) > 0);
        // We only support regular files and directories.
        assert(((entry->mode & S_IFMT) & ~(S_IFREG | S_IFDIR)) == 0);

        char target[PATH_MAX];
        if (snprintf(target, sizeof(target), "%s/%s", envdata->dtemp, entry->name)
                >= (int) sizeof(target)) {
            warn("%s: Entry name too long", entry->name);
            return false;
        }

        if (entry->mode & S_IFDIR) {
            if (mkdir(target, entry->mode & 07777) != 0) {
                warn("mkdir %s", entry->name);
                return false;
            }
        } else {
            int fd = open(target, O_WRONLY | O_CREAT | O_EXCL, entry->mode & 07777);

            if (fd == -1) {
                warn("creat %s", entry->name);
                return false;
            }

            close(fd);
        }
    }

    umask(oldmask);
    return true;
}

bool test_env_setup2(struct env_data **envdata, enum test_env_opt options,
        const struct fs_entry entries[])
{
    assert(envdata != NULL);
    assert(entries != NULL);

    if ((*envdata = malloc(sizeof(**envdata))) == NULL) {
        warn("Cannot allocate memory");
        return false;
    }

    memset(*envdata, 0, sizeof(**envdata));
    (*envdata)->options = options;

    if (!_setup_tmpdir(*envdata)) {
        warnx("Cannot create temporary directory");
        free(*envdata);
        return false;
    }

    if (!_setup_files(*envdata, entries)) {
        warnx("Cannot create test file set");
        free(*envdata);
        return false;
    }

    if ((*envdata)->options & TEOPT_CHDIR && chdir((*envdata)->dtemp) != 0) {
        warn("chdir");
        return false;
    }

    return true;
}

static
bool _test_env_teardown_data_file(struct env_data *envdata)
{
    if (envdata->data_file[0] == '\0')
        return true;

    if (unlink(envdata->data_file) != 0) {
        warn("Cannot remove temporary data file");
        return false;
    }

    return true;
}

static
bool _test_env_teardown_dir(struct env_data *envdata)
{
    if (envdata->options & TEOPT_INSPECT)
        return true;

    char command[16 + PATH_MAX];
    if (snprintf(command, sizeof(command), "chmod -R u=rwx %s", envdata->dtemp)
                > (int) sizeof(command)
            || system(command) != 0) {
        warnx("Failed to chmod target directory");
        return false;
    }

    if (snprintf(command, sizeof(command), "rm -rf %s", envdata->dtemp)
                > (int) sizeof(command)
            || system(command) != 0) {
        warnx("Cannot clean up directories");
        return false;
    }

    return true;
}

bool test_env_teardown(struct env_data *envdata)
{
    assert(envdata != NULL);
    if (chdir(envdata->oldwd) != 0) {
        warn("chdir");
        return false;
    }

    bool status = _test_env_teardown_data_file(envdata)
            && _test_env_teardown_dir(envdata);

    free(envdata);
    return status;
}

bool test_env_path4(const struct env_data *envdata, size_t path_size,
        char path[path_size], const char *filename)
{
    return snprintf(path, path_size, "%s/%s", test_env_oldwd(envdata), filename)
        < (int) path_size;
}

static
bool _replace_trailing(char **line, size_t *size, const char *what, const char *replacement)
{
    char *pos = strstr(*line, what);
    if (pos == NULL)
        return true;

    size_t what_len = strlen(what);
    off_t index = pos - *line;

    if (pos[what_len] != '\n')
        return false;

    if (index + what_len < *size) {
        char *moved_line = realloc(*line, *size + what_len + 2);

        if (moved_line == NULL)
            return false;

        *line = moved_line;
        *size += what_len + 2;
        pos = &(*line)[index];
    }

    strcpy(pos, replacement);
    strcat(pos, "\n");
    return true;
}

static
bool _replace_tags(char **line, size_t *size)
{
    static struct passwd *proc_user = NULL;
    static struct group *proc_group = NULL;

    if (proc_user == NULL && (proc_user = getpwuid(getuid())) == NULL)
        return false;
    if (proc_group == NULL && (proc_group = getgrgid(getgid())) == NULL)
        return false;

    return _replace_trailing(line, size, "{USER}", proc_user->pw_name)
            && _replace_trailing(line, size, "{GROUP}", proc_group->gr_name);
}

bool _test_data_file_copy(FILE *src, int fd_dst)
{
    char *line = NULL;
    size_t size = 0;

    bool status = false;
    while (getline(&line, &size, src) != -1) {
        if (!_replace_tags(&line, &size))
            goto cleanup;

        if (dprintf(fd_dst, "%s", line) < 0)
            goto cleanup;
    }

    status = feof(src) && !ferror(src);

cleanup:
    free(line);
    return status;
}

bool test_data_file4(struct env_data *envdata, size_t path_size,
        char path[path_size], const char *filename)
{
    if (envdata->data_file[0] != '\0') {
        warnx("Data file already prepared");
        return false;
    }

    char source[PATH_MAX];
    if (!test_env_path4(envdata, sizeof(source), source, filename)) {
        warnx("Source path too long");
        return false;
    }

    int status = false;

    if (!test_env_path4(envdata, sizeof(envdata->data_file), envdata->data_file, ".data.XXXXXX")
            || strlen(envdata->data_file) + 1 > path_size) {
        warnx("Temporary file name too long");
        goto ret;
    }

    FILE *src = fopen(source, "r");
    if (src == NULL) {
        warn("%s", source);
        goto ret;
    }

    int fd_dst = mkstemp(envdata->data_file);
    if (fd_dst == -1) {
        warn("mkstemp %s", envdata->data_file);
        goto close_src;
    }

    status = _test_data_file_copy(src, fd_dst);

    if (status)
        strncpy(path, envdata->data_file, path_size);

    close(fd_dst);

close_src:
    fclose(src);

ret:
    if (!status)
        envdata->data_file[0] = '\0';
    return status;
}

bool test_tmpdir_path4(struct env_data *envdata, size_t path_size,
        char path[path_size], const char *filename)
{
    return snprintf(path, path_size, "%s/%s/%s", test_env_oldwd(envdata),
                test_env_dir(envdata), filename)
        < (int) path_size;
}

const char *test_env_dir(const struct env_data *envdata)
{
    assert(envdata != NULL);
    return envdata->dtemp;
}

const char *test_env_oldwd(const struct env_data *envdata)
{
    assert(envdata != NULL);
    return envdata->oldwd;
}

static
bool _compare(FILE *stream[2])
{
    char *line[2] = { NULL };
    size_t lsz[2] = { 0 };

    bool status = false;

    while (true) {
        int ret[2];
        for (int i = 0; i < 2; ++i)
            ret[i] = getline(&line[i], &lsz[i], stream[i]);

        if (ret[0] == -1 && ret[1] == -1) {
            status = true;
            break;
        }

        if (ret[0] == -1 || ret[1] == -1)
            break;

        if (strcmp(line[0], line[1]) != 0) {
            warnx("> %s", line[0]);
            warnx("< %s", line[1]);
            break;
        }
    }

    for (int i = 0; i < 2; ++i)
        free(line[i]);

    return status;
}

bool compare_permfiles(const char *name1, const char *name2)
{
    bool status = false;

    FILE *stream1 = fopen(name1, "r");
    if (stream1 == NULL) {
        warn("%s", name1);
        goto ret;
    }

    FILE *stream2 = fopen(name2, "r");
    if (stream2 == NULL) {
        warn("%s", name2);
        goto close_stream1;
    }

    status = _compare((FILE *[]){ stream1, stream2 });

    fclose(stream2);

close_stream1:
    fclose(stream1);

ret:
    return status;
}

bool verify_perms1(const struct fs_entry entries[])
{
    for (const struct fs_entry *entry = entries; entry->name != NULL; ++entry) {
        struct stat st;
        if (stat(entry->name, &st) == -1) {
            warn("%s", entry->name);
            return false;
        }

        if ((st.st_mode & 0777) != (entry->mode & 0777)) {
            warn("%s: Expected mode %o, got %o", entry->name, entry->mode, st.st_mode);
            return false;
        }
    }

    return true;
}
