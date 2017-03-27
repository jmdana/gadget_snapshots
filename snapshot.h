
#include <stdio.h>

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
