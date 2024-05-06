#include "imgfs.h"
#include "error.h"
#include <vips/vips.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define NUM_IMGS 1




void clean_up(VipsImage *in, VipsImage *out, void* resized_buffer, void* image_buffer) {
        g_object_unref(in);
        g_object_unref(out);
        free(resized_buffer);
        free(image_buffer);
} 

int lazily_resize(int resolution, struct imgfs_file* imgfs_file, size_t index) {

    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file->file);
    M_REQUIRE_NON_NULL(imgfs_file->metadata);

    if (index >= imgfs_file->header.max_files || 
        imgfs_file->metadata[index].is_valid == EMPTY) {
        return ERR_INVALID_IMGID;
    }

    // Retrieve metadata for the image at the specified index
    struct img_metadata* metadata = &imgfs_file -> metadata[index];

    // Check if the requested resolution already exists
    if ( (metadata -> size[resolution] != 0) || (resolution == ORIG_RES ) ) {
        return ERR_NONE;
    }
    
    // Load the original image from its offset
    size_t length;
    void* image_buffer = malloc(metadata->size[ORIG_RES]);
    if (image_buffer == NULL) {
        return ERR_OUT_OF_MEMORY;  // Memory allocation error
    }

    if (fseek(imgfs_file->file, metadata->offset[ORIG_RES], SEEK_SET)) {
        free(image_buffer);
        return ERR_IO;  // File seek error
    }

    // Read the original image from the file
    if(fread(image_buffer, metadata->size[ORIG_RES], NUM_IMGS, imgfs_file->file) != NUM_IMGS ) {
        free(image_buffer);
        return ERR_IO;
    }

    //initializing the resized buffer 
    void* resized_buffer = NULL;
    size_t resized_length = 0;

    //initializing the 
    VipsImage *in, *out;

    // Load the original image from the buffer
    if (vips_jpegload_buffer(image_buffer, metadata -> size[ORIG_RES], &in, NULL)) {
        clean_up(in, out, resized_buffer, image_buffer);
        return ERR_IMGLIB;
    }

    // extracting the corresponding width from the header
    int target_width = imgfs_file -> header.resized_res[2 * resolution];

    // extracting the corresponding height from the header
    int target_height = imgfs_file -> header.resized_res[2 * resolution + 1];

    // Create a thumbnail of the image at the desired resolution
    if (vips_thumbnail_image(in, &out, target_width, "height", target_height, NULL)) {
        clean_up(in, out, resized_buffer, image_buffer);
        return ERR_IMGLIB;
    }



    // Saving the resized image to a buffer
    if (vips_jpegsave_buffer(out, &resized_buffer, &resized_length, NULL)) {
        clean_up(in, out, resized_buffer, image_buffer);
        return ERR_IMGLIB;
    }

    // initializing the position of the file pointer to the end of the file
    if (fseek(imgfs_file->file, 0, SEEK_END)) {
        clean_up(in, out, resized_buffer, image_buffer);
        return ERR_IO;  // File seek error
    }

    // updating the metadata of the image
    metadata -> offset[resolution] = ftell(imgfs_file -> file);
    metadata -> size[resolution]   = resized_length;

    // Write the resized image to the file
    if(fwrite(resized_buffer, resized_length, NUM_IMGS, imgfs_file -> file) != NUM_IMGS) {
        clean_up(in, out, resized_buffer, image_buffer);
        return ERR_IO;
    }

    // fiding the position of the metadata of the image in the file
    int metadata_file_pointer = sizeof(struct imgfs_header) + index * sizeof(struct img_metadata);

    // moving the file pointer to the metadata of the image
    if (fseek(imgfs_file->file, metadata_file_pointer, SEEK_SET)) {
        clean_up(in, out, resized_buffer, image_buffer);
        return ERR_IO;  // File seek error
    }

    // writing the metadata of the image to the file
    if(fwrite(metadata, sizeof(struct img_metadata), NUM_IMGS, imgfs_file -> file) != NUM_IMGS) {
        clean_up(in, out, resized_buffer, image_buffer);
        return ERR_IO;
    }

    // Clean up
    clean_up(in, out, resized_buffer, image_buffer);

    return ERR_NONE;
}

// ======================================================================
int get_resolution(uint32_t *height, uint32_t *width,
                   const char *image_buffer, size_t image_size)
{
    M_REQUIRE_NON_NULL(height);
    M_REQUIRE_NON_NULL(width);
    M_REQUIRE_NON_NULL(image_buffer);

    VipsImage* original = NULL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    const int err = vips_jpegload_buffer((void*) image_buffer, image_size,
                                         &original, NULL);
#pragma GCC diagnostic pop
    if (err != ERR_NONE) return ERR_IMGLIB;
    
    *height = (uint32_t) vips_image_get_height(original);
    *width  = (uint32_t) vips_image_get_width (original);
    
    g_object_unref(VIPS_OBJECT(original));
    return ERR_NONE;
}