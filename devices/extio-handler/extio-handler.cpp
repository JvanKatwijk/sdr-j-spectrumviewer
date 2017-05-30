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
#
#include	<QFileDialog>
#include	<QSettings>
#include	<QMessageBox>
#include	"extio-handler.h"
#include	"virtual-reader.h"
#include	"common-readers.h"
#include	"card-reader.h"

#ifndef	__MINGW32__
#include	"dlfcn.h"
#endif
using namespace std;

#ifdef __MINGW32__
#define	GETPROCADDRESS	GetProcAddress
#define	FREELIBRARY	FreeLibrary
#else
#define	GETPROCADDRESS	dlsym
#define	FREELIBRARY	dlclose
#endif
//

//	Interface routines
//	The callback from the Extio*.dll does not contain a "context",
//	we just do not know where the call comes from
//	so we have to create one artificially in order to access
//	our functions
//	Our context here is the instance of the class, stored in
//	a static variable
//
//	Note that when requesting a change in frequency from within
//	the program, the SDRplay extio responds with a lock command, directly
//	followed by an unlock command. The lack of latency between the
//	commands makes that  when signaling the GUI directly, the GUI
//	crashes, so we build a small thread only for handling the extio
//	requests, then that thread is capable of sending the requests
//	to the GUI thread.
static
extioHandler	*myContext	= NULL;

static
int	extioCallback (int cnt, int status, float IQoffs, void *IQData) {
	if (cnt > 0) { 	//	we got data
	   if (myContext != NULL && myContext -> isStarted) 
	      myContext -> theReader -> processData (IQoffs, IQData, cnt);
	}
	else	// we got something we cannot deal with
	if (cnt > 0)
	   fprintf (stderr, "XXXXXXXXXXXX%d (%d)\n", cnt, status);
	else
	if (cnt < 0) {
//	it is a signal to do something,
//	queue the signal (and wake up the handling thread
	   if (myContext != NULL) {
	      myContext -> putonQueue (status);
	   }
	}
	return 1;
}
//
//	the seemingly additional complexity is caused by the fact
//	that naming of files in windows is in MultiBytechars
//
//	We assume that if there are settings possible, they
//	are dealt with by the producer of the extio, so here 
//	no frame whatsoever.
	extioHandler::extioHandler (QSettings *s) {
#ifdef	__MINGW32__
char	temp [256];
wchar_t	*windowsName;
int16_t	wchars_num;
#endif
	extioSettings		= s;
	myFrame			= new QFrame (NULL);
	setupUi	(this -> myFrame);
	myFrame 	-> show ();
	theSelector	-> hide ();

	inputRate	= 192000;	// default
	lastFrequency	= Khz (25000);
	base_16		= s -> value ("base_16", 32767). toInt ();
	base_24		= s -> value ("base_24", 32767 * 256). toInt ();
	base_32		= s -> value ("base_32", 32767 * 32768). toInt ();
	isStarted	= false;
	theReader	= NULL;
	dll_open	= false;
	running		= false;

	QString	dll_file	= "foute boel";
	dll_file = QFileDialog::
	           getOpenFileName (NULL,
	                            tr ("load file .."),
	                            QDir::currentPath (),
#ifdef	__MINGW32__
	                            tr ("libs (Extio*.dll)"));
#else
	                            tr ("libs (*.so)"));
#endif
	dll_file	= QDir::toNativeSeparators (dll_file);
	if (dll_file == QString ("")) {
	   QMessageBox::warning (NULL, tr ("sdr"),
	                               tr ("incorrect filename\n"));
	   throw (2);
	}

#ifdef	__MINGW32__
	wchars_num = MultiByteToWideChar (CP_UTF8, 0,
	                              dll_file. toLatin1 (). data (),
	                              -1, NULL, 0);
	windowsName = new wchar_t [wchars_num];
	MultiByteToWideChar (CP_UTF8, 0,
	                     dll_file. toLatin1 (). data (),
	                     -1, windowsName, wchars_num);
	wcstombs (temp, windowsName, 128);
	Handle		= LoadLibrary (windowsName);
	fprintf (stderr, "Last error = %ld\n", GetLastError ());
