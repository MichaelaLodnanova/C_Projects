#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>

#include "lists.h"

#define SUCCESS 0
#define FAILURE (-1)
#define IMPORT 10
#define EXPORT 20

#define STICKY_BIT 01000

char help_text[] = "Usage: checkperms <MODE> [DIRECTORY_TO_CHECK]\n"
"Modes of operation:\n"
" -e, --export <PERMISSIONS_FILE>   read and save permissions\n"
" -i, --import <PERMISSIONS_FILE>   compare and correct permissions";


typedef struct export_entry {
    char *file_name;
    char *owner;
    char *group;
    char *permissions;
    char *flags;
} export_entry_t;

void generic_error(char *error) {
    fprintf(stderr, "%s\n", error);    
}

void allocation_error(){
    generic_error("allocation error");
}

void print_help(){
    puts(help_text);
}

bool validate_entry_path (char *path){
    return access(path, F_OK) != -1;
}

void complete_permissions(struct stat buffer, char *permission){
    (buffer.st_mode & S_IRUSR) ? strcpy(permission, "r") : strcpy(permission, "-");
    (buffer.st_mode & S_IWUSR) ? strcat(permission, "w") : strcat(permission, "-");
    (buffer.st_mode & S_IXUSR) ? strcat(permission, "x") : strcat(permission, "-");
    (buffer.st_mode & S_IRGRP) ? strcat(permission, "r") : strcat(permission, "-");
    (buffer.st_mode & S_IWGRP) ? strcat(permission, "w") : strcat(permission, "-");
    (buffer.st_mode & S_IXGRP) ? strcat(permission, "x") : strcat(permission, "-");
    (buffer.st_mode & S_IROTH) ? strcat(permission, "r") : strcat(permission, "-");
    (buffer.st_mode & S_IWOTH) ? strcat(permission, "w") : strcat(permission, "-");
    (buffer.st_mode & S_IXOTH) ? strcat(permission, "x") : strcat(permission, "-");
}

void complete_flags(struct stat buffer, char *flag){
    (buffer.st_mode & S_ISUID) ? strcpy(flag, "s") : strcpy(flag, "-");
    (buffer.st_mode & S_ISGID) ? strcat(flag, "s") : strcat(flag, "-");
    (buffer.st_mode & STICKY_BIT) ? strcat(flag, "t") : strcat(flag, "-");
}

void print_entry(FILE *file, const export_entry_t *entry, bool skip_newline)
{
    fprintf(file, "# file: %s\n", entry->file_name);
    fprintf(file, "# owner: %s\n", entry->owner);
    fprintf(file, "# group: %s\n", entry->group);
    if (entry->flags != NULL && strcmp(entry->flags, "---")) {
        fprintf(file, "# flags: %s\n", entry->flags);
    }
    fprintf(file, "user::%.3s\n", &entry->permissions[0]);
    fprintf(file, "group::%.3s\n", &entry->permissions[3]);
    fprintf(file, "other::%.3s\n", &entry->permissions[6]);

    if (!skip_newline) {
        fprintf(file, "\n");
    }
}

bool load_import_entry(FILE *file, const char *prefix, import_entry_t *import_entry, int *eof){
    char path[PATH_MAX + 1] = {0};
    char final_path[PATH_MAX + 1] = {0};
    char owner[255] = {0};
    char group[255] = {0};
    char flags[4] = {0};
    char user_perm[4] = {0};
    char group_perm[4] = {0};
    char other_perm[4] = {0};

    long int position = ftell(file);

    // scan with flags
    int status = fscanf(file,
                        "# file:%[^\n]\n"
                        "# owner: %s\n"
                        "# group: %s\n"
                        "# flags: %c%c%c\n"
                        "user::%c%c%c\n"
                        "group::%c%c%c\n"
                        "other::%c%c%c\n",
                        path, owner, group,
                        flags, flags + 1, flags + 2,
                        user_perm, user_perm + 1, user_perm + 2,
                        group_perm, group_perm + 1, group_perm + 2,
                        other_perm, other_perm + 1, other_perm + 2);
    if (status == EOF) {
        *eof = EOF;
        return true;
    }
    if (status != 15){
        // no flags, go back and try with no flags
        fseek(file, position, SEEK_SET);
        int status2 = fscanf(file, "# file:%[^\n]\n"
                                   "# owner: %s\n"
                                   "# group: %s\n"
                                   "user::%c%c%c\n"
                                   "group::%c%c%c\n"
                                   "other::%c%c%c\n",
                             path, owner, group,
                             user_perm, user_perm + 1, user_perm + 2,
                             group_perm, group_perm + 1, group_perm + 2,
                             other_perm, other_perm + 1, other_perm + 2);

        if (status2 != 12){ // wrong file
            goto error;
        }
    }

    char *new_path = path + 1;
    if (strcmp(new_path, ".") == SUCCESS){
        strcpy(final_path, prefix);
    }
    else {
        strcpy(final_path, prefix);
        strcat(final_path, "/");
        strcat(final_path, new_path);
    }

    strcpy(import_entry->file_path, final_path);
    strcpy(import_entry->file_name, new_path);
    strcpy(import_entry->owner, owner);
    strcpy(import_entry->group, group);
    strcpy(import_entry->user_perms, user_perm);
    strcpy(import_entry->group_perms, group_perm);
    strcpy(import_entry->other_perms, other_perm);
    if (strlen(flags) == 3){ // flags loaded
        strcpy(import_entry->flags, flags);
    }
    return true;

    error:
    generic_error("wrong input file bro");
    return false;
}

