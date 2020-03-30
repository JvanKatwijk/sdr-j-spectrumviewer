#
/*
 *    Copyright (C)  2010, 2011, 2012
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

#ifndef __SPECTRUM_VIEWER__
#define __SPECTRUM_VIEWER__

#include	<QDialog>
#include	"ui_sdrgui.h"
#include	<qwt.h>
#include	<qwt_slider.h>
#include	<qwt_plot.h>
#include	<qwt_plot_curve.h>
#include	<qwt_plot_marker.h>
#include	<qwt_plot_grid.h>
#include	<qwt_dial.h>
#include	<qwt_plot_spectrogram.h>
#include	<QTimer>
#include	<QWheelEvent>
#include	"waterfall-scope.h"
#include	"spectrum-scope.h"
#include	"mapper.h"
#include	"fir-filters.h"
#include	<atomic>
#include	<complex>
#include <QCloseEvent>
//
#include	<QObject>

class	QSettings;
class	deviceHandler;
class	decimator;
class	keyPad;
/*
 *	The main gui object. It inherits from
 *	QDialog and the generated form
 */
class Viewer: public QDialog,
		      private Ui_SDRplayViewer {
Q_OBJECT
public:
		Viewer	(QSettings *, QWidget *parent = NULL);
		~Viewer	(void);

private:
	QSettings	*spectrumSettings;
	spectrumScope	*HFScope_1;
	waterfallScope	*HFScope_2;
	spectrumScope	*IFScope;
	keyPad		*mykeyPad;
	float		*Window;
	int32_t		inputRate;
	int32_t		bandWidth;
	int32_t		displaySize;
	int16_t		displayRate;
	int16_t		rasterSize;
	int32_t		spectrumSize;
	std::atomic<bool>	running;
	deviceHandler	*theDevice;
	uint64_t	currentFrequency;
	int16_t		scanDelayTime;
	void		ClearPanel		(void);
	void		AddtoPanel		(int16_t);
	uint64_t	getPanel		(void);
	void		CorrectPanel		(void);
	freqmapper	*theMapper;
	freqmapper	*theMapper_2;
	decimator	*theDecimator;
	void		setTuner		(uint64_t);
	void		setnextFrequency	(uint64_t);
	QTimer		*lcd_timer;
	QTimer		*scanTimer;
	QTimer		*displayTimer;
	QTimer		*runTimer;
	int64_t		Panel;
	uint8_t		runMode;
	bool		freezer;
	bool		scanner;
	int32_t		freezeCount;
	void		doFreeze		(double *, double *, int32_t);
	void		IncrementFrequency	(int32_t);
/*
 */
	deviceHandler	*setDevice		(void);
private slots:
	void	stop_lcdTimer		(void);
	void	setStart		(void);
	void	setStop			(void);
	void	lcd_timeout		(void);
	void	updateTimeDisplay	(void);
	void	clickPause		(void);
	void	adjustFrequency		(int);
	void	adjustinLoupe		(int);

	void	TerminateProcess	(void);
	void	toggle_Freezer		(void);
	void	nextFrequency		(void);
	void	switchScanner		(void);

	void	Display			(uint64_t);
	void	Display			(uint64_t, bool);
public slots:
	void	handleSamples		(void);
	void	handle_freqButton	();
	void	newFrequency		(int);
	void	set_changeRate		(int);
	void	decimationHandler	(QString);
	void	closeEvent		(QCloseEvent *event);
        void    wheelEvent              (QWheelEvent *);
	void	decimationHandler	(int);
};

#endif

