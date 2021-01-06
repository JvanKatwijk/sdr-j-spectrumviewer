#
/*
 *    Copyright (C)  2012, 2013, 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the spectrum viewer
 *
 *    soectrumViewer is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    spectrumViewer is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with spectrumViewer; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__SPECTRUM_SCOPE__
#define	__SPECTRUM_SCOPE__
#include	<QObject>
#include	<stdint.h>
#include	<limits.h>
#include	<math.h>
#include	<complex>
#include        <qwt.h>
#include        <qwt_plot.h>
#include        <qwt_plot_curve.h>
#include        <qwt_plot_marker.h>
#include        <qwt_plot_grid.h>
#include        <qwt_color_map.h>
#include        <qwt_plot_spectrogram.h>
#include        <qwt_plot_zoomer.h>
#include        <qwt_plot_textlabel.h>
#include        <qwt_plot_panner.h>
#include        <qwt_plot_layout.h>
#include        <qwt_scale_widget.h>

#include	<qwt_picker_machine.h>


class	spectrumScope:public QObject {
Q_OBJECT
public:
		spectrumScope	(QwtPlot *, uint16_t);
		~spectrumScope	(void);
void		showFrame	(double		*Y1_value,
	                         uint32_t	rate,
	                         int		border,
	                         uint64_t	freq,
	                         double		amp);
void		showFrame	(double		*Y1_value,
	                         uint32_t	rate,
	                         uint64_t	freq,
	                         double		amp);
void		setBitDepth	(int16_t d);
private:
	float		get_db (float x);
	QwtPlot		*plotgrid;
	int16_t		displaySize;
	QwtPlotGrid	*grid;
	QwtPlotCurve	*SpectrumCurve;
	QBrush		*ourBrush;
	QwtPlotMarker	*Marker;
	QwtPlotPicker	*lm_picker;
	QwtPlotPicker	*rm_picker;
	int		IndexforMarker;
	int		normalizer;
	double	*displayBuffer;
	int		counter;
	QwtPlotTextLabel *maxLabel;
	QwtPlotTextLabel *minLabel;

public slots:
	void		leftMouseClick (const QPointF &point);
	void		rightMouseClick (const QPointF &point);

signals:
	void		leftClicked	(int);
	void		rightClicked	(int, int);

};

#endif