void create_import(import_entry_t *import_entry){
    memset(import_entry, 0, sizeof(import_entry_t));
}

bool set_permissions(struct import_entry *import_entry, bool *soft_fail){
    // check group and owner
    struct stat stat_struct;
    if (lstat(import_entry->file_path, &stat_struct) != SUCCESS){
        generic_error("Incorrect file type");
        return false;
    }

    if (!S_ISREG(stat_struct.st_mode) && !S_ISDIR(stat_struct.st_mode)) {
        generic_error("Incorrect file type");
        return false;
    }


    bool failed = false;
    errno = 0;
    struct group *gr = getgrgid(stat_struct.st_gid);
    if (gr == NULL && errno != 0) {
        perror("getting group id");
        failed = true;
    } else if (gr == NULL || strcmp(gr->gr_name, import_entry->group) != SUCCESS){
        fprintf(stderr, "Group of file %s is incorrect\n", import_entry->file_name);
        failed = true;
    }
    
    struct passwd *pw = getpwuid(stat_struct.st_uid);
    if (strcmp(pw->pw_name, import_entry->owner) != SUCCESS){
        fprintf(stderr, "User of file %s is incorrect\n", import_entry->file_name);
        failed = true;
    }

    if (failed) {
        *soft_fail = true;
        return true;
    }

    // set permissions
    mode_t user_mode = 0;
    if (import_entry->user_perms[0] == 'r'){
        user_mode = user_mode | S_IRUSR;
    } else if (import_entry->user_perms[0] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }
    if (import_entry->user_perms[1] == 'w'){
        user_mode = user_mode | S_IWUSR;
    } else if (import_entry->user_perms[1] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }
    if (import_entry->user_perms[2] == 'x'){
        user_mode = user_mode | S_IXUSR;
    } else if (import_entry->user_perms[2] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }

    mode_t group_mode = 0;
    if (import_entry->group_perms[0] == 'r'){
        group_mode = group_mode | S_IRGRP;
    } else if (import_entry->group_perms[0] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }
    if (import_entry->group_perms[1] == 'w'){
        group_mode = group_mode | S_IWGRP;
    } else if (import_entry->group_perms[1] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }
    if (import_entry->group_perms[2] == 'x'){
        group_mode = group_mode | S_IXGRP;
    } else if (import_entry->group_perms[2] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }

    mode_t other_mode = 0;
    if (import_entry->other_perms[0] == 'r'){
        other_mode = other_mode | S_IROTH;
    } else if (import_entry->other_perms[0] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }
    if (import_entry->other_perms[1] == 'w'){
        other_mode = other_mode | S_IWOTH;
    } else if (import_entry->other_perms[1] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }
    if (import_entry->other_perms[2] == 'x'){
        other_mode = other_mode | S_IXOTH;
    } else if (import_entry->other_perms[2] != '-') {
        generic_error("Wrong permission syntax");
        return false;
    }

    mode_t flag_mode = 0;
    if (strlen(import_entry->flags) == 3){
        if (import_entry->flags[0] == 's'){
            flag_mode = flag_mode | S_ISUID;
        } else if (import_entry->flags[0] != '-') {
            generic_error("Wrong permission syntax");
            return false;
        }
        if (import_entry->flags[1] == 's'){
            flag_mode = flag_mode | S_ISGID;
        } else if (import_entry->flags[1] != '-') {
            generic_error("Wrong permission syntax");
            return false;
        }
        if (import_entry->flags[2] == 't'){
            flag_mode = flag_mode | STICKY_BIT;
        } else if (import_entry->flags[2] != '-') {
            generic_error("Wrong permission syntax");
            return false;
        }
    }

    if (chmod(import_entry->file_path, user_mode | group_mode | other_mode | flag_mode) != SUCCESS){
        perror("chmod");
        return false;
    }

    return true;
}

