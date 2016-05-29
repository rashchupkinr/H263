#include "codec.h"
#include "misc.h"

#define hpel(x) ((x)&1 ? sign(x) : 0)

int diff_block_motion(MACROBLOCK *mb, int k)
{
	int hx, hy;
	hx = 2*get_block_x(mb, k) + mb->mv[k].hx;
	hy = 2*get_block_y(mb, k) + mb->mv[k].hy;
	if (!hpel(hx) && !hpel(hy))
		return diff_buf(mb->pic->prev, mb->data[k], BLOCK_SIZE, k, hx, hy);
	else {
		load_buf_ipol(mb->pic->prev, mb->pic->prev->dbuf, BLOCK_SIZE, k, hx, hy);
		return diff_buf_ipol(mb->data[k], mb->pic->prev->dbuf, BLOCK_SIZE,
					hpel(hx), hpel(hy));
	}
}

int add_block_motion(MACROBLOCK *mb, int k)
{
	int hx, hy;
	hx = 2*get_block_x(mb, k) + mb->mv[k].hx;
	hy = 2*get_block_y(mb, k) + mb->mv[k].hy;
	if (!hpel(hx) && !hpel(hy))
		return add_buf(mb->pic->prev, mb->data[k], BLOCK_SIZE, k, hx, hy);
	else {
		load_buf_ipol(mb->pic->prev, mb->pic->prev->dbuf, BLOCK_SIZE, k, hx, hy);
		return add_buf_ipol(mb->data[k], mb->pic->prev->dbuf, BLOCK_SIZE,
					hpel(hx), hpel(hy));
	}
}

int do_predict_mv(MACROBLOCK *mb, int k)
{
	MV mvnull;
	MV *mv1 = 0, *mv2 = 0, *mv3 = 0;
	mvnull.hx = mvnull.hy = 0;
	if (mb->mx == 0)
		mv1 = &mvnull;
	else
		mv1 = &mb->pic->mb[mb->my][mb->mx-1].mv[k];

	if (mb->mx == mb->pic->nmx-1)
		mv3 = &mvnull;
	if (mb->my == 0)
		mv2 = mv3 = mv1;
	if (!mv2)
		mv2 = &mb->pic->mb[mb->my-1][mb->mx].mv[k];
	if (!mv3)
		mv3 = &mb->pic->mb[mb->my-1][mb->mx+1].mv[k];
	mb->predicted_mv[k].hx = mv1->hx + mv2->hx + mv3->hx -
		mmin(mv1->hx, mmin(mv2->hx, mv3->hx)) -
    	mmax(mv1->hx, mmax(mv2->hx, mv3->hx));
	mb->predicted_mv[k].hy = mv1->hy + mv2->hy + mv3->hy -
		mmin(mv1->hy, mmin(mv2->hy, mv3->hy)) -
	    mmax(mv1->hy, mmax(mv2->hy, mv3->hy));
	return 0;
}

void predict_mv(MACROBLOCK *mb, int k)
{
	if (k == 0)
		do_predict_mv(mb, k);
	else {
		mb->predicted_mv[k].hx = mb->predicted_mv[0].hx;
		mb->predicted_mv[k].hy = mb->predicted_mv[0].hy;
//		if (k > 3) {
//			mb->predicted_mv[k].hx /= 2;
//			mb->predicted_mv[k].hy /= 2;
//		}
	}
}

int select_inter_mb_type(MACROBLOCK *mb)
{
	int k, i, j;
	int A=0, med=0;

	for (k=0; k<4; k++)
		for (i=0; i<BLOCK_LEN; i++)
			med += mb->data[k][i];
	med /= 4*BLOCK_LEN;
	for (k=0; k<4; k++)
		for (i=0; i<BLOCK_LEN; i++)
			A += abs(mb->data[k][i] - med);
 
	if (A < mb->sad[0] - 500) {
//	if (A < mb->sad[0] - 1000) {
//fprintf(stderr,"%d %d intra\n", mb->mx, mb->my);
		mb->type = MODE_INTRA;
	} else
		mb->type = MODE_INTER;
	return 0;
}

int half_pel_search(MACROBLOCK *mb, int k)
{
	int sad;
	int vx, vy, best_vx = 0, best_vy = 0;
	int hx, hy;
	hx = 2*get_block_x(mb, k) + mb->mv[k].hx;
	hy = 2*get_block_y(mb, k) + mb->mv[k].hy;

	load_buf_ipol(mb->pic->prev, mb->pic->prev->dbuf, BSIZE(k), k, hx, hy);

	for (vx=-1; vx<=1 && mb->sad[k]>0; vx++)
		for (vy=-1; vy<=1 && mb->sad[k]>0; vy++) {
			if (mb_out_bound(mb, mb->mv[k].hx+vx, mb->mv[k].hy+vy))
				continue;
			sad = sad_buf_ipol(mb->pic->ibuf, mb->pic->prev->dbuf, BSIZE(k), vx, vy);
			if (vx==0 && vy==0)
				sad -= 100;

			if (sad < mb->sad[k]) {
				mb->sad[k] = sad;
				best_vx = vx;
				best_vy = vy;
			}
		}

	mb->mv[k].hx += best_vx;
	mb->mv[k].hy += best_vy;
//fprintf(stderr, "%d %d: %d %d: %d\n", mb->mx, mb->my, mb->mv[k].hx, mb->mv[k].hy, mb->sad[k]);
	return 0;
}

