#
/*
 *    Copyright (C) 2010, 2011, 2012, 2013
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
 *
 * 	This particular driver is a very simple wrapper around the
 * 	librtlsdr.  In order to keep things simple, we dynamically
 * 	load the dll (or .so). The librtlsdr is osmocom software and all rights
 * 	are greatly acknowledged
 */


#include	<QThread>
#include	"rtl-sdr.h"
#include	"rtlsdr-handler.h"

#ifdef	__MINGW32__
#define	GETPROCADDRESS	GetProcAddress
#else
#define	GETPROCADDRESS	dlsym
#endif

#define	READLEN_DEFAULT	8192
//
//	For the callback, we do need some environment which
//	is passed through the ctx parameter
//
//	This is the user-side call back function
//	ctx is the calling task
static
void	RTLSDRCallBack (uint8_t *buf, uint32_t len, void *ctx) {
rtlsdrHandler	*theStick	= (rtlsdrHandler *)ctx;
int32_t	tmp;

	if ((theStick == NULL) || (len != READLEN_DEFAULT))
	   return;

	tmp = theStick -> _I_Buffer -> putDataIntoBuffer (buf, len);
	if ((len - tmp) > 0)
	   theStick	-> sampleCounter += len - tmp;
}
//
//	for handling the events in libusb, we need a controlthread
//	whose sole purpose is to process the rtlsdr_read_async function
//	from the lib.
class	dll_driver : public QThread {
private:
	rtlsdrHandler	*theStick;
public:

	dll_driver (rtlsdrHandler *d) {
	theStick	= d;
	start ();
	}

	~dll_driver (void) {
	}

private:
virtual void	run (void) {
	(theStick -> rtlsdr_read_async) (theStick -> device,
	                          (rtlsdr_read_async_cb_t)&RTLSDRCallBack,
	                          (void *)theStick,
	                          0,
	                          READLEN_DEFAULT);
	}
};
//
//	Our wrapper is a simple classs
	rtlsdrHandler::rtlsdrHandler (QSettings *s) {
int16_t	deviceCount;
int32_t	r;
int16_t	deviceIndex;
int16_t	i;

	dabSettings		= s;
	this	-> myFrame	= new QFrame (NULL);
	setupUi (this -> myFrame);
	this	-> myFrame	-> show ();
	inputRate		=  Khz (rateSelector -> currentText (). toInt ());
	libraryLoaded			= false;
	open				= false;
	_I_Buffer			= NULL;
	this	-> sampleCounter	= 0;
	this	-> vfoOffset		= 0;
	gains				= NULL;

#ifdef	__MINGW32__
	const char *libraryString = "rtlsdr.dll";
	Handle		= LoadLibrary ((wchar_t *)L"rtlsdr.dll");
#else
	const char *libraryString = "librtlsdr.so";
	Handle		= dlopen ("librtlsdr.so", RTLD_NOW);
#endif

	if (Handle == NULL) {
	   fprintf (stderr, "failed to open %s\n", libraryString);
	   throw (21);
	}

	libraryLoaded	= true;
	if (!load_rtlFunctions ()) {
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	   throw (22);
	}

//
//	Ok, from here we have the library functions accessible
	deviceCount 		= this -> rtlsdr_get_device_count ();
	if (deviceCount == 0) {
	   fprintf (stderr, "No devices found\n");
	   rtlsdr_close (device);
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	}

	deviceIndex = 0;	// default
	if (deviceCount > 1) {
	   dongleSelector	= new dongleSelect ();
	   for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex ++) {
	      dongleSelector ->
	           addtoDongleList (rtlsdr_get_device_name (deviceIndex));
	   }
	   deviceIndex = dongleSelector -> QDialog::exec ();
	   delete dongleSelector;
	}
