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
    if (argc == 0) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    const char* imgfs_filename = argv[0];
    --argc; ++argv; //filename used


    uint32_t max_files = default_max_files;
    uint16_t thumb_res[2] = default_thumb_res;
    uint16_t small_res[2] = default_small_res;

    while (argc > 0) {
        if(strcmp(argv[0], "-max_files") == 0) {
            if (argc < 2) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }

            max_files = atouint32(argv[1]);
            if (max_files == 0) {
                return ERR_MAX_FILES;
            }

            argc -= 2; argv +=2; //used "-max_files" and the value

        } else if(strcmp(argv[0], "-thumb_res") == 0) {
            if (argc < 3) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }

            thumb_res[0] = atouint16(argv[1]);
            thumb_res[1] = atouint16(argv[2]);
            if (thumb_res[0] == 0 || thumb_res[1] == 0 || thumb_res[0] > MAX_THUMB_RES || thumb_res[1] == MAX_THUMB_RES) {
                return ERR_RESOLUTIONS;
            }

            argc -= 3; argv += 3;

        } else if(strcmp(argv[0], "-small_res") == 0) {
            if (argc < 3) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }

            small_res[0] = atouint16(argv[1]);
            small_res[1] = atouint16(argv[2]);
            if (small_res[0] == 0 || small_res[1] == 0 || small_res[0] > MAX_THUMB_RES || small_res[1] == MAX_THUMB_RES) {
                return ERR_RESOLUTIONS;
            }

            argc -= 3; argv += 3;
        } else {
            return ERR_INVALID_COMMAND //TODO is it the good error code ?
        }
    }
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
