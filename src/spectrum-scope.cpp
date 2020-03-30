#
/*
 *    Copyright (C)  2012, 2013, 2014
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
#include	"spectrum-scope.h"
#include	<qwt_picker_machine.h>
#include	<inttypes.h>
/*
 *	The "scope" combines the Qwt widgets and control for both
 *	the spectrumdisplay and the waterfall display.
 */
	spectrumScope::spectrumScope (QwtPlot *scope,
	                              uint16_t displaysize) {
	plotgrid		= scope;
	this	-> displaySize	= displaysize;
	this	-> displayBuffer = new double [displaySize];
	memset (displayBuffer, 0, displaySize * sizeof (double));
	plotgrid-> setCanvasBackground (Qt::black);
	grid	= new QwtPlotGrid;
#if defined QWT_VERSION && ((QWT_VERSION >> 8) < 0x0601)
	grid	-> setMajPen (QPen(Qt::white, 0, Qt::DotLine));
#else
	grid	-> setMajorPen (QPen(Qt::white, 0, Qt::DotLine));
#endif
	grid	-> enableXMin (true);
	grid	-> enableYMin (true);
#if defined QWT_VERSION && ((QWT_VERSION >> 8) < 0x0601)
	grid	-> setMinPen (QPen(Qt::white, 0, Qt::DotLine));
#else
	grid	-> setMinorPen (QPen(Qt::white, 0, Qt::DotLine));
#endif
	grid	-> attach (plotgrid);

	setBitDepth (12);	// just a default
	SpectrumCurve	= new QwtPlotCurve ("");
   	SpectrumCurve	-> setPen (QPen(Qt::white));
//	SpectrumCurve	-> setStyle	(QwtPlotCurve::Sticks);
	SpectrumCurve	-> setOrientation (Qt::Horizontal);
	SpectrumCurve	-> setBaseline	(get_db (0));
	ourBrush	= new QBrush (Qt::white);
	ourBrush	-> setStyle (Qt::Dense3Pattern);
	SpectrumCurve	-> setBrush (*ourBrush);
	SpectrumCurve	-> attach (plotgrid);

	counter         = 0;

        maxLabel        = new QwtPlotTextLabel ();
        minLabel        = new QwtPlotTextLabel ();

	Marker		= new QwtPlotMarker ();
	Marker		-> setLineStyle (QwtPlotMarker::VLine);
	Marker		-> setLinePen (QPen (Qt::red));
	Marker		-> attach (plotgrid);
	plotgrid	-> enableAxis (QwtPlot::yLeft);

	lm_picker	= new QwtPlotPicker (plotgrid -> canvas ());
	QwtPickerMachine *lpickerMachine =
	              new QwtPickerClickPointMachine ();
	lm_picker	-> setStateMachine (lpickerMachine);
	lm_picker	-> setMousePattern (QwtPlotPicker::MouseSelect1,
	                                    Qt::LeftButton);
	connect (lm_picker, SIGNAL (selected (const QPointF&)),
	         this, SLOT (leftMouseClick (const QPointF &)));

	rm_picker	= new QwtPlotPicker (plotgrid -> canvas ());
	QwtPickerMachine *rpickerMachine =
	              new QwtPickerClickPointMachine ();
	rm_picker	-> setStateMachine (rpickerMachine);
	rm_picker	-> setMousePattern (QwtPlotPicker::MouseSelect1,
	                                    Qt::RightButton);
	connect (rm_picker, SIGNAL (selected (const QPointF&)),
	         this, SLOT (rightMouseClick (const QPointF &)));

	IndexforMarker	= 0;
}

	spectrumScope::~spectrumScope (void) {
	disconnect (lm_picker,
	            SIGNAL (selected (const QPointF &)),
	            this,
	            SLOT (leftMouseClick (const QPointF &)));
	disconnect (rm_picker,
	            SIGNAL (selected (const QPointF &)),
	            this,
	            SLOT (rightMouseClick (const QPointF &)));
	plotgrid	-> enableAxis (QwtPlot::yLeft, false);
	Marker		-> detach ();
	SpectrumCurve	-> detach ();
	grid		-> detach ();
	delete		Marker;
	delete		SpectrumCurve;
	delete		grid;
	delete		lm_picker;
	delete		rm_picker;
	delete		displayBuffer;
}

void	spectrumScope::leftMouseClick (const QPointF &point) {
	leftClicked ((int)(point. x()) - IndexforMarker);
}

void	spectrumScope::rightMouseClick (const QPointF &point) {
	rightClicked ((int)(point. x()) - IndexforMarker);
}

void	spectrumScope::showFrame (double	*Y1_value,
	                          uint32_t	rate,
	                          uint64_t	freq,
	                          double	amp) {
uint16_t	i;
double	X_axis	[displaySize];
double	Y_Values [displaySize];

	IndexforMarker	= (float)freq / 1000;
	for (i = 0; i < displaySize; i ++) 
	   X_axis [i] = (float)(freq / 1000) 
	                - ((double)rate / 2 - ((double)rate) / displaySize * i) / 1000;

	amp		= amp / 50 * (-get_db (0));
	plotgrid	-> setAxisScale (QwtPlot::xBottom,
				         X_axis [0],
				         X_axis [displaySize - 1]);
	plotgrid	-> enableAxis (QwtPlot::xBottom);
	plotgrid	-> setAxisScale (QwtPlot::yLeft,
				         get_db (0), get_db (0) + amp);
	double s = 0, max = 0;

	for (i = 0; i < displaySize; i ++) {
	   if (!(std::isnan (Y_Values [i]) || std::isinf (Y_Values [i])))
	      displayBuffer [i] =
	               0.9 * displayBuffer [i] +
	               0.1 * get_db (Y1_value [i]); 
	   s += get_db (Y1_value [i]);
           if (!isnan (Y1_value [i]) && (Y1_value [i] > max))
              max = Y1_value [i];
        }
        double avg      = s / displaySize;
	if (-- counter < 0) {
	   counter	= 5;
	   QwtText MarkerLabel_1  =  QwtText (QString::number (get_db (max)));
	   QwtText MarkerLabel_2  =  QwtText (QString::number (avg));
	   QFont font1 ("Courier New");
	   font1.       setPixelSize (30);;
	   MarkerLabel_1.    setFont (font1);
	   MarkerLabel_1.    setColor (Qt::white);
	   MarkerLabel_1. setRenderFlags (Qt::AlignLeft | Qt::AlignTop);
	   MarkerLabel_2.    setFont (font1);
	   MarkerLabel_2.    setColor (Qt::white);
	   MarkerLabel_2. setRenderFlags (Qt::AlignRight | Qt::AlignTop);
	   maxLabel     -> detach ();
	   maxLabel     -> setText (MarkerLabel_1);
	   maxLabel     -> attach (plotgrid);
	   minLabel     -> detach ();
	   minLabel     -> setText (MarkerLabel_2);
	   minLabel     -> attach (plotgrid);
	}
	SpectrumCurve	-> setBaseline (get_db (0));
	displayBuffer [0]		= get_db (0);
	displayBuffer [displaySize - 1] = get_db (0);

	SpectrumCurve	-> setSamples (X_axis, displayBuffer, displaySize);
	Marker		-> setXValue (IndexforMarker);
	plotgrid	-> replot(); 
}

float	spectrumScope::get_db (float x) {
	return 20 * log10 ((x + 1) / (float)(normalizer));
}

void	spectrumScope::setBitDepth	(int16_t d) {
	if (d < 0 || d > 32)
	   d = 24;

	normalizer	= 1;
	while (-- d >= 0) 
	   normalizer <<= 1;
}

