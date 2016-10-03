#
/*
 *    Copyright (C) 2014
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
 */

#include	<QThread>
#include	<QSettings>
#include	<QHBoxLayout>
#include	<QLabel>
#include	"sdrplay.h"

#define	DEFAULT_GAIN	40

	sdrplay::sdrplay  (QSettings *s, bool *success) {
int	err;
float	ver;
QString	str;
int	k;

	sdrplaySettings		= s;
	this	-> myFrame	= new QFrame (NULL);
	setupUi (this -> myFrame);
	this	-> myFrame	-> show ();
	*success		= false;
	_I_Buffer	= NULL;
	libraryLoaded	= false;
	*success	= false;

#ifdef	__MINGW32__
HKEY APIkey;
wchar_t APIkeyValue [256];
ULONG APIkeyValue_length = 255;
	if (RegOpenKey (HKEY_LOCAL_MACHINE,
	                TEXT("Software\\MiricsSDR\\API"),
	                &APIkey) != ERROR_SUCCESS) {
          fprintf (stderr,
	           "failed to locate API registry entry, error = %d\n",
	           (int)GetLastError());
	   return;
	}
	RegQueryValueEx (APIkey,
	                 (wchar_t *)L"Install_Dir",
	                 NULL,
	                 NULL,
	                 (LPBYTE)&APIkeyValue,
	                 (LPDWORD)&APIkeyValue_length);
//	Ok, make explicit it is in the 64 or 32 bits section
	wchar_t *x = wcscat (APIkeyValue, (wchar_t *)L"\\x86\\mir_sdr_api.dll");
//	wchar_t *x = wcscat (APIkeyValue, (wchar_t *)L"\\x64\\mir_sdr_api.dll");
//	fprintf (stderr, "Length of APIkeyValue = %d\n", APIkeyValue_length);
//	wprintf (L"API registry entry: %s\n", APIkeyValue);
	RegCloseKey(APIkey);

	Handle	= LoadLibrary (x);
	if (Handle == NULL) {
	  fprintf (stderr, "Failed to open mir_sdr_api.dll\n");
	  return;
	}
#else
//	Ǹote that under Ubuntu, the Mirics shared object does not seem to be
//	able to find the libusb. That is why we explicity load it here
	Handle		= dlopen ("libusb-1.0.so", RTLD_NOW | RTLD_GLOBAL);

	Handle		= dlopen ("libmirsdrapi-rsp.so", RTLD_NOW);
	if (Handle == NULL)
	   Handle	= dlopen ("libmir_sdr.so", RTLD_NOW);

	if (Handle == NULL) {
	   fprintf (stderr, "error report %s\n", dlerror ());
	   return;
	}
#endif
//
//	OK, library is loaded, now the functions
	libraryLoaded	= true;

	*success = loadFunctions ();
	if (!(*success)) {
	   fprintf (stderr, " No success in loading sdrplay lib\n");
	   return;
	}

	err			= my_mir_sdr_ApiVersion (&ver);
	if (ver != MIR_SDR_API_VERSION) {
	   fprintf (stderr, "Foute API: %f, %d\n", ver, err);
	   statusLabel	-> setText ("mirics error");
	}

	api_version	-> display (ver);
	_I_Buffer	= new RingBuffer<DSPCOMPLEX>(2 * 1024 * 1024);
	vfoFrequency	= Khz (94700);
	currentGain	= DEFAULT_GAIN;
	vfoOffset	= 0;

	sdrplaySettings		-> beginGroup ("sdrplaySettings");
	gainSlider 		-> setValue (
	            sdrplaySettings -> value ("externalGain", 10). toInt ());
	str		= 
	            sdrplaySettings -> value ("sdrplayRate", 960000). toString ();
	k		= rateSelector -> findText (str);
	if (k != -1) 
	   rateSelector	-> setCurrentIndex (k);
	sdrplaySettings	-> endGroup ();
//
//	Since the device is not actually running when we are here,
//	we just keep the values for later use
	inputRate	= rateSelector -> currentText (). toInt ();
	currentGain	= gainSlider	-> value ();
	gainDisplay	-> display (currentGain);

	connect (gainSlider, SIGNAL (valueChanged (int)),
	         this, SLOT (setExternalGain (int)));
	connect (agcControl, SIGNAL (stateChanged (int)),
	         this, SLOT (agcControl_toggled (int)));
	connect (rateSelector, SIGNAL (activated (const QString &)),
	         this, SLOT (setExternalRate (const QString &)));

	running		= false;
	agcMode		= false;
	*success	= true;
}

	sdrplay::~sdrplay	(void) {
	sdrplaySettings	-> beginGroup ("sdrplaySettings");
	sdrplaySettings	-> setValue ("externalGain", gainSlider -> value ());
	sdrplaySettings	-> setValue ("sdrplayRate", rateSelector -> currentText ());
	sdrplaySettings	-> endGroup ();
	stopReader ();
	if (_I_Buffer != NULL)
	   delete _I_Buffer;
	delete	myFrame;
}
//
static inline
int16_t	bankFor_sdr (int32_t freq) {
	if (freq < 12 * MHz (1))
	   return 1;
	if (freq < 30 * MHz (1))
	   return 2;
	if (freq < 60 * MHz (1))
	   return 3;
	if (freq < 120 * MHz (1))
	   return 4;
	if (freq < 250 * MHz (1))
	   return 5;
	if (freq < 420 * MHz (1))
	   return 6;
	if (freq < 1000 * MHz (1))
	   return 7;
	if (freq < 2000 * MHz (1))
	   return 8;
	return -1;
}

