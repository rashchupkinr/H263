NAME = codec

CSRC = main.c codec.c picture.c block.c putbits.c cif.c bitstream.c quant.c dct.c debug.c frame_I.c frame_P.c motest.c gethdr.c getvlc.c getbits.c getblk.c
HDRS = codec.h misc.h bitstream.h cif.h getvlc.h indices.h sactbls.h frame.h scan.h
OBJS = $(patsubst %.c,%.o,$(CSRC))

DEBUG =
DEFS = FAST_DCT DEBUG

CCFLAGS = -Wall -Wno-unused -O3 $(addprefix -D,$(DEFS)) 
LINKFLAGS = -lm

all:		$(NAME)

clean:
		rm -f $(OBJS) $(NAME)

$(NAME):	$(OBJS)
		gcc  $(LINKFLAGS) -o $@ $(OBJS)

$(OBJS):%.o:	%.c $(HDRS)
		gcc $(CCFLAGS) -c $<
