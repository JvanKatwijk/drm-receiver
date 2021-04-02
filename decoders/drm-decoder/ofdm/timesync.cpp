#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm receiver
 *
 *    drm receiverr is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	The "getMode" function is a reimplementation of the
 *	MMSE algorithm (described in e.g. Tsai), some parts of it
 *	are taken over (or at least inspired by) the RXAMADRM software
 *	and its origin, diorama. 
 */
#include	"timesync.h"
#include	"reader.h"
#include	"drm-decoder.h"
#include	"basics.h"


	timeSyncer::timeSyncer (Reader *theReader,
	                        int32_t	sampleRate, int16_t nSymbols) {

	this	-> theReader	= theReader;
	this	-> sampleRate	= sampleRate;
	this	-> nSymbols	= nSymbols;
//	for mode determination:
	this	-> nSamples	= nSymbols * Ts_of (Mode_A);
//	Tu_of (Mode_D) is the smallest of all, so the vectors
//	all have sufficient length
	summedCorrelations	= new std::complex<float> [nSamples - Tu_of (Mode_D)];
	summedSquares		= new float [nSamples - Tu_of (Mode_D)];
	EPSILON			= 1.0E-10;
}

	timeSyncer::~timeSyncer (void) {
	delete []	summedCorrelations;
	delete []	summedSquares;
}
//
//	Formula 5.16 from Tsai reads
//	Phi (m) = sum (r=0, R-1, abs (z[m - r] ^ 2)) +
//	          sum (r=0, R-1, abs (z[m - r - L]) ^ 2) -
//	       2 * abs (sum (r=0, R-1, z [m - r] * conj (z [m - r - L])));
//	The start of the symbol is there where
//	Phi (m) is minimal
//
void	timeSyncer::getMode (smodeInfo	*result) {
float	gammaRelative;
float	list_gammaRelative 	[]	= {0.0, 0.0, 0.0, 0.0};
float	list_epsilon		[]	= {0.0, 0.0, 0.0, 0.0};
int16_t	list_Offsets 		[]	= {0,   0,   0,   0};
int16_t b [nSymbols];
int16_t	averageOffset	= 0;
int16_t	theMode;

	theReader -> waitfor (nSamples + 100);

//	just try all modes
	for (int mode = Mode_A; mode <= Mode_D; mode++) {
	   compute_gammaRelative (mode,
	                          &list_gammaRelative	[mode - Mode_A],
	                          &list_epsilon		[mode - Mode_A],
	                          &list_Offsets		[mode - Mode_A]);
	}
//
//	we have collected for all modes the gamma and the theta
//	now decide for the mode to be detected 
	theMode		= Mode_B;		// default
	gammaRelative	= -1.0E20;
	for (int i = Mode_A; i <= Mode_D; i++) {
//	   fprintf (stderr, "%f ", list_gammaRelative [i - Mode_A]);
	   if (list_gammaRelative [i - Mode_A] > gammaRelative) {
	      gammaRelative = list_gammaRelative [i - Mode_A];
	      theMode = i;
	   }
	}
//	fprintf (stderr, "\n");

//	check if result is reliable 
	bool	maxOK = true;			/* assume reliable */
//	if (gammaRelative < 0.1)
	if (gammaRelative < 0.3)
//	if (gammaRelative < 0.5)
	   maxOK = false;
	else
	for (int i = Mode_A; i <= Mode_D; i++) {
	   if ((i != theMode) && (list_gammaRelative [i - Mode_A] >
	                           0.80 * gammaRelative))
	      maxOK = false;
	}
	
	if (!maxOK)	{		// no reliable mode found
	   result -> Mode		= -1;
	   result -> sampleRate_offset	= 0.0;
	   result	-> timeOffset_integer	= 0;
	   result	-> timeOffset_fractional	= 0;
	   result	-> freqOffset_integer	= 0;
	   result	-> freqOffset_fract	= 0.0;
	   return;
	}
//
//
//	OK, we seem to have a mode "theMode". The offset indicates the
//	offset in the buffer, and from that we compute the timeoffset.

	averageOffset = list_Offsets [theMode - 1];
//
        compute_bestIndices (theMode, b, averageOffset);
//
//	Now least squares to 0...symbols_to_check and b [0] .. */
	float	sumx	= 0.0;
	float	sumy	= 0.0;
	float	sumxx	= 0.0;
	float	sumxy	= 0.0;

	for (int i = 0; i < nSymbols - 1; i++) {
	   sumx	+= (float) i;
	   sumy += (float) b [i];
	   sumxx += (float) i * (float) i;
	   sumxy += (float) i * (float) b [i];
	}

	float	slope, boffs;
	slope = (float) (((nSymbols - 1) * sumxy - sumx * sumy) /
	                 ((nSymbols - 1) * sumxx - sumx * sumx));
	boffs = (float) ((sumy * sumxx - sumx * sumxy) /
	                 ((nSymbols - 1) * sumxx - sumx * sumx));

//	OK, all set, we are done

	result -> Mode		= theMode;
	result -> sampleRate_offset = slope / ((float)Ts_of (theMode));
	fprintf (stderr, "samplerate offset %f\n", result -> sampleRate_offset);
	float	timeOffset	= fmodf (
	                          (Tg_of (theMode) + Ts_of (theMode) / 2 +
	                                      averageOffset + boffs - 1),
	                                            (float)Ts_of (theMode));
	timeOffset	= boffs;
//	result	-> timeOffset_integer	= floor (timeOffset + 0.5);
	result	-> timeOffset_integer	= floor (timeOffset + 0.5);
	result	-> timeOffset_fractional = timeOffset -
	                                         result -> timeOffset_integer;
	result -> freqOffset_fract = list_epsilon [theMode - Mode_A];
}

