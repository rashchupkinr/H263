#include "codec.h"
#include "cif.h"
#include "misc.h"

int mb_out_bound(MACROBLOCK *mb, int vx, int vy)
{
	if (2*mb->mx*MB_SIZE+vx < 0) return -1;
	if (2*((mb->mx+1)*MB_SIZE-1)+vx > 2*(mb->pic->image.width-1)) return -1;
	if (2*mb->my*MB_SIZE+vy < 0) return -1;
	if (2*((mb->my+1)*MB_SIZE-1)+vy > 2*(mb->pic->image.height-1)) return -1;
	return 0;
}

int get_block_x(MACROBLOCK *mb, int k)
{
	int x;

	if (k < 4) {
		x = mb->mx*2*BLOCK_SIZE;
		x += (k&1)*BLOCK_SIZE;
	} else
		x = mb->mx*BLOCK_SIZE;
	return x;
}

int get_block_y(MACROBLOCK *mb, int k)
{
	int y;

	if (k < 4) {
		y = mb->my*2*BLOCK_SIZE;
		y += (k>1)*BLOCK_SIZE;
	} else
		y = mb->my*BLOCK_SIZE;
	return y;
}

int get_block_pos(MACROBLOCK *mb, int k)
{
	return get_block_x(mb, k) + mb->pic->cwidth[k]*get_block_y(mb, k);
}

int load_block_data(MACROBLOCK *mb, int k, int dx, int dy)
{
	int hx, hy;
	hx = 2*get_block_x(mb, k) + dx;
	hy = 2*get_block_y(mb, k) + dy;
	
	return load_buf(mb->pic, mb->data[k], BLOCK_SIZE, k, hx, hy);
}

int load_mb_data(MACROBLOCK *mb, int dx, int dy)
{
	int k;
	mb->cbpc = mb->cbpy = 0;
	for (k=0; k<6; k++)
		if (load_block_data(mb, k, dx, dy) < 0)
			return -1;
	return 0;
}

int write_mb_data(MACROBLOCK *mb)
{
	int k;
	for (k=0; k<6; k++)
		store_block_data(mb, k);
	return 0;
}

int store_block_data(MACROBLOCK *mb, int k)
{
	int hx, hy;
	hx = 2*get_block_x(mb, k);
	hy = 2*get_block_y(mb, k);
	
	return store_buf(mb->pic, mb->data[k], BLOCK_SIZE, k, hx, hy);
}

int load_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy)
{
	int i, j;
	unsigned char *p;
	int height, width;
	p = pic->cdata[k] + pic->cwidth[k]*hy/2 + hx/2;
	height = pic->image.height+(k<4?BLOCK_SIZE:0);
	if (p<pic->cdata[k] || p+(pic->cwidth[k])*size>pic->cdata[k]+pic->cwidth[k]*height)
		return -1;
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			*buf++ = *p++;
		p += pic->cwidth[k] - size;
	}
	return 0;
}

int store_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy)
{
	int i, j;
	unsigned char *p;
	p = pic->cdata[k] + pic->cwidth[k]*hy/2 + hx/2;
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			*p++ = *buf++;
		p += pic->cwidth[k] - size;
	}
	return 0;
}

int diff_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy)
{
	int i, j;
	unsigned char *p;
	p = pic->cdata[k] + pic->cwidth[k]*hy/2 + hx/2;
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			*buf++ -= *p++;
		p += pic->cwidth[k] - size;
	}
	return 0;
}

int add_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy)
{
	int i, j;
	unsigned char *p;
	p = pic->cdata[k] + pic->cwidth[k]*hy/2 + hx/2;
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			*buf++ += *p++;
		p += pic->cwidth[k] - size;
	}
	return 0;
}

int clip_buf(int *buf, int size, int min, int max, int skip_dc)
{
	int i;
	buf += skip_dc;
	for (i=skip_dc; i<size*size; i++) {
		*buf = *buf<=max ? *buf : max;
		*buf = *buf>=min ? *buf : min;
		buf++;
	}
	return 0;
}

