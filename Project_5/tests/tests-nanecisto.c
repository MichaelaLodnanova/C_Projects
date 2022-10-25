// #desc: Testy nanečisto
// #name: nanecisto
#include "cut.h"
#include "mainwrap.h"

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

/* These are the example tests that will be used in ‹nanecisto› mode.
 * You are free (and advised) to write your own.
 *
 * The test suite also contains some utilities that might ease your testing.
 * Read the comments below to see how the tests work. */

/* Do not mind the following incantations. */
// #desc: Export ze zadaného adresáře
// #data: test-export.txt
// #hook: overlay=empty
TEST(export)
{
    /* This test checks ‹-e› option with the explicit argument. */

    /* The ‹env_data› structure holds information and resources necessary
     * for temporary directory structure. */
    struct env_data *envd;

    /* The ‹test_env_setup()› creates a temporary directory structure in your
     * current working directory. It also creates list of files with specified
     * permissions. There is one option available for the test:
     *   • ‹TEOPT_NONE›     No special options,
     *   • ‹TEOPT_CHDIR›    The rest of the test will run in the given
     *                      temporary directory structure.
     *   • ‹TEOPT_INSPECT›  This will cause the temporary directory to remain
     *                      when ‹test_env_teardown()› gets called. You can use
     *                      it to inspect what your program has done.
     *
     * The last argument is a list of structures with two attributes,
     * a file or directory name to be created and its permissions.
     * (It is actually a macro that hides the type away, but since we are
     * writing tests, such macros are often used to write tests more easily).
     *
     * NOTE: «Always» use ‹ASSERT()› for this function. If you don't and you
     *       mess up the files (or some resource allocaiton fails), the rest
     *       of the test will be meaningless. */
    ASSERT(test_env_setup(&envd, TEOPT_NONE,
        /* To create a directory, pass ‹S_IFDIR› or'ed into the permission set. */
        { "dir", S_IFDIR | 0755 },
        /* You can create deeper structures as well, simply insert path with
         * all components:
         *
         *      { "dir/file", 0766 },
         *
         * in the list. However, note that you have to create the parent
         * directory first, this function will not do that on its own. Also,
         * you cannot create files inside inaccessible subtree. That is,
         *
         *      { "dir", 0000 },
         *      { "dir/file", 0777 },
         *
         * will «NOT» work and the function will fail. If you want a scenario
         * like this, create the directory with broader permissions and then
         * use ‹chmod()› on the path created using ‹test_tmpdir_path()›:
         * 
         *      char file[PATH_MAX];
         *      // This will store the absolute path to ‹dir› in ‹file›.
         *      ASSERT(test_tmp_dir(envd, file, "dir"));
         * */

        /* Files only need mode (but you can pass ‹S_IFREG› if you want). */
        { "test", 0666 }
    ));

    /* ‹test_env_path()› provides an unified interface to create a path to file
     * in your current working directory, no matter what option you specified
     * to ‹test_env_setup()›. You have to create a string that can hold
     * a path though, like this: */
    char export_path[PATH_MAX];
    /* The function takes the environment, the path (which «must» be an array,
     * not a pointer! (again, this is a macro)), and the file name. */
    ASSERT(test_env_path(envd, export_path, "test-export.out"));

    char expected[PATH_MAX];
    /* Data files prepared for these tests need special handling. Since we
     * (the authors) have no idea how your user and group is called, provided
     * data files contain ‹{USER}› and ‹{GROUP}› placeholders that must be
     * replaced before the test.
     *
     * This is what this function does: It will take the given data file
     * (‹test-export.txt› in this case) and it will replace those tags,
     * saving the result in a new temporary file. The path to this new file
     * will be stored in ‹expected›.
     *
     * NOTE: You can only use this function «once» per test. If you need
     * to use more than one data file, you are probably writing a wrong test. */
    ASSERT(test_data_file(envd, expected, "test-export.txt"));

    /* The ‹app_main()› calls your program with the given arguments.
     * Since we did not ‹chdir()› into the temporary directory, we need to pass
     * the name of that directory as the third argument.
     * This is what ‹test_env_dir(envd)› does.
     *
     * Note that ‹export_path› is the path to the temporary file we have
     * created earlier. */
    CHECK(app_main("-e", export_path, test_env_dir(envd)) == 0);

    /* Finally, to compare files, you can use ‹compare_permfiles()› function
     * in the util test. All you need to do is pass paths to files. */
    CHECK(compare_permfiles(export_path, expected));

    /* The final part deals with cleanup. First, we need to unlink the temporary
     * file. If you want to inspect what your program has created, simply
     * comment out the next line. The file will remain as ‹test-export.txt›. */
    unlink(export_path);

    /* Finally, to undo the temporary structure created by ‹test_env_setup()›,
     * we use ‹test_env_teardown()›. This function will attempt to remove
     * the temporary directory and converted data files for this test.
     *
     * CAUTION: If your code crashes with a signal, this function will «NOT»
     * be called and you will have to remove those temporary files yourself.
     * For example:
     *
     *          rm -f .data.*
     *          chmod -R u=rwX pb071-hw05.tmp.*
     *          rm -rf pb071-hw05.tmp.*
     * */
    ASSERT(test_env_teardown(envd));
}

