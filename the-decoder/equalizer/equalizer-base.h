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

#pragma once
#include	"basics.h"
#include	<stdint.h>
#include	<complex>
#include	<vector>
#include	"my-array.h"
class	equalizer_base {
public:
			equalizer_base 	(uint8_t Mode, uint8_t Spectrum);
virtual			~equalizer_base	();
virtual	bool		equalize	(std::complex<float> *,
	                                 int16_t,
	                                 myArray<theSignal> *,
	                                 std::vector<std::complex<float>> &);
virtual	bool		equalize	(std::complex<float> *,
	                                 int16_t,
	                                 myArray<theSignal> *,
	                                 float *, float *, float *,
	                                 std::vector<std::complex<float>> &);
	int16_t		indexFor	(int16_t);
	std::complex<float>	**getChannels	();
	float		getMeanEnergy	();
protected:
	std::complex<float>	**testFrame;
	std::complex<float>	**refFrame;
	uint8_t		Mode;
	uint8_t		Spectrum;
	int16_t		K_min;
	int16_t		K_max;
	int16_t		symbolsinFrame;
	int16_t		carriersinSymbol;
	int16_t		actualRow		(int16_t, int16_t);
	float		meanEnergy;
	int16_t		nrCells;
	void		init_gain_ref_cells	(void);
};


