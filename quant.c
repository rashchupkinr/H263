#include "misc.h"
#include "codec.h"
#include "cif.h"

int quant(MACROBLOCK *mb, int k, int q)
{
	int i;
	int *data;
	
	data = mb->data[k];
	if (!q) {
		trace("quant(%p, 0)\n", data, q);
		return -1;
	}

	if (mb->type == MODE_INTRA || mb->type == MODE_INTRA_Q) {

		data[0] = sign(data[0])*mmax(0, mmin (254, (abs(data[0])+4)/8));
		for (i=1; i<BLOCK_LEN; i++)
			data[i] = sign(data[i])*mmin(127, mmax(-127, abs(data[i])/(2*q)));
	} else
		for (i=0; i<BLOCK_LEN; i++)
			data[i] = sign(data[i])*mmin(127, mmax(-127, (abs(data[i])-q/2)/(2*q)));
	return 0;
}

int dequant(MACROBLOCK *mb, int k, int q)
{
	int i;
	int *data;
	int bnum;
	data = mb->data[k];
	if (!data) return -1;
	if (!q) {
		trace("dequant(%p, 0)\n", data, q);
		return -1;
	}

	i = 0;
	if (mb->type == MODE_INTRA || mb->type == MODE_INTRA_Q) {
		data[0] = data[0]*8;
		i++;
	}
	if (q & 1) {
		for (; i<BLOCK_LEN; i++)
			if (data[i])
	data[i] = sign(data[i]) * q*(2*abs(data[i])+1);
	} else {
		for (; i<BLOCK_LEN; i++)
			if (data[i])
	data[i] = sign(data[i]) * (q*(2*abs(data[i])+1)-1);
	}
	return 0;
}

