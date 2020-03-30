
#ifndef	__DECIMATOR__
#define	__DECIMATOR__
#include	<stdint.h>
#include        <math.h>
#include        <complex>
#include        <stdint.h>
#include        <unistd.h>
#include        <vector>
#include        <limits>
#include        <samplerate.h>
#include	"fir-filters.h"

class	decimator {
public:
			decimator	(int32_t, int32_t);
			~decimator	(void);
	bool		Pass		(std::complex<float>, std::complex<float> *);
	std::complex<float> Pass	(std::complex<float>);
	int		rateOut		();
	int32_t         inRate;
        int32_t         outRate;
        double          ratio;
        int32_t         outputLimit;
        int32_t         inputLimit;
        SRC_STATE       *converter;
        SRC_DATA        *src_data;
        std::vector<float> inBuffer;
        std::vector<float> outBuffer;
        int32_t         inp;
	int32_t		inSize;
	int		outP;
	int		outAvail;
	decimatingFIR	*theFilter;
};

#endif


