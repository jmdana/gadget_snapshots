#include <stdlib.h>
#include "snapshot.h"

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


int write_block(FILE *dst, block *b) {
    printf("Writing %d + %ld * 2 bytes\n", b->size1, sizeof(int));
    fwrite(&b->size1, sizeof(int), 1, dst);
    fwrite(b->data, sizeof(char), b->size1, dst);
    fwrite(&b->size2, sizeof(int), 1, dst);

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

    if(b->size1 != b->size2) {
        printf("The delimiters don't agree on the size of the block!\n");
        printf("%d != %d\n", b->size1, b->size2);
        printf("Exiting...\n");
        exit(EXIT_FAILURE);
    }

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

