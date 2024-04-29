#include "imgfs.h"
#include "error.h"
#include <vips/vips.h>
#include "image_dedup.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



int do_name_and_content_dedup(struct imgfs_file* imgfs_file, uint32_t index){

    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file -> metadata);
    M_REQUIRE_NON_NULL(imgfs_file -> file);

    if (index >= imgfs_file -> header.max_files
        || imgfs_file -> metadata[index].is_valid == EMPTY) {
        return ERR_INVALID_ARGUMENT;
    }

    struct img_metadata* metadata = &(imgfs_file -> metadata[index]);
    if(metadata -> is_valid == EMPTY){
        return ERR_INVALID_ARGUMENT; //TODO check the error type
    }

    int count = 0;
    for (size_t i = 0; i < imgfs_file->header.max_files; i++) {

        struct img_metadata* current_metadata = &(imgfs_file -> metadata[i]);
        if (i != index && current_metadata -> is_valid == NON_EMPTY) {

            if (!strcmp(metadata -> img_id, current_metadata -> img_id))
                return ERR_DUPLICATE_ID;

            else if (!strcmp(metadata -> SHA, current_metadata -> SHA)) {
                memcmy(metadata -> offset, current_metadata -> offset, NB_RES);
                count++;
            }
        }
    }

    if (count == 0){
        metadata -> offset[ORIG_RES] = 0;
        metadata -> size[ORIG_RES] = 0;
    }
    return ERR_NONE;
}



