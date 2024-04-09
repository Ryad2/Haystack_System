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
    int ret = 0;

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        argc--; argv++; // skips command call name

        int comm_qte = 4;
        command chosen_comm = (void*);
        command_mapping[] commands = {{"list", do_list_cmd},
                                      {"create", do_create_cmd},
                                      {"help", help},
                                      {"delete", do_delete_cmd}};

        for(int i = 0; i < comm_qte; ++i) {
            if (strcmp(argv[0], commands[i][0]) == 0) {
                chosen_comm = commands[i][1];
            }
        }
        argc--; argv++; // option used, now it can be discarded
        if (chosen_comm == (void*)) {
            ret = ERR_INVALID_COMMAND;
            help(argc, argv);
        } else { // FIXME do we need to add a NULL to argv
            chosen_comm(argc, argv);
        }

    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERR_MSG(ret));
        help(argc, argv);
    }

    return ret;
}
