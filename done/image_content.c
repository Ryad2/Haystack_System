#include "imgfs.h"
#include "error.h"
#include <vips/vips.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



int lazily_resize(int resolution, struct imgfs_file* imgfs_file, size_t index) {

    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file->file);//TODO check if this is necessary
    M_REQUIRE_NON_NULL(imgfs_file->metadata);//TODO check if this is necessary

    if (index >= imgfs_file->header.max_files
    || imgfs_file->metadata[index].is_valid == EMPTY) {
        return ERR_INVALID_IMGID;
    }

    // Retrieve metadata for the image at the specified index
    struct img_metadata* metadata = &imgfs_file -> metadata[index];

    // Check if the requested resolution already exists
    if ( (metadata -> size[resolution] != 0) || (resolution == ORIG_RES ) ){
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
    }//TODO check error

    if(metadata->size[ORIG_RES] > fread(image_buffer, metadata->size[ORIG_RES], 1, imgfs_file->file)) {
        free(image_buffer);
        return ERR_IO;
    }//TODO check error

    VipsImage *in, *out;

    if (vips_jpegload_buffer(image_buffer, metadata -> size[ORIG_RES], &in, NULL)) {
        g_object_unref(in);
        g_object_unref(out);
        free(image_buffer);
        return ERR_IMGLIB;  //TODO check error
    }

    // extracting the corresponding width from the header
    int target_width = imgfs_file -> header.resized_res[2 * resolution];

    // extracting the corresponding height from the header
    int target_height = imgfs_file -> header.resized_res[2 * resolution + 1];

    // Create a thumbnail of the image at the desired resolution
    if (vips_thumbnail_image(in, &out, target_width, target_height, NULL)) {
        g_object_unref(in);
        g_object_unref(out);
        free(image_buffer);
        return ERR_IMGLIB;  //TODO check error
    }

    // Saving the resized image to a buffer
    void* resized_buffer = NULL;
    size_t resized_length = 0;

    if ( vips_jpegsave_buffer(out, &resized_buffer, &resized_length, NULL) ) {
        g_object_unref(in);
        g_object_unref(out);
        free(resized_buffer);
        free(image_buffer);
        return ERR_IMGLIB;  //todo check error
    }

    //open the file in append mode
    FILE *file = fopen(imgfs_file -> file, "ab");
    if (file == NULL) {
        g_object_unref(in);
        g_object_unref(out);
        free(resized_buffer);
        free(image_buffer);
        close (file);
        return ERR_IO;      //todo check error
    }
    fwrite(out, sizeof(char), resized_length, file); //char is the size of one Byte

    // Update the metadata with the new image size and offset
    metadata -> size[resolution]   = resized_length;
    metadata -> offset[resolution] = ftell(file) - resized_length;


    // Clean up
    close (file);
    g_object_unref(in);
    g_object_unref(out);
    free(resized_buffer);
    free(image_buffer);
    return ERR_NONE; // Success

}

