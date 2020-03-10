
#include	<complex>
#include	"decimator.h"
#include	"fir-filters.h"

	decimator::decimator (int32_t inRate, int decimation) {
int	err;
 	this    -> inRate       = inRate;
        this    -> outRate      = inRate / decimation;;
	inSize			= 100000;
        inputLimit		= inSize;
        ratio                   = double(outRate) / inRate;
	theFilter		= new decimatingFIR (decimation + 1,
	                                             outRate / 2,
	                                             inRate,
	                                             decimation);
        outputLimit             = inSize * ratio;
//      converter               = src_new (SRC_SINC_BEST_QUALITY, 2, &err);
        converter               = src_new (SRC_LINEAR, 2, &err);
//      converter               = src_new (SRC_SINC_MEDIUM_QUALITY, 2, &err);
        src_data                = new SRC_DATA;
        inBuffer. resize (2 * inputLimit + 20);
        outBuffer. resize (2 * outputLimit + 20);
        src_data-> data_in      = inBuffer. data ();
        src_data-> data_out     = outBuffer. data ();
        src_data-> src_ratio    = ratio;
        src_data-> end_of_input = 0;
        inp                     = 0;
	outAvail		= 0;
	outP			= 0;
}

	decimator::~decimator (void) {
	src_delete      (converter);
	delete          src_data;
	delete		theFilter;
}


bool	decimator::Pass	(std::complex<float> in, std::complex<float> *out) {
bool	outVal	= false;
	return (theFilter -> Pass (in, out));
	   
	inBuffer [2 * inp    ]       = real (in);
	inBuffer [2 * inp + 1]       = imag (in);
	if (outAvail > 0) {
	   *out = std::complex<float> (outBuffer [2 * outP],
	                               outBuffer [2 * outP + 1]);
	   outAvail --;
	   outP ++;
	   outVal	= true;
	}
	if (++inp < inputLimit)
	   return outVal;

	src_data	-> input_frames         = inp;
	src_data	-> output_frames        = outputLimit + 10;
	int res		= src_process (converter, src_data);
	if (res != 0) {
	   fprintf (stderr, "error %s\n", src_strerror (res));
	   return false;
	}
	inp			= 0;
	outAvail		= src_data -> output_frames_gen;
	outP			= 0;
	if (!outVal) {
	   *out	= std::complex<float> (outBuffer [2 * outP],
	                                    outBuffer [2 * outP + 1]);
	   outP ++;
	   outAvail --;
	   return true;
	}
	return outVal;
}


