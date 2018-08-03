
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
	decimator	(int32_t, int32_t, int16_t);
	~decimator	(void);
int	getRate		(void);
void	addBuffer	(std::complex<float> *buffer, int32_t amount);
bool	handleBuffer	(std::complex<float> *inBuffer,
	                 int32_t amount,
	                 std::complex<float> *outBuffer);
	LowPassFIR	filter;
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
};

#endif


