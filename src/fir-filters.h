#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J.
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FIR_FILTERS__
#define __FIR_FILTERS__

#include	<stdio.h>
#include	<complex>
#include	<vector>


class	decimatingFIR {
public:
		decimatingFIR (int16_t,	// order
	                               int32_t, 	// cutoff frequency
	                               int32_t,	// samplerate
	                               int16_t	// decimation factor
	                              );
		~decimatingFIR (void);
	bool	Pass	(std::complex<float>, std::complex<float> *);
private:
	std::vector<std::complex<float> > Buffer;
	std::vector<std::complex<float> > filterKernel;
	int	filterSize;
	int	ip;
	int	decimationFactor;
	int	decimationCounter;
};
//
#endif

