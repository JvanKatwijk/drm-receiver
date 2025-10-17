#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm receiver
 *
 *    drm receiver is free software; you can redistribute it and/or modify
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
 */
#
#include	"referenceframe.h"
#include	"equalizer-base.h"

		equalizer_base::equalizer_base (uint8_t Mode,
	                                        uint8_t Spectrum) {
int16_t	i;
	this	-> Mode		= Mode;
	this	-> Spectrum	= Spectrum;
	this	-> K_min	= Kmin (Mode, Spectrum);
	this	-> K_max	= Kmax (Mode, Spectrum);
	this	-> symbolsinFrame	= symbolsperFrame (Mode);
	this	-> carriersinSymbol	= Kmax (Mode, Spectrum) -
	                                  Kmin (Mode, Spectrum) + 1;
	
	init_gain_ref_cells ();
	this	-> refFrame	= new std::complex<DRM_FLOAT> *[symbolsinFrame];
	for (i = 0; i < symbolsinFrame; i ++)
	   refFrame [i] = new std::complex<DRM_FLOAT> [carriersinSymbol];
	this	-> testFrame	= new std::complex<DRM_FLOAT> *[symbolsinFrame];
	for (i = 0; i < symbolsinFrame; i ++)
	   testFrame [i] = new std::complex<DRM_FLOAT> [carriersinSymbol];
}

		equalizer_base::~equalizer_base () {
int16_t	i;
	for (i = 0; i < symbolsinFrame; i ++) {
	   delete [] refFrame [i];
	   delete [] testFrame [i];
	}
	delete []	refFrame;
	delete []	testFrame;
}

int16_t	equalizer_base::indexFor (int16_t carrier) {
	return carrier - Kmin (Mode, Spectrum);
}
	
void	equalizer_base::init_gain_ref_cells () {
int16_t symbol, carrier;
DRM_FLOAT	mean_energy	= 0;
//int16_t	cellCount	= 0;
int16_t	totalCount	= 0;

	for (symbol = 0; symbol < symbolsperFrame (Mode); symbol ++) {
//	   int16_t prevCarriers = symbol * (carriersinSymbol);
	   for (carrier = K_min; carrier <= K_max; carrier ++) {
	      totalCount ++;
	      if (isPilotCell (Mode, symbol, carrier)) {
//	         gain_ref_cells_k [cellCount] = carrier + prevCarriers;
//	         gain_ref_cells_v [cellCount ++] =
//	                 getPilotValue (Mode, Spectrum, symbol, carrier);
	
	         mean_energy += 
	                 abs (getPilotValue (Mode, Spectrum, symbol, carrier)) *
	                 abs (getPilotValue (Mode, Spectrum, symbol, carrier));
	      }
	      else
	      if (isFreqCell (Mode, symbol, carrier)) {
	         mean_energy += abs (getFreqRef (Mode, symbol, carrier)) *
	                        abs (getFreqRef (Mode, symbol, carrier));
	          
	      }
	      else
	      if (isTimeCell (Mode, symbol, carrier)) {
	         mean_energy += abs (getTimeRef (Mode, symbol, carrier)) *
	                        abs (getTimeRef (Mode, symbol, carrier));
	      }
	      else
	         mean_energy ++;
	   }
	}
	mean_energy /= totalCount;
	meanEnergy	= mean_energy;
	nrCells		= totalCount;
//	fprintf (stderr, " energy %f, cells  %d (total %d)\n",
//	                       meanEnergy, cellCount, totalCount);
}

int16_t	equalizer_base::actualRow (int16_t symbol, int16_t firstRow) {
	return symbol + firstRow >= symbolsinFrame ?
	         symbol + firstRow - symbolsinFrame : symbol + firstRow;
}

float	equalizer_base::getMeanEnergy	() {
	return meanEnergy;
}

std::complex<float> **equalizer_base::getChannels	() {
	return refFrame;
}

//
//
bool	equalizer_base::equalize (std::complex<float> *testRow,
	                          int16_t index,
	                          myArray<theSignal> *outFrame,
	                          std::vector<std::complex<float>> &v) {
	(void) testRow;
	(void) index;
	(void) outFrame; 
	(void)v;
	return false;
}

bool	equalizer_base::equalize (std::complex<DRM_FLOAT> *testRow,
	                          int16_t index,
	                          myArray<theSignal> *outFrame,
	                          float	 *fractout,
	                          float	 *freqOut,
	                          float	 *sampleclockOffset,
	                          std::vector<std::complex<float>> &v) {
	(void) testRow;
	(void) index;
	(void) outFrame; 
	(void)v;
	*fractout	= 0;
	*freqOut	= 0;
	*sampleclockOffset	= 0;
	return false;
}

