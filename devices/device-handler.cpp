#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of DRM-2
 *
 *    DRM-2 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM-2 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM-2; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"device-handler.h"
#include	"radio.h"

	deviceHandler::deviceHandler	() {
	lastFrequency	= Mhz (100);
}

	deviceHandler::~deviceHandler	() {
}

void	deviceHandler::setVFOFrequency	(int32_t f) {
	lastFrequency	= f;
}

int32_t	deviceHandler::getVFOFrequency	() {
	return lastFrequency;
}

bool	deviceHandler::restartReader	() {
	return true;
}

void	deviceHandler::stopReader	() {
}

void	deviceHandler::resetBuffer	() {
}

int16_t	deviceHandler::bitDepth		() {
	return 10;
}

