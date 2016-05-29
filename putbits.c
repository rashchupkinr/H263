#include "misc.h"

/* private data */
static unsigned char outbfr;
static int outcnt;
static int bytecnt;
FILE *_o_stream;

/* initialize buffer, call once before first putbits or alignbits */
void initbits(CONTEXT context)
{
  _o_stream = context.o_stream;
  outcnt = 8;
  bytecnt = 0;
}

/* write rightmost n (0<=n<=32) bits of val to outfile */
void putbits(int n, int val)
{
  int i;
  unsigned int mask;
  char bitstring[32];

  if (debug > 1)
  {
    if (n > 0)
    {
      BitPrint(n, val, bitstring);
      trace(bitstring);
    }
  }
  mask = 1 << (n - 1);          /* selects first (leftmost) bit */

  for (i = 0; i < n; i++)
  {
    outbfr <<= 1;

    if (val & mask)
      outbfr |= 1;

    mask >>= 1;                 /* select next bit */
    outcnt--;
    if (outcnt == 0)            /* 8 bit buffer full */
    {
      putc(outbfr, _o_stream);
      outcnt = 8;
      bytecnt++;
    }
  }
}


/* zero bit stuffing to next byte boundary (5.2.3, 6.2.1) */
int alignbits()
{
  int ret_value;
  if (outcnt != 8)
  {
    ret_value = outcnt;         /* outcnt is reset in call to putbits () */
    trace("align:\t");
    putbits (outcnt, 0);
//    fflush(o_stream);
    return ret_value;
  } else
    return 0;
}

/* return total number of generated bits */
int bitcount()
{
  return 8 * bytecnt + (8 - outcnt);
}

/* convert to binary number */
void BitPrint(int length, int val, char *bit)
{
  int m;

  m = length;
  bit[0] = '"';
  while (m--)
  {
    bit[length - m] = (val & (1 << m)) ? '1' : '0';
  }
  bit[length + 1] = '"';
  bit[length + 2] = '\n';
  bit[length + 3] = '\0';
  return;
}

