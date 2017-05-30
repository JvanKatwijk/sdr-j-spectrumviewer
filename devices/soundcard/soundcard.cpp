#
/*
 *    Copyright (C) 2015 
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
 */
#include	"pa-reader.h"
#include	"soundcard.h"
#include	<QLabel>
#include	<QSettings>
#include	<QMessageBox>

#define	RUNNABLE	01
#define	FAILING		00
#define	NOT_READY	02

	soundcard::soundcard (QSettings *s) {
bool	success;
	(void)s;
	myFrame		= new QFrame (NULL);
	setupUi (myFrame);
	myFrame	-> show ();
	inputRate	= rateSelector -> currentText (). toInt ();
	myReader	= new paReader (inputRate, cardSelector, &success);
//	if (success)
//	   throw (21);
	connect (cardSelector, SIGNAL (activated (int)),
	         this, SLOT (set_streamSelector (int)));
	connect (rateSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (set_rateSelector (const QString &)));
	connect (gainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_gainSlider (int)));
	gainFactor	= 1.0;
	status	-> setText ("idle");
	runMode		= NOT_READY;
}

	soundcard::~soundcard		(void) {
	delete myReader;
	myReader	= NULL;
	delete myFrame;
}

int32_t	soundcard::getRate		(void) {
	return inputRate;
}

bool	soundcard::restartReader	(void) {
bool r;
	if (runMode != RUNNABLE)
	   return false;
	r =  myReader -> restartReader ();
	status -> setText (r ? "running" : "failed");
	return r;
}

void	soundcard::stopReader	(void) {
	status	-> setText ("stopped");
	myReader -> stopReader ();
}

int32_t	soundcard::Samples	(void) {
	return myReader	-> Samples ();
}

int32_t	soundcard::getSamples	(DSPCOMPLEX *b, int32_t a, int32_t m) {
int16_t	i;
int16_t	ra = myReader -> getSamples (b, a, m);
	for (i = 0; i < a; i ++)
	   b [i] = cmul (b [i], gainFactor);
	return ra;
}

void	soundcard::set_streamSelector (int idx) {
	runMode	= RUNNABLE;
	if (!myReader	-> set_StreamSelector (idx - 1)) {
	   QMessageBox::warning (myFrame, tr ("sdr"),
	                               tr ("Selecting  input stream failed\n"));
	   runMode	= FAILING;
	}
}

void	soundcard::set_gainSlider	(int s) {
	gainFactor	= float (s) / 100;
	showGain	-> display (s);
}

void	soundcard::set_rateSelector (const QString &s) {
int32_t	newRate	= s. toInt ();
int16_t	i;
bool	success;
	if (newRate == inputRate)
	   return;

	myReader	-> stopReader ();
	disconnect (cardSelector, SIGNAL (activated (int)),
	            this, SLOT (set_streamSelector (int)));
	for (i = cardSelector -> count (); i > 0; i --)
	   cardSelector -> removeItem (cardSelector -> currentIndex ()); 
	delete myReader;

	inputRate	= newRate;
	myReader	= new paReader (inputRate, cardSelector, &success);
	if (success) {
	   status -> setText ("idle");
	   connect (cardSelector, SIGNAL (activated (int)),
	            this, SLOT (set_streamSelector (int)));
	   emit set_changeRate (newRate);
	}
	else {
	   QMessageBox::warning (myFrame, tr ("sdr"),
	                               tr ("no valid devices\n giving up"));
	   exit (1);
	}
	runMode	= RUNNABLE;
}

int16_t	soundcard::bitDepth	(void) {
	return 24;
}

