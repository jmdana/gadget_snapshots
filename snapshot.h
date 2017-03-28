
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

#define TAG_SIZE    8
#define HEADER_SIZE 256

int SWAP;

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

datablock *read_datablock(FILE *src);
int free_datablock(datablock *db);
int write_datablock(FILE *dst, datablock *db);
int print_header(header h);
int endianness(FILE *f);
header construct_header(datablock *db);
int mass_handler(header h, datablock *db);
int onlygas_handler(header h, datablock *db);
