#
/*
 *    Copyright (C)  2012, 2013, 2014
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
#include	"waterfall-scope.h"
#include        <qwt_picker_machine.h>

/*
 *	The waterfall display
 */
	waterfallScope::waterfallScope (QwtPlot	*plot,
	                                uint16_t	displaySize,
	                                uint16_t	rasterSize):
	                                QwtPlotSpectrogram () {
int	i, j;
	colorMap  = new QwtLinearColorMap (Qt::darkCyan, Qt::red);
	QwtLinearColorMap *c2 = new QwtLinearColorMap (Qt::darkCyan, Qt::red);
	plotgrid	= plot;
	this		-> displaySize	= displaySize;
	this		-> rasterSize	= rasterSize;
	colorMap 	-> addColorStop	(0.1, Qt::cyan);
	colorMap 	-> addColorStop	(0.4, Qt::green);
	colorMap	-> addColorStop	(0.7, Qt::yellow);
	c2 		-> addColorStop	(0.1, Qt::cyan);
	c2 		-> addColorStop	(0.4, Qt::green);
	c2		-> addColorStop	(0.7, Qt::yellow);
	this 		-> setColorMap (colorMap);
	OneofTwo	= 0;
	rightAxis 	= plotgrid -> axisWidget (QwtPlot::yRight);
// A color bar on the right axis
	rightAxis -> setColorBarEnabled (true);

	plotData = new double [2 * displaySize * rasterSize];
	for (i = 0; i < rasterSize; i ++)
 	   for (j = 0; j < displaySize; j ++)
	      plotData [i * displaySize + j] = (double)i / rasterSize;

	waterfallData	= new SpectrogramData (plotData,
	                                       10000,
	                                       1000,
	                                       rasterSize,
	                                       displaySize,
	                                       50.0);
	this -> setData (waterfallData);
	this -> setDisplayMode (QwtPlotSpectrogram::ImageMode, true);
	rightAxis -> setColorMap (this -> data () -> interval (Qt::YAxis),
	                          c2);
	plotgrid	-> setAxisScale (QwtPlot::yRight, 0, 50.0);
	plotgrid	-> enableAxis (QwtPlot::yRight);
	plotgrid	-> setAxisScale (QwtPlot::xBottom,
	                                 10000,
	                                 11000);
	plotgrid	-> enableAxis (QwtPlot::xBottom);
	plotgrid	-> enableAxis (QwtPlot::yLeft);
	plotgrid	-> setAxisScale (QwtPlot::yLeft, 0, rasterSize);

	Marker		= new QwtPlotMarker ();
	Marker		-> setLineStyle (QwtPlotMarker::VLine);
	Marker		-> setLinePen (QPen (Qt::black));
	Marker		-> attach (plotgrid);
//	this		-> attach (plotgrid);
	IndexforMarker	= 0;

	lm_picker	= new QwtPlotPicker (plot -> canvas ());
	QwtPickerMachine *lpickerMachine =
	              new QwtPickerClickPointMachine ();
	lm_picker	-> setStateMachine (lpickerMachine);
	lm_picker	-> setMousePattern (QwtPlotPicker::MouseSelect1,
	                                    Qt::LeftButton);
	connect (lm_picker, SIGNAL (selected (const QPointF&)),
	         this, SLOT (leftMouseClick (const QPointF &)));

	plotgrid	-> replot ();
}

	waterfallScope::~waterfallScope (void) {
	plotgrid	-> enableAxis (QwtPlot::yRight, false);
	plotgrid	-> enableAxis (QwtPlot::xBottom, false);
	plotgrid	-> enableAxis (QwtPlot::yLeft, false);
	this		-> detach ();
}

void	waterfallScope::leftMouseClick (const QPointF &point) {
	leftClicked ((int)(point. x()) - IndexforMarker);
}

static	bool a = true;

void	waterfallScope::showFrame (double	*Y1_value,
	                           int32_t	rate,
	                           uint64_t	freq,
	                           double	amp) {
double X_axis [displaySize];
int	orig;
int	width;
SpectrogramData	*oldData;
int	i;

	if (a) {
	   a = false;
	   return;
	}
	a = true;
	IndexforMarker	= freq / 1000;
	for (i = 0; i < displaySize; i ++)
	   X_axis [i] = (freq -
	                (float)rate / 2 + rate / displaySize * i) / 1000;
	orig	= (int)(X_axis [0]);
	width	= (int)(X_axis [displaySize - 1] - orig);
/*
 *	shift one row, faster with memmove than writing out
 *	the loops. Note that source and destination overlap
 *	and we therefore use memmove rather than memcpy
 */
	memmove (&plotData [0],
	         &plotData [displaySize], 
	         (rasterSize - 1) * displaySize * sizeof (double));
/*
 *	... and insert the new line
 */
	memcpy (&plotData [(rasterSize - 1) * displaySize],
	        &Y1_value [0],
	        displaySize * sizeof (double));

//	if (WaterfallData != NULL)
//	   delete	WaterfallData;
//
	waterfallData = new SpectrogramData (plotData,
	                                     orig,
	                                     width,
	                                     rasterSize,
	                                     displaySize,
	                                     amp);

	this		-> detach	();
	this		-> setData	(waterfallData);
	this		-> setDisplayMode (QwtPlotSpectrogram::ImageMode,
	                                                               true);
	plotgrid	-> setAxisScale (QwtPlot::xBottom,
	                                 orig,
	                                 orig + width);
	plotgrid	-> enableAxis (QwtPlot::xBottom);
	Marker		-> setXValue (0);
	this		-> attach     (plotgrid);
	plotgrid	-> replot();
}

