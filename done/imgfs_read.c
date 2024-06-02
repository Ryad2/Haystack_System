#include "imgfs.h"
#include "error.h"
#include "image_content.h"

#include <stdlib.h>
#include <string.h>

// Function to read an image from the imgFS file
int do_read(const char* img_id, int resolution, char** image_buffer,
            uint32_t* image_size, struct imgfs_file* imgfs_file) 
{
    // Ensure that the pointers are not NULL
    M_REQUIRE_NON_NULL(image_buffer);
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(image_size);

    // Find the index of the image with the specified ID in the metadata
    int index = -1, i = 0;
    while (i < imgfs_file->header.max_files && index == -1) {
        if (!strncmp(imgfs_file->metadata[i].img_id, img_id, MAX_IMG_ID)) {
            index = i;
        }
        i++;
    }

    // If the image is not found, return an error
    if (index == -1) {
        return ERR_IMAGE_NOT_FOUND;
    }

    // Get the metadata for the image
    struct img_metadata* md = &imgfs_file->metadata[index];
    int errcode = ERR_NONE;


    // If the requested resolution is not the original and
    // the resolution data is not present, resize the image
    if (resolution != ORIG_RES && (md->offset[resolution] == 0 || md->size[resolution] == 0)) {
        errcode = lazily_resize(resolution, imgfs_file, (size_t) index);
    }

    // If there was an error resizing the image, return the error code
    if (errcode != ERR_NONE) {
        return errcode;
    }

    // Set the size of the image
    *image_size = md->size[resolution];

    // Allocate memory for the image buffer
    *image_buffer = malloc(md->size[resolution]);
    if (*image_buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    // Read the image from the file into the buffer
    if (imgfs_file->file == NULL || 
        fseek(imgfs_file->file, md->offset[resolution], SEEK_SET) || 
        fread(*image_buffer, md->size[resolution], 1, imgfs_file->file) != 1)
    {
        return ERR_IO;
    }

    // Return success
    return ERR_NONE;
}