//
//	OK, now open the hardware
	r			= this -> rtlsdr_open (&device, deviceIndex);
	if (r < 0) {
	   fprintf (stderr, "Opening dabstick failed\n");
	   rtlsdr_close (device);
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	}

	open			= true;
	r			= this -> rtlsdr_set_sample_rate (device,
	                                                          inputRate);
	if (r < 0) {
	   fprintf (stderr, "Setting samplerate failed\n");
	   rtlsdr_close (device);
#ifdef __MINGW32__
	   FreeLibrary (Handle);
#else
	   dlclose (Handle);
#endif
	}

	r			= this -> rtlsdr_get_sample_rate (device);
	fprintf (stderr, "samplerate set to %d\n", r);

	gainsCount = rtlsdr_get_tuner_gains (device, NULL);
	fprintf(stderr, "Supported gain values (%d): ", gainsCount);
	gains		= new int [gainsCount];
	gainsCount = rtlsdr_get_tuner_gains (device, gains);
	gainSlider	-> setMaximum (gainsCount);
	for (i = gainsCount; i > 0; i--)
		fprintf(stderr, "%.1f ", gains [i - 1] / 10.0);
	rtlsdr_set_tuner_gain_mode (device, 1);
	rtlsdr_set_tuner_gain (device, gains [gainsCount / 2]);

	_I_Buffer		= new RingBuffer<uint8_t>(2048 * 1024);
	fprintf (stderr, "size = %d\n", _I_Buffer -> GetRingBufferWriteAvailable ());
	workerHandle		= NULL;
	connect (gainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (set_gainSlider (int)));
	connect (agcChecker, SIGNAL (stateChanged (int)),
	         this, SLOT (set_Agc (int)));
	connect (f_correction, SIGNAL (valueChanged (int)),
	         this, SLOT (freqCorrection  (int)));
	connect (rateSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (set_rateSelector (const QString &)));
	dabSettings	-> beginGroup ("dabstick");
	gainSlider -> setValue (dabSettings -> value ("gainSlider", 10). toInt ());
	f_correction -> setValue (dabSettings -> value ("f_correction", 0). toInt ());
	KhzOffset	-> setValue (dabSettings -> value ("KhzOffset", 0). toInt ());
	dabSettings	-> endGroup ();
	(void)(this -> rtlsdr_set_center_freq (device, defaultFrequency ()));
}

	rtlsdrHandler::~rtlsdrHandler	(void) {
	if (open)
	   this -> rtlsdr_close (device);
	if (_I_Buffer != NULL)
	   delete _I_Buffer;
	if (gains != NULL)
	   delete[] gains;

	dabSettings	-> beginGroup ("dabstick");
	dabSettings	-> setValue ("gainSlider", gainSlider -> value ());
	dabSettings	-> setValue ("f_correction", f_correction -> value ());
	dabSettings	-> setValue ("KhzOffset", KhzOffset -> value ());
	dabSettings	-> endGroup ();
	delete myFrame;
	open = false;
}

void	rtlsdrHandler::setVFOFrequency	(uint64_t f) {
	(void)(this -> rtlsdr_set_center_freq (device, (uint32_t)f + vfoOffset));
}

uint64_t rtlsdrHandler::getVFOFrequency	(void) {
	return (uint64_t)(this -> rtlsdr_get_center_freq (device)) - vfoOffset;
}

bool	rtlsdrHandler::legalFrequency (uint64_t f) {
	return  Mhz (1) <= f && f <= Mhz (1800);
}

uint64_t rtlsdrHandler::defaultFrequency	(void) {
	return Khz (94700);
}
//
//
bool	rtlsdrHandler::restartReader	(void) {
int32_t	r;

	if (workerHandle != NULL)
	   return true;

	_I_Buffer	-> FlushRingBuffer ();
	r = this -> rtlsdr_reset_buffer (device);
	if (r < 0)
	   return false;

	this -> rtlsdr_set_center_freq (device, 
	                   (int32_t)(this -> rtlsdr_get_center_freq (device)) +
	                                 vfoOffset);
	workerHandle	= new dll_driver (this);
	return true;
}

void	rtlsdrHandler::stopReader	(void) {
	if (workerHandle == NULL)
	   return;

	this -> rtlsdr_cancel_async (device);
	if (workerHandle != NULL) {
	   while (!workerHandle -> isFinished ()) 
	      usleep (100);

	   delete	workerHandle;
	}

	workerHandle	= NULL;
}
//
//	Note that this function is neither used for the
//	dabreceiver nor for the fmreceiver
//
int32_t	rtlsdrHandler::setRate	(int32_t newRate) {
int32_t	r;

	if (newRate < 900000) return inputRate;
	if (newRate > 3200000) return inputRate;

	if (workerHandle == NULL) {
	   r	= this -> rtlsdr_set_sample_rate (device, newRate);
	   if (r < 0) {
	      this -> rtlsdr_set_sample_rate (device, inputRate);
	   }

	   r	= this -> rtlsdr_get_sample_rate (device);
	}
	else {	
//	we stop the transmission first
	   stopReader ();
	   r	= this -> rtlsdr_set_sample_rate (device, newRate);
	   if (r < 0) {
	      this -> rtlsdr_set_sample_rate (device, inputRate);
	   }
	   r	= this -> rtlsdr_get_sample_rate (device);
	fprintf (stderr, "samplerate = %d\n", r);
//	ok all set, continue
//	   restartReader (); we need a proper restart from the user
	}

	inputRate	= newRate;
	return r;
}

