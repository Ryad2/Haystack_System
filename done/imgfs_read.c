#include "imgfs.h"
#include "error.h"

#include <string.h>

int do_read(const char* img_id, int resolution, char** image_buffer,
            uint32_t* image_size, struct imgfs_file* imgfs_file) 
{
    M_REQUIRE_NON_NULL(image_buffer);
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(image_size);

    int index = -1, i = 0;
    while (i < imgfs_file->header.max_files && index == -1) {
        if (!strncmp(imgfs_file->metadata[i].img_id, img_id, MAX_IMG_ID)) {
            index = i;
        }
        i++;
    }

    if (index == -1) {
        return ERR_IMAGE_NOT_FOUND;
    }

    struct img_metadata* md = &imgfs_file->metadata[index];
    int errcode = ERR_NONE;

    if (resolution != ORIG_RES && (md->offset[resolution] == 0 || md->size[resolution] == 0)) {
        errcode = lazily_resize(resolution, imgfs_file, (size_t) index);
    }

    if (errcode != ERR_NONE) {
        return errcode;
    }

    // TODO buffer and size ? + check errors
    *image_size = md->size[resolution];
    *image_buffer = malloc(md->size[resolution]);
    if (image_buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }
    
    if (imgfs_file->file == NULL || 
        fseek(imgfs_file->file, md->offset[resolution], SEEK_SET) || 
        fwrite(*image_buffer, md->size[resolution], 1, imgfs_file->file) != 1) {

        return ERR_IO;
    }


    return ERR_NONE;
}