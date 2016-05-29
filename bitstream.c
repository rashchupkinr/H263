#include "codec.h"
#include "cif.h"
#include "misc.h"
#include "vlc.h"
#include "indices.h"
#include "bitstream.h"
#include "scan.h"
#include "math.h"

extern yuv_param yuv_param_table[];

int read_h263_picture(int pictpos, PICTURE *pic)
{
    int pictsize=0;
    int i;
    yuv_param *pictparam;

    pictsize+=getheader(pic);
    
    if (!pic->image.lum) {
        yuv_init_image(&pic->image);
        init_picture(pic);
        pic->prev->image.src_format = pic->image.src_format;
        yuv_init_image(&pic->prev->image);
        init_picture(pic->prev);
    }

    return pictsize;
}

void write_h263_picture(PICTURE *pic)
{
	int i;
	trace("\n\nPICTURE\n");
	pic->stat.header = bitcount();
	alignbits();
	trace("PSC:\t");		putbits(22, 0x000020);
	trace("TR:\t");		    putbits(8, pic->tr);
	trace("pic->image.src_format:\t");		putbits(8, 0x87);
	
	trace("UFEP:\t");		putbits(3, 1);
	putbits(3, pic->image.src_format);
	putbits(1, 0);
	putbits(1, annex & ANNEX_UMV);
	putbits(1, annex & ANNEX_SAC);
	putbits(1, annex & ANNEX_ADV_PRED);
	putbits(1, annex & ANNEX_ADV_INTRA);
	putbits(1, annex & ANNEX_DEB_FILT);
	putbits(1, annex & ANNEX_SLICE);
	putbits(1, annex & ANNEX_REF_PIC_SEL);
	putbits(1, annex & ANNEX_INDEP_SEG_DEC);
	putbits(1, annex & ANNEX_ALT_INTERVLC);
	putbits(1, annex & ANNEX_MQUANT);
	putbits(4, 0x08);
	
	trace("MPPTYPE:\n");
	putbits(3, pic->ptype);
	putbits(1, annex & ANNEX_REF_PIC_RESAMPLE);
	putbits(1, annex & ANNEX_RRU);
	putbits(1, pic->rtype);
	putbits(3, 1);
	
	
	trace("CPM:\t"); 		    putbits(1, 0);
	trace("PQUANT:\t"); 		putbits(5, pic->pquant);
	if (pic->ptype == PCT_PB) {
		trace("TRB:\t"); 		putbits(3, pic->trb);
		trace("DBQUANT:\t"); 	putbits(2, pic->dbquant);
	}	
	if (pic->npsupp) {
		if (pic->psupp)
		for (i=0; i<pic->npsupp; i++) {
			trace("PEI:\t");		putbits(1, 1);
			trace("PSUPP:\t");		putbits(8, pic->psupp[i]);
		}
	} else {
		trace("PEI:\t");			putbits(1, 0);
	} 
	pic->stat.header = bitcount() - pic->stat.header;
}

/*
void write_h263_gob(GOB *gob)
{
	int i;
	trace("\nGOB %03d\n", gob - gob->pic->gob);
	if (!gob || !gob->pic) {
		trace("write_h263_gob: can't put %p\n", gob);
		return;
	}	
	
	if (gob->pic->gob != gob) {
		alignbits();
		trace("GBSC:\t");	putbits(17, 0x0001);
		trace("GN:\t");		putbits(5, gob->gn);
//		trace("GSBI:\t");		putbits(2, 1);
		trace("GFID:\t");	putbits(2, gob->gfid);
		trace("GQUANT:\t");	putbits(5, gob->gquant);
	}
}*/

void prepare_mv(int *v, int pv)
{
	if (annex & ANNEX_UMV) {
		if (*v<-32)
			*v += 64;
		else if (*v>31)
			*v -= 64;
	} else {
		if (pv<-31 && *v<-63)
			*v += 64;
		else if (pv>32 && *v>63)
			*v -= 64;
	}
	if (*v<0)
		*v += 64;
	if (*v>=32)
		*v = *v-64;
}

void write_h263_mv(MACROBLOCK* mb, int k)
{
	int dx, dy;
	dx = mb->mv[k].hx - mb->predicted_mv[k].hx;
	dy = mb->mv[k].hy - mb->predicted_mv[k].hy;

	prepare_mv(&dx, mb->predicted_mv[k].hx);
	prepare_mv(&dy, mb->predicted_mv[k].hy);

	putbits(mvtab[abs(dx)].len, mvtab[abs(dx)].code);
	if (dx) putbits(1, dx<0);
	putbits(mvtab[abs(dy)].len, mvtab[abs(dy)].code);
	if (dy) putbits(1, dy<0);
    trace("pmv %d %d\n",mb->predicted_mv[k].hx,mb->predicted_mv[k].hy);
    trace("mv %d %d\n",mb->mv[0].hx,mb->mv[0].hy);
}

