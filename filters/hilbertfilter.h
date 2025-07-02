#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-wspr decoder
 *
 *    qt-wspr is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    qt-wspr is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-wspr; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once
#include	<stdio.h>
#include	<fftw3.h>
#include        <complex>
#include        <cstring>

using namespace std;


class	hilbertFilter {
public:
		hilbertFilter	(int size);
		~hilbertFilter	();
std::complex<float> Pass	(float);

private:
	std::complex<float>	*fftBuffer_1;
	std::complex<float>	*fftBuffer_2;
	fftwf_plan		plan_in;
        fftwf_plan		plan_uit;
	int			size;
	int			inp;
};


