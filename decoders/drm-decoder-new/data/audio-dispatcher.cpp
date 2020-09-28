#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM backend
 *
 *    DRM backend is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM backend is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM backend; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include        "drm-decoder.h"
#include        "state-descriptor.h"
#include        <stdio.h>
#include        <float.h>
#include        <math.h>
#include        "basics.h"
#include	"audio-dispatcher.h"

	audioDispatcher::audioDispatcher (stateDescriptor *theState,
	                                  drmDecoder *drm):
	                                     my_aacProcessor (theState, drm),
	                                     my_celpProcessor (theState, drm),
	                                     my_hvxcProcessor (theState, drm) {

	this	-> theState	= theState;
	this	-> drmMaster	= drm;
	connect (this, SIGNAL (show_audioMode (QString)),
	         drmMaster, SLOT (show_audioMode (QString)));
	show_audioMode ("");
}

	audioDispatcher::~audioDispatcher	() {}


void	audioDispatcher::process_audio (uint8_t *v, int16_t mscIndex,
	                               int16_t startHigh, int16_t lengthHigh,
	                               int16_t startLow,  int16_t lengthLow) {
uint8_t	audioCoding		= theState -> streams [mscIndex]. audioCoding;

	switch (audioCoding) {
	   case 0:		// AAC
	      show_audioMode (QString ("AAC"));
	      my_aacProcessor. process_aac (v, mscIndex,
	                                    startHigh, lengthHigh,
	                                    startLow,  lengthLow);
	      return;

	   case 1:		// CELP
	      show_audioMode (QString ("CELP"));
	      my_celpProcessor. process_celp (v, mscIndex,
	                                      startHigh, lengthHigh,
	                                      startLow,  lengthLow);
	      return;

	   case 2:		// HVXC
	      show_audioMode (QString ("HVXC"));
	      my_hvxcProcessor. process_hvxc (v, mscIndex,
	                                      startHigh, lengthHigh,
	                                      startLow,  lengthLow);
	      return;

	   default:
	   case 3:
	      fprintf (stderr,
	               "unsupported format audioCoding (%d)\n", audioCoding);
	      return;
	}
}

