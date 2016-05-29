// based on tmn codec source file gethdr.c from tmndecode:
/************************************************************************
 *
 *  gethdr.c, header decoding for tmndecode (H.263 decoder)
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
 */


#include <stdio.h>
#include <stdlib.h>

#include "codec.h"


void getpicturehdr(PICTURE *pic, int pictpos);
void startcode();
extern int getbitcnt;
/*
 * decode headers from one input stream
 * until an End of Sequence or picture start code
 * is found
 */

int getheader(PICTURE *pic, int pictpos)
{
  unsigned int code, gob;
  int hdrsize;
  int i;
  int bc=getbitcnt;
  /* look for startcode */
  startcode();
#define PSC_LENGTH 17
  code = getbits(PSC_LENGTH);
  gob = getbits(5);
//  if (gob == SE_CODE) 
  if (gob !=0){ 
      printf("Error reading gob\n");
    return 0;
    }
  if (gob == 0) {
    getpicturehdr(pic, pictpos);
//    if (syntax_arith_coding)        /* reset decoder after receiving */
  //    decoder_reset();	        /* fixed length PSC string */
  }
//  return gob + 1;
  hdrsize = getbitcnt-bc;
  return hdrsize;
}

/* align to start of next startcode */

void startcode()
{
  /* search for new picture start code */
//  while (showbits(PSC_LENGTH)!=1l) 
  while (showbits(PSC_LENGTH)!=1) 
         flushbits(1);
}

/* decode picture header */

void getpicturehdr(PICTURE *pic, int pictpos)
{
  int pos, pei, tmp;
int temp_ref;
int  prev_temp_ref;
int  trd;
int pt;
  int pict_type;
  int mv_outside_frame;
  int long_vectors;
  int syntax_arith_coding;
  int adv_pred_mode;

  pos = pictpos;
temp_ref=0;
prev_temp_ref = temp_ref;
temp_ref = getbits(8);
trd = temp_ref - prev_temp_ref;

  if (trd < 0)
    trd += 256;
/*
  tmp = getbits(1); 
  if (!tmp)
    if (!quiet)
      printf("warning: spare in picture header should be \"1\"\n");
  tmp = getbits(1);
  if (tmp)
    if (!quiet)
      printf("warning: H.261 distinction bit should be \"0\"\n");
  tmp = getbits(1);
  if (tmp) {
    if (!quiet)
      printf("error: split-screen not supported in this version\n");
    exit (-1);
  }
  tmp = getbits(1);
  if (tmp)
    if (!quiet)
      printf("warning: document camera indicator not supported in this version\n");

  tmp = getbits(1);
  if (tmp)
    if (!quiet)
      printf("warning: frozen picture not supported in this version\n");
*/
 pt = getbits(8);
if (pt!=0x87)
{
    printf("Error parsing header\n");
    exit(1);
}
getbits(3);
  pic->image.src_format = getbits(3);
  pict_type = getbits(1);//-
  mv_outside_frame = getbits(1);//
  long_vectors = (mv_outside_frame ? 1 : 0);//
  syntax_arith_coding = getbits(1);//
  adv_pred_mode = getbits(1);//
  mv_outside_frame = (adv_pred_mode ? 1 : mv_outside_frame);
//  int pb_frame = getbits(1);//
getbits(11);
    pic->ptype=getbits(3);
getbits(2);
    pic->rtype=getbits(1);
    getbits(3);
  tmp = getbits(1);//
  if (tmp) {
      printf("error: CPM not supported in this version\n");
    exit(-1);
  }
  pic->pquant = getbits(5);
  if (pic->ptype == PCT_PB) {
      getbits(3, pic->trb);
      getbits(2, pic->dbquant);
  }
//npsupp
  pei=getbits(1);
  if (pei) {
      printf("merror: pei not supported\n");
      exit(0);
  } 

/*
  if (pb_frame) {
    trb = getbits(3);
    bquant = getbits(2);
  }
  else {
    trb = 0;
  }

#ifdef USE_TIME
  if (framerate > 0 && trd > 0)
    doframerate(0);
#endif	

  pei = getbits(1);
pspare:
  if (pei) {
    getbits(8)
    pei = getbits(1);
    if (pei) goto pspare;
  }
*/
}