#else
	Handle		= dlopen (dll_file. toLatin1 (). data (), RTLD_NOW);
#endif
	if (Handle == NULL) {
	   QMessageBox::warning (NULL, tr ("sdr"),
	                               tr ("loading dll failed\n"));
	   throw (20);
	}

	if (!loadFunctions ()) {
	   QMessageBox::warning (NULL, tr ("sdr"),
	                               tr ("loading functions failed\n"));
	   throw (21);
	}

//	apparently, the library is open, so record that
	dll_open	= true;
	myContext	= (extioHandler *)this;
//	
//	and start the rig
	rigName		= new char [128];
	rigModel	= new char [128];
	if (!((*InitHW) (rigName, rigModel, hardwareType))) {
	   QMessageBox::warning (NULL, tr ("sdr"),
	                               tr ("init failed\n"));
	   FREELIBRARY (Handle);
	}
	deviceName	-> setText (rigName);
	theBuffer	= new RingBuffer<DSPCOMPLEX>(1024 * 1024);
	fprintf (stderr, "hardware type = %d\n", hardwareType);
	switch (hardwareType) {
	   case exthwNone:
	   case exthwSDRX:
	   case exthwHPSDR:
	   case exthwSDR14:
	   default:
	      QMessageBox::warning (NULL, tr ("sdr"),
	                               tr ("device not supported\n"));
	      throw (23);

	   case exthwSCdata:
	      theSelector -> show ();
	      theReader	= new cardReader (theBuffer, theSelector);
	      if (!((cardReader *)theReader) -> worksFine ()) {
	         QMessageBox::warning (NULL, tr ("sdr"),
	                                     tr ("cannot handle soundcard"));
	         throw (24);
	      }
	      connect (theSelector, SIGNAL (activated (int)),
	               this, SLOT (set_streamSelector (int)));
	      break;
	   case exthwUSBdata16:
	      theReader	= new reader_16 (theBuffer, base_16);
	      break;
	   case exthwUSBdata24:
	      theReader	= new reader_24 (theBuffer, base_24);
	      break;
	   case exthwUSBdata32:
	      theReader	= new reader_32 (theBuffer, base_32);
	      break;
	   case exthwUSBfloat32:
	      theReader	= new reader_float (theBuffer);
	      break;
	}

	SetCallback (extioCallback);
	if (!(*OpenHW)()) {
	   QMessageBox::warning (NULL, tr ("sdr"),
	                               tr ("Opening hardware failed\n"));
	   FREELIBRARY (Handle);
	   throw (25);
	}
	ShowGUI ();
	fprintf (stderr, "Hw open successful\n");
	start ();
}

	extioHandler::~extioHandler (void) {
	if (dll_open) {
	   HideGUI ();
	   StopHW ();
	   CloseHW ();
	}
	if (running) {
	   commandQueue. clear	();
	   commandQueue. enqueue (-1);
	   commandHandler. wakeAll ();
	   while (!isFinished ())
	      usleep (100);
	}
	if (Handle != NULL)
	   FREELIBRARY (Handle);

	if (theReader != NULL)
	   delete theReader;
//	just in case
	theSelector	-> hide ();
	delete	myFrame;
}

