
#ifndef	__FREQ_MAPPER__
#define	__FREQ_MAPPER__

#include	<stdint.h>
#include	<complex>
#include	"fft.h"

class	freqmapper {
public:
		freqmapper	(int32_t);
		~freqmapper	(void);
	void	convert	(std::complex<float> *, double *);
private:
	common_fft	the_fft;
	std::complex<float> *fftVector;
	int	displaySize;
};

#endif

