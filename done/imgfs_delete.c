#include "imgfs.h"
#include <string.h>

int do_delete(const char* img_id, struct imgfs_file* imgfs_file){
    int pos = -1;
    for(int i = 0; i < imgfs_file->header.max_files; ++i) {
        if (strcmp(img_id, imgfs_file->metadata[i].img_id) == 0) {
            pos = i;
        }
    }

    if (pos == -1) {
        return ERR_IMAGE_NOT_FOUND; //TODO is it the good err code ?
    }

    imgfs_file->metadata[pos].is_valid = EMPTY;

    // rewrite metadata to disk
    long offset = sizeof(struct imgfs_header) + pos * sizeof(struct img_metadata);
    fseek(imgfs_file->file, offset, SEEK_SET);
    if (fwrite(&imgfs_file->metadata[pos], sizeof(struct img_metadata), 1, imgfs_file->file) != 1) {
      return ERR_IO;
    }

    imgfs_file->header.nb_files--;
    imgfs_file.header.version++;

    //rewrite header to disk
    fseek(imgfs_file->file, 0, SEEK_SET);
    if (fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, imgfs_file->file) != 1) {
        return ERR_IO;
    }

    return ERR_NONE;
}