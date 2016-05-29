#include "misc.h"
#include "codec.h"
#include "cif.h"

extern int Decode;

int frame_P_encode_MB(MACROBLOCK *mb)
{
	int k;

	mb->type = MODE_INTER;

	for (k=0; k<6; k++) {
		motion_estimate_block(mb, k);

		if (mb->type != MODE_INTRA && mb->type != MODE_INTRA_Q)
			diff_block_motion(mb, k);
		dct(mb->data[k]);
		quant(mb, k, mb->pic->pquant);
	}
	return 0;
}

int frame_P_decode_MB(MACROBLOCK *mb)
{
	int k;

	for (k=0; k<6; k++) {

		dequant(mb, k, mb->pic->pquant);
		clip_buf(mb->data[k], BLOCK_SIZE, -2048, 2047, 0);

		idct(mb->data[k]);
		if (mb->type != MODE_INTRA && mb->type != MODE_INTRA_Q){
			add_block_motion(mb, k);
        }
		clip_buf(mb->data[k], BLOCK_SIZE, 0, 255, 0);
	}

	return 0;
}

