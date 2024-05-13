#include "imgfs.h"
#include "error.h"
#include "image_content.h"
#include "image_dedup.h"

#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH and SHA256()

int do_insert(const char* image_buffer, size_t image_size,
              const char* img_id, struct imgfs_file* imgfs_file)
{
    M_REQUIRE_NON_NULL(image_buffer);
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);

    if (imgfs_file->header.max_files == imgfs_file->header.nb_files) {
        return ERR_IMGFS_FULL;
    }

    int index = -1, i = 0;
    while (i < imgfs_file->header.max_files && index == -1) {
        if (imgfs_file->metadata[i].is_valid == EMPTY) {
            index = i;
        }
        i++;
    }

    if (index == -1) {
        return ERR_IMGFS_FULL; // can only happen if .max_files is wrong
    }

    struct img_metadata* md = &imgfs_file->metadata[index];

    strcpy(md->img_id, img_id);
    SHA256((const unsigned char*) image_buffer, image_size, md->SHA);
    md->size[ORIG_RES] = (uint32_t) image_size;
    md->size[THUMB_RES] = 0;
    md->size[SMALL_RES] = 0;
    md->is_valid = NON_EMPTY;
    md->unused_16 = 0;

    int errcode = do_name_and_content_dedup(imgfs_file, index);
    if (errcode != ERR_NONE) {
        return errcode;
    }

    uint32_t height = 0, width = 0;
    errcode = get_resolution(&height, &width, image_buffer, image_size);
    if (errcode != ERR_NONE) {
        return errcode;
    }
    
    md->orig_res[0] = width;
    md->orig_res[1] = height;

    if (md->offset[ORIG_RES] == 0) {
        // no duplicate, so we need to write to disk
        // TODO

        // writing at the end
        if (fseek(imgfs_file->file, 0, SEEK_END)) {
            //todo u have to clean_up
            return ERR_IO;  // File seek error
        }

        long offset = ftell(imgfs_file->file);
        if (offset < 0) {
            return ERR_IO;
        }

        md->offset[ORIG_RES] = (uint64_t) offset;
        md->offset[THUMB_RES] = 0;
        md->offset[SMALL_RES] = 0;

        // writing image
        if(fwrite(image_buffer, image_size, 1, imgfs_file -> file) != 1) {
            //todo u have to clean_up
            return ERR_IO;
        }
    }

    imgfs_file->header.nb_files += 1;
    imgfs_file->header.version += 1;

    // TODO write updated header and metadata to disk

    // fiding the position of the metadata of the image in the file
    int metadata_file_pointer = sizeof(struct imgfs_header) + index * sizeof(struct img_metadata);

    // moving the file pointer to the metadata of the image
    if (fseek(imgfs_file->file, metadata_file_pointer, SEEK_SET)) {
       //todo u have to clean_up
        return ERR_IO;  // File seek error
    }

    // writing the metadata of the image to the file
    if(fwrite(md, sizeof(struct img_metadata), 1, imgfs_file -> file) != 1) {
        //todo u have to clean_up
        return ERR_IO;
    }


    //todo j'ai l'impression que le header est deja ecrit dans le fichier à verifier

    // moving the file pointer to the end of the header of the image
    if (fseek(imgfs_file->file, 0, SEEK_SET)) {
        //todo u have to clean_up
        return ERR_IO;  // File seek error
    }

    // writing the header of the image to the file
    if(fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, imgfs_file -> file) != 1) {
        //todo u have to clean_up
        return ERR_IO;
    }

    return ERR_NONE;
}