void	rtlsdrHandler::set_gainSlider (int gain) {
static int	oldGain	= 0;

	if (gain == oldGain)
	   return;
	if ((gain < 0) || (gain >= gainsCount))
	   return;

	oldGain	= gain;
	rtlsdr_set_tuner_gain (device, gains [gainsCount - gain]);
	showGain -> display ((int)(rtlsdr_get_tuner_gain (device) / 10));
	return;
}


void	rtlsdrHandler::set_Agc	(int state) {
	if (agcChecker -> isChecked ())
	   (void)rtlsdr_set_agc_mode (device, 1);
	else
	   (void)rtlsdr_set_agc_mode (device, 0);
}

//
//	correction is in Hz
void	rtlsdrHandler::freqCorrection	(int32_t ppm) {
	this -> rtlsdr_set_freq_correction (device, ppm);
}

//
//	The brave old getSamples. For the dab stick, we get
//	size: still in I/Q pairs, but we have to convert the data from
//	uint8_t to DSPCOMPLEX *
int32_t	rtlsdrHandler::getSamples (DSPCOMPLEX *V, int32_t size) { 
int32_t	amount, i;
uint8_t	*tempBuffer = (uint8_t *)alloca (2 * size * sizeof (uint8_t));
//
	amount = _I_Buffer	-> getDataFromBuffer (tempBuffer, 2 * size);
	for (i = 0; i < amount / 2; i ++)
	    V [i] = DSPCOMPLEX ((float (tempBuffer [2 * i] - 127)) / 128.0,
	                        (float (tempBuffer [2 * i + 1] - 127)) / 128.0);
	return amount / 2;
}

//	and especially for our beloved spectrum viewer we provide
int32_t	rtlsdrHandler::getSamples 	(DSPCOMPLEX  *V,
	                         int32_t size, int32_t segmentSize) {
int32_t	amount, i;
uint8_t	*tempBuffer = (uint8_t *)alloca (2 * size * sizeof (uint8_t));
//
	amount = _I_Buffer	-> getDataFromBuffer (tempBuffer, 2 * size);
	for (i = 0; i < amount / 2; i ++)
	    V [i] = DSPCOMPLEX ((float (tempBuffer [2 * i] - 128)) / 128.0,
	                        (float (tempBuffer [2 * i + 1] - 128)) / 128.0);

	_I_Buffer	-> skipDataInBuffer (2 * (segmentSize - size));

	return amount / 2;
}

int32_t	rtlsdrHandler::Samples	(void) {
	return _I_Buffer	-> GetRingBufferReadAvailable () / 2;
}
//
//	vfoOffset is in Hz, we have two spinboxes influencing the
//	settings
void	rtlsdrHandler::setKhzOffset	(int k) {
	vfoOffset	= Khz (k);
}