int process_import(FILE *file, char *path){
    // import parsing
    import_entry_t import_entry;
    create_import(&import_entry);
    int end_of_file = 0;
    bool soft_fail = false;
    while (end_of_file != EOF){
        if (!load_import_entry(file, path, &import_entry, &end_of_file)){
            return FAILURE;
        }
        if (end_of_file == EOF) {
            break;
        }
        if (!validate_entry_path(import_entry.file_path)){
            generic_error("no such file in a directory");
            soft_fail = true;
            continue; // not a failure
        }
        if (!set_permissions(&import_entry, &soft_fail)) {
            return FAILURE;
        }
        create_import(&import_entry);
    }
    
    return soft_fail ? FAILURE : SUCCESS;
}

int is_dir(const struct dirent *dir){
    if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
        return 0;
    }
    return dir->d_type == DT_DIR ? 1 : 0;
}

int is_file(const struct dirent *file){
    return file->d_type == DT_DIR ? 0:1;
}

int read_dir(struct path_list *list, char *src) {
    int failed = false;
    struct dirent **list_regulars = NULL;

    int regs = scandir(src, &list_regulars, is_file, alphasort);
    if (regs == -1) {
        perror("scandir()");
        return FAILURE;
    }

    struct dirent **list_dirs = NULL;
    int dirs = scandir(src, &list_dirs, is_dir, alphasort);
    if (dirs == -1) {
        failed = true;
        perror("scandir");
        return FAILURE;
    }
    for (int index = 0; index < dirs && !failed; index++) {
        struct dirent *entry = list_dirs[index];
        char *dest = malloc(sizeof(char) * (strlen(entry->d_name) + 1 + strlen(src) + 1));
        if (dest == NULL) {
            failed = true;
            allocation_error();
            break;
        }
        strcpy(dest, src);
        strcat(dest, "/");
        strcat(dest, entry->d_name);
        add_path(list, dest);

        if (read_dir(list, dest) == FAILURE) {
            failed = true;
            break;
        }
        free(dest);
        dest = NULL;
    }

    for (int index = 0; index < regs && !failed; index++) {
        struct dirent *entry = list_regulars[index];

        char *dest = malloc(sizeof(char) * (strlen(entry->d_name) + 1 + strlen(src) + 1));

        if (dest == NULL) {
            failed = true;
            allocation_error();
            break;
        }

        strcpy(dest, src);
        strcat(dest, "/");
        strcat(dest, entry->d_name);

        struct stat st;
        if (lstat(dest, &st) == -1) {
            perror("lstat");
            failed = true;
            free(dest);
            break;
        }

        if (!S_ISREG(st.st_mode)) {
            generic_error("Not a regular file");
            failed = true;
            free(dest);
            break;
        }

        add_path(list, dest);

        free(dest);
    }


    for (int index = 0; index < regs; index++) {
        free(list_regulars[index]);
    }
    free(list_regulars);

    for (int index = 0; index < dirs; index++) {
        free(list_dirs[index]);
    }
    free(list_dirs);

    return failed ? FAILURE : SUCCESS;
}




int fill_owner (export_entry_t *export_entry, struct stat buffer){
    struct passwd *pw = getpwuid(buffer.st_uid);
    if (pw == NULL){
        return FAILURE;
    }
    export_entry->owner = malloc(sizeof(char) * strlen(pw->pw_name) + 1);
    if (export_entry->owner == NULL){
        allocation_error();
        return FAILURE;
    }
    memcpy(export_entry->owner, pw->pw_name, sizeof(char) * strlen(pw->pw_name) + 1);
    return SUCCESS;
}

int fill_group (export_entry_t *export_entry, struct stat buffer){
    struct group *gr = getgrgid(buffer.st_gid);
    if(gr == NULL){
        return FAILURE;
    }
    export_entry->group = malloc(sizeof(char) * strlen(gr->gr_name) + 1);
    if (export_entry->group == NULL){
        allocation_error();
        return FAILURE;
    }
    memcpy(export_entry->group, gr->gr_name, sizeof(char)* strlen(gr->gr_name) + 1);
    return SUCCESS;
}

int fill_permission(export_entry_t *export_entry, struct stat buffer){
    export_entry->permissions = malloc(sizeof(char ) * 10);
    if (export_entry->permissions == NULL){
        allocation_error();
        return FAILURE;
    }
    complete_permissions(buffer, export_entry->permissions);
    return SUCCESS;
}

int fill_flag(export_entry_t *export_entry, struct stat buffer){
    export_entry->flags = malloc(sizeof(char) * 10);
    if (export_entry->flags == NULL){
        allocation_error();
        return FAILURE;
    }
    complete_flags(buffer, export_entry->flags);
    return SUCCESS;
}

