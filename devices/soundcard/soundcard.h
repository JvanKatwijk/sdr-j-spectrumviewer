#
/*
 *    Copyright (C) 2008, 2009, 2010
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J (JSDR).
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
#ifndef __SOUNDCARD__
#define	__SOUNDCARD__

#include	<QWidget>
#include	<QFrame>
#include	<QSettings>
#include	<QComboBox>
#include	"spectrum-constants.h"
#include	"device-handler.h"
#include	"ui_soundcard-widget.h"
/*
 */
class	paReader;

class	soundcard: public deviceHandler, public Ui_soundcardWidget {
Q_OBJECT
public:
		soundcard (QSettings *);
		~soundcard		(void);
	int32_t	getRate			(void);
	bool	restartReader		(void);
	void	stopReader		(void);
	int32_t	Samples			(void);
	int32_t	getSamples		(DSPCOMPLEX *, int32_t, int32_t);
	int16_t	bitDepth		(void);
//	remaining functions from virtualInput not implemented here
private:
	QFrame		*myFrame;
	int32_t		inputRate;
	paReader	*myReader;
	float		gainFactor;
	uint8_t		runMode;
private slots:
	void		set_streamSelector	(int);
	void		set_rateSelector	(const QString &);
	void		set_gainSlider		(int);
};
#endif

