#
/*
 *    Copyright (C)  2020
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
#include	"signal-viewer.h"
#include	<inttypes.h>

	SignalView::SignalView (QwtPlot *scope) {
	plotgrid		= scope;
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

	SpectrumCurve	= new QwtPlotCurve ("");
   	SpectrumCurve	-> setPen (QPen(Qt::white));
//	SpectrumCurve	-> setStyle	(QwtPlotCurve::Sticks);
	SpectrumCurve	-> setOrientation (Qt::Horizontal);
	SpectrumCurve	-> setBaseline	(-1);
	ourBrush	= new QBrush (Qt::white);
	ourBrush	-> setStyle (Qt::Dense3Pattern);
	SpectrumCurve	-> setBrush (*ourBrush);
	SpectrumCurve	-> attach (plotgrid);
	plotgrid	-> enableAxis (QwtPlot::yLeft);
}

	SignalView::~SignalView (void) {
	plotgrid	-> enableAxis (QwtPlot::yLeft, false);
	SpectrumCurve	-> detach ();
	grid		-> detach ();
	delete		SpectrumCurve;
	delete		grid;
}

void	SignalView::showFrame (double	*Y1_value, int 	amount) {
uint16_t	i;
double	X_axis	[amount];
double	max	= 0;
	for (i = 0; i < amount; i ++)  {
	   X_axis [i] = i;
	   if (fabs (Y1_value [i]) > max)
	      max = fabs (Y1_value [i]);
	}
	plotgrid	-> setAxisScale (QwtPlot::xBottom,
				         X_axis [0],
				         X_axis [amount - 1]);
	plotgrid	-> enableAxis (QwtPlot::xBottom);
	plotgrid	-> setAxisScale (QwtPlot::yLeft,
				         -max, max);

	SpectrumCurve	-> setSamples (X_axis, Y1_value, amount);
	plotgrid	-> replot(); 
}

