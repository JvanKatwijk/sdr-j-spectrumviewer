#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J
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
#include	"gui.h"
#include	<QSettings>
#include	<QMessageBox>
#include	<QFileDialog>
#include	<QDebug>
#include	<qwt_color_map.h>
#include	<qwt_plot_spectrogram.h>
#include	<qwt_scale_widget.h>
#include	<qwt_scale_draw.h>
#include	<qwt_plot_zoomer.h>
#include	<qwt_plot_panner.h>
#include	<qwt_plot_layout.h>
#include	<QDateTime>
#include	"virtual-input.h"
#ifdef	HAVE_SDRPLAY
#include	"sdrplay.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#ifdef	HAVE_DABSTICK
#include	"dabstick.h"
#endif
#ifdef	HAVE_ELAD_S1
#include	"elad-s1.h"
#endif
#ifdef	HAVE_EXTIO
#include	"extio-handler.h"
#endif
#ifdef	HAVE_SOUNDCARD
#include	"soundcard.h"
#endif
#include	"scope.h"
#ifdef __MINGW32__
#include	<iostream>
#endif
#
//
//	the default
#define	SCAN_DELAY	200
#define	IDLE		0100
#define	PAUSED		0101
#define	RUNNING		0102
//
/*
 *	We use the creation function merely to set up the
 *	user interface and make the connections between the
 *	gui elements and the handling agents. All real action
 *	is embedded in actions, initiated by gui buttons
 */
	RadioInterface::RadioInterface (QSettings	*Si,
	                                QWidget		*parent): QDialog (parent) {
int	i;
int k;
// 	the setup for the generated part of the ui
	setupUi (this);
	spectrumSettings		= Si;
	this		-> rasterSize	= 50;
	this	-> inputRate  		= MHz (2);	// default
	this	-> displaySize =
	             spectrumSettings -> value ("displaySize", 2048). toInt ();
	this	-> displayRate	=
	             spectrumSettings -> value ("displayRate", 10). toInt ();
	if (displayRate < 5 || displayRate > 20)
	   displayRate = 10;
	this -> spectrumFactor	= 4;
	currentFrequency = MHz (100);		// default
	theDevice	= new virtualInput ();
	if ((displaySize & (displaySize - 1)) != 0)
	   displaySize = 2048;
	spectrumSize	= spectrumFactor * displaySize;
//	we use a blackman window
	Window				= new DSPFLOAT [spectrumSize];
	for (i = 0; i < spectrumSize; i ++)
	   Window [i] = 0.42 - 0.5 * cos ((2.0 * M_PI * i) / spectrumSize) +
	                       0.08 * cos ((4.0 * M_PI * i) / spectrumSize);

	HFScope		= new Scope (hfscope,
	                             this -> displaySize,
	                             this -> rasterSize);
	HFViewmode	= SPECTRUM_MODE;
	HFScope		-> SelectView (SPECTRUM_MODE);
	connect (HFScope, SIGNAL (clickedwithLeft (int)),
	         this, SLOT (adjustFrequency (int)));
	this		-> X_axis		= new double [displaySize];
	this		-> displayBuffer	= new double [displaySize];
	this		-> freezeBuffer		= new double [displaySize];
	memset (freezeBuffer, 0, displaySize * sizeof (double));
	this		-> spectrum_fft	= new common_fft (spectrumSize);
	this		-> spectrumBuffer	= spectrum_fft -> getVector ();
	
#ifdef	HAVE_SDRPLAY
	deviceSelector	-> addItem ("sdrplay");
#endif
#ifdef	HAVE_DABSTICK
	deviceSelector	-> addItem ("dabstick");
#endif
#ifdef	HAVE_AIRSPY
	deviceSelector	-> addItem ("airspy");
#endif
#ifdef	HAVE_EXTIO
	deviceSelector	-> addItem ("extio");
#endif
#ifdef	HAVE_ELAD_S1
	deviceSelector	-> addItem ("elad-192000");
	deviceSelector	-> addItem ("elad-384000");
	deviceSelector	-> addItem ("elad-768000");
	deviceSelector	-> addItem ("elad-1536000");
	deviceSelector	-> addItem ("elad-3072000");
	deviceSelector	-> addItem ("elad-6144000");

#endif

#ifdef	HAVE_SOUNDCARD
	deviceSelector	-> addItem ("soundcard");
#endif
//
//	set some sliders to their values
	k	= spectrumSettings -> value ("spectrumAmplitudeSlider", 20). toInt ();
	spectrumAmplitudeSlider	-> setValue (k);
	k	= spectrumSettings -> value ("lowerLimit", 86). toInt ();
	lowerLimit			-> setValue (k);

	k	= spectrumSettings -> value ("upperLimit", 110). toInt ();
	upperLimit			-> setValue (k);

	k	= spectrumSettings -> value ("stepSize", 500). toInt ();
	stepSize			-> setValue (k);

	k	= spectrumSettings -> value ("delayBox", SCAN_DELAY). toInt ();
	delayBox			-> setValue (k);
	scanDelayTime		= k;

	ClearPanel		();
	connect (deviceSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (setDevice (const QString &)));
	connect (startButton, SIGNAL (clicked (void)),
	         this, SLOT (setStart (void)));
	connect (QuitButton, SIGNAL (clicked (void)),
	         this, SLOT (TerminateProcess (void)));
	connect (pauseButton, SIGNAL (clicked (void)),
	         this, SLOT (clickPause (void)));
	
	connect (viewmodeButton, SIGNAL (clicked (void)),
	         this, SLOT (setViewmode (void)));
	connect (freezeButton, SIGNAL (clicked (void)),
	         this, SLOT (setFreezer (void)));
	connect (delayBox, SIGNAL (valueChanged (int)),
	         this, SLOT (setScanDelay (int)));
/*
 *	connections for the console, first the digits
 */
	connect (add_one,   SIGNAL (clicked() ), this, SLOT (addOne() ) );
	connect (add_two,   SIGNAL (clicked() ), this, SLOT (addTwo() ) );
	connect (add_three, SIGNAL (clicked() ), this, SLOT (addThree() ) );
	connect (add_four,  SIGNAL (clicked() ), this, SLOT (addFour() ) );
	connect (add_five,  SIGNAL (clicked() ), this, SLOT (addFive() ) );
	connect (add_six,   SIGNAL (clicked() ), this, SLOT (addSix() ) );
	connect (add_seven, SIGNAL (clicked() ), this, SLOT (addSeven() ) );
	connect (add_eight, SIGNAL (clicked() ), this, SLOT (addEight() ) );
	connect (add_nine,  SIGNAL (clicked() ), this, SLOT (addNine() ) );
	connect (add_zero,  SIGNAL (clicked() ), this, SLOT (addZero() ) );
/*
 *	function buttons
 */
	connect (khzSelector, SIGNAL (clicked ()),
	                      this, SLOT (AcceptFreqinKhz ()));
	connect (mhzSelector, SIGNAL (clicked ()),
	                      this, SLOT (AcceptFreqinMhz ()));
	connect (add_correct, SIGNAL (clicked() ), this, SLOT (addCorr() ) );
	connect (add_clear,   SIGNAL (clicked() ), this, SLOT (addClear() ) );

//	Create a timer for dealing with a non acting (or reacting) user
//	on the keyboard
	lcd_timer		= new QTimer ();
	lcd_timer		-> setSingleShot (true);	// single shot
	lcd_timer		-> setInterval (5000);
	connect (lcd_timer, SIGNAL (timeout ()),
	         this, SLOT (lcd_timeout ()));

	runTimer		= new QTimer ();
	runTimer		-> setSingleShot (false);
	runTimer		-> setInterval (1000 / displayRate);	// milliseconds

	connect (runTimer, SIGNAL (timeout (void)),
	         this, SLOT (handleSamples (void)));

	scanTimer		= new QTimer ();
	scanTimer		-> setSingleShot (false);
	scanTimer		-> setInterval (scanDelayTime * 100);
	connect (scanTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextFrequency (void)));
	connect (scanstartButton, SIGNAL (clicked (void)),
	         this, SLOT (switchScanner (void)));

