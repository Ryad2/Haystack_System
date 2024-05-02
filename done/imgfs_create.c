#include "imgfs.h"
#include <string.h>
#include <stdlib.h>

int do_create(const char* imgfs_filename, struct imgfs_file* imgfs_file)
{
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);
    FILE* output = fopen(imgfs_filename, "wb");

    // Initialisation of every field
    imgfs_file->header.nb_files = 0;
    imgfs_file->header.version = 0;
    strncpy(imgfs_file->header.name, CAT_TXT, strlen(CAT_TXT));
    imgfs_file->header.unused_32 = 0;
    imgfs_file->header.unused_64 = 0;

    imgfs_file->metadata = calloc(imgfs_file->header.max_files, sizeof(struct img_metadata));
    if (imgfs_file->metadata == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    imgfs_file->file = output;

    // Writing header
    if(fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, output) != 1) {
        return ERR_IO;
    }

    // Writing metadatas
    if(fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, output) != imgfs_file->header.max_files) {
        return ERR_IO;
    }
    
    // This printf is requested by the instruction
    printf("%i item(s) written\n",imgfs_file->header.max_files + 1);

    return ERR_NONE;
}