// #desc: Import do aktuálního adresáře
// #data: test-import.txt
// #hook: overlay=empty
TEST(import)
{
    /* The ‹import› test works similarly as the one above, but instead we
     * run in inside the temporary directory. */
    struct env_data *envd;
    /* To achieve this, we pass ‹TEOPT_CHDIR› option, which will make sure
     * that the rest of the test runs inside the temporary directory structre. */
    ASSERT(test_env_setup(&envd, TEOPT_CHDIR,
        { "dir", S_IFDIR | 0755 },
        { "exe", 0666 },
        { "file", 0777 }
    ));

    /* Similarly to above, ‹test_data_file()› prepares the actual file for your
     * user and group. This function works even inside ‹chdir()›'d test. */
    char import_path[PATH_MAX];
    ASSERT(test_data_file(envd, import_path, "test-import.txt"));

    /* This time, we don't pass the third argument, because we are already
     * inside the temporary directory at this point. */
    CHECK(app_main("-i", import_path) == 0);

    /* The ‹verify_perms()› function is another magic macro, that will check
     * wether your program has set up correct permission bits according to the
     * imput file. The idea is that you can just copy & paste arguments for
     * ‹test_env_setup()› here and adjust the permissions. You can, however,
     * also split the call for each argument separately to see which one
     * has failed. */
    CHECK(verify_perms(
        { "dir", 0777 },
        { "exe", 0777 },
        { "file", 0600 }
    ));

    /* As always, do not forget to tear down the temporary directory structure.
     * Remember, you can use ‹TEOPT_INSPECT› or'ed into options for
     * ‹test_env_setup()› to prevent it from being deleted, so you can inspect
     * its contents after the test. It will be named
     * ‹pb071-hw05.tmp.[SOMETHING RANDOM]›. */
    ASSERT(test_env_teardown(envd));
}

/* To recapitulate, when writing your own tests using the above framework,
 * remember these things:
 *
 *  1. Use ‹test_env_setup()› and ‹test_env_teardown()› to set up and destroy
 *     temporary directory structure.
 *  2. «Always» use ‹ASSERT()› for utility functions (‹test_…()›).
 *  3. «Prefer» to use ‹CHECK()› for ‹app_main()›, so that the test will
 *     correctly clean up resources.
 *  4. For ‹-e› tests, use ‹compare_permfiles()› to compare generated files.
 *     Also use ‹test_data_file()› to convert ‹{USER}› and ‹{GROUP}› tags
 *     before comparison.
 *  5. For ‹-i› tests, use ‹verify_perms()› to check the resulting directory
 *     structure.
 *
 * You can always use CUT's other utilities, like ‹ASSERT_FILE()› to check
 * standard output or standard error output.
 *
 * Good luck! */
