#include "misc.h"
#include "codec.h"
#include "cif.h"

int frame_I_encode_MB(MACROBLOCK *mb)
{
	int k;
	mb->type = MODE_INTRA;

	for (k=0; k<6; k++) {
		dct(mb->data[k]);
		quant(mb, k, mb->pic->pquant);
	}
	return 0;
}

int frame_I_decode_MB(MACROBLOCK *mb)
{
	int k;
	for (k=0; k<6; k++) {
		dequant(mb, k, mb->pic->pquant);
		clip_buf(mb->data[k], BLOCK_SIZE, -2048, 2047, 1);
		
		idct(mb->data[k]);
		clip_buf(mb->data[k], BLOCK_SIZE, 0, 255, 0);
	}

	return 0;
}

