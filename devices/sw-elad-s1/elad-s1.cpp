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
#include	<QMessageBox>
#include	<QDir>

#include	"elad-s1.h"	// our header
#include	"elad-worker.h"	// the thread, reading in the data
#include	"elad-loader.h"	// function loader

#include	<stdio.h>

DSPCOMPLEX	makeSample_31bits (uint8_t *);
DSPCOMPLEX	makeSample_15bits (uint8_t *);

// ADC output unsigned 14 bit input to FPGA output signed 32 bit
// multiply output FPGA by SCALE_FACTOR to normalize 32bit signed values to ADC range signed 14 bit
#define SCALE_FACTOR_32to14    (0.000003814) //(8192/2147483648)
// ADC out unsigned 14 bit input to FPGA output signed 16 bit
#define SCALE_FACTOR_16to14    (0.250)       //(8192/32768)  


//	Currently, we do not have lots of settings,
//	it just might change suddenly, but not today
		eladHandler::eladHandler (QSettings	*s,
	                                  int32_t	theRate) {
int16_t	theSuccess;
	this	-> eladSettings	= s;
	this	-> inputRate	= theRate;
	deviceOK		= false;
	myFrame			= new QFrame;
	setupUi (myFrame);
	myFrame	-> show ();
	_I_Buffer		= NULL;
	theLoader		= NULL;
	theWorker		= NULL;
	rateDisplay	-> display (inputRate);
	conversionNumber	= inputRate == 192000 ? 1:
	                          inputRate <= 3072000 ? 2 : 3;
	eladSettings	-> beginGroup ("eladSettings");
	depth			= eladSettings -> value ("bitDepth", 14). toInt ();
	eladSettings	-> endGroup ();
//
//	number of bytes per IQ value
	iqSize			= conversionNumber == 3 ? 4 : 8;
//
	theLoader	= new eladLoader (inputRate, &theSuccess);
	fprintf (stderr, "we zijn terug\n");
	if (theSuccess != 0) {
	   if (theSuccess == -1)
	   QMessageBox::warning (myFrame, tr ("viewer"),
	                         tr ("No success in loading libs\n"));
	   else
	   if (theSuccess == -2)
	   QMessageBox::warning (myFrame, tr ("viewer"),
	                         tr ("No success in setting up USB\n"));
	   else
	   if (theSuccess == -3)
	   QMessageBox::warning (myFrame, tr ("viewer"),
	                         tr ("No success in FPGA init\n"));
	   else
	   if (theSuccess == -4)
	   QMessageBox::warning (myFrame, tr ("viewer"),
	                         tr ("No success in hardware init\n"));
	
	   statusLabel -> setText ("not functioning");
	   delete theLoader;
	   throw (21);
	}
//
//	Note (10.10.2014: 
//	It turns out that the elad provides for 32 bit samples
//	packed as bytes
	statusLabel	-> setText ("Loaded");
	deviceOK	= true;
//
//	Note the maximum speed is 1536000, for that we need a buffersize
//	of at least 153600 samples, which in terms of "uint8_t's" is
//	8 * 153600 (which is 1228800), so we settle for a buffersize
//	To handle a lower refreshrate we take an even larger buffer
	_I_Buffer	= new RingBuffer<uint8_t>(2048 * 2048);
	vfoFrequency	= Khz (94700);
//
//	since localFilter and gainReduced are also used as
//	parameter for the API functions, they are int's rather
//	than bool.
	localFilter	= 0;
	filterText	-> setText ("no filter");
	gainReduced	= 0;
	gainLabel	-> setText ("0");
	connect (hzOffset, SIGNAL (valueChanged (int)),
	         this, SLOT (setOffset (int)));
	connect (gainReduction, SIGNAL (clicked (void)),
	         this, SLOT (setGainReduction (void)));
	connect (filter, SIGNAL (clicked (void)),
	         this, SLOT (setFilter (void)));
}

	eladHandler::~eladHandler	(void) {
	eladSettings	-> beginGroup ("eladSettings");
	eladSettings	-> setValue ("elad-offset", vfoOffset);
	eladSettings	-> setValue ("bitDepth", depth);
	eladSettings	-> endGroup ();
	stopReader ();
	if (_I_Buffer != NULL)
	   delete _I_Buffer;
	if (theLoader != NULL)
	   delete theLoader;
	if (theWorker != NULL)
	   delete theWorker;
}

int32_t	eladHandler::getRate	(void) {
	return inputRate;
}

void	eladHandler::setVFOFrequency	(int32_t newFrequency) {
int32_t	realFreq = newFrequency - vfoOffset;

	if (theWorker == NULL) {
	   vfoFrequency = newFrequency - vfoOffset;
	   return;
	}

	theWorker -> setVFOFrequency (realFreq);
	vfoFrequency = theWorker -> getVFOFrequency ();
}

int32_t	eladHandler::getVFOFrequency	(void) {
	return vfoFrequency + vfoOffset;
}

bool	eladHandler::restartReader	(void) {
bool	success;

	if ((theWorker != NULL) || !deviceOK)
	   return true;

	_I_Buffer	-> FlushRingBuffer ();
	theWorker	= new eladWorker (inputRate,
	                                  vfoFrequency,
	                                  theLoader,
	                                  _I_Buffer,
	                                  iqSize,
	                                  &success);
	connect (theWorker, SIGNAL (samplesAvailable (int)),
	         this, SIGNAL (samplesAvailable (int)));
	fprintf (stderr,
	         "restarted eladWorker with %s\n",
	                  success ? "success" : "no success");
	return success;
}

