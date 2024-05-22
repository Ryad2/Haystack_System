#include "imgfs.h"
#include "util.h"
#include "string.h"
#include <json-c/json.h>

int do_list(const struct imgfs_file* imgfs_file, enum do_list_mode output_mode, char** json) 
{    
    M_REQUIRE_NON_NULL(imgfs_file);

    if (output_mode == STDOUT) {
        print_header(&imgfs_file->header);
        if (imgfs_file->header.nb_files == 0) printf("<< empty imgFS >>\n");
        else {
            int foundImgs = 0, i = 0;
            while(foundImgs < imgfs_file->header.nb_files && i <imgfs_file->header.max_files) {
                
                if(imgfs_file->metadata[i].is_valid) {
                    print_metadata(&imgfs_file->metadata[i]);
                    ++foundImgs;
                }
                ++i;
            }
        }
    } else if (output_mode == JSON) {
        struct json_object* values = json_object_new_array_ext(imgfs_file->header.nb_files);
        int foundImgs = 0, i = 0;
        while(foundImgs < imgfs_file->header.nb_files && i <imgfs_file->header.max_files) {
                
                if(imgfs_file->metadata[i].is_valid) { // new_string() copies the string so freeing after doesnt touch .img_id
                    struct json_object* name = json_object_new_string(imgfs_file->metadata[i].img_id);
                    if (json_object_array_add(values, name)) {
                        // TODO see what needs to be freed
                        return ERR_RUNTIME;
                    }
                    ++foundImgs;
                }
                ++i;
            }

        struct json_object* obj = json_object_new_object();
        if (json_object_object_add(obj, "Images", values)) {
            // TODO see what nees to be freed
            return ERR_RUNTIME;
        }

        char* temp = json_object_to_json_string(obj);
        *json = calloc(1, strlen(temp) + 1);
        if (*json == NULL) {
            return ERR_OUT_OF_MEMORY;
        }
        strncpy(*json, temp, strlen(temp));
        json_object_put(obj);
    }
    
    return ERR_NONE;
}