//	just as a gadget, we show the actual time
	displayTimer		= new QTimer ();
	displayTimer		-> setInterval (1000);
	connect (displayTimer, SIGNAL (timeout ()),
	         this, SLOT (updateTimeDisplay ()));

	QString t = QString ("SDR-J spectrumviewer ");
	t. append (CURRENT_VERSION);
	systemindicator		-> setText (t);
	displayTimer		-> start (1000);
//
//	all elements seem to exist: set the control state to
//	its default values
	runMode			= IDLE;
	scanner			= false;
	freezer			= false;
}

	RadioInterface::~RadioInterface (void) {
//	On normal program exit, we save some of the values
	spectrumSettings -> setValue ("delayBox", delayBox -> value ());
	spectrumSettings -> setValue ("currentFrequency",
	                                     (int)currentFrequency / Khz (1));
	spectrumSettings -> setValue ("spectrumAmplitudeSlider",
	                              spectrumAmplitudeSlider -> value ());
	spectrumSettings -> setValue ("lowerLimit",
	                              lowerLimit -> value ());
	spectrumSettings -> setValue ("upperLimit",
	                              upperLimit -> value ());
	spectrumSettings -> setValue ("stepSize",
	                              stepSize -> value ());
//
//	and then we delete
	delete		lcd_timer;
	delete		theDevice;
	delete		HFScope;
	delete		displayTimer;
	delete[]	Window;
	delete		runTimer;
	delete		X_axis;
	delete		displayBuffer;
	delete		freezeBuffer;
}

