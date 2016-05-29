#include "misc.h"
#include "codec.h"
#include "cif.h"

int debugm = 0;
int debug, trace_mv = 0;
int annex = 0;
extern int Decode;
extern PICTURE *pic;
int frame_num = 0;
int force_intra;
int use_gob;


int select_picture_type(CONTEXT context);
int encode_image(PICTURE *pic);
int decode_image(PICTURE *pic);




int codec_encode_image(PICTURE *pic,IMAGE *image)
{
  if (!image) return -1;
  trace("codec_encode_image(%p)\n", image);	
  printf("frame %d\n",pic->tr);
  memset(&pic->stat, 0, sizeof(pic->stat));
  force_intra = load_picture_image(pic, image);
  pic->pquant = pic->image.codec_context.op_quant;

  pic->ptype = select_picture_type(pic->image.codec_context);
  ms_new_frame(pic);
  write_h263_picture(pic);
  encode_image(pic);
  trace("framestat %d\n", frame_num);
  trace_stat(&pic->stat);

  decode_image(pic);

  pic = pic->prev;
  trace("\n");
  frame_num++;
  return 0;
}

int codec_decode_picture(PICTURE *pic)
{
  if (!pic) return -1;
  trace("codec_decode_picture(%p)\n", pic);
  memset(&pic->stat, 0, sizeof(pic->stat));
  decode_image(pic);
  if (Decode)	
	  yuv_write_image(pic->image.codec_context, frame_num, &(pic->image));

  trace("\n");
  frame_num++;
  return 0;
}

int encode_image(PICTURE *pic)
{
  int i, j;
  MACROBLOCK *mb;
  int statcnt;
  int framesize;
  framesize = bitcount();
  trace("frame %d: encode_image(%p)\n", frame_num, pic);
  pic->tr = pic->prev->tr + 1;
  for (i=0; i<pic->nmy; i++)
    for (j=0; j<pic->nmx; j++) {
      trace("\nMB %d %d\n", i, j);
      mb = &pic->mb[i][j];

      load_mb_data(mb, 0, 0);

      switch (pic->ptype) {
      case PCT_INTRA:
	    frame_I_encode_MB(mb);
	    break;
      case PCT_INTER:
	    frame_P_encode_MB(mb);
	    break;
      }
      write_h263_mb(mb);
    }
  alignbits();
  framesize = bitcount()-framesize;
  pic->stat.framesize = framesize;
  return 0;
}

int decode_image(PICTURE *pic)
{
  int i, j;
  MACROBLOCK *mb;
  pic->tr = pic->prev->tr + 1;
  trace("decode_image(%p)\n", pic);

  for (i=0; i<pic->nmy; i++)
    for (j=0; j<pic->nmx; j++) {
      mb = &pic->mb[i][j];
      if (Decode) {
          trace("MACROBLOCK[%d][%d]\n", i, j);
          read_h263_mb(mb);
      }

      switch (pic->ptype) {
      case PCT_INTRA:
	    frame_I_decode_MB(mb);
	    break;
      case PCT_INTER:
	    frame_P_decode_MB(mb);
	    break;
      }
        write_mb_data(mb);
    }

  if (trace_mv && pic->ptype == PCT_INTER)
    trace_pic_mv(pic);

  return 0;
}

int select_picture_type(CONTEXT context)
{
  int intracycle = 30;
  if (force_intra || !(frame_num % context.insert_intra_cycle)) {
    force_intra = 0;
    return PCT_INTRA;
  } else {
    return PCT_INTER;
  }
}

extern FILE *_t_stream;
extern int debug;

void codec_init(CONTEXT context)
{
  _t_stream = context.t_stream;
  debug = context.debug;
  trace("codec_init()\n\n");
  pic = malloc(sizeof(PICTURE));
  memset(pic,0,sizeof(*pic));
  pic->prev = malloc(sizeof(PICTURE));
  memset(pic->prev,0,sizeof(*(pic->prev)));
  pic->prev->prev = pic;

  pic->image.codec_context.debug = 5;
  pic->image.codec_context.op_quant = 1;
  pic->image.codec_context.motest_alg = MOTEST_CBOSA;
  memcpy(&pic->image.codec_context, &context, sizeof(context));
  memcpy(&pic->prev->image.codec_context, &context, sizeof(context));
  initbits(context);
  initgetbits(context);
}

int codec_finit()
{
  if (!Decode) {
	trace("EOS:\t");	putbits(22, 0x00003f);
	alignbits();
  }

  trace("codec_finit()\n\n");
  finit_picture(pic->prev);
  free(pic->prev);
  finit_picture(pic);
  free(pic);
  return 0;
}