void write_h263_mb(MACROBLOCK *mb)
{
	int idx, i, bf, m;
	int tmpcnt;
	trace("\nput MACROBLOCK[%03d][%03d]\n", mb->my, mb->mx);

	init_mb_cbp(mb);

	if (mb->type != MODE_INTRA && mb->type != MODE_INTRA_Q &&
			(mb->cbpc==0) && (mb->cbpy==0) && (mb->mv[0].hx==0) && (mb->mv[0].hy==0))
		mb->cod = 1;
	else
		mb->cod = 0;
	
	bf= bitstream_field_mb[mb->pic->ptype][mb->type];
	if (bf & BITSTREAM_MB_COD) {
		trace("COD:\t");
		if (mb->cod) {
			putbits(1, 1);
			return;
		} else
			putbits(1, 0);
		mb->pic->stat.cod++;
	}

	if (bf & BITSTREAM_MB_MCBPC) {
		trace("MCBPC:\t");
		tmpcnt = bitcount();
		if (mb->pic->ptype == PCT_INTRA) {
			idx = (mb->cbpc << 2) | (mb->type >> 1);
			putbits(cbpcm_intra_tab[idx].len, cbpcm_intra_tab[idx].code);
		} else {
			idx = (mb->cbpc << 3) | mb->type;
			putbits(cbpcm_inter_tab[idx].len, cbpcm_inter_tab[idx].code);
		}
		mb->pic->stat.mcbpc += bitcount() - tmpcnt;;
	}

	if (bf & BITSTREAM_MB_CBPY) {
		trace("CBPY:\t");
		idx = mb->cbpy;
		if (mb->type == PCT_INTRA)
			idx = 15-idx;
		putbits(cbpy_tab[idx].len, cbpy_tab[idx].code);

		mb->pic->stat.cbpy+=cbpy_tab[idx].len;
	}

	if (bf & BITSTREAM_MB_DQUANT) {
		trace("DQUANT:\t");	putbits(2, 2);

		mb->pic->stat.dquant+=2;
	}

	tmpcnt = bitcount();
	if (bf & BITSTREAM_MB_MVD) {
		trace("MVD:\t"); 
		write_h263_mv(mb, 0);
	}
	mb->pic->stat.vectors += bitcount()-tmpcnt;

	tmpcnt = bitcount();
	for (i=0; i<4; i++)
		write_h263_block(mb, i, mb->cbpy & (1<<(3-i)));
	mb->pic->stat.coeff_y += bitcount()-tmpcnt;

	tmpcnt = bitcount();
	for (	 ; i<6; i++)
		write_h263_block(mb, i, mb->cbpc & (1<<(5-i)));
	mb->pic->stat.coeff_c += bitcount()-tmpcnt;
}

void init_block_cbp(MACROBLOCK *mb, int bnum)
{
	int i, j, z=1;

	i = 0;
	if ((mb->type == MODE_INTRA || mb->type == MODE_INTRA_Q) && 
			mb->pic->ptype != PCT_B)
		i++;

	for (; i<BLOCK_LEN; i++)
		if (mb->data[bnum][i]) {
			z = 0;
			break;
		}
	
	if (!z) {
		if (bnum < 4)
			mb->cbpy |= (1 << (3-bnum));
		else
			mb->cbpc |= (1 << (5-bnum));
	}
}

void init_mb_cbp(MACROBLOCK *mb)
{
	int k;
	for (k=0; k<6; k++)
		init_block_cbp(mb, k);
}

void scan(MACROBLOCK *mb , int k, int *buf)
{
	int i, j;
	const int (*scan_tab)[BLOCK_SIZE][BLOCK_SIZE];

	scan_tab = (const int (*)[8][8])scan_zigzag;
	for (i=0; i<BLOCK_SIZE; i++)
		for (j=0; j<BLOCK_SIZE; j++)
			buf[(*scan_tab)[i][j]] = mb->data[k][i*BLOCK_SIZE+j];
}

double log2(double v)
{
	return log(v)/log(2);
}

void write_h263_block(MACROBLOCK *mb, int k, int write_coef)
{
	int i, j, run, suc=0, last=0, level, _level;
	VLCtable const *vlc;
	int buf[BLOCK_LEN];
	trace("block %d\n", k);
//trace_block(mb->data[k]);
	i = 0;
	if ((mb->type == MODE_INTRA || mb->type == MODE_INTRA_Q) && 
			mb->pic->ptype != PCT_B) {
		trace("DC:\t"); putbits(8, intradctab[(unsigned)mb->data[k][0]-1]);
		i++;
	}

	if (!write_coef) return;
	
	scan(mb, k, buf);

	while (buf[i]==0 && i<BLOCK_LEN) i++, suc++;
	while (!last) {
		run = suc;
		suc = 0;

		level = buf[i];
		i++;
		while (buf[i]==0 && i<BLOCK_LEN) i++, suc++;
		if (level == 0)
			continue;
		if (i >= BLOCK_LEN)
			last = 1;
		trace("last %02d\trun %02d\tlevel %02d\n", last, run, level);

		_level = level>0 ? level : -level;
		vlc = 0;
		if (last == 0) {
			if (run<2 && _level<=12)
				vlc = &coeff_tab0[run][_level-1];
			else if (run>=2 && run<=26 && _level<=4)
				vlc = &coeff_tab1[run-2][_level-1];
		} else {
			if (run<2 && _level<4)
				vlc = &coeff_tab2[run][_level-1];
			else if (run>=2 && run<42)
				vlc = &coeff_tab3[run-2];
		}
		if (vlc && vlc->len) {
			putbits(vlc->len, vlc->code);
			putbits(1, level < 0);
		}
		else {
			putbits(7, ESCAPE);
			putbits(1, lasttab[last]);
			putbits(6, runtab[run]);
//			if (level > 253) level = 253;
			if (level == -128 || level == 0) level++;
			putbits(8, level);//leveltab[level+127+(level>0)]);
		}
	}
}