void	RadioInterface::setDevice (const QString &s) {
bool	success	= false;

	if (theDevice != NULL) {
	   theDevice	-> stopReader ();
	   delete	theDevice;
	   theDevice	= NULL;
	   runMode	= IDLE;
	}

	lcd_inputRate	-> display (0);

	if (s == "no device")  {
	   theDevice	= new virtualInput ();
	   inputRate	= MHz (100);
	   return;
	}
#ifdef	HAVE_SDRPLAY
	if (s == "sdrplay") {
	   theDevice	= new sdrplay (spectrumSettings, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening SDRplay failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
//	basically, ready to run
	}
	else 
#endif
#ifdef	HAVE_DABSTICK
	if (s == "dabstick") {
	   theDevice	= new dabStick (spectrumSettings, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening dabstick failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
//	basically, ready to run
	}
	else
#endif
#ifdef	HAVE_AIRSPY
	if (s == "airspy") {
	   theDevice	= new airspyHandler (spectrumSettings, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening airspy failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();;
	   }
	   inputRate	= theDevice -> getRate ();
	}
#endif
#ifdef	HAVE_EXTIO
	if (s == "extio") {
	   theDevice	= new extioHandler (spectrumSettings, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening extio failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();;
	   }
	   inputRate	= theDevice -> getRate ();
	}
#endif
#ifdef	HAVE_ELAD_S1
	if (s == "elad-192000") {
	   theDevice	= new eladHandler (spectrumSettings, 192000, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening elad 192000 failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
	}
	else
	if (s == "elad-384000") {
	   theDevice	= new eladHandler (spectrumSettings, 384000, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening elad 384000 failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
	}
	else
	if (s == "elad-768000") {
	   theDevice	= new eladHandler (spectrumSettings, 768000, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening elad 768000 failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
	}
	else
	if (s == "elad-1536000") {
	   theDevice	= new eladHandler (spectrumSettings, 1536000, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening elad 1536000 failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
	}
		else
	if (s == "elad-3072000") {
	   theDevice	= new eladHandler (spectrumSettings, 3072000, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening elad 3072000 failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
	}
	if (s == "elad-6144000") {
	   theDevice	= new eladHandler (spectrumSettings, 6144000, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening elad 6144000 failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
        }	
#endif
#ifdef	HAVE_SOUNDCARD
	else
	if (s == "soundcard") {
	   theDevice	= new soundcard (spectrumSettings, &success);
	   if (!success) {
	      QMessageBox::warning (this, tr ("sdr"),
	                                  tr ("Opening soundcard failed\n"));
	      delete theDevice;
	      theDevice = new virtualInput ();
	   }
	   inputRate	= theDevice -> getRate ();
	}
#endif
	connect (theDevice, SIGNAL (set_changeRate (int)),
	         this, SLOT (set_changeRate (int)));
	inputRate	= theDevice -> getRate ();

//	we are not running, so the X axis will be set when computing
//	a new spectrum
	lcd_inputRate	-> display (inputRate);
	HFScope	-> setBitDepth (theDevice -> bitDepth ());
	setTuner (theDevice -> defaultFrequency ());
}
//
void	RadioInterface::set_changeRate (int newRate) {
	inputRate		= newRate;
	setTuner (currentFrequency);
	lcd_inputRate -> display (inputRate);
	runTimer	-> stop ();
	runMode		= IDLE;
}

//
//	Now, "start" is only meaningful if we are not running already
void	RadioInterface::setStart	(void) {
bool	r = 0;

	if (theDevice == NULL)
	   return;
	if (runMode != IDLE)
	   return;

	setTuner (currentFrequency);
	Display (currentFrequency);
	theDevice	-> stopReader ();	// just in case
	r = theDevice	-> restartReader ();
	if (!r) {
	   QMessageBox::warning (this, tr ("sdr"),
	                               tr ("Opening  device failed\n"));
	   exit (102);
	}
//	and we are on the move, so let the timer run
	runTimer	-> start (1000 / displayRate);
	runMode		= RUNNING;
}

void	RadioInterface::TerminateProcess () {
	runMode		= IDLE;
	setDevice (QString ("no device"));
	runTimer	-> stop ();
	accept ();
	qDebug () <<  "End of termination procedure";
}

void	RadioInterface::abortSystem (int d) {
	qDebug ("aborting for reason %d\n", d);
	exit (11);
	accept ();
	
}
/*
 * 	Handling the numeric keypad is boring, but needed
 */
void RadioInterface::addOne (void) {
	AddtoPanel (1);
}

void RadioInterface::addTwo (void) {
	AddtoPanel (2);
}

void RadioInterface::addThree (void) {
	AddtoPanel (3);
}

void RadioInterface::addFour (void) {
	AddtoPanel (4);
}

void RadioInterface::addFive (void) {
	AddtoPanel (5);
}

void RadioInterface::addSix (void) {
	AddtoPanel (6);
}

void RadioInterface::addSeven (void) {
	AddtoPanel (7);
}

void RadioInterface::addEight (void) {
	AddtoPanel (8);
}

void RadioInterface::addNine (void) {
	AddtoPanel (9);
}

void RadioInterface::addZero (void) {
	AddtoPanel (0);
}
//
//	If we are keying, the lcd timer is on, so
//	if it is not on, we are not keying and clear does not
//	mean anything
void RadioInterface::addClear (void) {
	if (!lcd_timer -> isActive ())
	   return;

	stop_lcdTimer	();

	ClearPanel ();
	Display (currentFrequency);
}
//
//
void RadioInterface::AcceptFreqinKhz (void) {
int32_t	p;

	if (!lcd_timer -> isActive ())
	   return;

	stop_lcdTimer	();
//	we know that the increment timer is not running,
//	so there is no reason to stop it
	p = getPanel ();
	ClearPanel ();
	setTuner (Khz (p));
	Display (currentFrequency);
}

void RadioInterface::AcceptFreqinMhz (void) {
int32_t	p;

	if (!lcd_timer -> isActive ())
	   return;

	stop_lcdTimer	();
//	we know that the increment timer is not running,
//	so there is no reason to stop it
	p = getPanel ();
	ClearPanel ();
	setTuner (Mhz (p));
	Display (currentFrequency);
}

void RadioInterface::addCorr (void) {
	if (!lcd_timer -> isActive ()) 
	   return;

	stop_lcdTimer	();
//	increment timer cannot run now so no reason
//	to stop it
	Panel = (Panel - (Panel % 10)) / 10;
	Display	(Panel, true);
	lcd_timer	-> start (5000);		// restart timer
}
//
//	In AddtoPanel, we might have any of the two
//	timers running, do not know which one though
void	RadioInterface::AddtoPanel (int16_t n) {
	stop_lcdTimer	();
	Panel		= 10 * Panel + n;
	Display (Panel, true);
	lcd_timer	-> start (5000);		// restart timer
}

void	RadioInterface::ClearPanel (void) {
	Panel = 0;
}

int	RadioInterface::getPanel (void) {
	return Panel;
}

void	RadioInterface::adjustFrequency (int n) {
	stop_lcdTimer ();

	setTuner (currentFrequency + Khz (n));
	Display (currentFrequency);
}

//	SetTuner accepts freq in Hz.
//
void	RadioInterface::setTuner (int32_t n) {
int32_t	i;
	if (!theDevice -> legalFrequency (n - inputRate / 2) ||
	    !theDevice -> legalFrequency (n + inputRate / 2))
	   return;
	scanTimer	-> stop ();
	theDevice	-> setVFOFrequency (n);
	currentFrequency = theDevice -> getVFOFrequency ();

//	a new frequency implies a new X_axis
	for (i = 0; i < displaySize; i ++) {
	   X_axis [i] = (currentFrequency  - inputRate / 2 +
	                 i * (inputRate / displaySize)) / ((double)KHz (1));
	   displayBuffer [i] = 0;
	}
	if (freezer)
	   setFreezer	();	// just  ensure 
}
//	setnextFrequency () is almost identical to setTuner
//
void	RadioInterface::setnextFrequency (int32_t n) {
int32_t	i;

	if (!theDevice -> legalFrequency (n - inputRate / 2) ||
	    !theDevice -> legalFrequency (n + inputRate / 2))
	   return;
	theDevice	-> setVFOFrequency (n);
	currentFrequency	= theDevice -> getVFOFrequency ();

	Display (currentFrequency);
//	a new frequency implies a new X_axis
	for (i = 0; i < displaySize; i ++) {
	   X_axis [i] = (currentFrequency  - inputRate / 2 +
	                 i * (inputRate / displaySize)) / ((double)KHz (1));
	   displayBuffer [i] = 0;
	}
	if (freezer)
	   setFreezer	();	// just  ensure 
}
//
void	RadioInterface::stop_lcdTimer (void) {
	if (lcd_timer -> isActive ())
	   lcd_timer -> stop ();
}

void	RadioInterface::updateTimeDisplay (void) {
QDateTime	currentTime = QDateTime::currentDateTime ();

	timeDisplay	-> setText (currentTime.
	                            toString (QString ("dd.MM.yy:hh:mm:ss")));
}

void	RadioInterface::lcd_timeout (void) {
	Panel		= 0;			// throw away anything
	Display (currentFrequency);
}

void	RadioInterface::clickPause (void) {
	if (runMode == IDLE)
	   return;

	if (runMode == RUNNING) {
	   theDevice	-> stopReader ();
	   pauseButton	-> setText (QString ("Continue"));
	   if (runTimer -> isActive ())
	      runTimer -> stop ();
	   runMode = PAUSED;
	   return;
	}
	if (runMode == PAUSED) {
	   theDevice	-> restartReader ();
	   pauseButton	-> setText (QString ("Pause"));
	   setTuner (currentFrequency);
	   Display (currentFrequency);
//	   and we are on the move, so let the timer run
	   runTimer	-> start (50);
	   runMode = RUNNING;
	   return;
	}
//
//	We should not be here
}
	
static inline
DSPFLOAT	decayingAverage (DSPFLOAT old,
	                         DSPFLOAT input, DSPFLOAT weight) {
	if (weight <= 1)
	   return input;
	return input * (1.0 / weight) + old * (1.0 - (1.0 / weight));
}
//
//	we want to process inputRate / 10 length  segments,
//	which may amount to up to 800000 samples,
//	so the _I_Buffer should be large enough.
void	RadioInterface::handleSamples (void) {
DSPCOMPLEX	dataIn [spectrumSize];
int32_t i, j;

	if ((runMode != RUNNING) || (theDevice == NULL))
	   return;

	if (theDevice -> Samples () < inputRate / displayRate)
	   return;

	theDevice -> getSamples (dataIn, spectrumSize, inputRate / displayRate);
	for (i = 0; i < spectrumSize; i ++)
	   spectrumBuffer [i] = dataIn [i] * Window [i];
	spectrum_fft	-> do_FFT ();
//
//	first map the negative frequencies
	for (i = 0; i < displaySize / 2; i ++) {
	   displayBuffer [i] = 0;
	   for (j = 0; j < spectrumFactor; j ++)
	      displayBuffer [i] +=
	         abs (spectrumBuffer [spectrumSize / 2 + spectrumFactor * i + j]);
	   displayBuffer [i] /= spectrumFactor;
	}
//
//	then the positive ones
	for (i = 0; i < displaySize / 2; i ++) {
	   displayBuffer [displaySize / 2 + i] = 0;
	   for (j = 0; j < spectrumFactor; j ++)
	      displayBuffer [displaySize / 2 + i] +=
	         abs (spectrumBuffer [spectrumFactor * i + j]);
	   displayBuffer [displaySize / 2 + i] /= spectrumFactor;
	}
//
//	OK, the displaybuffer is set
	if (freezer)
	   doFreeze (displayBuffer, freezeBuffer, freezeCount ++);
	else
	   doFreeze (displayBuffer, freezeBuffer, 5);
//
//	and finally
	HFScope -> Display (X_axis,
	                    displayBuffer,
	                    spectrumAmplitudeSlider -> value (),
	                    currentFrequency / Khz (1));
}

//
void	RadioInterface::doFreeze (double *bufferIn,
	                          double *bufferRes, int32_t count) {

int32_t	i;

	for (i = 0; i < displaySize; i ++) {
	   if (bufferIn [i] == bufferIn [i])
	   bufferRes [i] =
	                ((double)(count - 1)) / count * bufferRes [i] +
	                1.0f / count * bufferIn [i];
	   bufferIn [i] = bufferRes [i];
	}
}
//
//	For displaying values, we use different scales, depending
//	on the size of the value
static inline
int32_t numberofDigits (int32_t f) {

	if (f < 100000)
	   return 6;
	if (f < 100000000)
	   return 8;
	if (f < 1000000000)
	   return 9;
	else
	   return 10;
}

void	RadioInterface::Display (int32_t freq, bool b) {
int32_t nd	= numberofDigits (freq);
	(void)b;
	lcd_Frequency	-> setDigitCount (nd);
	lcd_Frequency	-> display (freq);
}

void	RadioInterface::Display (int32_t freq) {
int32_t nd	= numberofDigits (freq);
	lcd_Frequency	-> setDigitCount (nd);
	lcd_Frequency	-> display ((int)freq / Khz (1));
}

void	RadioInterface::setViewmode (void) {
	if (HFViewmode == SPECTRUM_MODE)
	   HFViewmode = WATERFALL_MODE;
	else
	   HFViewmode = SPECTRUM_MODE;

	HFScope -> SelectView (HFViewmode);
}

void	RadioInterface::setFreezer	(void) {
int16_t	i;
	freezer = !freezer;
	if (freezer)
	   freezeButton	-> setText ("Freezing On");
	else 
	   freezeButton	-> setText ("Freeze");
	freezeCount = 5;
	for (i = 0; i < displaySize; i ++)
	   freezeBuffer [i] = 0;
}

void	RadioInterface::nextFrequency 	(void) {
int32_t temp = currentFrequency +  Khz (stepSize -> value ());
int32_t	lowerBound	= Mhz (lowerLimit -> value ());
int32_t	upperBound	= Mhz (upperLimit -> value ());

	if (!theDevice -> legalFrequency (lowerBound) ||
	    !theDevice -> legalFrequency (upperBound))
	   return;

	if (lowerBound >= upperBound)
	   return;

	if (temp > upperBound)
	   temp = lowerBound;
	if (temp < lowerBound)
	   temp = upperBound;

	setnextFrequency (temp);
}

void	RadioInterface::switchScanner	(void) {
	scanner	= !scanner;
	scanstartButton -> setText (scanner ? "scanning" : "set scan");
	if (scanner) {
	   scanTimer	-> start (scanDelayTime * 100);
	   if (freezer)
	      setFreezer ();	// will be switched off
	}
	else
	   scanTimer	-> stop ();
}

void	RadioInterface::setScanDelay	(int d) {
	scanDelayTime	= d;
	if (scanner) {
	   scanTimer	-> stop ();
	   scanTimer	-> start (scanDelayTime * 100);
	}
}

int32_t	RadioInterface::bandwidthFor (int32_t rate) {
	switch (rate) {
	   case 8000000:
	   case 7000000:
	   case 6000000:
	   case 5000000:
	      return rate;

	   default:
	      return 1536000;
	}
}

