// based on tmn codec source file getbits.c from tmndecode:
/************************************************************************
 *
 *  getbits.c, bit level routines for tmndecode (H.263 decoder)
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


#include <stdlib.h>

#include "codec.h"
#ifdef WIN32
#include <io.h>
#endif
unsigned int getbits(int n);
void flushbits(int n);
static unsigned char inbfr[16];
static unsigned char *rdptr;
static unsigned char rdbfr[2051];
FILE *infile;
 int incnt, getbitcnt;


/* to mask the n least significant bits of an integer */

static unsigned int msk[33] =
{
  0x00000000,0x00000001,0x00000003,0x00000007,
  0x0000000f,0x0000001f,0x0000003f,0x0000007f,
  0x000000ff,0x000001ff,0x000003ff,0x000007ff,
  0x00000fff,0x00001fff,0x00003fff,0x00007fff,
  0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
  0x000fffff,0x001fffff,0x003fffff,0x007fffff,
  0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
  0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
  0xffffffff
};


/* initialize buffer, call once before first getbits or showbits */

void initgetbits(CONTEXT context)
{
  incnt = 0;
  rdptr = rdbfr + 2048;
  getbitcnt = 0;
  infile = context.i_stream;
}

void fillbfr()
{
  int l;

  inbfr[0] = inbfr[8];
  inbfr[1] = inbfr[9];
  inbfr[2] = inbfr[10];
  inbfr[3] = inbfr[11];

  if (rdptr>=rdbfr+2048)
  {
//    l = read(infile,rdbfr,2048);
    l = fread(rdbfr,1,2048,infile);
    rdptr = rdbfr;
    if (l<2048)
    {
      if (l<0)
        l = 0;

      while (l<2048)   /* Add recognizable sequence end code */
      {
        rdbfr[l++] = 0;
        rdbfr[l++] = 0;
#define SE_CODE 31
        rdbfr[l++] = (1<<7) | (SE_CODE<<2);
      }
    }
  }

  for (l=0; l<8; l++)
    inbfr[l+4] = rdptr[l];

  rdptr+= 8;
  incnt+= 64;
}


/* return next n bits (right adjusted) without advancing */

unsigned int showbits(n)
int n;
{
  unsigned char *v;
  unsigned int b;
  int c;

  if (incnt<n)
    fillbfr();

  v = inbfr + ((96 - incnt)>>3);
  b = (v[0]<<24) | (v[1]<<16) | (v[2]<<8) | v[3];
  c = ((incnt-1) & 7) + 25;
  return (b>>(c-n)) & msk[n];
}


/* return next bit (could be made faster than getbits(1)) */

unsigned int getbits1()
{
  return getbits(1);
}


/* advance by n bits */

void flushbits(int n)
{

  getbitcnt+= n;
  incnt-= n;
  if (incnt < 0)
    fillbfr();
}


/* return next n bits (right adjusted) */

unsigned int getbits(int n)
{
  unsigned int l;

  l = showbits(n);
  flushbits(n);

  return l;
}