void	eladHandler::stopReader	(void) {
	if ((theWorker == NULL) || !deviceOK)
	   return;

	theWorker	-> stop ();
	while (theWorker -> isRunning ())
	   usleep (100);
	delete theWorker;
	theWorker = NULL;
}

bool	eladHandler::legalFrequency	(int32_t f) {
	return f > Khz (100) && f < Khz (100000);
}

//
//	The brave old getSamples. For the elad
//	reading of the "raw" data (i.e. the bytes!!!), we use
//	the "worker". Here we unpack and make the samples into I/Q samples

DSPCOMPLEX	makeSample_31bits (uint8_t *buf) {
int ii = 0; int qq = 0;
int16_t	i = 0;
	
uint32_t uii=0, uqq=0;

	uint8_t i0 = buf [i++]; //i+0
	uint8_t i1 = buf [i++]; //i+1
	uint8_t i2 = buf [i++]; //i+2
	uint8_t i3 = buf [i++]; //i+3
	
	uint8_t q0 = buf [i++]; //i+4
	uint8_t q1 = buf [i++]; //i+5
	uint8_t q2 = buf [i++]; //i+6
	uint8_t q3 = buf [i++]; //i+7

// Andrea Montefusco recipe
// from four unsigned 8bit little endian order to unsigned 32bit (just move),
// then cast it to signed 32 bit
	uii = (i3 << 24) | (i2 << 16) | (i1 << 8) | i0;
	uqq = (q3 << 24) | (q2 << 16) | (q1 << 8) | q0;
	
	ii =(int)uii;
        qq =(int)uqq;
	
	return DSPCOMPLEX ((float)ii * SCALE_FACTOR_32to14,
	                   (float)qq * SCALE_FACTOR_32to14);
}
/*Giovanni Franza recipe  --(short code)  c1 and c2 scale factor to Giovanni's app
	if(m_bytes_per_sample==2) {
	float c=1/8.0/1024.0;            
	for( int j=0; j<q; j++ ) {
		short r=(short)((uint16_t *)p)[j];
		out[j]=r*c;
	}
} else {
	float c=1/256.0/1024.0/1024.0;      
	for( int j=0; j<q; j++ ) {
		int r=(int)((uint32_t *)p)[j];
		out[j]=r*c;
	}
*/	


//
DSPCOMPLEX	makeSample_15bits (uint8_t *buf) {
int16_t	i = 0;	
	
int32_t ii= 0 , qq= 0;
int16_t sqq=0 , sii=0;
	
	uint8_t i0 = buf [i++]; //i+0
	uint8_t i1 = buf [i++]; //i+1
	uint8_t q0 = buf [i++]; //i+2
	uint8_t q1 = buf [i++]; //i+3

	
	// from two unsigned 8 bit little endian order to unsigned 16 bit , then signed 16 bit
	sii =(short) ((i1 << 8) | i0);
	sqq =(short) ((q1 << 8) | q0);
	// extension signed 16 bit to signed 32 bit 
	ii  =(int)sii;
	qq  =(int)sqq;

	return DSPCOMPLEX ((float)ii * SCALE_FACTOR_16to14,
			   (float)qq * SCALE_FACTOR_16to14);

}
//
//	Please realize that the _I_Buffer contains BYTES rather than
//	complex samples
//	
int32_t	eladHandler::getSamples (DSPCOMPLEX *V, int32_t size) { 
int32_t		amount, i;
uint8_t		buf [iqSize * size];

	if (!deviceOK) 
	   return 0;

	amount = _I_Buffer	-> getDataFromBuffer (buf, iqSize * size);

	for (i = 0; i < amount / iqSize; i ++)  {
	   switch (conversionNumber) {
	      case 1: default:
	         V [i] = makeSample_31bits (&buf [iqSize * i]);
	         break;
	      case 2:
	         V [i] = makeSample_31bits (&buf [iqSize * i]);
	         break;
	      case 3:
	         V [i] = makeSample_15bits (&buf [iqSize * i]);
	         break;
	   }
	}
	return amount / iqSize;
}

int32_t	eladHandler::getSamples (DSPCOMPLEX *V,
	                         int32_t size, int32_t segmentSize) { 
int32_t	skipAmount = segmentSize - size;	// this is in IQ samples
int32_t	amount;

	if (!deviceOK) 
	   return 0;

	amount	= getSamples (V, size);
//	so, we skip iqSize buffer elements
	if (skipAmount > 0)
	   _I_Buffer -> skipDataInBuffer (iqSize * skipAmount);
	return amount;
}

int32_t	eladHandler::Samples	(void) {
	if (!deviceOK)
	   return 0;
	return _I_Buffer	-> GetRingBufferReadAvailable () / iqSize;
}
//
//	Although we are getting 30-more bits in, the adc in the
//	elad gives us 14 bits. That + 20 db gain results in app 105 db
//	plus a marge it is app 120 a 130 db, so the bit depth for the scope
//	is 21
int16_t	eladHandler::bitDepth	(void) {
	return depth;
}

//
void	eladHandler::setOffset	(int k) {
	vfoOffset	= k;
}

void	eladHandler::setGainReduction	(void) {
	gainReduced = gainReduced == 1 ? 0 : 1;
	theLoader -> set_en_ext_io_ATT20 (theLoader -> getHandle (),
	                                     &gainReduced);
	gainLabel -> setText (gainReduced == 1 ? "-20" : "0");
}

void	eladHandler::setFilter	(void) {
	localFilter = localFilter == 1 ? 0 : 1;
	theLoader -> set_en_ext_io_LP30 (theLoader -> getHandle (),
	                                     &localFilter);
	filterText	-> setText (localFilter == 1 ? "30 Mhz" : "no filter");
}