bool	rtlsdrHandler::load_rtlFunctions (void) {
//
//	link the required procedures
	rtlsdr_open	= (pfnrtlsdr_open)
	                       GETPROCADDRESS (Handle, "rtlsdr_open");
	if (rtlsdr_open == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_open\n");
	   return false;
	}
	rtlsdr_close	= (pfnrtlsdr_close)
	                     GETPROCADDRESS (Handle, "rtlsdr_close");
	if (rtlsdr_close == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_close\n");
	   return false;
	}

	rtlsdr_set_sample_rate =
	    (pfnrtlsdr_set_sample_rate)GETPROCADDRESS (Handle, "rtlsdr_set_sample_rate");
	if (rtlsdr_set_sample_rate == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_set_sample_rate\n");
	   return false;
	}

	rtlsdr_get_sample_rate	=
	    (pfnrtlsdr_get_sample_rate)GETPROCADDRESS (Handle, "rtlsdr_get_sample_rate");
	if (rtlsdr_get_sample_rate == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_get_sample_rate\n");
	   return false;
	}

	rtlsdr_set_agc_mode =
	    (pfnrtlsdr_set_agc_mode)GETPROCADDRESS (Handle, "rtlsdr_set_agc_mode");
	if (rtlsdr_set_agc_mode == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_set_agc_mode\n");
	   return false;
	}
	

	rtlsdr_get_tuner_gains		= (pfnrtlsdr_get_tuner_gains)
	                     GETPROCADDRESS (Handle, "rtlsdr_get_tuner_gains");
	if (rtlsdr_get_tuner_gains == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_get_tuner_gains\n");
	   return false;
	}


	rtlsdr_set_tuner_gain_mode	= (pfnrtlsdr_set_tuner_gain_mode)
	                     GETPROCADDRESS (Handle, "rtlsdr_set_tuner_gain_mode");
	if (rtlsdr_set_tuner_gain_mode == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_set_tuner_gain_mode\n");
	   return false;
	}

	rtlsdr_set_tuner_gain	= (pfnrtlsdr_set_tuner_gain)
	                     GETPROCADDRESS (Handle, "rtlsdr_set_tuner_gain");
	if (rtlsdr_set_tuner_gain == NULL) {
	   fprintf (stderr, "Cound not find rtlsdr_set_tuner_gain\n");
	   return false;
	}

	rtlsdr_get_tuner_gain	= (pfnrtlsdr_get_tuner_gain)
	                     GETPROCADDRESS (Handle, "rtlsdr_get_tuner_gain");
	if (rtlsdr_get_tuner_gain == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_get_tuner_gain\n");
	   return false;
	}
	rtlsdr_set_center_freq	= (pfnrtlsdr_set_center_freq)
	                     GETPROCADDRESS (Handle, "rtlsdr_set_center_freq");
	if (rtlsdr_set_center_freq == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_set_center_freq\n");
	   return false;
	}

	rtlsdr_get_center_freq	= (pfnrtlsdr_get_center_freq)
	                     GETPROCADDRESS (Handle, "rtlsdr_get_center_freq");
	if (rtlsdr_get_center_freq == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_get_center_freq\n");
	   return false;
	}

	rtlsdr_reset_buffer	= (pfnrtlsdr_reset_buffer)
	                     GETPROCADDRESS (Handle, "rtlsdr_reset_buffer");
	if (rtlsdr_reset_buffer == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_reset_buffer\n");
	   return false;
	}

	rtlsdr_read_async	= (pfnrtlsdr_read_async)
	                     GETPROCADDRESS (Handle, "rtlsdr_read_async");
	if (rtlsdr_read_async == NULL) {
	   fprintf (stderr, "Cound not find rtlsdr_read_async\n");
	   return false;
	}

	rtlsdr_get_device_count	= (pfnrtlsdr_get_device_count)
	                     GETPROCADDRESS (Handle, "rtlsdr_get_device_count");
	if (rtlsdr_get_device_count == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_get_device_count\n");
	   return false;
	}

	rtlsdr_cancel_async	= (pfnrtlsdr_cancel_async)
	                     GETPROCADDRESS (Handle, "rtlsdr_cancel_async");
	if (rtlsdr_cancel_async == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_cancel_async\n");
	   return false;
	}

	rtlsdr_set_direct_sampling = (pfnrtlsdr_set_direct_sampling)
	                  GETPROCADDRESS (Handle, "rtlsdr_set_direct_sampling");
	if (rtlsdr_set_direct_sampling == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_set_direct_sampling\n");
	   return false;
	}

	rtlsdr_set_freq_correction = (pfnrtlsdr_set_freq_correction)
	                  GETPROCADDRESS (Handle, "rtlsdr_set_freq_correction");
	if (rtlsdr_set_freq_correction == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_set_freq_correction\n");
	   return false;
	}
	
	rtlsdr_get_device_name = (pfnrtlsdr_get_device_name)
	                  GETPROCADDRESS (Handle, "rtlsdr_get_device_name");
	if (rtlsdr_get_device_name == NULL) {
	   fprintf (stderr, "Could not find rtlsdr_get_device_name\n");
	   return false;
	}

	fprintf (stderr, "OK, functions seem to be loaded\n");
	return true;
}

int16_t	rtlsdrHandler::bitDepth	(void) {
	return 8;
}

int32_t	rtlsdrHandler::getRate	(void) {
	return inputRate;
}

void	rtlsdrHandler::set_rateSelector (const QString &s) {
int32_t v	= s. toInt ();

	setRate		(Khz (v));
	set_changeRate	(Khz (v));		// and signal the main program
}