#define SAD_CACHE_SIZE 7
int sad_cache[2*SAD_CACHE_SIZE+1][2*SAD_CACHE_SIZE+1];
int sad_cache_hits;

void sad_cache_clear()
{
	int i, j;
	for (i=0;i<2*SAD_CACHE_SIZE+1;i++)
		for (j=0;j<2*SAD_CACHE_SIZE+1;j++)
			sad_cache[i][j] = INT_MAX;
	sad_cache_hits = 0;
}

int sad_cache_load(int vx, int vy)
{
	int sad;
	if (vx>=-SAD_CACHE_SIZE && vx<=SAD_CACHE_SIZE &&
			vy>=-SAD_CACHE_SIZE && vy<=SAD_CACHE_SIZE) {
		sad = sad_cache[vx+SAD_CACHE_SIZE][vy+SAD_CACHE_SIZE];
		if (sad != INT_MAX)
			sad_cache_hits++;
		return sad;
	}
	return INT_MAX;
}

int sad_cache_store(int vx, int vy, int sad)
{
	if (abs(vx)<=SAD_CACHE_SIZE && abs(vy)<=SAD_CACHE_SIZE)
		sad_cache[vx+SAD_CACHE_SIZE][vy+SAD_CACHE_SIZE] = sad;
	return 0;
}

int ms_test_mv(MACROBLOCK *mb, int k, int vx, int vy)
{
	int hx, hy;
	int sad = INT_MAX;

	sad = sad_cache_load(vx, vy);
	if (sad == INT_MAX) {
		if (mb_out_bound(mb, vx, vy))
			return -1;

		hx = 2*get_block_x(mb, k);
		hy = 2*get_block_y(mb, k);
		sad = sad_buf(mb->pic->prev, mb->pic->ibuf, BSIZE(k), k, hx+vx, hy+vy);
		mb->motest_sp++;

		sad_cache_store(vx, vy, sad);
	}
//	trace("tmv y%d x%d vy%d vx%d: %d \n",mb->my,mb->mx,vy,vx, sad);

	if (sad < mb->sad[k]) {
		mb->sad[k] = sad;
		mb->mv[k].hx = vx;
		mb->mv[k].hy = vy;
		return 1;
	}
	return 0;
}

int ms_fs(MACROBLOCK *mb, int k)
{
	int vx, vy;
	int sad;

	for (vx=-30; vx<31 && mb->sad[k]>0; vx+=2)
		for (vy=-30; vy<31 && mb->sad[k]>0; vy+=2)
			ms_test_mv(mb, k, vx, vy);

	return 0;
}

int ms_cbosa(MACROBLOCK *mb, int k)
{
	int vx = 0, _vx, vy = 0;

	ms_test_mv(mb, k, vx-4, vy);
	ms_test_mv(mb, k, vx-2, vy);
	ms_test_mv(mb, k, vx,	 vy);
	ms_test_mv(mb, k, vx+2, vy);
	ms_test_mv(mb, k, vx+4, vy);
	vx = mb->mv[k].hx;
	if (vx)
		ms_test_mv(mb, k, 6*sign(vx), 0);
	
	ms_test_mv(mb, k, vx, vy-4);
	ms_test_mv(mb, k, vx, vy-2);
	ms_test_mv(mb, k, vx, vy+2);
	ms_test_mv(mb, k, vx, vy+4);
	vy = mb->mv[k].hy;
	if (vy)
		ms_test_mv(mb, k, 0, 6*sign(vy));
	
	_vx = vx;
	if (vy) {
		ms_test_mv(mb, k, vx-2, vy);
		ms_test_mv(mb, k, vx+2, vy);
		vx = mb->mv[k].hx;
	}

	if (vx && vx!=_vx) {
		if (vy != 2)
			ms_test_mv(mb, k, vx, vy-2);
		if (vy != -2)
			ms_test_mv(mb, k, vx, vy+2);
		vy = mb->mv[k].hy;
	}
	
	ms_test_mv(mb, k, vx-1, vy);
	ms_test_mv(mb, k, vx+1, vy);
	vx = mb->mv[k].hx;

	ms_test_mv(mb, k, vx, vy-1);
	ms_test_mv(mb, k, vx, vy+1);
	vy = mb->mv[k].hy;
	
	return 0;
}