int sad_buf(PICTURE *pic, int *buf, int size, int k, int hx, int hy)
{
	int i, j;
	unsigned char *p;
	int sad = 0;
	p = pic->cdata[k] + pic->cwidth[k]*hy/2 + hx/2;
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			sad += abs(*buf++ - *p++);
		p += pic->cwidth[k] - size;
	}
	return sad;
}

// interpolate [hx-1][hy-1] to buf[2*size+1][2*size+1]
// hx,hy - as even 
int load_buf_ipol(PICTURE *pic, unsigned char *buf, int size,
					int k, int hx, int hy)
{
	int i, j;
	unsigned char *p;
	int width;
	int rc;

	rc = pic->rtype;
	width = pic->cwidth[k];
	p = pic->cdata[k] + hy/2*width + hx/2;
//    if (k>3)
  //      p=pic->cdata[k]+hy/4*width+hx/4;

	if (hx>1 && hy>1)
		*buf++ = (*(p-width-1) + *(p-width) + *(p-1) + *p + 2 - rc)/4;
	else	
		*buf++ = 0;
	for (j=0; j<size; j++) {
		if (hy>1) {
			*buf++ = (*p + *(p-width) + 1 - rc)/2;
			*buf++ = (*(p-width) + *(p-width+1) + *p + *(p+1) + 2 - rc)/4;
			p++;
		} else {
			*buf++ = 0; *buf++ = 0;
			p++;
		}
	}
	p-=size;

	for (i=0; i<size; i++) {
		if (hx>1) {
			*buf = (*(p-1) + *p + 1 - rc)/2;
			*(buf+2*size+1) = (*(p-1) + *(p-1+width)	+ *p + *(p+width) + 2 - rc)/4;
		} else {
			*buf = 0;
			*(buf+2*size+1) = 0;
		}
		buf++;	
		
		for (j=0; j<size; j++) {
			*buf = *p;
			*(buf+2*size+1) = (*p + *(p+width) + 1 - rc)/2;
			buf++;
			*buf = (*p + *(p+1) + 1 - rc)/2;
			*(buf+2*size+1) = (*p + *(p+1) + *(p+width) + *(p+width+1) + 2 - rc)/4;
			buf++;
			p++;
		}
		
		p += width - size;
		buf += 2*size+1;
	}
	return 0;
}

// ibuf[2*size+1][2*size+1] - interolated
// -1 <= hx, hy <= 1
int sad_buf_ipol(int *dbuf, unsigned char *ibuf, int size, int vx, int vy)
{
	int i, j;
	unsigned char *ipol;
	int sad = 0;

	ipol = ibuf + (vx+1) + (vy+1)*(2*size+1);

	for (i=0; i<size; i++) {
		for (j=0; j<size; j++) {
			sad += abs(*ipol - *dbuf);
			ipol += 2;
			dbuf++;
		}
		ipol += 2*size+1 + 1;
	}
	return sad;
}

int diff_buf_ipol(int *dbuf, unsigned char *ibuf, int size, int vx, int vy)
{
	int i, j;
	unsigned char *ipol;

	ipol = ibuf + (vx+1) + (vy+1)*(2*size+1);

	for (i=0; i<size; i++) {
		for (j=0; j<size; j++) {
			*dbuf -= *ipol;
			ipol += 2;
			dbuf++;
		}
		ipol += 2*size+1 + 1;
	}
	
	return 0;
}

int add_buf_ipol(int *dbuf, unsigned char *ibuf, int size, int vx, int vy)
{
	int i, j;
	unsigned char *ipol;

	ipol = ibuf + (vx+1) + (vy+1)*(2*size+1);
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++) {
			*dbuf += *ipol;
			ipol += 2;
			dbuf++;
		}
		ipol += 2*size+1 + 1;
	}
	
	return 0;
}

