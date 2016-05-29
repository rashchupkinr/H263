#include "codec.h"
#include "cif.h"
#include "misc.h"


int init_picture(PICTURE *pic)
{
	int i, j, k;
	pic->ptype = PCT_INTRA;
	pic->tr = 0;
	pic->trb = 0;
	pic->dbquant = 0;
	pic->npsupp = 0;
	pic->psupp = 0;
	pic->rtype = 0;
	pic->ibuf = malloc(MB_SIZE*MB_SIZE*sizeof(*pic->ibuf));
	pic->dbuf = malloc((2*MB_SIZE+1)*(2*MB_SIZE+1)*sizeof(*pic->dbuf));
	
	if (use_gob) {
		if (pic->image.height <= 400)
			k = 1;
		else if (pic->image.height <= 800)
			k = 2;
		else if (pic->image.height <= 1152)
			k = 4;
		else 
			k = 1;
		pic->ngob = (pic->image.height + k*MB_SIZE-1)/(k*MB_SIZE);
		pic->gob = (GOB *)malloc(sizeof(GOB));
		if (!pic->gob)
			return -1;
	} else if (annex & ANNEX_SLICE) {
	}

	pic->nmx = pic->image.width/MB_SIZE;
	pic->nmy = pic->image.height/MB_SIZE;
	pic->mb = (MACROBLOCK **)malloc(pic->nmy * sizeof(MACROBLOCK *));
	if (!pic->mb) return -1;
	for (i=0; i<pic->nmy; i++) {
		pic->mb[i] = malloc(pic->nmx * sizeof(MACROBLOCK));
		if (!pic->mb[i]) return -1;
		for (j=0; j<pic->nmx; j++) {
			MACROBLOCK *mb;
			int k;

			mb = &pic->mb[i][j];
			mb->pic = pic;
			mb->mx = j;
			mb->my = i;
            for (k=0;k<6;k++)
               mb->mv[k].hx=mb->mv[k].hy=mb->predicted_mv[k].hx=mb->predicted_mv[k].hy=0;
		}
	}

	pic->cdata[0] = pic->cdata[1] = pic->cdata[2] =
			pic->cdata[3] = pic->image.lum; 
	pic->cdata[4] = pic->image.Cr;
	pic->cdata[5] = pic->image.Cb;
	pic->cwidth[0] = pic->cwidth[1] = pic->cwidth[2] =
		pic->cwidth[3] = pic->image.width;
	pic->cwidth[4] = pic->cwidth[5] = pic->image.width/2;
	
	return 0;
}

void finit_picture(PICTURE *pic)
{
	int i, j;
	for (i=0; i<pic->nmy; i++)
		free(pic->mb[i]);
	free(pic->mb);
	free(pic->ibuf);
	free(pic->dbuf);
}

// sometimes changes pic->prev->image
int load_picture_image(PICTURE *pic, IMAGE *image)
{
	if (pic->image.src_format != image->src_format) {
		finit_picture(pic);
		if (pic->image.width != image->width || pic->image.height != image->height) {
			if (pic->image.width)
				yuv_finit_image(&pic->image);
			pic->image.width = image->width;
			pic->image.height = image->height;
		}
		pic->image.src_format = image->src_format;
		yuv_init_image(&pic->image);


		if (init_picture(pic) == -1)
			return -1;

		yuv_copy_image(&pic->image, image);

		load_picture_image(pic->prev, image); // to keep the same format
		return 1;
	}

	yuv_copy_image(&pic->image, image);
	return 0;
}

