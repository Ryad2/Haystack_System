#include "imgfs.h"
#include <string.h>

int do_delete(const char* img_id, struct imgfs_file* imgfs_file)
{
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);
    
    // Searching index
    int pos = -1;
    for(unsigned int i = 0; i < imgfs_file->header.max_files; ++i) {
        if (strcmp(img_id, imgfs_file->metadata[i].img_id) == 0) {
            pos = i;
        }
    }

    if (pos == -1 || imgfs_file->metadata[pos].is_valid == EMPTY) {
        return ERR_IMAGE_NOT_FOUND;
    }

    // Invalidating image
    imgfs_file->metadata[pos].is_valid = EMPTY;

    // Rewrite metadata to disk
    long offset = sizeof(struct imgfs_header) + pos * sizeof(struct img_metadata);
    fseek(imgfs_file->file, offset, SEEK_SET);
    if (fwrite(&imgfs_file->metadata[pos], sizeof(struct img_metadata), 1, imgfs_file->file) != 1) {
        return ERR_IO;
    }

    imgfs_file->header.nb_files--;
    imgfs_file->header.version++;

    // Rewrite header to disk
    fseek(imgfs_file->file, 0, SEEK_SET);
    if (fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, imgfs_file->file) != 1) {
        return ERR_IO;
    }

    return ERR_NONE;
}
