#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the spectrumViewer
 *
 *    spectrumViewer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    spectrumViewer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with spectrumViewer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ELAD_WORKER__
#define	__ELAD_WORKER__

#include	<stdlib.h>
#include	<stdio.h>
#include	<math.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdint.h>
#include	<QThread>
#include	"ringbuffer.h"
#include	"elad-loader.h"

#define		IQ_SIZE		8
class	eladWorker: public QThread {
Q_OBJECT
public:
			eladWorker	(int32_t,	// the rate,
	                                 int32_t,	// initial freq
	                                 eladLoader *,
	                                 RingBuffer<uint8_t> *,
	                                 int16_t,	// iqSize
	                                 bool *);
			~eladWorker	(void);
	void		setVFOFrequency	(uint64_t);
	uint64_t	getVFOFrequency	(void);
	void		stop		(void);
private:
	void			run	(void);
	eladLoader		*functions;	// 
	RingBuffer<uint8_t>	*_I_Buffer;
	int16_t			iqSize;
	int32_t			defaultFreq;
	long int		lastFrequency;
	bool			runnable;
	int32_t			theRate;
signals:
	void			samplesAvailable	(int);
};
#endif

