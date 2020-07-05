#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__KEYPAD__
#define	__KEYPAD__
#include	<QWidget>
#include	<QGridLayout>
#include	<QButtonGroup>
#include	<QPushButton>
#include	<QLCDNumber>

class	Viewer;

class	keyPad:QObject {
Q_OBJECT
public:
		keyPad		(Viewer *);
		~keyPad		(void);
	void	showPad		(void);
	void	hidePad		(void);
	bool	isVisible	(void);
private slots:
	void	collectData	(int);
private:
	Viewer	*myRadio;
	QWidget		*theFrame;
	QGridLayout	*theLayout;
	QButtonGroup	*thePad;
	QPushButton		*zeroButton;
	QPushButton		*oneButton;
	QPushButton		*twoButton;
	QPushButton		*threeButton;
	QPushButton		*fourButton;
	QPushButton		*fiveButton;
	QPushButton		*sixButton;
	QPushButton		*sevenButton;
	QPushButton		*eightButton;
	QPushButton		*nineButton;
	QPushButton		*KHzButton;
	QPushButton		*MHzButton;
	QPushButton		*clearButton;
	QPushButton		*correctButton;
	QLCDNumber		*theDisplay;
	qint64		panel;
	bool		shown;
signals:
	void		newFrequency	(qint64);
};

#endif

