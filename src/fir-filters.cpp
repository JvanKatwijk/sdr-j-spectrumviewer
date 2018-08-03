#
/*
 *
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J.
 *    Many of the ideas as implemented in SDR-J are derived from
 *    other work, made available through the GNU general Public License. 
 *    All copyrights of the original authors are recognized.
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
 *
 */

#include	"fir-filters.h"


	decimatingFIR::decimatingFIR (int16_t filterSize,
	                              int32_t low,
	                              int32_t fs,
	                              int16_t Dm) {
float	tmp [filterSize];
float	f	= (float)low / fs;
int16_t	i;
float	sum	= 0.0;

	this	-> filterSize	= filterSize;
	decimationFactor	= Dm;
	decimationCounter	= 0;
	filterKernel. resize (filterSize);
	Buffer. resize (filterSize);

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/2);
//
//	Blackman window
	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++)
	   filterKernel [i] = std::complex<float> (tmp [i] / sum, 0);
	ip	= 0;
}

	decimatingFIR::~decimatingFIR (void) {
}

bool	decimatingFIR::Pass (std::complex<float> z_in,
	                     std::complex<float> *z_out) {
int16_t         i;
std::complex<float>      tmp     = 0;
int16_t         index;

        Buffer [ip] = z_in;
        if (++ decimationCounter < decimationFactor) {
           ip =  (ip + 1) % filterSize;
           return false;
        }

        decimationCounter = 0;
//
//
//      we are working with a circular buffer, we take two steps
//      we move from ip - 1 .. 0 with i going from 0 .. ip -1
        for (i = 1; i <= ip; i ++) {
           index =  ip - i;
           tmp  += Buffer [index] * filterKernel [i];
        }
//      and then we take the rest
        for (i = ip + 1; i < filterSize; i ++) {
           index =  filterSize + ip - i;
           tmp  += Buffer [index] * filterKernel [i];
        }
        *z_out = tmp;
        return true;
}

