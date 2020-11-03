#
/*
 *    Copyright (C) 2014 - 2018
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
#include	"viewer.h"
#include	<QSettings>
#include	<QMessageBox>
#include	<QFileDialog>
#include	<QDebug>
#include	<QDateTime>
#include	"device-handler.h"
#include	"decimator.h"
#include	"popup-keypad.h"
#ifdef	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#endif
#ifdef	HAVE_PLUTO
#include	"pluto-handler.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
#ifdef	HAVE_LIME
#include	"lime-handler.h"
#endif
#ifdef	HAVE_ELAD_S1
#include	"elad-s1.h"
#endif
#include	"spectrum-scope.h"
#include	"waterfall-scope.h"
#ifdef __MINGW32__
#include	<iostream>
#endif
#

/*
 *	We use the creation function merely to set up the
 *	user interface and make the connections between the
 *	gui elements and the handling agents. All real action
 *	is embedded in actions, initiated by gui buttons
 */
	Viewer::Viewer (QSettings	*Si,
	                QWidget		*parent): QDialog (parent) {
int k;
// 	the setup for the generated part of the ui
	setupUi (this);
	spectrumSettings		= Si;
	this	-> rasterSize		= 50;
	this	-> displaySize =
	             spectrumSettings -> value ("displaySize", 4096). toInt ();
//	if ((displaySize & (displaySize - 1)) != 0)
//	   displaySize	= 1024;

	HFScope_1	= new spectrumScope (hf_spectrumscope,
	                                           displaySize);
	connect (HFScope_1, SIGNAL (leftClicked (int)),
	         this, SLOT (adjustFrequency (int)));

	HFScope_2	= new waterfallScope (hf_waterfallscope, displaySize,
	                                                         rasterSize);
	connect (HFScope_2, SIGNAL (leftClicked (int)),
	         this, SLOT (adjustFrequency (int)));

	IFScope		= new spectrumScope (detailScope, 4 * displaySize);
	connect (IFScope, SIGNAL (leftClicked (int)),
	         this, SLOT (adjustinLoupe (int)));
	signalView	= new SignalView (signalScope);

	theMapper	= new freqmapper (displaySize);
	theMapper_2	= new freqmapper (4 * displaySize);
	theDevice	= setDevice ();
	if (theDevice == nullptr) {
	   fprintf (stderr, "no device found\n");
	   delete IFScope;
	   delete HFScope_2;
	   delete HFScope_1;
	   exit (21);
	}
	theDevice	-> setVFOFrequency (theDevice -> defaultFrequency ());
	currentFrequency	= theDevice -> defaultFrequency ();
	HFScope_1	-> setBitDepth (theDevice -> bitDepth ());
	IFScope		-> setBitDepth (theDevice -> bitDepth ());

//	set some sliders to their values
	k	= spectrumSettings -> value ("spectrumAmplitudeSlider", 20). toInt ();
	spectrumAmplitudeSlider	-> setValue (k);
	k	= spectrumSettings -> value ("lowerLimit", 86). toInt ();
	lowerLimit		-> setValue (k);

	k	= spectrumSettings -> value ("upperLimit", 110). toInt ();
	upperLimit		-> setValue (k);

	k	= spectrumSettings -> value ("stepSize", 500). toInt ();
	stepSize		-> setValue (k);

	k	= spectrumSettings -> value ("delayBox", 10). toInt ();
	delayBox		-> setValue (k);
	scanDelayTime		= k;

	ClearPanel		();

//	connect (decimationSelector, SIGNAL (activated (QString)),
//	         this,  SLOT (decimationHandler (QString)));
	connect (decimationSpinner, SIGNAL (valueChanged (int)),
	         this, SLOT (decimationHandler (int)));

	connect (pauseButton, SIGNAL (clicked (void)),
	         this, SLOT (clickPause (void)));

	mykeyPad	= new keyPad (this);
	connect (freqButton, SIGNAL (clicked (void)),
	         this, SLOT (handle_freqButton (void)));

	displayRate	= 10;
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

	QString t = QString ("spectrumviewer ");
	t. append (CURRENT_VERSION);
//	systemindicator		-> setText (t);
	displayTimer		-> start (1000);

	
//	all elements seem to exist: set the control state to
//	its default values
	running. store (false);
	setStart ();
}

	Viewer::~Viewer (void) {
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
	delete		mykeyPad;
	delete		lcd_timer;
	delete		HFScope_1;
	delete		HFScope_2;
	delete		IFScope;
	delete		displayTimer;
	delete		runTimer;
}

