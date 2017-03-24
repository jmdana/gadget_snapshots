
/*
 *
 * Modify both ALLOWED_TAGS and N_ALLOWED_TAGS in order 
 * to cherry-pick the tags for the destination file
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]) {
    FILE *src;
    FILE *dst;
  
    if(argc < 2) {
        printf("Usage: \n");
        printf("\tRead Mode:\n");
        printf("\t\t%s <snapshot>\n", argv[0]);
        printf("\tWrite Mode:\n");
        printf("\t\t%s <src_snapshot> <dst_snapshot>\n", argv[0]);
        exit(1);
    }

    src = fopen(argv[1], "r");

    if(argc > 2)
        dst = fopen(argv[2], "w");
    else
        dst = NULL;


    read_snapshot(dst, src);

    fclose(src);

    if(dst)
        fclose(dst);
    
    return 0;
}