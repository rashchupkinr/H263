#include "misc.h"
#include "cif.h"
#include "codec.h"

yuv_param yuv_param_table[] = {
	{0, 0, 0},
	{SF_SQCIF, 128, 96},
	{SF_QCIF, 176, 144},
	{SF_CIF, 352, 288},
	{SF_4CIF, 704, 576},
	{SF_16CIF, 1408, 1152},
	{0, 0, 0},
	{0, 0, 0}
};


int yuv_write_image(CONTEXT context, int nframe, IMAGE* image)
{
	int offset;
	if (!context.o_stream || !image || nframe<0) return -1;

	offset = nframe*image->width*image->height * 3/2;
	if (fseek(context.o_stream, offset, SEEK_SET))
		return -1;
	if (fwrite(image->lum, sizeof(char), image->width*image->height	, context.o_stream) !=
				image->width*image->height)
		return -1;
	if (fwrite(image->Cr , sizeof(char), image->width*image->height/4, context.o_stream) != 
				image->width*image->height/4)
		return -1;
	if (fwrite(image->Cb , sizeof(char), image->width*image->height/4, context.o_stream) !=
				image->width*image->height/4)
		return -1;
	return 0;
}

int yuv_read_image(CONTEXT context, int nframe, IMAGE* image)
{
	int offset;
	if (!context.i_stream || !image || nframe<0) return -1;
  
    if (!image->lum)
        yuv_init_image(image);

	offset = nframe*image->width*image->height * 3/2;
	if (fseek(context.i_stream, offset, SEEK_SET))
		return -1;
	if (fread(image->lum, sizeof(char), image->width*image->height, context.i_stream) !=
				image->width*image->height)
		return -1;
	if (fread(image->Cr , sizeof(char), image->width*image->height/4, context.i_stream) != 
				image->width*image->height/4)
		return -1;
	if (fread(image->Cb , sizeof(char), image->width*image->height/4, context.i_stream) !=
				image->width*image->height/4)
		return -1;
	return 0;
}

int yuv_init_image(IMAGE *image)
{
	if (!image || !image->src_format || !yuv_param_table[image->src_format].type)
		return -1;
	image->width = yuv_param_table[image->src_format].width;
	image->height = yuv_param_table[image->src_format].height;
	image->lum = (char *)malloc(image->width*image->height);
	image->Cr = (char *)malloc(image->width*image->height / 4);
	image->Cb = (char *)malloc(image->width*image->height / 4);

	if (!image->lum || !image->Cr || !image->Cb) {
		fprintf(stderr, "can't allocate memory for image\n");
		return -1;
	}
	return 0;	
}

void yuv_finit_image(IMAGE *image)
{
	if (!image) return;
	free(image->lum); 
	free(image->Cr); 
	free(image->Cb);
}

int yuv_copy_image(IMAGE *dst, IMAGE *src)
{
	if (!dst || !dst->lum || !src || !src->lum ||
			dst->height != src->height || dst->width != src->width)
		return -1;
	memcpy(dst->lum, src->lum, dst->width*dst->height);
	memcpy(dst->Cr, src->Cr, dst->width*dst->height / 4);
	memcpy(dst->Cb, src->Cb, dst->width*dst->height / 4);
	return 0;
}