void	timeSyncer::compute_gammaRelative (uint8_t	mode,
	                                   float	*gammaRelative,
	                                   float	*Epsilon,
	                                   int16_t	*Offsets) {
int16_t	Ts		= Ts_of (mode);
int16_t Tu		= Tu_of (mode);
int16_t Tg		= Ts - Tu;
std::complex<float> gamma	[Ts];	// large enough
float	squareTerm	[Ts];
int32_t i, j, k, theOffset;

	memset (gamma,	0, Ts * sizeof (std::complex<float>));
	memset (squareTerm, 0, Ts * sizeof (float));

	for (i = 0; i < Ts; i ++) {
	   gamma [i]	= std::complex<float> (0, 0);
	   squareTerm [i] = float (0);
	   for (j = 0; j < nSymbols; j ++) {
	      int32_t base = theReader -> currentIndex + i;
	      int32_t mask = theReader -> bufSize - 1;
	      for (k = 0; k < Tg; k ++) {
	         std::complex<float> f1	=
	               theReader -> data [(base + j * Ts + k     ) & mask];
	         std::complex<float> f2	=
	               theReader -> data [(base + j * Ts + Tu + k) & mask];
	         gamma [i]	+= f1 * conj (f2);
	         squareTerm [i] += (real (f1 * conj (f1)) +
	                                  real (f2 * conj (f2)));
	      }
	   }
	}

	theOffset		= 0;
	float minValue		= 10000.0;
	for (i = 0; i < Ts; i ++) {
	   float mmse = abs (squareTerm [i] - 2 * abs (gamma [i]));
	   if (mmse < minValue) {
	      minValue = mmse;
	      theOffset = i;
	   }
	}
	*gammaRelative	= abs (gamma [theOffset]) / squareTerm [theOffset];
	*Epsilon	= (float) arg (gamma [theOffset]);
	*Offsets	= theOffset;
}

void	timeSyncer::compute_bestIndices (uint8_t Mode, 
	                                 int16_t   *b,
                                         float     averageOffset) {
	for (int i = 0; i < nSymbols; i ++)
	   b [i] = get_intOffset (Mode, averageOffset, nSymbols, 8);
}

int	timeSyncer::get_intOffset	(uint8_t Mode, int base,
	                                 int nrSymbols, int range) {
int	bestIndex = -1;
double	min_mmse = 10E20;
int32_t bufMask = theReader -> bufSize - 1;

	for (int i = base   - range / 2; i < base + range / 2; i ++) {
	   int index = (theReader -> currentIndex + base + i) & bufMask;
	   double mmse = compute_mmse (Mode, index, nrSymbols);
	   if (mmse < min_mmse) {
	      min_mmse = mmse;
	      bestIndex = i;
	   }
	}
	
	return bestIndex;
}

double	timeSyncer::compute_mmse (uint8_t Mode,
	                         int starter, int nrSymbols) {
int	Tg	= Tg_of (Mode);
int	Ts	= Ts_of (Mode);
int	Tu	= Tu_of (Mode);
std::complex<float>	gamma = std::complex<float> (0, 0);
double	squares = 0;
int32_t		bufMask	= theReader -> bufSize - 1;

	theReader -> waitfor (nrSymbols * Ts + Ts);
	for (int i = 0; i < nrSymbols - 1; i ++) {
	   int startSample = starter + i * Ts;
	   for (int j = 0; j < Tg; j ++) {
	      std::complex<float> f1 =
	             theReader -> data [(startSample + j) & bufMask];
	      std::complex<float> f2 =
	             theReader -> data [(startSample + Tu + j) & bufMask];
	      gamma	+= f1 * conj (f2);
	      squares	+= real (f1 * conj (f1)) + real (f2 * conj (f2));
	   }
	}
	return abs (squares - 2 * abs (gamma));
}

