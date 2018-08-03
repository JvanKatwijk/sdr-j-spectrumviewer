
#include	"mapper.h"
#include	<cstring>

	freqmapper::freqmapper	(int32_t displaySize):
	                            the_fft (displaySize) {
	this	-> displaySize	= displaySize;
	fftVector		= the_fft. getVector ();
}

	freqmapper::~freqmapper	(void) {
}

void	freqmapper::convert	(std::complex<float> *in, double *out) {
int	i;
	memcpy (the_fft. getVector (), in,
	             displaySize * sizeof (std::complex<float>));
	the_fft. do_FFT ();
	for (i = 0; i < displaySize / 2; i ++) {
	   out [i] = abs ((the_fft. getVector ()) [displaySize / 2 + i]);
	   out [displaySize / 2 + i] = abs ((the_fft. getVector ()) [i]);
	}
}

