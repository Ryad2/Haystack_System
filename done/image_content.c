#include "imgfs.h"        // Assurez-vous que ce fichier contient les définitions nécessaires pour le système de fichiers d'images
#include "error.h"        // Pour les codes d'erreur que votre fonction peut retourner
#include <vips/vips.h>    // Pour les fonctions de manipulation d'images de la librairie VIPS

// Inclure d'autres en-têtes standard si nécessaire, par exemple stdlib.h pour les fonctions de gestion de la mémoire
#include <stdlib.h>
#include <string.h>


int lazily_resize(int resolution,
                  struct imgfs_file* imgfs_file,
                  size_t index){
    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file->file);//TODO check if this is necessary
    M_REQUIRE_NON_NULL(imgfs_file->metadata);//TODO check if this is necessary

    // Retrieve metadata for the image at the specified index
    struct img_metadata* metadata = &imgfs_file->metadata[index];

    // Check if the requested resolution already exists
    if ((resolution == THUMB_RES && metadata->thumb_offset != 0 && metadata->thumb_size != 0) ||
        (resolution == SMALL_RES && metadata->small_offset != 0 && metadata->small_size != 0)) {
        return ERR_NONE;  // Image already resized in the requested resolution
    }

    // Load the original image from its offset
    VipsImage *in, *out;
    size_t length;
    void* image_buffer = get_image_buffer(imgfs_file, metadata->orig_offset, metadata->orig_size, &length);

    if (vips_jpegload_buffer(image_buffer, length, &in, NULL)) {
        return ERR_VIPS_LOAD_FAIL;  // VIPS load error
    }

    int target_width = resolution == THUMB_RES ? 128 : 640; // Example sizes for thumbnail and small

    // Create a thumbnail of the image at the desired resolution
    if (vips_thumbnail_image(in, &out, target_width, "height", target_width, NULL)) {
        g_object_unref(in);  // Clean up original image object
        return ERR_VIPS_RESIZE_FAIL;  // VIPS resize error
    }

    // Save the resized image to a buffer
    void* resized_buffer;
    size_t resized_length;
    if (vips_jpegsave_buffer(out, &resized_buffer, &resized_length, NULL)) {
        g_object_unref(in);
        g_object_unref(out);
        return ERR_VIPS_SAVE_FAIL;  // VIPS save error
    }

    // Update the metadata and imgfs file
    if (resolution == THUMB_RES) {
        metadata->thumb_offset = append_to_imgfs(imgfs_file, resized_buffer, resized_length);
        metadata->thumb_size = resized_length;
    } else { // SMALL_RES
        metadata->small_offset = append_to_imgfs(imgfs_file, resized_buffer, resized_length);
        metadata->small_size = resized_length;
    }

    // Clean up
    g_object_unref(in);
    g_object_unref(out);
    free(resized_buffer);

    return ERR_NONE; // Success

}

