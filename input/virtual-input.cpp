#
/*
 *    Copyright (C) 2010, 2011, 2012
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
 *
 * 	Default (void) implementation of
 * 	virtual input class
 */
#include	"virtual-input.h"

	virtualInput::virtualInput (void) {
	lastFrequency	= 100000;
}

	virtualInput::~virtualInput (void) {
}

int32_t	virtualInput::getRate	(void) {
	return KHz (192);
}

void	virtualInput::setVFOFrequency (int32_t f) {
	lastFrequency = f;
}

int32_t	virtualInput::getVFOFrequency	(void) {
	return lastFrequency;
}

bool	virtualInput::legalFrequency	(int32_t f) {
	(void)f;
	return true;
}

int32_t	virtualInput::defaultFrequency	(void) {
	return Khz (94700);
}

bool	virtualInput::restartReader	(void) {
	return true;
}

void	virtualInput::stopReader	(void) {
}

uint8_t	virtualInput::myIdentity	(void) {
	return NO_STICK;
}

int32_t	virtualInput::getSamples	(DSPCOMPLEX *v, int32_t amount) {
	(void)v; 
	(void)amount; 
	return 0;
}

int32_t	virtualInput::getSamples	(DSPCOMPLEX *v,
	                                 int32_t amount, int32_t segment) {
	(void)segment;
	(void)v; 
	(void)amount; 
	return 0;
}

int32_t	virtualInput::Samples		(void) {
	return 0;
}

void	virtualInput::resetBuffer	(void) {
}

int16_t	virtualInput::bitDepth		(void) {
	return 10;
}