int ms_4ss(MACROBLOCK *mb, int k)
{
	int vx, vy, _vx, _vy;
	int sad;
	int i=0;

	for (vx=-4; vx<5; vx+=4)
		for (vy=-4; vy<5; vy+=4)
			ms_test_mv(mb, k, vx, vy);
	
	do {
		vx = mb->mv[k].hx;
		vy = mb->mv[k].hy;
		if (!(vx&vy)) {
			ms_test_mv(mb, k, vx, vy+4*sign(vy));
			ms_test_mv(mb, k, vx+4*sign(vx), vy);
			ms_test_mv(mb, k, vx+4*sign(vx), vy+4*sign(vy));
		} else if (!vx) {
			ms_test_mv(mb, k, vx+4*sign(vx), vy-4);
			ms_test_mv(mb, k, vx+4*sign(vx), vy);
			ms_test_mv(mb, k, vx+4*sign(vx), vy+4);
		} else if (!vy) {
			ms_test_mv(mb, k, vx-4, vy+4*sign(vy));
			ms_test_mv(mb, k, vx,	 vy+4*sign(vy));
			ms_test_mv(mb, k, vx+4, vy+4*sign(vy));
		}
		i++;
	} while (i<2 && (vx-mb->mv[k].hx || vy-mb->mv[k].hy));

	_vx = vx; _vy = vy;
	for (vx=_vx-2; vx<3; vx+=2)
		for (vy=_vy-2; vy<3; vy+=2)
			if (vx&vy)
	ms_test_mv(mb, k, vx, vy);
	return 0;
}

struct ms_hasa_data_struct ms_hasa_data = {
	MS_HASA_START_TH1,
	MS_HASA_START_TH2,
	0,0,0,0
};

int ms_hasa_new_frame(PICTURE *pic)
{
	if (!trace_mv || pic->ptype == PCT_INTRA)
		return 0;
	if (ms_hasa_data.N1)
		ms_hasa_data.TH1 = ms_hasa_data.E1/ms_hasa_data.N1;
	if (ms_hasa_data.N2)
		ms_hasa_data.TH2 = ms_hasa_data.E2/ms_hasa_data.N2;
	ms_hasa_data.N1 = ms_hasa_data.N2 = ms_hasa_data.E1 = ms_hasa_data.E2 = 0;
	fprintf(stderr, "TH %d %d\n", ms_hasa_data.TH1, ms_hasa_data.TH2);
	return 0;
}

int ms_hasa(MACROBLOCK *mb, int k)
{
	int vx, vy;
	int sad;
	
	ms_test_mv(mb, k, 0, 0);
	sad = mb->sad[k];

	if (sad > ms_hasa_data.TH2)
		ms_4ss(mb, k);
	else if (sad > ms_hasa_data.TH1)
		ms_cbosa(mb, k);

	vx = mb->mv[k].hx;
	vy = mb->mv[k].hy;
	sad = mb->sad[k];
	if (!(vx|vy)) {
		ms_hasa_data.N1++;
		ms_hasa_data.E1 += sad;
	} else if (abs(vx)<=3 && abs(vy)<=3) {
		ms_hasa_data.N2++;
		ms_hasa_data.E2 += sad;
	}
		
	return 0;
}

typedef int (*MOTEST_ALG)(MACROBLOCK *mb, int k);
MOTEST_ALG motest_alg_tbl[] = {
	ms_fs,
	ms_cbosa,
	ms_4ss,
	ms_hasa
};

int ms_new_frame(PICTURE *pic)
{
	if (pic->image.codec_context.motest_alg == MOTEST_HASA)
		return ms_hasa_new_frame(pic);
	return 0;
}

int do_motion_search(MACROBLOCK *mb, int k)
{
	int hx, hy;
	mb->mv[k].hx = mb->mv[k].hy;
	mb->sad[k] = INT_MAX;
	sad_cache_clear();
	mb->motest_sp = 0;

	hx = 2*get_block_x(mb, k);
	hy = 2*get_block_y(mb, k);
	if (load_buf(mb->pic, mb->pic->ibuf, BSIZE(k), k, hx, hy) < 0)
		return -1;

	motest_alg_tbl[mb->pic->image.codec_context.motest_alg](mb, k);

	select_inter_mb_type(mb);
	if (mb->type == MODE_INTRA || mb->type == MODE_INTRA_Q)
		mb->mv[k].hx = mb->mv[k].hy = 0;
	else
		half_pel_search(mb, k);

	return 0;
}

void motion_search(MACROBLOCK *mb, int k)
{
	if (k==0)
		do_motion_search(mb, k);
	else {
		mb->mv[k].hx = mb->mv[0].hx;
		mb->mv[k].hy = mb->mv[0].hy;
		if (k>3) {
			mb->mv[k].hx = (((int)mb->mv[k].hx & -4)/2) | (((int)mb->mv[k].hx & 3) > 0); 
			mb->mv[k].hy = (((int)mb->mv[k].hy & -4)/2) | (((int)mb->mv[k].hy & 3) > 0); 
		}
	}
}

void motion_estimate_block(MACROBLOCK *mb, int k)
{
	predict_mv(mb, k);
	motion_search(mb, k);
}

