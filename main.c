#include "misc.h"
#include "codec.h"
#include "cif.h"
#include "bitstream.h"

#include "time.h"

int Decode = 0;
static IMAGE image;
PICTURE *pic;
static int op_start_image, op_end_image;
static int format = SF_QCIF;
CONTEXT codeccontext;

int process_arg(int argc, char **argv);

int main(int argc, char **argv)
{
	int i;
    int pictpos = 0;
	codeccontext.o_stream = stdout;
	codeccontext.i_stream = stdin;
	codeccontext.t_stream = 0;
	codeccontext.debug = 0;
	codeccontext.op_quant = 1;
	codeccontext.motest_alg = MOTEST_4SS;
	codeccontext.insert_intra_cycle = 25;

	if (process_arg(argc, argv))
        exit(1);

	image.src_format = format;
	codec_init(codeccontext);

	for (i=op_start_image; i<=op_end_image; i++) {
		trace("Frame %d\n", i);
        if (Decode == 0) {
			if (yuv_read_image(codeccontext, i, &image)) {
	    		fprintf(stderr, "Can't read frame %d\n", i);
		    	return 1;
	    	}
//		fprintf(stderr, "Frame %3d\n", i);
		    codec_encode_image(pic, &image);
        } else {
            int pictsize = 0;
            if ((pictsize = read_h263_picture(pictpos, pic)) <= 0) {
                fprintf(stderr, "Can't read frame %d\n", i);
                return 1;
            }
            pictpos += pictsize;
            codec_decode_picture(pic);
        }
        pic=pic->prev;
	}

	yuv_finit_image(&image);
	codec_finit();

	return 0;
}

int process_arg(int argc, char **argv)
{
	opterr = 0;
	optind = 1;
	op_start_image = 0;
	op_end_image = INT_MAX;
	for (;;)
	{
		int c;

		c = getopt(argc, argv, "i:o:f:d:t:a:b:m:s:M:DGJq:");
		if (c==EOF) break;
		switch (c)
		{
        case 'D':
            Decode = 1;    
            break;
		case 'i':
	    	codeccontext.i_stream = stdin;
    		if (!(codeccontext.i_stream = fopen(optarg, "rb"))) {
	    		fprintf(stderr, "Can't open %s\n", optarg);
		    	return 1;
   			}
    		break;
		case 'o':
			codeccontext.o_stream = stdout;
			if (!(codeccontext.o_stream = fopen(optarg, "wb"))) {
				fprintf(stderr, "Can't open %s\n", optarg);
				return 1;
			}
			break;
        case 'f':
            format = atoi(optarg);
            break;
		case 't':
			codeccontext.t_stream = stdout;
			if (!(codeccontext.t_stream = fopen(optarg, "w"))) {
				fprintf(stderr, "Can't open %s\n", optarg);
				return 1;
			}
			break;
		case 'd':
			codeccontext.debug = atoi(optarg);
			if (debug < 0 || codeccontext.debug > 5) {
				fprintf(stderr, "Error: incorrect debug level\n");
				return 1;
			}
			break;
		case 'a':
			op_start_image = atoi(optarg);
			if (op_start_image < 0) {
				fprintf(stderr, "Error: incorrect op_start_image\n");
				return 1;
			}
			break;
		case 'b':
			op_end_image = atoi(optarg);
			if (op_end_image < 0) {
				op_end_image = INT_MAX;
			}
			break;
		case 'm':
			codeccontext.motest_alg = atoi(optarg);
			if (codeccontext.motest_alg < 0 || codeccontext.motest_alg > MOTEST_MAX) {
				fprintf(stderr, "unknown motest_alg:\nFS\t%d\n4SS\t%d\nCBOSA\t%d\n" \
						"HASA\t%d\n\n", MOTEST_FS, MOTEST_4SS, MOTEST_CBOSA, MOTEST_HASA);
				exit(1);
			}
			break;
		case 'q':
			codeccontext.op_quant = atoi(optarg);
			break;
		case 's':
			codeccontext.insert_intra_cycle = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Unknown option: %s\n", argv[optind-1]);
			return 1;
		}
	}
	return 0;
}

