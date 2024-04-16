/**
 * @file imgfscmd_functions.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h"   // for _unused

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

// default values
static const uint32_t default_max_files = 128;
static const uint16_t default_thumb_res =  64;
static const uint16_t default_small_res = 256;

// max values
static const uint16_t MAX_THUMB_RES = 128;
static const uint16_t MAX_SMALL_RES = 512;

/**********************************************************************
 * Displays some explanations.
 ********************************************************************** */
int help(int useless _unused, char** useless_too _unused)
{
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE.
     * **********************************************************************
     */

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
}

/**********************************************************************
 * Opens imgFS file and calls do_list().
 ********************************************************************** */
int do_list_cmd(int argc, char** argv) { //todo Check if the function implementation is correct

    if (argc == 0) {
        printf("Usage: no file name\n");
        return ERR_INVALID_ARGUMENT;
    } else if (argc > 1) {
        printf("Usage: too many arguments\n");
        return ERR_INVALID_COMMAND;
    }

    const char* imgfs_file_name = argv[0];
    struct imgfs_file imgfs_file;

    int open_result = do_open(imgfs_file_name, "rb", &imgfs_file); //todo Assuming "rb" mode as we're just reading the file

    if (open_result != ERR_NONE) {
        //todo Handle error (e.g., file not found, could not read, etc.)
        return open_result;
    }

    int list_result = do_list(&imgfs_file, STDOUT, NULL); // STDOUT might be a placeholder for your actual output mode handling

    do_close(&imgfs_file);

    return list_result; // todo Return the result of listing operation
}

/**********************************************************************
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd(int argc, char** argv)
{
    puts("Create");
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
}

/**********************************************************************
 * Deletes an image from the imgFS.
 */
int do_delete_cmd(int argc, char** argv)
{
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
}