bool	extioHandler::loadFunctions (void) {
//	start binding addresses, 
	InitHW		= (pfnInitHW)GETPROCADDRESS (Handle, "InitHW");
	if (InitHW == NULL) {
	   fprintf (stderr, "Failed to load InitHW\n");
	   return false;
	}

	OpenHW		= (pfnOpenHW)GETPROCADDRESS (Handle, "OpenHW");
	if (OpenHW == NULL) {
	   fprintf (stderr, "Failed to load OpenHW\n");
	   return false;
	}

	StartHW		= (pfnStartHW)GETPROCADDRESS (Handle, "StartHW");
	if (StartHW == NULL) {
	   fprintf (stderr, "Failed to load StartHW\n");
	   return false;
	}

	StopHW		= (pfnStopHW)GETPROCADDRESS (Handle, "StopHW");
	if (StopHW == NULL) {
	   fprintf (stderr, "Failed to load StopHW\n");
	   return false;
	}

	CloseHW		= (pfnCloseHW)GETPROCADDRESS (Handle, "CloseHW");
	if (CloseHW == NULL) {
	   fprintf (stderr, "Failed to load CloseHW\n");
	   return false;
	}

	GetHWLO		= (pfnGetHWLO)GETPROCADDRESS (Handle, "GetHWLO");
	if (GetHWLO == NULL) {
	   fprintf (stderr, "Failed to load GetHWLO\n");
	   return false;
	}

	SetHWLO		= (pfnSetHWLO)GETPROCADDRESS (Handle, "SetHWLO");
	if (SetHWLO == NULL) {
	   fprintf (stderr, "Failed to load SetHWLO\n");
	   return false;
	}

	GetStatus	= (pfnGetStatus)GETPROCADDRESS (Handle, "GetStatus");
	if (GetStatus == NULL) {
	   fprintf (stderr, "Failed to load GetStatus\n");
	   return false;
	}

	SetCallback	= (pfnSetCallback)
	                      GETPROCADDRESS (Handle, "SetCallback");
	if (SetCallback == NULL) {
	   fprintf (stderr, "Failed to load SetCallback\n");
	   return false;
	}
//
//	the "non essentials", packed in envelope functions:
	L_ShowGUI	= (pfnShowGUI)GETPROCADDRESS (Handle, "ShowGUI");
	L_HideGUI	= (pfnHideGUI)GETPROCADDRESS (Handle, "HideGUI");
	L_GetHWSR	= (pfnGetHWSR)GETPROCADDRESS (Handle, "GetHWSR");
	L_GetFilters	= (pfnGetFilters)GETPROCADDRESS (Handle, "GetFilters");
	L_GetTune	= (pfnGetTune)GETPROCADDRESS (Handle, "GetTune");
	L_GetMode	= (pfnGetMode)GETPROCADDRESS (Handle, "GetMode");
//
	return true;
}

int32_t	extioHandler::getRate	(void) {
	if (hardwareType == exthwSCdata)
	   return 192000;
	return GetHWSR ();
}

void	extioHandler::setVFOFrequency (int32_t f) {
	fprintf (stderr, "setting freq to %d\n", f);
int	h =  (*SetHWLO) ((int)f);
	lastFrequency = f;
}

int32_t	extioHandler::getVFOFrequency (void) {
//	lastFrequency = (*GetHWLO)();
	return lastFrequency;
}

bool	extioHandler::legalFrequency	(int32_t f) {
	return true;
}

int32_t	extioHandler::defaultFrequency	(void) {
	return Khz (24700);
}
//
//	envelopes for functions that might or might not
//	be available
void	extioHandler::ShowGUI		(void) {
	if (L_ShowGUI != NULL)
	   (*L_ShowGUI) ();
}

void	extioHandler::HideGUI		(void) {
	if (L_HideGUI != NULL)
	   (*L_HideGUI) ();
}

long	extioHandler::GetHWSR		(void) {
	return L_GetHWSR != NULL ? (*L_GetHWSR) () : 192000;
}

void	extioHandler::GetFilters		(int &x, int &y, int &z) {
	if (L_GetFilters != NULL)
	   L_GetFilters (x, y, z);
}

long	extioHandler::GetTune		(void) {
	return L_GetTune != NULL ? (*L_GetTune) () : MHz (25);
}

uint8_t	extioHandler::GetMode		(void) {
	return L_GetMode != NULL ? (*L_GetMode)() : 0;
}
//
//
//	Handling the data
bool	extioHandler::restartReader	(void) {
	fprintf (stderr, "restart reader entered (%d)\n", lastFrequency);
int32_t	size	= (*StartHW)(lastFrequency);
	fprintf (stderr, "restart reader returned with %d\n", size);
	theReader -> restartReader (size);
	fprintf (stderr, "now we have restarted the reader\n");
	isStarted	= true;
	return true;
}

void	extioHandler::stopReader	(void) {
	if (isStarted) {
	   (*StopHW)();
	   theReader	-> stopReader ();
	   isStarted = false;
	}
}

int32_t	extioHandler::Samples		(void) {
int32_t	x = theBuffer -> GetRingBufferReadAvailable ();
	if (x < 0)
	   fprintf (stderr, "toch een fout in ringbuffer\n");
	return x;
}

