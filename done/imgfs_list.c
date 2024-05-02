#include "imgfs.h"
#include "util.h"


int do_list(const struct imgfs_file* imgfs_file, enum do_list_mode output_mode, char** json) 
{    
    M_REQUIRE_NON_NULL(imgfs_file);

    if (output_mode == STDOUT) {
        print_header(&imgfs_file->header);
        if (imgfs_file->header.nb_files == 0) printf("<< empty imgFS >>\n");
        else {
            int foundImgs = 0, i = 0;
            while(foundImgs < imgfs_file->header.max_files && i <imgfs_file->header.max_files) {
                
                if(imgfs_file->metadata[i].is_valid) {
                    print_metadata(&imgfs_file->metadata[i]);
                    ++foundImgs;
                }
                ++i;
            }
        }
    } else if (output_mode == JSON) {
        TO_BE_IMPLEMENTED();
    }
}