int fill_export(export_entry_t *export_entry, char *path){
    struct stat buffer;
    int stt = stat(path, &buffer);
    if (stt != SUCCESS){
        generic_error("stat() failure");
        return FAILURE;
    }

    int load_owner = fill_owner(export_entry, buffer);
    if (load_owner != SUCCESS){
        generic_error("unable to know the owner");
        return FAILURE;
    }
    int load_group = fill_group(export_entry, buffer);
    if (load_group != SUCCESS){
        generic_error("unable to know the group");
        return FAILURE;
    }
    int fill_permissions = fill_permission(export_entry, buffer);
    if (fill_permissions != SUCCESS){
        generic_error("unable to load permissions");
        return FAILURE;
    }
    int fill_flags = fill_flag(export_entry, buffer);
    if (fill_flags != SUCCESS){
        generic_error("unable to load permissions");
        return FAILURE;
    }
    return SUCCESS;
}

int process_export(char *working_dir_path, FILE *export_file){
    struct path_list list;

    create_paths(&list);

    char start[PATH_MAX] = "";

    strcpy(start, working_dir_path);
    if (working_dir_path[strlen(working_dir_path)] != '/') {
        strcat(start, "/");
    }
    strcat(start, ".");

    add_path(&list, start);
    if (read_dir(&list, working_dir_path) == FAILURE) {
        destroy_paths(&list);
        return FAILURE;
    }

    bool failed = false;
    for (size_t i = 0; i < list.used; i++) {
        export_entry_t export_entry;

        export_entry.file_name = list.paths[i] + strlen(working_dir_path) + 1;

        int filling = fill_export(&export_entry, list.paths[i]);

        if (filling != FAILURE && export_entry.permissions != NULL
            && export_entry.file_name != NULL
            && export_entry.owner != NULL
            && export_entry.group != NULL){
            print_entry(export_file, &export_entry, i + 1 == list.used);
        } else {
            failed = true;
        }
        free(export_entry.flags);
        free(export_entry.group);
        free(export_entry.owner);
        free(export_entry.permissions);

        if (failed) {
            break;
        }
        memset(&export_entry, 0, sizeof(export_entry_t));
    }

    destroy_paths(&list);
    if (failed) {
        return FAILURE;
    }
    return SUCCESS;
}

int current_directory_processing(int command, FILE* file){
    char working_dir[PATH_MAX];
    if (getcwd(working_dir, sizeof(working_dir)) == NULL){
        generic_error("open dir failure");
        return EXIT_FAILURE;
    }
    if (command == IMPORT){
        int import = process_import(file, working_dir);
        if (import == FAILURE){
            return FAILURE;
        }
    }
    else{
        int export = process_export(working_dir, file);
        if (export == FAILURE){
            return FAILURE;
        }
    }
    return SUCCESS;
}

int given_working_directory(char* path, int command, FILE* file){
    if (!validate_entry_path(path)){
        generic_error("unable to open directory");
        return FAILURE;
    }

    if (command == IMPORT){
        int import_p = process_import(file, path);
        if (import_p != SUCCESS){
            return FAILURE;
        }
        return SUCCESS;
    }
    else if (command == EXPORT){
        int export_p = process_export(path, file);
        if (export_p != SUCCESS){
            return FAILURE;
        }
        return SUCCESS;
    }
    return SUCCESS;
}

static struct option options[] = {
        {"export", required_argument, NULL, 'e'},
        {"import", required_argument, NULL, 'i'},
        {0, 0, 0, 0}
};

int main(int argc, char** argv) {
    if (argc < 2 || argc > 4) {
        generic_error("wrong number of arguments");
        print_help();
        return EXIT_FAILURE;
    }
    //process second argument (command)
    int type_comm = getopt_long(argc, argv, "e:i:", options, NULL);
    int command = 0;
    if (type_comm == FAILURE){
        generic_error("argv[1]");
        print_help();
        return EXIT_FAILURE;
    }
    switch (type_comm) {
        case 'e':
            command = EXPORT;
            break;
        case 'i':
            command = IMPORT;
            break;
        case '?':
            print_help();
            return EXIT_FAILURE;
        case ':':
            generic_error("invalid number of arguments");
            print_help();
            return EXIT_FAILURE;
    }
    //process third argument (working file)
    char *permission_mode = (command == IMPORT) ? "r" : "w";

    FILE* file = fopen(optarg, permission_mode);
    if (!file) {
        perror(optarg);
        return EXIT_FAILURE;
    }
    bool using_cwd = argc == optind;
    if (using_cwd) {
        if (current_directory_processing(command, file) != SUCCESS){
            fclose(file);
            return EXIT_FAILURE;
        }
        fclose(file);
        return EXIT_SUCCESS;
    }
    if (given_working_directory(argv[optind++], command, file) != SUCCESS){
        fclose(file);
        return EXIT_FAILURE;
    }
    
    fclose(file);
    return EXIT_SUCCESS;
}
