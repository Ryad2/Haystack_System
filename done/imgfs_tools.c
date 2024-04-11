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

/*******************************************************************
 * Human-readable SHA
 */
static void sha_to_string(const unsigned char* SHA,
                          char* sha_string) {

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
    struct imgfs_file * ptr = calloc(sizeof(struct imgfs_file), 1);
    M_REQUIRE_NON_NULL(ptr);
    image = ptr;

    FILE* ptr = fopen(fileName, opningMode);
    M_REQUIRE_NON_NULL(ptr);
    image -> file = ptr;

    struct imgfs_header header;
    if (fread(&header, sizeof(struct imgfs_header), 1, image -> file) != 1) {
        fprintf(stderr, "Error while reading the header\n");//todo check if ok this error managing is great
        fclose(image -> file); // file closing in case of an error
        return ERR_INVALID_ARGUMENT;
    } else {
        image -> header = header;
    }

    struct imgfs_metadata metadata[header.max_files];
    if (fread(metadata, sizeof(struct imgfs_metadata), header.max_files , image -> file) != 1) {//todo check with assistants if lseek work perfectly in this case
        fprintf(stderr, "Error while reading the header\n");//todo check if ok this error managing is great
        fclose(image -> file); // file closing in case of an error
        return ERR_INVALID_ARGUMENT;
    } else {
        image -> metadata = calloc(header.max_files, sizeof(struct imgfs_metadata));
        M_REQUIRE_NON_NULL(image -> metadata);
        memcpy(image -> metadata, metadata, header.max_files * sizeof(struct imgfs_metadata));//copying metadata from the temp gotten from the file to the image
    }

    return ERR_NONE;
}

void do_close(struct imgfs_file * image) {
    M_REQUIRE_NON_NULL(image);
    fclose(image -> file);
    free(image -> metadata);
    free(image);
}


