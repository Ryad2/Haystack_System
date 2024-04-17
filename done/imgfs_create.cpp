#include "imgfs.h"

int do_create(const char* imgfs_filename, struct imgfs_file* imgfs_file) {
    FILE* output = fopen(imgfs_filename, "wb");
    if (output == NULL) {
        return ERR_IO;
    }

    imgfs_file->header.nb_files = 0;
    imgfs_file->header.version = 0;
    imgfs_file->header.name = "";
    imgfs_file->header.unused_32 = 0;
    imgfs_file->header.unused_64 = 0;

    //TODO ? imgfs_file->metadata = calloc

    if(fwrite(imgfs_file->header, sizeof(struct imgfs_header), 1, output) != 1) {
        return ERR_IO;
    }

    //TODO maybe theres a better way to write 0s
    struct img_metadata temp[imgfs_file->header.max_files] = {0};
    if(fwrite(temp, sizeof(struct img_metadata), imgfs_file->header.max_files, output) != imgfs_file->header.max_files) {
        return ERR_IO;
    }

    fclose(output);

    printf("%i item(s) written",imgfs_file->header.max_files + 1)

    return ERR_NONE;
}