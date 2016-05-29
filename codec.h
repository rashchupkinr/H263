#ifndef CODEC_CODEC_H
#define CODEC_CODEC_H

#include "stdio.h"

typedef struct _context {
	FILE *i_stream, *o_stream, *t_stream;
	int op_quant;
	int insert_intra_cycle;
	int debug;
	int motest_alg;
} CONTEXT;

extern int debugm;
#define DEBUGM_1 0x1
#define DEBUGM_2 0x2
#define DEBUGM_3 0x4
#define DEBUGM_4 0x8
extern int debug, trace_mv;


#define ANNEX_MP		0x00001
#define ANNEX_UMV		0x00002
#define ANNEX_SAC		0x00004
#define ANNEX_ADV_PRED		0x00008
#define ANNEX_PB		0x00010
#define ANNEX_FEC		0x00020
#define ANNEX_ADV_INTRA		0x00040
#define ANNEX_DEB_FILT		0x00080
#define ANNEX_SLICE		0x00100
#define ANNEX_SE_INF_SPEC	0x00200
#define ANNEX_IPB		0x00400
#define ANNEX_REF_PIC_SEL	0x00800
#define ANNEX_T_SNR_SS		0x01000
#define ANNEX_REF_PIC_RESAMPLE	0x02000
#define ANNEX_RRU		0x04000
#define ANNEX_INDEP_SEG_DEC	0x08000
#define ANNEX_ALT_INTERVLC	0x10000
#define ANNEX_MQUANT		0x20000
extern int annex;

#define MOTEST_FS	0
#define MOTEST_CBOSA	1
#define MOTEST_4SS	2
#define MOTEST_HASA	3
#define MOTEST_MAX	3

#define MS_HASA_START_TH1	3.61*BLOCK_LEN
#define MS_HASA_START_TH2	10.15*BLOCK_LEN
struct ms_hasa_data_struct {
  int TH1, TH2;
  int N1, N2, E1, E2;
};
extern struct ms_hasa_data_struct ms_hasa_data;

extern int use_gob;

typedef struct _block BLOCK;
typedef struct _mv MV;
typedef struct _mb MACROBLOCK;
typedef struct _gob GOB;
typedef struct _picture PICTURE;

#define MB_SIZE 16
#define BLOCK_SIZE 8
#define BLOCK_LEN	(BLOCK_SIZE * BLOCK_SIZE)
#define BLOCKS_MB 6

#define BSIZE(k) ((k)<4 ? MB_SIZE : BLOCK_SIZE)

typedef struct _image {
  char *lum;
  char *Cr;
  char *Cb;
  int src_format;
  int width, height;
  CONTEXT codec_context;
} IMAGE;


struct _mv {
  char hx;
  char hy;
};

struct _mb {
  int cod;
  unsigned char type;
  short cbp;
  char cbpc;
  char modb;
  char cbpb;
  char cbpy;
  int dquant;
  MV mv[6];
  MV predicted_mv[6];
  int sad[6];
  int motest_sp;

  int data[6][BLOCK_LEN];
  int mx, my;

  PICTURE *pic;
//  GOB *gob;
};

struct _gob {
  char gn;
  char gfid;
  char gquant;
  int  nmb;
  MACROBLOCK *mb;

  PICTURE *pic;
};

typedef struct _pct_stat {
  int header;
  int coeff_y;
  int coeff_c;
  int vectors;
  int cbpy;
  int mcbpc;
  int modb;
  int cbpb;
  int cod;
  int dquant;
  int framesize;
} PCT_STAT;

struct _picture {
  int tr;
  IMAGE image;

  unsigned int ptype;
  int pquant;
  int trb;
  int dbquant;
  int npsupp;
  char *psupp;
  int rtype;

  int nmx, nmy;				// mb->mx<nmx; mb->my<nmy
  MACROBLOCK **mb;			// addressed pic->mb[my][mx]
  int ngob;
  GOB *gob;

  unsigned char *cdata[6];	// image->component for block k
  int cwidth[6];			// width in image->component for mb->block[k]

  unsigned char *dbuf;		// [2*MB_SIZE+1][2*MB_SIZE+1]
  int *ibuf;				// [MB_SIZE][MB_SIZE]

  PICTURE *prev;
  PICTURE *next;

  PCT_STAT stat;
};


#define PCT_INTRA                       0
#define PCT_INTER                       1
#define PCT_IPB                         2
#define PCT_B                           3
#define PCT_EI                          4
#define PCT_EP                          5
#define PCT_PB                          6

#define MODE_INTER                      0
#define MODE_INTER_Q                    1
#define MODE_INTER4V                    2
#define MODE_INTRA                      3
#define MODE_INTRA_Q                    4
#define MODE_INTER4V_Q                  5


/* estimation types. */
#define P_PICTURE_ESTIMATION            0
#define PB_PICTURE_ESTIMATION           1
#define B_PICTURE_ESTIMATION            2
#define EI_EP_PICTURE_ESTIMATION        3


void codec_init(CONTEXT context);
int codec_decode_picture(PICTURE *pic);
int codec_encode_image(PICTURE *pic, IMAGE *img);
int codec_finit();

#endif
