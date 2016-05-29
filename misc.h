#ifndef CODEC_MISC_H
#define CODEC_MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "wingetopt.h"
//#include "unistd.h"
#include <stdarg.h>

#include "codec.h"
#include "frame.h"


#define sign(a)		((a) < 0 ? -1 : 1)
#define mmax(a, b)	((a) > (b) ? (a) : (b))
#define mmin(a, b)	((a) < (b) ? (a) : (b))


void dct(int *block);
void idct(int *block);
void scan(MACROBLOCK *mb, int k, int *buf);
void init_mb_cbp(MACROBLOCK *mb);
int quant(MACROBLOCK *mb, int k, int q);
int dequant(MACROBLOCK *mb, int k, int q);

void write_h263_picture(PICTURE *pic);
void write_h263_gob(GOB *gob);
void write_h263_mb(MACROBLOCK *mb);
void write_h263_mv(MACROBLOCK *mb, int k);
void write_h263_block(MACROBLOCK *mb, int k, int write_coef);

int load_picture_image(PICTURE *pic, IMAGE *image);
int init_picture(PICTURE *pic);
void finit_picture(PICTURE *pic);
int init_mb(MACROBLOCK *mb);
void finit_mb(MACROBLOCK *mb);
int mb_out_bound(MACROBLOCK *mb, int vx, int vy);
int get_block_pos(MACROBLOCK *mb, int k);
int get_block_x(MACROBLOCK *mb, int k);
int get_block_y(MACROBLOCK *mb, int k);
int load_block_data(MACROBLOCK *mb, int k, int dx, int dy);
int load_mb_data(MACROBLOCK *mb, int dx, int dy);
int store_block_data(MACROBLOCK *mb, int k);
int write_mb_data(MACROBLOCK *mb);
int load_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy);
int store_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy);
int diff_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy);
int add_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy);
int clip_buf(int *buf, int size, int min, int max, int skip_dc);
int sad_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy);
int load_buf_ipol(PICTURE *pic, unsigned char *buf, int size,
    			int k, int hx, int hy);
int sad_buf_ipol(int *ibuf, unsigned char *dbuf, int size, int vx, int vy);
int diff_buf_ipol(int *ibuf, unsigned char *dbuf, int size, int vx, int vy);
int add_buf_ipol(int *ibuf, unsigned char *dbuf, int size, int vx, int vy);

int diff_block_motion(MACROBLOCK *mb, int k);
int add_block_motion(MACROBLOCK *mb, int k);
void motion_estimate_block(MACROBLOCK *mb, int k);
int ms_new_frame(PICTURE *pic);


void initbits(CONTEXT context);
void initgetbits(CONTEXT context);
int  alignbits();
void putbits(int n, int val);
int  bitcount();
void BitPrint(int length, int val, char *bit);

void trace(char *fmt, ...);
void trace_block(int *block);
void trace_cblock(unsigned char *block, int size);
void trace_iblock(int *block, int size);
void trace_pic_mv(PICTURE *pic);

#endif
