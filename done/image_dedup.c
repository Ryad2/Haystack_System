#include "imgfs.h"
#include "error.h"
#include <vips/vips.h>
#include "image_dedup.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



int do_name_and_content_dedup(struct imgfs_file* imgfs_file, uint32_t index) {

    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file -> metadata);
    M_REQUIRE_NON_NULL(imgfs_file -> file);

    // check if the index is valid
    if (index >= imgfs_file -> header.max_files
        || imgfs_file -> metadata[index].is_valid == EMPTY) {
        return ERR_IMAGE_NOT_FOUND;
    }

    // Retrieve metadata for the image at the specified index
    struct img_metadata* metadata = &(imgfs_file -> metadata[index]);

    // iterate over all the metadata to check for duplicates and update the offset and size
    for (size_t i = 0; i < imgfs_file->header.max_files; i++) {
        struct img_metadata* current_metadata = &(imgfs_file -> metadata[i]);
        if (i != index && current_metadata -> is_valid == NON_EMPTY) {
            if (!strncmp(metadata -> img_id, current_metadata -> img_id, MAX_IMG_ID )) {
                return ERR_DUPLICATE_ID;

            }

            if (!memcmp(metadata -> SHA, current_metadata -> SHA, SHA256_DIGEST_LENGTH)) {
                memcpy(metadata -> offset, current_metadata -> offset, NB_RES*sizeof(uint64_t));
                //double checking that we have exactly the same size 
                memcpy(metadata -> size, current_metadata -> size, NB_RES*sizeof(uint32_t));
                return ERR_NONE;
            }
        }
    }

    metadata -> offset[ORIG_RES] = 0;
    return ERR_NONE;
}



