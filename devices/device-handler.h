#
/*
 *    Copyright (C) 2021
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM-2
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
 *
 *	We have to create a simple virtual class here, since we
 *	want the interface with different devices (including  filehandling)
 *	to be transparent
 */

#pragma once

#include	<stdint.h>
#include	"radio-constants.h"
#include	<QObject>
#include	<QThread>
#include	<QDialog>


/**
  *	\class deviceHandler
  *	base class for devices for the fm software
  *	The class is not completely virtual, since it is
  *	used as a default in case selecting a "real" class did not work out
  */

#include	<stdint.h>

class	RadioInterface;
class	deviceHandler: public QThread {
Q_OBJECT
public:
			deviceHandler 	();
virtual			~deviceHandler 	();
virtual		void	setVFOFrequency	(int32_t);
virtual		int32_t	getVFOFrequency	();
virtual		bool	restartReader	();
virtual		void	stopReader	();
virtual		void	resetBuffer	();
virtual		int16_t	bitDepth	();
//
protected:
		int32_t	lastFrequency;
signals:
		void	dataAvailable	(int);
};


