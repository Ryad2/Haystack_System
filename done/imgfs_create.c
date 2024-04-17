#include "imgfs.h"
#include <string.h>
#include <stdlib.h>

int do_create(const char* imgfs_filename, struct imgfs_file* imgfs_file) {
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);
    FILE* output = fopen(imgfs_filename, "wb");


    imgfs_file->header.nb_files = 0;
    imgfs_file->header.version = 0;
    strncpy(imgfs_file->header.name, CAT_TXT, strlen(CAT_TXT));
    imgfs_file->header.unused_32 = 0;
    imgfs_file->header.unused_64 = 0;
    //TODO calloc error handling
    imgfs_file->metadata = calloc(imgfs_file->header.max_files, sizeof(struct img_metadata));
    imgfs_file->file = output;

    if(fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, output) != 1) {
        return ERR_IO;
    }

    if(fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, output) != imgfs_file->header.max_files) {
        return ERR_IO;
    }

    //free(imgfs_file->metadata);
    /*if (fclose(output)) {
        return ERR_IO;
    }*/

    printf("%i item(s) written\n",imgfs_file->header.max_files + 1);

    return ERR_NONE;
}