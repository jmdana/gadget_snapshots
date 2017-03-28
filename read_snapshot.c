
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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

int read_snapshot(FILE *dst, FILE *src) {
    char tag[5];
    datablock *db;
    header h;

    if(endianness(src) == BIG_ENDIAN)
        printf("The file is Big-Endian\n");

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
    FILE *dst;
  
    if(argc < 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(access(argv[1], F_OK)) {
        printf("Input file doesn't exist.\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    src = fopen(argv[1], "r");

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
        dst = fopen(argv[2], "w");
    }
    else
        dst = NULL;


    read_snapshot(dst, src);

    fclose(src);

    if(dst)
        fclose(dst);
    
    return EXIT_SUCCESS;
}
