

#include	"decimator.h"

	decimator::decimator (int32_t inRate,
	                      int32_t outRate, int16_t displaySize):
	                        filter (255, outRate / 2, inRate, inRate / outRate) {
int	err;
 	this    -> inRate       = inRate;
        this    -> outRate      = outRate;
	inSize			= 100000;
        inputLimit		= inSize;
        ratio                   = double(outRate) / inRate;
//      fprintf (stderr, "ratio = %f\n", ratio);
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
}

	decimator::~decimator (void) {
	src_delete      (converter);
	delete          src_data;
}

int	decimator::getRate	(void) {
	return inRate;
}

void	decimator::addBuffer (std::complex<float> *buffer, int32_t amount) {
int	i;

	inp	= 0;
	for (i = 0; i < amount; i ++) {
	   std::complex<float> x = filter. Pass (buffer [i]);
	   inBuffer [2 * inp    ]	= real (x);
	   inBuffer [2 * inp + 1]	= imag (x);
	}
}

bool	decimator::handleBuffer (std::complex<float> *buffer,
	                         int32_t amount,
	                         std::complex<float> *out) {
int	i;
int32_t	framesOut;
int	res;

	for (i = 0; i < amount; i ++) {
	   std::complex<float> x = filter. Pass (buffer [i]);
	   inBuffer [2 * inp    ]       = real (x);
           inBuffer [2 * inp + 1]       = imag (x);

	   if (++inp < inputLimit)
	      continue;

	   src_data	-> input_frames         = inp;
	   src_data	-> output_frames        = outputLimit + 10;
	   res             = src_process (converter, src_data);
	   if (res != 0) {
	      fprintf (stderr, "error %s\n", src_strerror (res));
	      return false;
	   }
	   inp             = 0;
	   framesOut       = src_data -> output_frames_gen;
	   for (i = 0; i < framesOut; i ++)
	   out [i] = std::complex<float> (outBuffer [2 * i],
                                          outBuffer [2 * i + 1]);
	   return true;
	}
}


