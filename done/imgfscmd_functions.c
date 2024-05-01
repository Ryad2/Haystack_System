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
    printf("imgfscmd [COMMAND] [ARGUMENTS]\n"
           "  help: displays this help.\n"
           "  list <imgFS_filename>: list imgFS content.\n"
           "  create <imgFS_filename> [options]: create a new imgFS.\n"
           "      options are:\n"
           "          -max_files <MAX_FILES>: maximum number of files.\n"
           "                                  default value is 128\n"
           "                                  maximum value is 4294967295\n"
           "          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n"
           "                                  default value is 64x64\n"
           "                                  maximum value is 128x128\n"
           "          -small_res <X_RES> <Y_RES>: resolution for small images.\n"
           "                                  default value is 256x256\n"
           "                                  maximum value is 512x512\n"
           "  delete <imgFS_filename> <imgID>: delete image imgID from imgFS.\n"); // copied and pasted directly
    fflush(stdout);
    return ERR_NONE;
}

/**********************************************************************
 * Opens imgFS file and calls do_list().
 ********************************************************************** */
int do_list_cmd(int argc, char** argv) {

    if (argc == 0) {
        printf("Usage: no file name\n");
        return ERR_INVALID_ARGUMENT;
    } else if (argc > 1) {
        printf("Usage: too many arguments\n");
        return ERR_INVALID_COMMAND;
    }

    const char* imgfs_file_name = argv[0];
    struct imgfs_file imgfs_file;

    int open_result = do_open(imgfs_file_name, "rb", &imgfs_file);

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
    M_REQUIRE_NON_NULL(argv);

    if (argc == 0) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    const char* imgfs_filename = argv[0];
    --argc; ++argv; //filename used


    struct imgfs_file newfile;
    newfile.header.max_files = default_max_files;
    newfile.header.resized_res[0] = newfile.header.resized_res[1] = default_thumb_res;
    newfile.header.resized_res[2] = newfile.header.resized_res[3] = default_small_res;

    while (argc > 0) {
        if(strcmp(argv[0], "-max_files") == 0) {
            if (argc < 2) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }

            newfile.header.max_files = atouint32(argv[1]);
            if (newfile.header.max_files == 0) {
                return ERR_MAX_FILES;
            }

            argc -= 2; argv +=2; //used "-max_files" and the value

        } else if(strcmp(argv[0], "-thumb_res") == 0) {
            if (argc < 3) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }

            newfile.header.resized_res[0] = atouint16(argv[1]);
            newfile.header.resized_res[1] = atouint16(argv[2]);
            if (newfile.header.resized_res[0] == 0 || newfile.header.resized_res[1] == 0 ||
                newfile.header.resized_res[0] > MAX_THUMB_RES || newfile.header.resized_res[1] > MAX_THUMB_RES) {
                return ERR_RESOLUTIONS;
            }

            argc -= 3; argv += 3;

        } else if(strcmp(argv[0], "-small_res") == 0) {
            if (argc < 3) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }

            newfile.header.resized_res[2] = atouint16(argv[1]);
            newfile.header.resized_res[3] = atouint16(argv[2]);
            if (newfile.header.resized_res[2] == 0 || newfile.header.resized_res[3] == 0 ||
                newfile.header.resized_res[2] > MAX_SMALL_RES || newfile.header.resized_res[3] > MAX_SMALL_RES) {
                return ERR_RESOLUTIONS;
            }

            argc -= 3; argv += 3;
        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }
    int lastErr = do_create(imgfs_filename, &newfile);
    if (lastErr != ERR_NONE) {
        return lastErr;
    }
    do_close(&newfile);
    return ERR_NONE;
}

/**********************************************************************
 * Deletes an image from the imgFS.
 */
int do_delete_cmd(int argc, char** argv)
{
    M_REQUIRE_NON_NULL(argv);
    if(argc < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    } else if (0 == strlen(argv[1])|| MAX_IMG_ID < strlen(argv[1])) {
        return ERR_INVALID_IMGID;
    }

    struct imgfs_file imgfs_file;
    int lastErr = ERR_NONE;
    lastErr = do_open(argv[0], "rb+", &imgfs_file);

    if (lastErr != ERR_NONE) {
        do_close(&imgfs_file);
        return lastErr;
    }

    lastErr = do_delete(argv[1], &imgfs_file);

    do_close(&imgfs_file);
    return lastErr;
}