static inline
mir_sdr_Bw_MHzT bandwidth_for (int32_t rate) {

	if (rate < Khz (300))
	   return mir_sdr_BW_0_200;
	if (rate < Khz (600))
	   return mir_sdr_BW_0_300;
	if (rate < Khz (1536))
	   return mir_sdr_BW_0_600;
	if (rate < Khz (5000))
	   return mir_sdr_BW_1_536;
	if (rate < Khz (6000))
	   return mir_sdr_BW_5_000;
	if (rate < Khz (7000))
	   return mir_sdr_BW_6_000;
	if (rate < Khz (8000))
	   return mir_sdr_BW_7_000;
	else
	   return mir_sdr_BW_8_000;
}

bool	sdrplay::legalFrequency (int32_t f) {
	return (bankFor_sdr (f) != -1);
}

int32_t	sdrplay::defaultFrequency	(void) {
	return Khz (94700);
}

void	sdrplay::setVFOFrequency	(int32_t newFrequency) {
mir_sdr_ErrT	err;
int32_t	realFreq = newFrequency + vfoOffset;
int	gRdBSystem;
int	samplesPerPacket;

	if (bankFor_sdr (realFreq) == -1)
	   return;

	if (!running) {
	   vfoFrequency = newFrequency + vfoOffset;
	   return;
	}

	if (bankFor_sdr (realFreq) != bankFor_sdr (vfoFrequency)) {
	   err = my_mir_sdr_Reinit (&currentGain,
	                            double (inputRate) / Mhz (1),
	                            double (realFreq) / Mhz (1),
	                            bandwidth_for (inputRate),
	                            mir_sdr_IF_Zero,
	                            mir_sdr_LO_Undefined,	// LOMode
	                            0,	// LNA enable
	                            &gRdBSystem,
	                            agcMode,	
	                            &samplesPerPacket,
	                            mir_sdr_CHANGE_RF_FREQ);
	}
	else
	   err =  my_mir_sdr_SetRf (double (vfoFrequency), 1, 0);

	if (err != mir_sdr_Success)
	   fprintf (stderr, "Error %d in frequency change to %d\n",
	                             err, realFreq);
	else
	   vfoFrequency = realFreq;
}

int32_t	sdrplay::getVFOFrequency	(void) {
	return vfoFrequency - vfoOffset;
}

void	sdrplay::setExternalGain	(int newGain) {
	if (newGain < 0 || newGain > 102)
	   return;
	currentGain = newGain;
	if (running)
	   my_mir_sdr_SetGr (currentGain, 1, 0);
	gainDisplay	-> display (currentGain);
}

void	sdrplay::setExternalRate	(const QString &s) {
int rate	= s. toInt ();
mir_sdr_ErrT	err;
int	gRdBSystem;
int	samplesPerPacket;

	if (rate == inputRate)
	   return;

	inputRate	= rate;
	if (!running)		// save values for later
	   return;
	
	stopReader ();
	restartReader ();
	set_changeRate (inputRate);
}

int16_t	sdrplay::maxGain	(void) {
	return 101;
}

int32_t	sdrplay::getRate	(void) {
	return inputRate;
}

