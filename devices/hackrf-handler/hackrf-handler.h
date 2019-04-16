#
/*
 *    Copyright (C) 2017 .. 2018
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the sdr-j-spectrumviewer
 *    sdr-j-spectrumviewer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    sdr-j-spectrumviewer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdr-j-spectrumvieuwer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __HACKRF_HANDLER__
#define	__HACKRF_HANDLER__

#include	<QObject>
#include	<QFrame>
#include	<QSettings>
#include	<atomic>
#include	"spectrum-constants.h"
#include	"ringbuffer.h"
#include	"device-handler.h"
#include	"ui_hackrf-widget.h"
#include	"libhackrf/hackrf.h"

typedef int (*hackrf_sample_block_cd_fn)(hackrf_transfer *transfer);

#ifdef __MINGW32__
#define GETPROCADDRESS  GetProcAddress
#else
#define GETPROCADDRESS  dlsym
#endif
//
//	Dll and ".so" function prototypes

typedef	int	(*pfn_hackrf_init)	(void);
typedef	int	(*pfn_hackrf_open)	(hackrf_device ** device);
typedef	int	(*pfn_hackrf_close)	(hackrf_device *device);
typedef	int	(*pfn_hackrf_exit)	(void);
typedef	int	(*pfn_hackrf_start_rx)	(hackrf_device *,
	                                 hackrf_sample_block_cd_fn, void *);
typedef	int	(*pfn_hackrf_stop_rx)	(hackrf_device *);
typedef	hackrf_device_list_t	*(*pfn_hackrf_device_list)	(void);
typedef	int	(*pfn_hackrf_set_baseband_filter_bandwidth) (hackrf_device *,
	                                 const uint32_t bandwidth_hz);
typedef	int	(*pfn_hackrf_set_lna_gain) (hackrf_device *, uint32_t);
typedef	int	(*pfn_hackrf_set_vga_gain) (hackrf_device *, uint32_t);
typedef	int	(*pfn_hackrf_set_freq)	(hackrf_device *, const uint64_t);
typedef	int	(*pfn_hackrf_set_sample_rate) (hackrf_device *,
	                                     const double freq_hz);
typedef	int	(*pfn_hackrf_is_streaming) (hackrf_device *);
typedef	const char	*(*pfn_hackrf_error_name) (enum hackrf_error errcode);
typedef	const char	*(*pfn_hackrf_usb_board_id_name) (enum hackrf_usb_board_id);
// contributes by Fabio
typedef int     (*pfn_hackrf_set_antenna_enable)
                                 (hackrf_device *, const uint8_t);
typedef int     (*pfn_hackrf_set_amp_enable) (hackrf_device *, const uint8_t);
typedef int     (*pfn_hackrf_si5351c_read)
                                 (hackrf_device *, const uint16_t, uint16_t *);
typedef int     (*pfn_hackrf_si5351c_write)
                            (hackrf_device *, const uint16_t, const uint16_t);
// fine aggiunta


///////////////////////////////////////////////////////////////////////////
class	hackrfHandler: public deviceHandler, public Ui_hackrfWidget {
Q_OBJECT
public:
			hackrfHandler		(QSettings *);
			~hackrfHandler		(void);
	void		setVFOFrequency		(uint64_t);
	uint64_t	getVFOFrequency		(void);
        uint64_t	defaultFrequency        (void);
        bool            legalFrequency		(uint64_t);

	bool		restartReader		(void);
	void		stopReader		(void);
	int32_t		getSamples		(std::complex<float> *,
	                                                          int32_t);
	int32_t		getSamples		(std::complex<float> *,
	                                                     int32_t, int32_t);
	int32_t		Samples			(void);
	int16_t		bitDepth		(void);
	int32_t		getRate			(void);
//
//	The buffer should be visible by the callback function
	RingBuffer<std::complex<float>>	*_I_Buffer;
	hackrf_device	*theDevice;
private:

	bool			load_hackrfFunctions	(void);
	pfn_hackrf_init		hackrf_init;
	pfn_hackrf_open		hackrf_open;
	pfn_hackrf_close	hackrf_close;
	pfn_hackrf_exit		hackrf_exit;
	pfn_hackrf_start_rx	hackrf_start_rx;
	pfn_hackrf_stop_rx	hackrf_stop_rx;
	pfn_hackrf_device_list	hackrf_device_list;
	pfn_hackrf_set_baseband_filter_bandwidth
	                        hackrf_set_baseband_filter_bandwidth;
	pfn_hackrf_set_lna_gain	hackrf_set_lna_gain;
	pfn_hackrf_set_vga_gain	hackrf_set_vga_gain;
	pfn_hackrf_set_freq	hackrf_set_freq;
	pfn_hackrf_set_sample_rate
	                        hackrf_set_sample_rate;
	pfn_hackrf_is_streaming	hackrf_is_streaming;
	pfn_hackrf_error_name	hackrf_error_name;
	pfn_hackrf_usb_board_id_name
	                        hackrf_usb_board_id_name;
//      aggiunta Fabio
        pfn_hackrf_set_antenna_enable hackrf_set_antenna_enable;
        pfn_hackrf_set_amp_enable hackrf_set_amp_enable;
        pfn_hackrf_si5351c_read hackrf_si5351c_read;
        pfn_hackrf_si5351c_write hackrf_si5351c_write;
//      Fine aggiunta


	QSettings	*hackrfSettings;
	QFrame		*myFrame;
	int32_t		inputRate;
	uint64_t	vfoFrequency;
	std::atomic<bool>	running;
	HINSTANCE	Handle;
	bool		libraryLoaded;
private slots:
	void		setLNAGain	(int);
	void		setVGAGain	(int);
// contributed by Fabio
        void            EnableAntenna   (int);
        void            EnableAmpli     (int);
        void            set_ppmCorrection (int);
// Fine aggiunta
};
#endif

