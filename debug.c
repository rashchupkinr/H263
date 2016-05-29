#include "misc.h"

FILE *_t_stream;

void trace(char *fmt, ...)
{
	va_list ap;
	if (!_t_stream) return;

	va_start(ap,fmt);
	vfprintf(_t_stream, fmt, ap);
	va_end(ap);
//	fflush(_t_stream);
}	

void trace_block(int *block)
{
	int i, j;
	if (!_t_stream) return;
	if (!block)	trace("trace_block(0)\n");

	for (i=0; i<BLOCK_SIZE; i++) {
		for (j=0; j<BLOCK_SIZE; j++)
			trace("%4d ", (char)block[i*BLOCK_SIZE + j]);
		trace("\n");
	}
	fflush(_t_stream);
}

void trace_cblock(unsigned char *block, int size)
{
	int i, j;
	if (!_t_stream) return;
	if (!block)	trace("trace_block(0)\n");

	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			trace("%4d ", block[i*size + j]);
		trace("\n");
	}
	fflush(_t_stream);
}

void trace_iblock(int *block, int size)
{
	int i, j;
	if (!_t_stream) return;
	if (!block)	trace("trace_block(0)\n");

	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			trace("%4d ", block[i*size + j]);
		trace("\n");
	}
	fflush(_t_stream);
}

void trace_pic_mv(PICTURE *pic)
{
	MACROBLOCK *mb;
	int i, j, sad = 0, nsad = 0, motest_sp = 0;
	fprintf(stderr, "MV:\n");

	for (i=0; i<pic->nmy; i++) {
		for (j=0; j<pic->nmx; j++) {
			mb = &pic->mb[i][j];
			if (mb->type != MODE_INTRA && mb->type!=MODE_INTRA_Q) {
//				fprintf(stderr, "%2d,%2d	", mb->mv[0].hx, mb->mv[0].hy);
	nsad++;
	sad += mb->sad[0];
	motest_sp += mb->motest_sp;
			} else
;//	fprintf(stderr, " .,.	 ");
		}
//		fprintf(stderr, "\n");
	}
	if (nsad) {
		fprintf(stderr, "avg SAD == %d\n", sad/nsad);
		fprintf(stderr, "avg SP == %d\n", motest_sp/nsad);
	}
}

void trace_stat(PCT_STAT *stat)
{
	trace("Coeff_Y: %d\nCoeff_C: %d\nVectors: %d\nCBPY\t: %d\nMCBPC\t: %d\nMODB\t: %d\nCBPB\t: %d\nCOD\t: %d\nDQUANT\t: %d\nheader\t: %d\nTotal\t: %d\n",
	stat->coeff_y, stat->coeff_c, stat->vectors, stat->cbpy, stat->mcbpc, stat->modb, stat->cbpb, stat->cod, stat->dquant, stat->header, stat->header+stat->framesize);
}

