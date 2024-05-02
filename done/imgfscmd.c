/**
 * @file imgfscmd.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * Image Filesystem Command Line Tool
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h"   // for _unused

#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>



typedef int (*command)(int, char*[]);

struct command_mapping {
    const char* name;
    command comm;
};

/*******************************************************************************
 * MAIN
 */
int main(int argc, char* argv[])
{
    VIPS_INIT(argv[0]);
    int ret = 0;

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        argc--; argv++; // skips ./

        int comm_qte = 4;
        command chosen_comm = NULL;
        struct command_mapping commands[] = {
            {"list", do_list_cmd},
            {"create", do_create_cmd},
            {"help", help},
            {"delete", do_delete_cmd}
        };

        for(int i = 0; i < comm_qte; ++i) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                chosen_comm = commands[i].comm;
            }
        }

        argc--; argv++; // skips command call name
        if (chosen_comm == NULL) {
            ret = ERR_INVALID_COMMAND;
        } else {
            ret = chosen_comm(argc, argv);
        }

    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERR_MSG(ret));
        help(argc, argv);
    }

    vips_shutdown();
    return ret;
}
