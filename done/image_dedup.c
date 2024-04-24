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

    struct img_metadata* metadata = imgfs_file -> metadata[index];
    //TODO
    return ERR_NONE;
}



