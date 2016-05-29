#ifndef CODEC_CIF_H
#define CODEC_CIF_H

#include "misc.h"
#include "codec.h"


#define SF_SQCIF                        1  /* 001 */
#define SF_QCIF                         2  /* 010 */
#define SF_CIF                          3  /* 011 */
#define SF_4CIF                         4  /* 100 */
#define SF_16CIF                        5  /* 101 */
#define SF_CUSTOM                       6  /* 110 */
#define SF_EPTYPE                       7  /* 111 Extended PTYPE */
#define PTYPE_MIN 1
#define PTYPE_MAX 5

int yuv_init_image(IMAGE *image);
int yuv_read_image(CONTEXT context, int nframe, IMAGE *image);
int yuv_write_image(CONTEXT context, int nframe, IMAGE *image);
int yuv_copy_image(IMAGE *dst, IMAGE *src);
void yuv_finit_image(IMAGE *image);

typedef struct yuv_param_struct {
  unsigned int type;
  unsigned int width;
  unsigned int height;
} yuv_param;


#endif