static
void myStreamCallback (int16_t		*xi,
	               int16_t		*xq,
	               uint32_t		firstSampleNum, 
	               int32_t		grChanged,
	               int32_t		rfChanged,
	               int32_t		fsChanged,
	               uint32_t		numSamples,
	               uint32_t		reset,
	               void		*cbContext) {
int16_t	i;
sdrplay	*p	= static_cast<sdrplay *> (cbContext);
DSPCOMPLEX localBuf [numSamples];

	for (i = 0; i <  (int)numSamples; i ++)
	   localBuf [i] = DSPCOMPLEX (float (xi [i]) / 2048.0,
	                              float (xq [i]) / 2048.0);
	p -> _I_Buffer -> putDataIntoBuffer (localBuf, numSamples);
	(void)	firstSampleNum;
	(void)	grChanged;
	(void)	rfChanged;
	(void)	fsChanged;
	(void)	reset;
}

void	myGainChangeCallback (uint32_t	gRdB,
	                      uint32_t	lnaGRdB,
	                      void	*cbContext) {
	(void)gRdB;
	(void)lnaGRdB;	
	(void)cbContext;
}


bool	sdrplay::restartReader	(void) {
int	gRdBSystem;
int	samplesPerPacket;
mir_sdr_ErrT	err;

	if (running)
	   return true;

	err	= my_mir_sdr_StreamInit (&currentGain,
	                                 double (inputRate) / MHz (1),
	                                 double (vfoFrequency) / Mhz (1),
	                                 bandwidth_for (inputRate),
	                                 mir_sdr_IF_Zero,
	                                 0,	// lnaEnable do not know yet
	                                 &gRdBSystem,
	                                 agcMode, // useGrAltMode,do not know yet
	                                 &samplesPerPacket,
	                                 (mir_sdr_StreamCallback_t)myStreamCallback,
	                                 (mir_sdr_GainChangeCallback_t)myGainChangeCallback,
	                                 this);
	if (err != mir_sdr_Success) {
	   fprintf (stderr, "problem with streamInit %d\n", err);
	   return false;
	}

	err		= my_mir_sdr_SetDcMode (4, 1);
	err		= my_mir_sdr_SetDcTrackTime (63);
//
	my_mir_sdr_SetSyncUpdatePeriod ((int)(inputRate / 2));
	my_mir_sdr_SetSyncUpdateSampleNum (samplesPerPacket);
//	my_mir_sdr_AgcControl (1, -30, 0, 0, 0, 0, 0);
	my_mir_sdr_DCoffsetIQimbalanceControl (0, 1);
	running 	= true;
	return true;
}

void	sdrplay::stopReader	(void) {
	if (!running)
	   return;

	my_mir_sdr_StreamUninit	();
	running		= false;
}

//
//	The brave old getSamples. For the mirics stick, we get
//	size still in I/Q pairs
//	Note that the sdrPlay returns 10 bit values
int32_t	sdrplay::getSamples (DSPCOMPLEX *V, int32_t size) { 
//
	return _I_Buffer	-> getDataFromBuffer (V, size);
}

//	and especially for our beloved spectrum viewer we provide
int32_t	sdrplay::getSamples 	(DSPCOMPLEX  *V,
	                         int32_t size, int32_t segmentSize) {
int32_t	amount;
	amount = _I_Buffer	-> getDataFromBuffer (V, size);
	_I_Buffer	-> skipDataInBuffer (segmentSize - size);
	return amount;
}

int32_t	sdrplay::Samples	(void) {
	return _I_Buffer	-> GetRingBufferReadAvailable ();
}

uint8_t	sdrplay::myIdentity	(void) {
	return 0;
}

void	sdrplay::resetBuffer	(void) {
	_I_Buffer	-> FlushRingBuffer ();
}

int16_t	sdrplay::bitDepth	(void) {
	return 14;
}

