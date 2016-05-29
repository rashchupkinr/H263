// based on tmn codec source file getblk.c from tmndecode:
/************************************************************************
*
 *  getblk.c, DCT block decoding for tmndecode (H.263 decoder)
 *  Copyright (C) 1995, 1996  Telenor R&D, Norway
 *        Karl Olav Lillevold <Karl.Lillevold@nta.no>
 *  
 *  Contacts: 
 *  Karl Olav Lillevold               <Karl.Lillevold@nta.no>, or
 *  Robert Danielsen                  <Robert.Danielsen@nta.no>
 *
 *  Telenor Research and Development  http://www.nta.no/brukere/DVC/
 *  P.O.Box 83                        tel.:   +47 63 84 84 00
 *  N-2007 Kjeller, Norway            fax.:   +47 63 81 00 76
 *  
 ************************************************************************/

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any
 * license fee or royalty on an "as is" basis.  Telenor Research and
 * Development disclaims any and all warranties, whether express,
 * implied, or statuary, including any implied warranties or
 * merchantability or of fitness for a particular purpose.  In no
 * event shall the copyright-holder be liable for any incidental,
 * punitive, or consequential damages of any kind whatsoever arising
 * from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs
 * and user's customers, employees, agents, transferees, successors,
 * and assigns.
 *
 * Telenor Research and Development does not represent or warrant that
 * the programs furnished hereunder are free of infringement of any
 * third-party patents.
 *
 * Commercial implementations of H.263, including shareware, are
 * subject to royalty fees to patent holders.  Many of these patents
 * are general enough such that they are unavoidable regardless of
 * implementation design.
 * */


/*
 * based on mpeg2decode, (C) 1994, MPEG Software Simulation Group
 * and mpeg2play, (C) 1994 Stefan Eckart
 *                         <stefan@lis.e-technik.tu-muenchen.de>
 *
 */


#include <stdio.h>
#include <stdlib.h>

#include "misc.h"
#include "codec.h"

#define INDICES
#include "indices.h"

#define SACTABLES
#include "sactbls.h"
#include "bitstream.h"
unsigned char zig_zag_scan[64]=
{
  0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
  12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
  35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
  58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};
void readblock(MACROBLOCK *mb, int comp, int coef_present);


typedef struct {
  char run, level, len;
} DCTtab;
typedef struct {
  int val, len;
} VLCtabI;
typedef struct {
  int val, run, sign;
} RunCoef;
/* local prototypes */
RunCoef vlc_word_decode (int symbol_word, int *last); 
RunCoef Decode_Escape_Char (int intra, int *last);
int DecodeTCoef (int position, int intra);
static int quiet=0, fault=0;
int trd,trb,bscan,bquant;
int bscan_tab[]= {2,4,6,8};
int bquant_tab[]= {5,6,7,8};




extern VLCtabI DCT3Dtab0[],DCT3Dtab1[],DCT3Dtab2[];
#define ESCAPE 7167
#define ESCAPE_INDEX 102
extern int getbitcnt;

int block[64];


int motion_decode(int vec, int pmv)
{
  if (vec > 31) vec -= 64;
  vec += pmv;
  if (!(annex & ANNEX_UMV)) {
    if (vec > 31)
      vec -= 64;
    if (vec < -32)
      vec += 64;
  }
  else {
    if (pmv < -31 && vec < -63)
      vec += 64;
    if (pmv > 32 && vec > 63)
      vec -= 64;
  }
  return vec;
}

