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
        "help: displays this help.\n"
        "list <imgFS_filename>: list imgFS content.\n"
        "create <imgFS_filename> [options]: create a new imgFS.\n"
        "    options are:\n"
        "        -max_files <MAX_FILES>: maximum number of files.\n"
        "                                default value is 128\n"
        "                                maximum value is 4294967295\n"
        "        -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n"
        "                                default value is 64x64\n"
        "                                maximum value is 128x128\n"
        "        -small_res <X_RES> <Y_RES>: resolution for small images.\n"
        "                                default value is 256x256\n"
        "                                maximum value is 512x512\n"
        "read   <imgFS_filename> <imgID> [original|orig|thumbnail|thumb|small]:\n"
        "    read an image from the imgFS and save it to a file.\n"
        "    default resolution is \"original\".\n"
        "insert <imgFS_filename> <imgID> <filename>: insert a new image in the imgFS.\n"
        "delete <imgFS_filename> <imgID>: delete image imgID from imgFS.\n"); // copied and pasted directly
    fflush(stdout);
    return ERR_NONE;
}

/**********************************************************************
 * Opens imgFS file and calls do_list().
 ********************************************************************** */
int do_list_cmd(int argc, char** argv)
{

    if (argc == 0) {
        return ERR_INVALID_ARGUMENT;
    } else if (argc > 1) {
        return ERR_INVALID_COMMAND;
    }

    const char* imgfs_file_name = argv[0];
    struct imgfs_file imgfs_file;

    int open_result = do_open(imgfs_file_name, "rb", &imgfs_file);

    if (open_result != ERR_NONE) {
        return open_result;
    }

    int list_result = do_list(&imgfs_file, STDOUT, NULL); // STDOUT might be a placeholder for your actual output mode handling

    do_close(&imgfs_file);

    return list_result;
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

    // Filename used
    const char* imgfs_filename = argv[0];
    --argc; ++argv; 


    struct imgfs_file newfile;
    newfile.header.max_files = default_max_files;
    newfile.header.resized_res[0] = newfile.header.resized_res[1] = default_thumb_res;
    newfile.header.resized_res[2] = newfile.header.resized_res[3] = default_small_res;

    // Going through optional arguments
    while (argc > 0) {
        // -------------------- MAX FILES --------------------
        if(strcmp(argv[0], "-max_files") == 0) {
            if (argc < 2) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }

            newfile.header.max_files = atouint32(argv[1]);
            if (newfile.header.max_files == 0) {
                return ERR_MAX_FILES;
            }

            // Used "-max_files" and the value
            argc -= 2; argv +=2; 

        // -------------------- THUMB RES --------------------
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

            // Used "-thumb_res" and the two values
            argc -= 3; argv += 3;

        // -------------------- SMALL RES --------------------
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

            // Used "-small_res" and the two values
            argc -= 3; argv += 3;

        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }

    int create_error = do_create(imgfs_filename, &newfile);
    
    // No need to check if we had an error here, we always close the file and return the error code
    do_close(&newfile);
    return create_error;
}

/**********************************************************************
 * Deletes an image from the imgFS.
 */
int do_delete_cmd(int argc, char** argv)
{
    M_REQUIRE_NON_NULL(argv);

    if(argc < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    } else if (0 == strlen(argv[1]) || MAX_IMG_ID < strlen(argv[1])) {
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

// ======================================================================
int do_read_cmd(int argc, char **argv)
{
    M_REQUIRE_NON_NULL(argv);
    if (argc != 2 && argc != 3) return ERR_NOT_ENOUGH_ARGUMENTS;

    const char * const img_id = argv[1];

    const int resolution = (argc == 3) ? resolution_atoi(argv[2]) : ORIG_RES;
    if (resolution == -1) return ERR_RESOLUTIONS;

    struct imgfs_file myfile;
    zero_init_var(myfile);
    int error = do_open(argv[0], "rb+", &myfile);
    if (error != ERR_NONE) return error;

    char *image_buffer = NULL;
    uint32_t image_size = 0;
    error = do_read(img_id, resolution, &image_buffer, &image_size, &myfile);
    do_close(&myfile);
    if (error != ERR_NONE) {
        return error;
    }

    // Extracting to a separate image file.
    char* tmp_name = NULL;
    create_name(img_id, resolution, &tmp_name);
    if (tmp_name == NULL) return ERR_OUT_OF_MEMORY;
    error = write_disk_image(tmp_name, image_buffer, image_size);
    free(tmp_name);
    free(image_buffer);

    return error;
}

// ======================================================================
int do_insert_cmd(int argc, char **argv)
{
    M_REQUIRE_NON_NULL(argv);
    if (argc != 3) return ERR_NOT_ENOUGH_ARGUMENTS;

    struct imgfs_file myfile;
    zero_init_var(myfile);
    int error = do_open(argv[0], "rb+", &myfile);
    if (error != ERR_NONE) return error;

    char *image_buffer = NULL;
    uint32_t image_size;

    // Reads image from the disk.
    error = read_disk_image (argv[2], &image_buffer, &image_size);
    if (error != ERR_NONE) {
        do_close(&myfile);
        return error;
    }

    error = do_insert(image_buffer, image_size, argv[1], &myfile);
    free(image_buffer);
    do_close(&myfile);
    return error;
}

static void create_name(const char* img_id, int resolution, char** new_name) 
{
    strcpy(new_name, img_id);
    if (resolution == THUMB_RES) {
        strcpy(new_name, "_thumb");
    } else if (resolution == SMALL_RES) {
        strcpy(new_name, "_small");
    } else {
        strcpy(new_name, "_orig");
    }
    strcpy(new_name, ".jpg");
}

static int write_disk_image(const char *filename, const char *image_buffer, uint32_t image_size)
{
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        return ERR_IO;
    }
    if (fwrite(image_buffer, image_size, 1, file) != 1) {
        return ERR_IO;
    }
    fclose(file);
    return ERR_NONE;
}

static int read_disk_image(const char *path, char **image_buffer, uint32_t *image_size)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return ERR_IO;
    }

    // we are supposed to fill image_size but how are we supposed to know it ?
    *image_buffer = calloc(1, image_size);
    if (*image_buffer = NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    if (fread(*image_buffer, image_size, 1, file) != 1) {
        return ERR_IO;
    }
    fclose(file);
    return ERR_NONE;
}