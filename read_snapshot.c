
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
    

    return 0;
}

int read_block(char **buffer, FILE *src) {
    // Delimiter
    int size = read_size(src);

    if(!size)
        return 0;

    *buffer = (char *) malloc(size);

    fread(*buffer, sizeof(char), size, src);

    if(size == TAG_SIZE) {
        char tag[5];
        strncpy(tag, *buffer, 4);
        tag[4] = '\0';
        printf("--------------------------------------------\n");
        printf("%s\n", tag);

        if(!is_allowed(tag)) {
            free(*buffer);
            size = 0;
        }
    }
    else if(size == HEADER_SIZE) {
        print_header(*((header *) *buffer));
    }
    else
        printf("SIZE: %d\n", size);

    // Delimiter
    read_size(src);

    return size;
}

int read_snapshot(FILE *dst, FILE *src) {
    int size;
    char *buffer;
    int dump_next;

    dump_next = 0;

    while(!feof(src)) {
        size = read_block(&buffer, src);

        // skip the tag and the block itself
        if(!size) {
            dump_next = 1;
            continue;
        }
        
        // write the block if there is a destination
        // file and dump_next is not flagged
        if(dst && !dump_next) {
            fwrite(&size, sizeof(int), 1, dst);
            fwrite(buffer, sizeof(char), size, dst);
            fwrite(&size, sizeof(int), 1, dst);
        }
        
        // reset the flag
        dump_next = 0;
        
        if(buffer)
            free(buffer);
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
        exit(1);
    }

    if(access(argv[1], F_OK)) {
        printf("Input file doesn't exist.\n");
        usage(argv[0]);
        exit(1);
    }

    src = fopen(argv[1], "r");

    if(argc > 2) {
        if(!access(argv[2], F_OK)) {
            printf("The file exists! Do you want to overwrite? (y/n): ");
            if(getonechar() != 'y') {
                printf("\nExiting...\n");
                exit(1);
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
    
    return 0;
}
