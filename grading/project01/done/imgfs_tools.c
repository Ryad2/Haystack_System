/* ** NOTE: undocumented in Doxygen
 * @file imgfs_tools.c
 * @brief implementation of several tool functions for imgFS
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "util.h"

#include <inttypes.h>      // for PRIxN macros
#include <openssl/sha.h>   // for SHA256_DIGEST_LENGTH
#include <stdint.h>        // for uint8_t
#include <stdio.h>         // for sprintf
#include <stdlib.h>        // for calloc
#include <string.h>        // for strcmp
#define NUM_OF_FILES 1

/*******************************************************************
 * Human-readable SHA
 */
static void sha_to_string(const unsigned char* SHA,
                          char* sha_string)
{

    if (SHA == NULL) return;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(sha_string + (2 * i), "%02x", SHA[i]);
    }

    sha_string[2 * SHA256_DIGEST_LENGTH] = '\0';
}

/*******************************************************************
 * imgFS header display.
 */
void print_header(const struct imgfs_header* header)
{
    printf("*****************************************\n\
********** IMGFS HEADER START ***********\n");
    printf("TYPE: " STR_LENGTH_FMT(MAX_IMGFS_NAME) "\
\nVERSION: %" PRIu32 "\n\
IMAGE COUNT: %" PRIu32 "\t\tMAX IMAGES: %" PRIu32 "\n\
THUMBNAIL: %" PRIu16 " x %" PRIu16 "\tSMALL: %" PRIu16 " x %" PRIu16 "\n",
           header->name, header->version, header->nb_files, header->max_files, header->resized_res[THUMB_RES * 2],
           header->resized_res[THUMB_RES * 2 + 1], header->resized_res[SMALL_RES * 2],
           header->resized_res[SMALL_RES * 2 + 1]);
    printf("*********** IMGFS HEADER END ************\n\
*****************************************\n");
}

/*******************************************************************
 * Metadata display.
 */
void print_metadata (const struct img_metadata* metadata)
{
    char sha_printable[2 * SHA256_DIGEST_LENGTH + 1];
    sha_to_string(metadata->SHA, sha_printable);

    printf("IMAGE ID: %s\nSHA: %s\nVALID: %" PRIu16 "\nUNUSED: %" PRIu16 "\n\
OFFSET ORIG. : %" PRIu64 "\t\tSIZE ORIG. : %" PRIu32 "\n\
OFFSET THUMB.: %" PRIu64 "\t\tSIZE THUMB.: %" PRIu32 "\n\
OFFSET SMALL : %" PRIu64 "\t\tSIZE SMALL : %" PRIu32 "\n\
ORIGINAL: %" PRIu32 " x %" PRIu32 "\n",
           metadata->img_id, sha_printable, metadata->is_valid, metadata->unused_16, metadata->offset[ORIG_RES],
           metadata->size[ORIG_RES], metadata->offset[THUMB_RES], metadata->size[THUMB_RES],
           metadata->offset[SMALL_RES], metadata->size[SMALL_RES], metadata->orig_res[0], metadata->orig_res[1]);
    printf("*****************************************\n");
}

int do_open(const char* fileName, const char* openingMode, struct imgfs_file * image) {

    M_REQUIRE_NON_NULL(fileName);
    M_REQUIRE_NON_NULL(openingMode);
    M_REQUIRE_NON_NULL(image);

    // Opening file
    image -> file = fopen(fileName, openingMode);
    if (image -> file == NULL) {
        return ERR_IO;
    }

    // Reading header
    if (fread(&image->header, sizeof(struct imgfs_header), NUM_OF_FILES, image -> file) != NUM_OF_FILES) {
        fclose(image -> file); 
        return ERR_IO;
    }

    //Reading metadatas
    image -> metadata = calloc((image -> header).max_files, sizeof(struct img_metadata));
    if (fread(image->metadata, sizeof(struct img_metadata), (image -> header).max_files, image -> file) 
     != (image -> header).max_files) {
        free(image->metadata);
        fclose(image -> file);
        return ERR_IO;
    }

    return ERR_NONE;
}

void do_close(struct imgfs_file * image)
{
    // M_REQUIRE_NON_NULL(image); returns an error code so not usable in void func
    if (image != NULL ) {
        if (image -> metadata != NULL){
            free(image->metadata);
        } 
        if (image->file != NULL) {
            fclose(image->file);
        }
    }
}
