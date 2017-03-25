
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
#include <termios.h>

/*
 *
 * Modify both ALLOWED_TAGS and N_ALLOWED_TAGS in order
 * to cherry-pick the tags for the destination file
 *
 */

#define N_ALLOWED_TAGS  9

const char* ALLOWED_TAGS[] = { 
    "HEAD",
    "POS ",
    "VEL ",
    "ID  ",
    "MASS",
    "U   ",
    "RHO ",
    "HSML",
    "AGSH",
};

#define TAG_SIZE    8
#define HEADER_SIZE 256

typedef struct {
    int size1;
    char *data;
    int size2;
} block;

typedef struct {
    block *tag;
    block *data;
} datablock;

typedef struct gadget_header {
  int npart[6];
  double mass[6];
  double time;
  double redshift;
  int flag_sfr;
  int flag_feedback;
  int npartTotal[6];
  int flag_cooling;
  int num_files;
  double BoxSize;
  double Omega0;
  double OmegaLambda;
  double HubbleParam;
} header;

int getonechar() {
    int c;
    static struct termios oldt, newt;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    c = getchar();

    /*restore the old settings*/
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return c;
}

int is_allowed(char *tag) {
    int i;

    for(i=0; i<N_ALLOWED_TAGS;i++)
        if(!strcmp(tag, ALLOWED_TAGS[i]))
            return 1;

    return 0;
}

int read_size(FILE *f) {
    int size;

    fread(&size, sizeof(int), 1, f);

    if(feof(f))
        return 0;

    return size;
}

int print_header(header h) {
    printf("********************************************\n");

    printf("Gas   | #: %10d\t| Mass: %f\n", h.npart[0], h.mass[0]);
    printf("Halo  | #: %10d\t| Mass: %f\n", h.npart[1], h.mass[1]);
    printf("Disk  | #: %10d\t| Mass: %f\n", h.npart[2], h.mass[2]);
    printf("Bulge | #: %10d\t| Mass: %f\n", h.npart[3], h.mass[3]);
    printf("Stars | #: %10d\t| Mass: %f\n", h.npart[4], h.mass[4]);
    printf("Bndry | #: %10d\t| Mass: %f\n", h.npart[5], h.mass[5]);

    printf("Time: %f\n", h.time);
    printf("Redshift: %f\n", h.redshift);

    printf("FlagSfr: %d\n", h.flag_sfr);
    printf("FlagFeedback: %d\n", h.flag_feedback);
    printf("FlagCooling: %d\n", h.flag_cooling);

    printf("NumFiles: %d\n", h.num_files);

    printf("BoxSize: %f\n", h.BoxSize);

    printf("Omega0: %f\n", h.Omega0);
    printf("OmegaLambda: %f\n", h.OmegaLambda);
    printf("HubbleParam: %f\n", h.HubbleParam);
    
    printf("********************************************\n");

    return 0;
}

int write_block(FILE *dst, block *b) {
    fwrite(&b->size1, sizeof(int), 1, dst);
    fwrite(b->data, sizeof(char), b->size1, dst);
    fwrite(&b->size2, sizeof(int), 1, dst);
    printf("Writing %d + %ld * 2 bytes\n", b->size1, sizeof(int));

    return 0;
}

int write_datablock(FILE *dst, datablock *db) {
    write_block(dst, db->tag);
    write_block(dst, db->data);

    return 0;
}

block *read_block(FILE *src) {
    block *b = NULL;
    int size = read_size(src);

    if(!size)
        return b;

    b = (block *) malloc(sizeof(block));
    b->data = (char *) malloc(sizeof(char) * size);

    b->size1 = size;
    fread(b->data, sizeof(char), size, src);

    size = read_size(src);
    b->size2 = size;

    return b;
}

datablock *read_datablock(FILE *src) {
    datablock *db;

    db = (datablock *) malloc(sizeof(*db));
    db->tag = read_block(src);

    if(!db->tag) {
        free(db);
        return NULL;
    }

    db->data = read_block(src);

    if(!db->data) {
        free(db->tag);
        free(db);
        return NULL;
    }

    return db;
}

int free_block(block *b) {
    free(b->data);
    free(b);

    return 0;
}

int free_datablock(datablock *db) {
    free_block(db->tag);
    free_block(db->data);
    free(db);

    return 0;
}

int read_snapshot(FILE *dst, FILE *src) {
    char tag[5];
    datablock *db;

    while(!feof(src)) {
        db = read_datablock(src);

        if(db) {
            strncpy(tag, db->tag->data, 4);
            tag[4] = '\0';
            printf("--------------------------------------------\n");
            printf("%s [data: %d B]\n", tag, db->data->size1);

            if(!strcmp("HEAD", tag))
                print_header(*(header *)db->data->data);

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
