// based on tmn codec source file getvlc.c from tmndecode:
/************************************************************************
 *
 *  getvlc.c, variable length decoding for tmndecode (H.263 decoder)
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
#include "misc.h"
#include "getvlc.h"
int fault = 0;
/* trace output */
void printbits(code,bits,len)
int code,bits,len;
{
  int i;
  for (i=0; i<len; i++)
   trace("%d",(code>>(bits-1-i))&1);
}

int getTMNMV()
{
  int code;

  if (trace)
   trace("motion_code (");

  if (getbits1())
  {
    if (trace)
     trace("1): 0\n");
    return 0;
  }

  if ((code = showbits(12))>=512)
  {
    code = (code>>8) - 2;
    flushbits(TMNMVtab0[code].len);

    if (trace)
    {
     trace("0");
      printbits(code+2,4,TMNMVtab0[code].len);
     trace("): %d\n", TMNMVtab0[code].val);
    }

    return TMNMVtab0[code].val;
  }

  if (code>=128)
  {
    code = (code>>2) -32;
    flushbits(TMNMVtab1[code].len);

    if (trace)
    {
     trace("0");
      printbits(code+32,10,TMNMVtab1[code].len);
     trace("): %d\n",TMNMVtab1[code].val);
    }

    return TMNMVtab1[code].val;
  }

  if ((code-=5)<0)
  {
      trace("Invalid motion_vector code\n");
      fprintf(stderr,"Invalid motion_vector code\n");
    fault=1;
    return 0;
  }

  flushbits(TMNMVtab2[code].len);

  if (trace)
  {
   trace("0");
    printbits(code+5,12,TMNMVtab2[code].len);
   trace("): %d\n",TMNMVtab2[code].val);
  }

  return TMNMVtab2[code].val;
}


int getMCBPC()
{
  int code;

  if (trace)
   trace("MCBPC (");

  code = showbits(13);

  if (code >> 4 == 1) {
    /* macroblock stuffing */
    if (trace)
     trace("000000001): stuffing\n");
    flushbits(9);
    return 255;
  }

  if (code == 0) {
      trace("Invalid MCBPC code\n");
      fprintf(stderr,"Invalid MCBPC code\n");
    fault = 1;
    return 0;
  }
   if (code >= 4096)
  {
    flushbits (1);
//    if (trace)
//      fprintf (trace_file, "1): %d\n", 0);
    return 0;
  }
  if (code >= 16)
  {
    flushbits (MCBPCtab0[code >> 4].len);
    if (trace)
    {
      printbits (code >> 4, 9, MCBPCtab0[code >> 4].len);
//      fprintf (trace_file, "): %d\n", MCBPCtab0[code >> 4].val);
    }
    return MCBPCtab0[code >> 4].val;
  } else
  {
    flushbits (MCBPCtab1[code - 8].len);
    if (trace)
    {
      printbits (code, 13, MCBPCtab1[code - 8].len);
//      fprintf (trace_file, "): %d\n", MCBPCtab1[code - 8].val);
    }
    return MCBPCtab1[code - 8].val;
  }
    /*
  if (code>=256)
  {
    flushbits(1);
    if (trace)
     trace("1): %d\n",0);
    return 0;
  }
    
  flushbits(MCBPCtab[code].len);

  if (trace)
  {
    printbits(code,9,MCBPCtab[code].len);
   trace("): %d\n",MCBPCtab[code].val);
  }
  */
  return MCBPCtab[code].val;
}

int getMODB()
{
  int code;
  int MODB;

  if (trace)
   trace("MODB (");

  code = showbits(2);

  if (code < 2) {
    if (trace)
     trace("0): MODB = 0\n");
    MODB = 0;
    flushbits(1);
  }
  else if (code == 2) {
    if (trace)
     trace("10): MODB = 1\n");
    MODB = 1;
    flushbits(2);
  }
  else { /* code == 3 */
    if (trace)
     trace("11): MODB = 2\n");
    MODB = 2;
    flushbits(2);
  }
  return MODB;
}


int getMCBPCintra()
{
  int code;

  if (trace)
   trace("MCBPCintra (");

  code = showbits(9);

  if (code == 1) {
    /* macroblock stuffing */
    if (trace)
     trace("000000001): stuffing\n");
    flushbits(9);
    return 255;
  }

  if (code < 8) {
      trace("Invalid MCBPCintra code\n");
      fprintf(stderr,"Invalid MCBPCintra code\n");
    fault = 1;
    return 0;
  }

  code >>= 3;
    
  if (code>=32)
  {
    flushbits(1);
    if (trace)
     trace("1): %d\n",3);
    return 3;
  }

  flushbits(MCBPCtabintra[code].len);

  if (trace)
  {
    printbits(code,6,MCBPCtabintra[code].len);
   trace("): %d\n",MCBPCtabintra[code].val);
  }

  return MCBPCtabintra[code].val;
}

int getCBPY()
{
  int code;

  if (trace)
   trace("CBPY (");

  code = showbits(6);
 trace("debug cbpy");
//  printbits(code,6);
  fflush(stdout);
  if (code < 2) {
      trace("Invalid CBPY code\n");
      fprintf(stderr,"Invalid CBPY code\n");
fault=-1;
    return -1;
  }
    
  if (code>=48)
  {
    flushbits(2);
    if (trace)
     trace("11): %d\n",0);
    return 0;
  }

  flushbits(CBPYtab[code].len);

  if (trace)
  {
    printbits(code,6,CBPYtab[code].len);
   trace("): %d\n",CBPYtab[code].val);
  }

  return CBPYtab[code].val;
}