deviceHandler	*Viewer::setDevice (void) {
deviceHandler	*theDevice;
#ifdef	HAVE_SDRPLAY
	try {
	   theDevice	= new sdrplayHandler (spectrumSettings);
	   connect (theDevice, SIGNAL (set_changeRate (int)),
	            this, SLOT (set_changeRate (int)));
	   return theDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_PLUTO
	try {
	   theDevice	= new plutoHandler (spectrumSettings);
	   connect (theDevice, SIGNAL (set_changeRate (int)),
	            this, SLOT (set_changeRate (int)));
	   return theDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_RTLSDR
	try {
	   theDevice	= new rtlsdrHandler (spectrumSettings);
	   return theDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_AIRSPY
	try {
	   theDevice	= new airspyHandler (spectrumSettings);
	   return theDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_HACKRF
	try {
	   theDevice	= new hackrfHandler (spectrumSettings);
	   return theDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_LIME
	try {
	   theDevice	= new limeHandler	(spectrumSettings);
	   return theDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_ELAD_S1
	try {
	   theDevice	= new eladHandler (spectrumSettings, 3072000);
	   return theDevice;
	} catch (int e) {
	}
#endif
	fprintf (stderr, "could not detect a device\n");
	return nullptr;
}
//
void	Viewer::set_changeRate (int newRate) {
	lcd_rate_display	-> display (theDevice -> getRate ());
	fprintf (stderr, "newRate = %d\n", newRate);
	(void)newRate;
}

//
//	starting is used to start and to restart after selecting
//	a different bandwidth.
//	We assume the system is not running

void	Viewer::setStart (void) {

	if (running. load ())
	   setStop ();
//
//	parameters to use a bandwidth, frequency and decimation
//	to be done,
	theDevice	-> setVFOFrequency (currentFrequency);
	Display (currentFrequency);
	lcd_rate_display	-> display (theDevice -> getRate ());
	theDecimator	= new decimator (theDevice -> getRate (), 20);
	bool r = theDevice	-> restartReader ();
	if (!r) {
	   QMessageBox::warning (this, tr ("sdr"),
	                               tr ("Opening  device failed\n"));
	   exit (102);
	}
//	and we are on the move, so let the timer run
	runTimer	-> start (1000 / displayRate);
	running. store (true);
}

void	Viewer::setStop	(void) {
	if (!running. load ())
	   return;
	scanTimer	-> stop ();
	theDevice	-> stopReader ();
	runTimer	-> stop ();
	if (theDecimator != nullptr)
	   delete	theDecimator;
	theDecimator	= nullptr;
	running. store (false);
}

void	Viewer::TerminateProcess () {
	setStop ();
	accept ();
	delete	theDevice;
	qDebug () <<  "End of termination procedure";
}

//	In AddtoPanel, we might have any of the two
//	timers running, do not know which one though
void	Viewer::AddtoPanel (int16_t n) {
	stop_lcdTimer	();
	Panel		= 10 * Panel + n;
	Display (Panel, true);
	lcd_timer	-> start (5000);		// restart timer
}

void	Viewer::ClearPanel (void) {
	Panel = 0;
}

uint64_t	Viewer::getPanel (void) {
	return Panel;
}

void	Viewer::adjustFrequency (int n) {
	stop_lcdTimer ();
	setTuner (theDevice -> getVFOFrequency () + Khz (n));
	Display (currentFrequency);
}

void	Viewer::adjustinLoupe	(int n) {
int	rate	= theDecimator	-> rateOut ();
	stop_lcdTimer ();

	setTuner (theDevice -> getVFOFrequency () + n * rate / 100);
	Display (currentFrequency);
}


//	SetTuner accepts freq in Hz.
//
void	Viewer::setTuner (uint64_t n) {
//	if (!theDevice -> legalFrequency (n - inputRate / 2) ||
//	    !theDevice -> legalFrequency (n + inputRate / 2))
//	   return;
	scanTimer	-> stop ();
	theDevice	-> setVFOFrequency (n);
	currentFrequency = theDevice -> getVFOFrequency ();
}
//
//	
void	Viewer::setnextFrequency (uint64_t n) {

	if (!theDevice -> legalFrequency (n - inputRate / 2) ||
	    !theDevice -> legalFrequency (n + inputRate / 2))
	   return;
	theDevice	-> setVFOFrequency (n);
	currentFrequency	= theDevice -> getVFOFrequency ();

	Display (currentFrequency);
}
//
void	Viewer::stop_lcdTimer (void) {
	if (lcd_timer -> isActive ())
	   lcd_timer -> stop ();
}

void	Viewer::updateTimeDisplay (void) {
QDateTime	currentTime = QDateTime::currentDateTime ();

	timeDisplay	-> setText (currentTime.
	                            toString (QString ("dd.MM.yy:hh:mm:ss")));
}

void	Viewer::lcd_timeout (void) {
	Panel		= 0;			// throw away anything
	Display (currentFrequency / KHz (1));
}

void	Viewer::clickPause (void) {
}
	
//
//	we want to process inputRate / 10 length  segments,
//	which may amount to up to 800000 samples,
//	so the _I_Buffer should be large enough.
void	Viewer::handleSamples (void) {
std::complex<float>	dataIn [displaySize];
int32_t i;
double showDisplay [displaySize];
	currentFrequency	= theDevice -> getVFOFrequency ();

	if (!running. load ())
	   return;

	if (theDevice -> Samples () < inputRate / displayRate)
	   return;

	theDevice -> getSamples (dataIn, displaySize);
	theMapper	-> convert (dataIn, showDisplay);

//	we have the buffer, the scopes can do the work
	HFScope_1	-> showFrame (showDisplay,
	                              theDevice	-> getRate (),
                                      currentFrequency,
	                              spectrumAmplitudeSlider -> value ());
	HFScope_2	-> showFrame (showDisplay,
	                              theDevice	-> getRate (),
	                              currentFrequency,
	                              spectrumAmplitudeSlider -> value ());
	int decimationFactor	= decimationSpinner -> value ();
	int bufferSize		= decimationFactor * displaySize;
	if (theDevice -> Samples () < bufferSize)
	   return;

	int fillP = 0;
	std::complex<float> xbuf [4 * displaySize];
	double ybuf [4 * displaySize];
	while (true) {
	   std::complex<float> temp;
	   std::complex<float> lbuf [displaySize];
	   double theDisplay [4 * displaySize];
	   theDevice	-> getSamples (lbuf, displaySize);
	   for (int i = 0; i < displaySize; i ++) {
	      if (theDecimator -> Pass (lbuf [i], &temp)) {
	         xbuf [fillP] = temp;
	         ybuf [fillP] = real (temp);
	         fillP ++;
	         if (fillP >= 4 * displaySize) {
	            theMapper_2	-> convert (xbuf, theDisplay);
	            IFScope	-> showFrame (theDisplay,
	                                      theDevice -> getRate () / decimationFactor,
	                                      theDevice -> getVFOFrequency (),
	                                      spectrumAmplitudeSlider -> value ());
	            signalView	-> showFrame (ybuf, displaySize);
	            theDevice -> resetBuffer ();
	            return;
	         }
	      }
	   }
	}
}

//
void	Viewer::doFreeze (double *bufferIn,
	                          double *bufferRes, int32_t count) {
	(void)bufferIn; (void)bufferRes;(void)count;
}
//
//	For displaying values, we use different scales, depending
//	on the size of the value
static inline
int32_t numberofDigits (uint64_t f) {

	if (f < 100000)
	   return 6;
	if (f < 100000000)
	   return 8;
	if (f < 1000000000)
	   return 9;
	else
	   return 10;
}

void	Viewer::Display (uint64_t freq, bool b) {
int32_t nd	= numberofDigits (freq);
	(void)b;
	lcd_freq_display	-> setDigitCount (nd);
	lcd_freq_display	-> display ((int)((uint64_t)freq));
}

void	Viewer::Display (uint64_t freq) {
int32_t nd	= numberofDigits (freq);
	lcd_freq_display	-> setDigitCount (nd);
	lcd_freq_display	-> display ((int)((uint64_t)freq / Khz (1)));
}

void	Viewer::toggle_Freezer	(void) {
}

//
void	Viewer::newFrequency	(qint64 newFreq) {
	setTuner (newFreq);
        Display (currentFrequency);
}

void	Viewer::decimationHandler	(QString s) {
int	decimate	= s. toInt ();
	delete theDecimator;
	theDecimator	= new decimator (theDevice -> getRate (), decimate);
}

void	Viewer::decimationHandler	(int amount) {
	delete theDecimator;
	theDecimator	= new decimator (theDevice -> getRate (), amount);
}
//
//      Whenever the mousewheel is changed, the frequency
//      is adapted
void    Viewer::wheelEvent (QWheelEvent *e) {
        if (e -> delta () > 0)
           adjustFrequency (mouseIncrement -> value ());
        else
           adjustFrequency (-mouseIncrement -> value ());
}

#include <QCloseEvent>
void Viewer::closeEvent (QCloseEvent *event) {

        QMessageBox::StandardButton resultButton =
                        QMessageBox::question (this, "spectrumViewer",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
        if (resultButton != QMessageBox::Yes) {
           event -> ignore();
        } else {
           TerminateProcess ();
           event -> accept ();
        }
}

/////////////////////////////////////////////////////////////////////////
//
static inline
bool    frequencyInBounds (int32_t f, int32_t l, int32_t u) {
        return l <= f && f <= u;
}

void	Viewer::switchScanner	(void) {
	if (scanTimer -> isActive ())
	   scanTimer -> stop ();
	else
	   scanTimer -> start (delayBox -> value () * 1000);
}

void	Viewer::nextFrequency (void) {
int32_t	frequency;
int32_t	low, high;

	low	= MHz (lowerLimit -> value ());
	high	= MHz (upperLimit -> value ());
	frequency	= theDevice -> getVFOFrequency () + 
	                               KHz (stepSize -> value ());
	if ((stepSize -> value () < 0) &&
	   !frequencyInBounds (frequency, low, high))
	   frequency = high;

	if ((stepSize -> value () > 0) &&
	   !frequencyInBounds (frequency, low, high))
	   frequency = low;

	setTuner (frequency);
	Display (theDevice -> getVFOFrequency ());
	scanTimer	-> start (delayBox -> value () * 1000);
}

void    Viewer::handle_freqButton (void) {
        if (mykeyPad -> isVisible ())
           mykeyPad -> hidePad ();
        else
           mykeyPad     -> showPad ();
}

