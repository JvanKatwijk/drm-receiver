#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the rtty plugin
 *
 *    rtty plugin is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    rtty plugin is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with rtty plugin; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"decimator.h"
#define  _USE_MATH_DEFINES

#include <math.h>

	decimator::decimator (int16_t filterSize,
	                      int32_t low,
	                      int32_t high,
	                      int32_t sampleRate,
	                      int16_t Dm):
	                                Buffer (filterSize),
	                                filterKernel (filterSize) {
float	*tmp	= (float *)alloca (filterSize * sizeof (float));
float	lo	= (float)((high - low) / 2) / sampleRate;
float	shift	= (float) ((high + low) / 2) / sampleRate;
float	sum	= 0.0;

	this	-> sampleRate		= sampleRate;
	this	-> decimationFactor	= Dm;
	this	-> decimationCounter	= 0;
	this	-> filterSize		= filterSize;

	for (int i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else 
	      tmp [i] = sin (2 * M_PI * lo * (i - filterSize /2)) / (i - filterSize/2);
//
//	windowing
	   tmp [i]  *= (0.42 -
		        0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		        0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}
//
//	and modulate the kernel
	for (int i = 0; i < filterSize; i ++) {	// shifting
	   float v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = std::complex<float> (tmp [i] * cos (v) / sum, 
	                                           tmp [i] * sin (v) / sum);
	}

	for  (int i = 0; i < filterSize; i ++)
	   Buffer [i] = 0;
	ip	= 0;
}

	decimator::~decimator () {
}

bool	decimator::Pass (std::complex<float> z, std::complex<float> *z_out) {
int16_t		i;
std::complex<float>	tmp	= 0;
int16_t		index;

	Buffer [ip] = z;
	if (++decimationCounter < decimationFactor) {
	   ip =  (ip + 1) % filterSize;
	   return false;
	}
	decimationCounter = 0;
//
//	we are working with a circular buffer, we take two steps
//	we move from ip - 1 .. 0 with i going from 0 .. ip -1
	for (i = 0; i <= ip; i ++) {
	   index =  ip - i;
	   tmp 	+= Buffer [index] * filterKernel [i];
	}
//	and then we take the rest
	for (i = ip + 1; i < filterSize; i ++) {
	   index =  filterSize + ip - i;
	   tmp 	+= Buffer [index] * filterKernel [i];
	}

	ip = (ip + 1) % filterSize;
	*z_out = tmp;
	return true;
}