bool	sdrplay::loadFunctions	(void) {

	my_mir_sdr_StreamInit	= (pfn_mir_sdr_StreamInit)
	                    GETPROCADDRESS (this -> Handle,
	                                    "mir_sdr_StreamInit");
	if (my_mir_sdr_StreamInit == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_StreamInit\n");
	   return false;
	}

	my_mir_sdr_StreamUninit	= (pfn_mir_sdr_StreamUninit)
	                    GETPROCADDRESS (this -> Handle,
	                                    "mir_sdr_StreamUninit");
	if (my_mir_sdr_StreamUninit == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_StreamUninit\n");
	   return false;
	}

	my_mir_sdr_SetRf	= (pfn_mir_sdr_SetRf)
	                    GETPROCADDRESS (Handle, "mir_sdr_SetRf");
	if (my_mir_sdr_SetRf == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetRf\n");
	   return false;
	}

	my_mir_sdr_SetFs	= (pfn_mir_sdr_SetFs)
	                    GETPROCADDRESS (Handle, "mir_sdr_SetFs");
	if (my_mir_sdr_SetFs == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetFs\n");
	   return false;
	}

	my_mir_sdr_SetGr	= (pfn_mir_sdr_SetGr)
	                    GETPROCADDRESS (Handle, "mir_sdr_SetGr");
	if (my_mir_sdr_SetGr == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetGr\n");
	   return false;
	}

	my_mir_sdr_SetGrParams	= (pfn_mir_sdr_SetGrParams)
	                    GETPROCADDRESS (Handle, "mir_sdr_SetGrParams");
	if (my_mir_sdr_SetGrParams == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetGrParams\n");
	   return false;
	}

	my_mir_sdr_SetDcMode	= (pfn_mir_sdr_SetDcMode)
	                    GETPROCADDRESS (Handle, "mir_sdr_SetDcMode");
	if (my_mir_sdr_SetDcMode == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetDcMode\n");
	   return false;
	}

	my_mir_sdr_SetDcTrackTime	= (pfn_mir_sdr_SetDcTrackTime)
	                    GETPROCADDRESS (Handle, "mir_sdr_SetDcTrackTime");
	if (my_mir_sdr_SetDcTrackTime == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetDcTrackTime\n");
	   return false;
	}

	my_mir_sdr_SetSyncUpdateSampleNum = (pfn_mir_sdr_SetSyncUpdateSampleNum)
	               GETPROCADDRESS (Handle, "mir_sdr_SetSyncUpdateSampleNum");
	if (my_mir_sdr_SetSyncUpdateSampleNum == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetSyncUpdateSampleNum\n");
	   return false;
	}

	my_mir_sdr_SetSyncUpdatePeriod	= (pfn_mir_sdr_SetSyncUpdatePeriod)
	                GETPROCADDRESS (Handle, "mir_sdr_SetSyncUpdatePeriod");
	if (my_mir_sdr_SetSyncUpdatePeriod == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_SetSyncUpdatePeriod\n");
	   return false;
	}

	my_mir_sdr_ApiVersion	= (pfn_mir_sdr_ApiVersion)
	                GETPROCADDRESS (Handle, "mir_sdr_ApiVersion");
	if (my_mir_sdr_ApiVersion == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_ApiVersion\n");
	   return false;
	}

	my_mir_sdr_AgcControl	= (pfn_mir_sdr_AgcControl)
	                GETPROCADDRESS (Handle, "mir_sdr_AgcControl");
	if (my_mir_sdr_AgcControl == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_AgcControl\n");
	   return false;
	}

	my_mir_sdr_Reinit	= (pfn_mir_sdr_Reinit)
	                GETPROCADDRESS (Handle, "mir_sdr_Reinit");
	if (my_mir_sdr_Reinit == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_Reinit\n");
	   return false;
	}

	my_mir_sdr_DCoffsetIQimbalanceControl	=
	                     (pfn_mir_sdr_DCoffsetIQimbalanceControl)
	                GETPROCADDRESS (Handle, "mir_sdr_DCoffsetIQimbalanceControl");
	if (my_mir_sdr_DCoffsetIQimbalanceControl == NULL) {
	   fprintf (stderr, "Could not find mir_sdr_DCoffsetIQimbalanceControl\n");
	   return false;
	}


//	my_mir_sdr_ResetUpdateFlags	= (pfn_mir_sdr_ResetUpdateFlags)
//	                GETPROCADDRESS (Handle, "mir_sdr_ResetUpdateFlags");
//	if (my_mir_sdr_ResetUpdateFlags == NULL) {
//	   fprintf (stderr, "Could not find mir_sdr_ResetUpdateFlags\n");
//	   return false;
	return true;
}

void	sdrplay::agcControl_toggled (int agcMode) {
	this	-> agcMode	= agcControl -> isChecked ();
	if (running) {
	   my_mir_sdr_AgcControl (this -> agcMode, -currentGain, 0, 0, 0, 1, 0);
	   if (agcMode == 0)
	      my_mir_sdr_SetGr (gainSlider -> value (), 1, 0);
	}
}


