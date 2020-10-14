#
/*
 *    Copyright (C) 2011, 2012, 2013
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the spectrumviewer
 *
 *    spectrumviewer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    spectrumviewer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with spectrumviewer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#ifndef __SPECTRUM_CONSTANTS__
#define	__SPECTRUM_CONSTANTS__

#define	QT_STATIC_CONST
#include	<math.h>
#include	<complex>
#include	<stdint.h>
#include	<unistd.h>
#include	<limits>
#include	"stdlib.h"

using namespace std;
#include	<malloc.h>

#ifdef __MINGW32__
#define	CURRENT_VERSION		"1.0:x64-Windows"
#include	"windows.h"
#else
#include	"alloca.h"
#define	CURRENT_VERSION		"1.0:x32-Linux"
#include	"dlfcn.h"
typedef	void	*HINSTANCE;
#endif

/*
 */
typedef	float DSPFLOAT;

typedef	std::complex<DSPFLOAT>	DSPCOMPLEX;

/*
 */
#define	MINIMUM(x, y)	((x) < (y) ? x : y)
#define	MAXIMUM(x, y)	((x) > (y) ? x : y)

#define	IandQ		0100
#define	QandI		0101
#define I_Only		0102
#define	Q_Only		0104
#define	Hz(x)	(x)
#define	Khz(x)	(Hz (x) * 1000)
#define	KHz(x)	(Hz (x) * 1000)
#define	Mhz(x)	(KHz (x) * 1000)
#define	MHz(x)	(KHz (x) * 1000)
//
//	common, simple but useful, functions
static inline
bool	isIndeterminate (DSPFLOAT x) {
	return x != x;
}

static inline
bool	isInfinite (double x) {
	return x == numeric_limits<DSPFLOAT>::infinity ();
}
//
static inline
DSPCOMPLEX cmul (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) * y, imag (x) * y);
}

static inline
DSPCOMPLEX cdiv (DSPCOMPLEX x, float y) {
	return DSPCOMPLEX (real (x) / y, imag (x) / y);
}

static inline
float	get_db (DSPFLOAT x) {
	return 20 * log10 ((x + 1) / (float)(1024));
}
#endif
