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
#include	"celp-processor.h"

static	inline
uint16_t	get_MSCBits (uint8_t *v, int16_t offset, int16_t nr) {
int16_t		i;
uint16_t	res	= 0;

	for (i = 0; i < nr; i ++) 
	   res = (res << 1) | (v [offset + i] & 01);

	return res;
}
	celpProcessor::celpProcessor (stateDescriptor *theState,
	                              drmDecoder *drm) {
	this	-> theState	= theState;
	this	-> drmMaster	= drmMaster;
}

	celpProcessor::~celpProcessor () {}

//
void	celpProcessor::process_celp (uint8_t *v, int16_t mscIndex,
	                             int16_t startHigh, int16_t lengthHigh,
	                            int16_t startLow,  int16_t lengthLow) {
uint8_t	audioCoding	= theState -> streams [mscIndex]. audioCoding;
uint8_t	SBR_flag	= theState -> streams [mscIndex]. SBR_flag;
//uint8_t audioMode	= theState -> streams [mscIndex]. audioMode;
//uint8_t audioSamplingRate	= theState -> streams [mscIndex].
//	                                                   audioSamplingRate;
//uint8_t textFlag	= theState -> streams [mscIndex]. textFlag;
//uint8_t enhancementFlag	= theState -> streams [mscIndex].
//	                                                   enhancementFlag;
//uint8_t coderField	= theState -> streams [mscIndex]. coderField;

	(void)v; (void)mscIndex;
	(void)startHigh; (void)startLow;
	(void)lengthHigh; (void)lengthLow;
	if (SBR_flag == 0)
	   fprintf (stderr, "rfa = %d, CELP_CRC = %d\n",
	                     (audioCoding & 02) >> 1, audioCoding & 01);
	else
	   fprintf (stderr, "sbr_header_flag = %d, CELP_CRC = %d\n",
	                    (audioCoding & 02) >> 1, audioCoding & 01);
}

