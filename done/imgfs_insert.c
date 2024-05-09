#include "imgfs.h"
#include "error.h"

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

    SHA256(image_buffer, image_size, md->SHA);
    strcpy(md->img_id, img_id);
    md->size[ORIG_RES] = (uint32_t) image_size;

    uint32_t *height, *width;
    int errcode = get_resolution(height, width, image_buffer, image_size);
    if (errcode != ERR_NONE) {
        return errcode;
    }
    
    md->orig_res[0] = *height;
    md->orig_res[1] = *width;
    md->is_valid = NON_EMPTY;

    errcode = do_name_and_content_dedup(imgfs_file, index);
    if (errcode != ERR_NONE) {
        return errcode;
    }

    if (md->offset[ORIG_RES] == 0) {
        // no duplicate, so we need to write to disk
        // TODO

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
    }

    imgfs_file->header.nb_files += 1;
    imgfs_file->header.version += 1;

    // TODO write updated header and metadata to disk

    // fiding the position of the metadata of the image in the file
    int metadata_file_pointer = sizeof(struct imgfs_header) + index * sizeof(struct img_metadata);
    int header_file_pointer = sizeof(struct imgfs_header);

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
    if (fseek(imgfs_file->file, header_file_pointer, SEEK_SET)) {
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
