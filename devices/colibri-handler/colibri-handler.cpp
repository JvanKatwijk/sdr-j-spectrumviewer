#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the spectrumViewer
 *
 *    spectrumViewer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation version 2 of the License.
 *
 *    spectrumViewer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with spectrumViewer. if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QThread>
#include	<QSettings>
#include	<QMessageBox>
#include	<QTime>
#include	<QDate>
#include	<QLabel>
#include	<QDebug>
#include	<QFileDialog>
#include	"colibri-handler.h"

	colibriHandler::colibriHandler  (QSettings *s):
	                                  _I_Buffer (4 * 1024 * 1024),
	                                  myFrame (nullptr) {
	colibriSettings		= s;
	setupUi (&myFrame);
	QString libName = "libcolibrinano_lib.so";
	if (!m_loader. load (libName. toLatin1 () .data ())) {
           QMessageBox::critical (nullptr, "colibri",
	                          tr("Failed to load colibrinano_lib.so"));
	   throw (21);
	}

	fprintf (stderr, "library is loaded\n");
	m_loader. initialize ();

	fprintf (stderr, "... and initialized\n");
	uint32_t t_devices	= m_loader. devices ();
	if (t_devices == 0) {
	   QMessageBox::critical (nullptr, "colibri",
	                          tr ("No device available\n"));
	   throw (22);
	}

	fprintf (stderr, "we found %d device(s)\n", t_devices);
//	set some defaults
	if (!m_loader.open (&m_deskriptor, 0)) {
	   QMessageBox::warning (nullptr, "colibri",
	                         tr("Failed to open ColibriNANO!"));
	   throw (23);
        }

	fprintf (stderr, "and opening device 0 was ok\n");
        m_loader.setFrequency (m_deskriptor, defaultFrequency ());

	this		-> lastFrequency	= defaultFrequency ();
	fprintf (stderr, "set on %d\n", (int32_t)lastFrequency);
	colibriSettings -> beginGroup ("colibriSettings");
	int gainSetting = colibriSettings -> value ("colibri-gain", 20). toInt ();
	gainSelector	-> setValue (gainSetting);
	colibriSettings -> endGroup ();
        m_loader.setPream (m_deskriptor, gainSelector ->value () * 0.5 + -31.5);
	actualGain	-> display (gainSelector -> value () * 0.5 + -31.5);

//	and be prepared for future changes in the settings
	connect (gainSelector, SIGNAL (valueChanged (int)),
	         this, SLOT (set_gainControl (int)));

	running. store (false);
	inputRate	= rateSelector -> currentText (). toInt ();
	connect (rateSelector, SIGNAL (activated (const QString &)),
                 this, SLOT (set_rateSelector (const QString &)));
}

	colibriHandler::~colibriHandler () {
	myFrame. hide ();
	stopReader();
	colibriSettings	-> beginGroup ("colibriSettings");
	colibriSettings	-> setValue ("colibri-gain", 
	                              gainSelector -> value ());
	colibriSettings	-> endGroup ();
}
//

void	colibriHandler::setVFOFrequency	(uint64_t newFrequency) {
        m_loader. setFrequency (m_deskriptor, newFrequency);
	this	-> lastFrequency	= newFrequency;
}

uint64_t	colibriHandler::getVFOFrequency () {
	return this -> lastFrequency;
}

void	colibriHandler::set_gainControl	(int newGain) {
float	gainValue	= -31.5 + newGain * 0.5;
	if (gainValue <= 6) {
           m_loader.setPream (m_deskriptor, gainValue);
	   actualGain	-> display (gainSelector -> value () * 0.5 + -31.5);
	}
}

static
bool	the_callBackRx (std::complex<float> *buffer, uint32_t len,
	                               bool overload, void *ctx) {
colibriHandler *p = static_cast<colibriHandler *>(ctx);
	(void)overload;
	p -> _I_Buffer. putDataIntoBuffer (buffer, len);
	return true;
}

bool	colibriHandler::restartReader	() {
	if (running. load())
	   return true;		// should not happen

	m_loader.start (m_deskriptor, (SampleRateIndex)indexforRate (inputRate),
                        the_callBackRx,
	                this);
	running. store (true);
	return true;
}

void	colibriHandler::stopReader() {
	if (!running. load())
	   return;
	m_loader. stop (m_deskriptor);
}


int32_t	colibriHandler::getSamples (std::complex<float> *V,
	                            int32_t size, int32_t segmentSize) {
	int amount = getSamples (V, size);
	_I_Buffer. skipDataInBuffer (segmentSize - size);
	return amount;
}

int32_t	colibriHandler::getSamples (std::complex<float> *V, int32_t size) { 
	return _I_Buffer. getDataFromBuffer (V, size);
}

int32_t	colibriHandler::Samples () {
	return _I_Buffer. GetRingBufferReadAvailable ();
}

int32_t	colibriHandler::getRate	() {
	return sampleRate (5);
}

void	colibriHandler::resetBuffer() {
	_I_Buffer. FlushRingBuffer();
}

int16_t	colibriHandler::bitDepth () {
	return 12;
}

bool	colibriHandler::legalFrequency	(uint64_t f) {
	(void)f;
	return true;
}

uint64_t colibriHandler::defaultFrequency	() {
	return 94700000;
}

int colibriHandler::sampleRate (int index) {
    switch (index) {
        case Sr_48kHz: return 48000;
        case Sr_96kHz: return 96000;
        case Sr_192kHz: return 192000;
        case Sr_384kHz: return 384000;
        case Sr_768kHz: return 768000;
        case Sr_1536kHz: return 1536000;
        case Sr_1920kHz: return 1920000;
        case Sr_2560kHz: return 2560000;
        case Sr_3072kHz: return 3072000;
        default: break;
    }

    return 48000;
}

static struct {
	SampleRateIndex key;
	int rate;
} rateTable [] = {
	{Sr_48kHz,	48000},
	{Sr_96kHz,	96000},
	{Sr_192kHz,	192000},
	{Sr_384kHz,	384000},
	{Sr_768kHz,	768000},
	{Sr_1536kHz,	1536000},
	{Sr_1920kHz,	1920000},
	{Sr_2560kHz,	2560000},
	{Sr_3072kHz,	3072000},
	{Sr_48kHz,	-1}
};

SampleRateIndex colibriHandler::indexforRate (int rate) {
	for (int i = 0; rateTable [i]. key != Sr_48kHz; i ++)
	   if (rateTable [i].rate == rate)
	      return rateTable [i]. key;
	return Sr_1536kHz;
}

void	colibriHandler::set_rateSelector (const QString &s) {
int32_t v       = s. toInt ();

	stopReader ();
	this	-> inputRate	= v;
	restartReader ();	// we need a proper restart
        set_changeRate  (v);	// and signal the main program
}