int read_h263_mb(MACROBLOCK *mb)
{
    int bc=getbitcnt;
    int k;
    int bf;
    int cbpy;
    int cbpc;
	int i;
    for (k=0;k<6;k++) {
        memset(mb->data[k],0,BLOCK_LEN*sizeof(int));
        mb->mv[k].hx = mb->mv[k].hy = 0;
        predict_mv(mb,k);
    }

    if (mb->pic->ptype != PCT_INTRA) {
        mb->cod=getbits(1);
        trace("COD %x\n",mb->cod);
        mb->type=MODE_INTER;
        if (mb->cod==1) {
			return getbitcnt-bc;
		}
    }
		if (mb->my==11&&mb->mx==18){
			if (mb->pic->tr==2)
				mb->my=11;
		}
	if (mb->my==12&&mb->mx==18){
			if (mb->pic->tr==2)
				mb->my=12;
		}
    
    if (mb->pic->ptype == PCT_INTRA) {
        mb->cbpc = getMCBPCintra();
    } else {
        mb->cbpc = getMCBPC();
    }
     mb->type = mb->cbpc & 0x7;
    if (mb->cbpc==255)
    {
    }

//    mb->cbpc
//    mb->type

//    mb->modb = getMODB();
    bf= bitstream_field_mb[mb->pic->ptype][mb->type];
    cbpy=0;
    if (bf & BITSTREAM_MB_CBPY) {
        mb->cbpy = getCBPY();
		if (mb->type==MODE_INTRA)
            cbpy = ~mb->cbpy;
        else
            cbpy = mb->cbpy;
    }
    if (bf & BITSTREAM_MB_DQUANT) {
        mb->dquant = getbits(2);
    }
    if (bf & BITSTREAM_MB_MVD) {
        int k, mvx, mvy;
        mvx = getTMNMV();
        mvy = getTMNMV();
        mb->mv[0].hx = mvx = motion_decode(mvx, mb->predicted_mv[0].hx);
        mb->mv[0].hy = mvy = motion_decode(mvy, mb->predicted_mv[0].hy);
        for (k=1;k<6;k++) {
            mb->mv[k].hx=mvx;
            mb->mv[k].hy=mvy;
            if (k>3) {
                if (abs(mb->mv[k].hx)%4==1||abs(mb->mv[k].hx)==1)
                    mb->mv[k].hx+=sign(mb->mv[k].hx);
                if (abs(mb->mv[k].hy)%4==1||abs(mb->mv[k].hy)==1)
                    mb->mv[k].hy+=sign(mb->mv[k].hy);
                mb->mv[k].hx/=2;
                mb->mv[k].hy/=2;
            }
        }
    trace("mv %d %d: %d %d\n",mb->my,mb->mx,mb->mv[0].hx,mb->mv[0].hy);
    }
    cbpc=mb->cbpc>>4;
    for (i=0; i<4; i++) {
            readblock(mb, i, (cbpy&(0x1<<(3-i))));
    } 
    for (   ; i<6; i++) {
            readblock(mb, i, (cbpc&(0x1<<(1-(i-4)))));
    }
        
    return getbitcnt-bc;
}


void readblock(MACROBLOCK *mb, int comp, int coef_present)
{
  int val, i, j, sign;
  unsigned int code;
  VLCtabI *tab;
  int run, last, level, QP;
  short *qval;
  int value;
if (mb->type == MODE_INTRA){
  int dc = getbits(8);
  for (i=0;i<254;i++)
      if (intradctab[i]==dc) {
          if (dc==255)
              dc=127;
          mb->data[comp][0]=dc;
          break;
      }
  trace("DC ", mb->data[comp][0]);
  printbits(dc,8,8);
  trace("\n");
}
  if (!coef_present)
      return;
  if (mb->pic->tr==2&&mb->my==13&&mb->mx==1)
	  mb->mx=1;
  /* decode AC coefficients */
  for (i=!(mb->type!=MODE_INTRA&&mb->type!=MODE_INTRA_Q); ; i++) {
    code = showbits(12);
    if (code>=512)
      tab = &DCT3Dtab0[(code>>5)-16];
    else if (code>=128)
      tab = &DCT3Dtab1[(code>>2)-32];
    else if (code>=8)
      tab = &DCT3Dtab2[(code>>0)-8];
    else {
      if (!quiet) {
        trace("invalid Huffman code in readblock()\n");
        fprintf(stderr,"invalid Huffman code in readblock()\n");
      }
      fault = 1;
      return;
    }

    flushbits(tab->len);

    run = (tab->val >> 4) & 255;
    level = tab->val & 15;
    last = (tab->val >> 12) & 1;

    if (trace) {
     trace(" (");
      printbits(code,12,tab->len);
    }

   if (tab->val==ESCAPE) { /* escape */
      if (trace) {
        putchar(' ');
        printbits(showbits(1),1,1);
      }
     last = getbits1();

      if (trace) {
        putchar(' ');
        printbits(showbits(6),6,6);
      }
      i += run = getbits(6);
      if (trace) {
        putchar(' ');
        printbits(showbits(8),8,8);
      }
      level = getbits(8);
 
      if (sign = (level>=128))
        val = 256 - level;
      else 
        val = level;
    }
    else {
      i+= run;
      val = level;
      sign = getbits(1);
      if (trace)
       trace("%d",sign);
    }

    if (i >= 64)
    {
      if (!quiet)  {
        fprintf(stderr,"DCT coeff index (i) out of bounds\n");
        trace("DCT coeff index (i=%d) out of bounds(run %d level%d)\n", i,run,level);
      }
      fault = 1;
      return;
    }

    if (trace) {
     trace("): %d/%d\t",run,sign ? -val : val);
    }
    

    value = sign?-val:val;
    j = zig_zag_scan[i];
//    trace("%d_%d_%d ",i,j,value);
    mb->data[comp][j]=value;
    if (last) {
      if (trace)
       trace("last\n");
      return;
    }
  }
}


