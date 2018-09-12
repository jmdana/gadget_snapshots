
/*
 * Copyright (c) 2017 Jose M. Dana
 *
 * This file is part of gadget_snapshots.
 * http://github.com/jmdana/gadget_snapshots
 *
 * gadget_snapshots is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation (version 3 of the License only).
 *
 * gadget_snapshots is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gadget_snapshots. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "utils.h"
#include "snapshot.h"

/*
 *
 * Modify both ALLOWED_TAGS and N_ALLOWED_TAGS in order
 * to cherry-pick the tags for the destination file
 *
 */

#define N_ALLOWED_TAGS  13

const char* ALLOWED_TAGS[] = { 
    "HEAD",
    "POS ",
    "VEL ",
    "ID  ",
    "MASS",
    "U   ",
    "RHO ",
    "HSML",
    "POT ",
    "ACCE",
    "ENDT",
    "TSTP",
    "AGSH",
};

#define UNITS_MASS "10^10 M./h"
#define UNITS_DENSITY "10^10 h^-1 M./(h^-1 kpc)^3"
#define UNITS_U "(km/s)^2"
#define UNITS_HSML "kpc/h"

int is_allowed(char *tag) {
    int i;

    for(i=0; i<N_ALLOWED_TAGS;i++)
        if(!strcmp(tag, ALLOWED_TAGS[i]))
            return 1;

    return 0;
}

int read_snapshot_v1(char *src_name) {
    FILE *src;
    int i = 0;
    datablock *db;
    header h;

    do {
        printf("[%s]\n", src_name);

        src = fopen(src_name, "r");

        if(src == NULL) {
            perror(src_name);
            return 0;
        }

        init_snapshot(src);

        db = read_datablock(src);

        printf("--------------------------------------------\n");
        printf("[data: %d B]\n", db->data->size1);

        h = construct_header(db);
        print_header(h);

        free_datablock(db);
        fclose(src);

        i++;

        if(h.num_files > 1 && isdigit(src_name[strlen(src_name) - 1]))
            src_name[strlen(src_name) - 1] = i + 48;
    } while(i < h.num_files);

    return 0;
}

int read_snapshot_v2(char *dst_name, char *src_name) {
    char tag[5];
    FILE *src;
    FILE *dst;
    datablock *db;
    header h;

    src = fopen(src_name, "r");

    if(dst_name)
        dst = fopen(dst_name, "w");
    else
        dst = NULL;

    init_snapshot(src);

    while(!feof(src)) {
        db = read_datablock(src);

        if(db) {
            strncpy(tag, db->tag->data, 4);
            tag[4] = '\0';
            printf("--------------------------------------------\n");
            printf("%s [data: %d B]\n", tag, db->data->size1);

            if(!strcmp("HEAD", tag)) {
                h = construct_header(db);
                print_header(h);
            }
            else if(!strcmp("MASS", tag)) {
                mass_handler(h, db);
                printf("Units : %s\n", UNITS_MASS);
            }
            else if(!strcmp("RHO ", tag)) {
                onlygas_handler(h, db);
                printf("Units : %s\n", UNITS_DENSITY);
            }
            else if(!strcmp("U   ", tag)) {
                onlygas_handler(h, db);
                printf("Units : %s\n", UNITS_U);
            }
            else if(!strcmp("HSML", tag)) {
                onlygas_handler(h, db);
                printf("Units : %s\n", UNITS_HSML);
            }

            if(dst) {
                if(is_allowed(tag))
                    write_datablock(dst, db);
                else
                    printf("Skipping...\n");
            }
            free_datablock(db);
        }
    }

    fclose(src);

    if(dst_name)
        fclose(dst);

    return 0;
}

int read_snapshot(char *dst_name, char *src_name) {
    int format;
    FILE *src;

    src = fopen(src_name, "r");

    format = snapformat(src);

    fclose(src);

    printf("SnapFormat = %d\n", format);

    switch(format) {
        case 0:
            printf("SnapFormat UNKNOWN!\n");
            break;
        case 1:
            read_snapshot_v1(src_name);
            break;
        case 2:
            read_snapshot_v2(dst_name, src_name);
            break;
    }

    return 0;
}

void usage(char *exec) {
    printf("Usage: \n");
    printf("\tRead Mode:\n");
    printf("\t\t%s <snapshot>\n", exec);
    printf("\tWrite Mode:\n");
    printf("\t\t%s <src_snapshot> <dst_snapshot>\n", exec);
}

int main(int argc, char *argv[]) {
    FILE *src;
    char *src_name;
    char *dst_name;

    if(argc < 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(access(argv[1], F_OK)) {
        printf("Input file doesn't exist.\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    src_name = argv[1];
    src = fopen(src_name, "r");

    if(argc > 2) {
        if(!access(argv[2], F_OK)) {
            if(same_file(argv[1], argv[2])) {
                printf("You don't want <src_snapshot> and <dst_snaphot> to be the same file. Trust me.\n");
                printf("Exiting...\n");
                return EXIT_FAILURE;
            }
            printf("The file exists! Do you want to overwrite? (y/n): ");
            if(getonechar() != 'y') {
                printf("\nExiting...\n");
                return EXIT_FAILURE;
            }
        }
        dst_name = argv[2];
    }
    else
        dst_name = NULL;

    fclose(src);

    read_snapshot(dst_name, src_name);

    return EXIT_SUCCESS;
}
