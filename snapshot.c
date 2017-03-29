
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

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <byteswap.h>
#include "snapshot.h"

inline long swap64(long x) {
    if(!SWAP)
        return x;

    return bswap_64(x);
}

inline int swap32(int x) {
    if(!SWAP)
        return x;

    return bswap_32(x);
}

inline short swap16(short x) {
    if(!SWAP)
        return x;

    return bswap_16(x);
}

inline float swapf(float x) {
    if(!SWAP)
        return x;

    float outf;
    char *in = (char *) &x;
    char *out = (char*) &outf;

    out[0] = in[3];
    out[1] = in[2];
    out[2] = in[1];
    out[3] = in[0];

    return outf;
}

inline double swapd(double x) {
    if(!SWAP)
        return x;

    double outd;
    char *in = (char *) &x;
    char *out = (char*) &outd;

    out[0] = in[7];
    out[1] = in[6];
    out[2] = in[5];
    out[3] = in[4];
    out[4] = in[3];
    out[5] = in[2];
    out[6] = in[1];
    out[7] = in[0];

    return outd;
}


int read_size(FILE *f) {
    int size;

    fread(&size, sizeof(int), 1, f);

    if(feof(f))
        return 0;
    
    return swap32(size);
}


header construct_header(datablock *db) {
    int i;
    header h = *(header *)db->data->data;

    if(SWAP) {
        for(i=0; i<6; i++) {
            h.npart[i] = swap32(h.npart[i]);
            h.mass[i] = swapd(h.mass[i]);
            h.npartTotal[i] = swap32(h.npartTotal[i]);
        }

        h.time = swapd(h.time);
        h.redshift = swapd(h.redshift);

        h.flag_sfr = swap32(h.flag_sfr);
        h.flag_feedback = swap32(h.flag_feedback);
        h.flag_cooling = swap32(h.flag_cooling);
        h.num_files = swap32(h.num_files);

        h.BoxSize = swapd(h.BoxSize);
        h.Omega0 = swapd(h.Omega0);
        h.OmegaLambda = swapd(h.OmegaLambda);
        h.HubbleParam = swapd(h.HubbleParam);

    }

    return h;
}

int endianness(FILE *f) {
    long pos;
    int size;

    pos = ftell(f);

    rewind(f);

    size = read_size(f);

    fseek(f, pos, SEEK_SET);

    if(size == TAG_SIZE)
        return BYTE_ORDER;
    else
        return BYTE_ORDER == LITTLE_ENDIAN ? BIG_ENDIAN : LITTLE_ENDIAN;
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
    int s1;
    int s2;

    printf("Writing %d + %ld * 2 bytes\n", b->size1, sizeof(int));

    s1 = swap32(b->size1);
    s2 = swap32(b->size2);

    fwrite(&s1, sizeof(int), 1, dst);
    fwrite(b->data, sizeof(char), b->size1, dst);
    fwrite(&s2, sizeof(int), 1, dst);

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

int init_snapshot(FILE *src) {
    int endian;

    endian = endianness(src);

    printf("Your machine is %s\n", BYTE_ORDER == LITTLE_ENDIAN ? "Little-Endian" : "Big-Endian");
    printf("The file is %s\n", endian == LITTLE_ENDIAN ? "Little-Endian" : "Big-Endian");

    SWAP = (endian != BYTE_ORDER);

    if(SWAP)
        printf("Byte swapping required \n");

    return 0;
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

int mass_handler(header h, datablock *db) {
    int i;
    int m;
    float *masses;
    float min;
    float max;
    int total_size = 0;

    masses = (float *) db->data->data;

    for(i=0; i<6; i++)
        if(h.npart[i] != 0 && h.mass[i] == 0) {
            total_size += h.npart[i] * sizeof(float);
            min = FLT_MAX;
            max = FLT_MIN;

            for(m=0; m < h.npart[i]; m++) {
                min = fmin(min, swapf(masses[m]));
                max = fmax(max, swapf(masses[m]));
            }

            if(min == max)
                printf("Type %d: [%f]\n", i, min);
            else
                printf("Type %d: [%f - %f]\n", i, min, max);

            masses += h.npart[i];
        }

    if(total_size != db->data->size1)
        printf("The measured size is different from the reported by the block delimiter!\n");

    return 0;
}

int onlygas_handler(header h, datablock *db) {
    int i;
    float *values;
    float min;
    float max;
    int total_size = 0;

    values = (float *) db->data->data;

    if(h.npart[0] != 0) {
        min = FLT_MAX;
        max = FLT_MIN;

        total_size += h.npart[0] * sizeof(float);

        for(i=0; i < h.npart[0]; i++) {
            min = fmin(min, swapf(values[i]));
            max = fmax(max, swapf(values[i]));
        }

        if(min == max)
            printf("Type 0: [%f]\n", min);
        else
            printf("Type 0: [%f - %f]\n", min, max);
    }

    if(total_size != db->data->size1)
        printf("The measured size is different from the reported by the block delimiter!\n");

    return 0;
}

