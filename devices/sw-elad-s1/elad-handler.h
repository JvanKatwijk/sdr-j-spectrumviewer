#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of spectrumviewer
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

#ifndef __ELAD_HANDLER__
#define	__ELAD_HANDLER__

#include	<QObject>
#include	<QFrame>
#include	<QFileDialog>
#include	"spectrum-constants.h"
#include	"ringbuffer.h"
#include	"ui_elad-widget.h"
#include	"device-handler.h"
#include	<libusb-1.0/libusb.h>

class	QSettings;
class	eladWorker;
class	eladLoader;
typedef	std::complex<float>(*makeSampleP)(uint8_t *);

class	eladHandler: public deviceHandler, public Ui_eladWidget {
Q_OBJECT
public:
		eladHandler		(QSettings *, int32_t);
		~eladHandler		(void);
	void	setVFOFrequency		(uint64_t);
	uint64_t	getVFOFrequency		(void);
	bool	restartReader		(void);
	void	stopReader		(void);
	int32_t	getSamples		(std::complex<float> *, int32_t);
	int32_t	getSamples		(std::complex<float> *, int32_t, int32_t);
	int32_t	Samples			();
	int32_t	getRate			();
	int16_t	bitDepth		();
	bool	legalFrequency		(int32_t);
private	slots:
	void	setGainReduction	(void);
	void	setOffset		(int);
	void	setFilter		();
	void	handle_dumpButton	();
private:
	QSettings	*eladSettings;
	int32_t		inputRate;
	QFrame		myFrame;
	RingBuffer<uint8_t>	_I_Buffer;
	FILE		*dumpFile;
	std::atomic<bool>	dumping;
	int16_t		depth;
	bool		deviceOK;
	eladLoader	*theLoader;
	eladWorker	*theWorker;
	uint64_t	vfoFrequency;
	int32_t		vfoOffset;
	int		gainReduced;
	int		localFilter;
	uint8_t		conversionNumber;
	int16_t		iqSize;
signals:
	void		samplesAvailable	(int);
};
#endif