int32_t	extioHandler::getSamples		(DSPCOMPLEX *buffer,
	                                         int32_t number) {
int32_t	amount, i;

	amount	= theBuffer -> getDataFromBuffer (buffer, number);
	return amount;
}

int32_t	extioHandler::getSamples		(DSPCOMPLEX *buffer,
	                                         int32_t number,
	                                         int32_t segmentSize) {
int32_t	amount, i;

	if (number > segmentSize)
	   number = segmentSize;
	amount	= theBuffer -> getDataFromBuffer (buffer, number);
	if (segmentSize - amount > 0)
	   theBuffer -> skipDataInBuffer (segmentSize - amount);
	return amount;
}
//
//	Signalling the main program:
void	extioHandler::set_Changed_SampleRate (int32_t newRate) {
	emit set_changeRate (newRate);
	return;
}

void	extioHandler::set_Changed_LO	(int32_t newFreq) {
	emit set_ExtLO (newFreq);
	lastFrequency	= newFreq;
}

void	extioHandler::set_Lock_LO	(void) {
	fprintf (stderr, "emit Lock\n");
	emit set_lockLO ();
}

void	extioHandler::set_Unlock_LO	(void) {
	fprintf (stderr, "emit unlock\n");
	emit set_unlockLO ();
}

void	extioHandler::set_Changed_TUNE	(int32_t newFreq) {
	emit set_ExtFrequency (newFreq);
	lastFrequency	= newFreq;
}

void	extioHandler::set_StartHW	(void) {
	emit set_startHW ();
}

void	extioHandler::set_StopHW		(void) {
	emit set_stopHW ();
}

int16_t	extioHandler::bitDepth		(void) {
	return	theReader	-> bitDepth ();
}

void	extioHandler::set_streamSelector (int idx) {
	fprintf (stderr, "Setting stream %d\n", idx);
	if (!((cardReader *)theReader) -> set_streamSelector (idx))
	   QMessageBox::warning (NULL, tr ("sdr"),
	                               tr ("Selecting  input stream failed\n"));
}

void	extioHandler::run	(void) {

	running		= true;
	while (running) {
	   helper. lock ();
	   commandHandler. wait (&helper, 100);
	   helper. unlock ();
	   while (!commandQueue. isEmpty ()) {
	      int command = commandQueue. dequeue ();
	      fprintf (stderr, "dequeuing command %d\n", command);
	      switch (command) {
	         case -1:
	            running = false;
	            goto L1;
	      // for the SDR14
	         case extHw_Disconnected:
	         case extHw_READY:
	         case extHw_RUNNING:
	         case extHw_ERROR:
	         case extHw_OVERLOAD:
	                   ;			// for now
	            break;

	         case extHw_Changed_SampleRate:	// 100
	            set_Changed_SampleRate (GetHWSR ());
	            break;
	         case extHw_Changed_LO:		// 101
	            set_Changed_LO (GetHWLO ()); 
	            break;
	         case extHw_Lock_LO:		// 102
	            set_Lock_LO ();
	            break;
	         case extHw_Unlock_LO:		// 103
	            set_Unlock_LO ();
	            break;
	         case extHw_Changed_LO_Not_TUNE:	// not implemented
	            break;
	         case extHw_Changed_TUNE:		// 105
	            {  long t =  GetTune ();
	               if (t != -1)
	                   set_Changed_TUNE (t);
	            }
	            break;
	         case extHw_Changed_MODE:		// 106
	            fprintf (stderr, "Mode is changed to %d\n", GetMode ());
	            break;
	         case extHw_Start:			// 107
	            set_StartHW ();
	            break;
	         case extHw_Stop:			// 108
	            set_StopHW ();
	            break;
	         case extHw_Changed_FILTER:		// 109
	            {  int x1, x2, x3;
	               GetFilters (x1, x2, x3);
	               fprintf (stderr, "Filters to be changed %d %d %d\n",
	                                                      x1, x2, x3);
	            }
	            break;
	         default:
	            break;
	      }
	   }
	}
L1:;
}

void	extioHandler::putonQueue	(int s) {
	commandQueue. enqueue (s);
	commandHandler. wakeAll ();
}

