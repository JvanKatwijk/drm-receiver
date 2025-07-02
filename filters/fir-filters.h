#
/*
 *    Copyright (C) 2010, 2011, 2012
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

#pragma once

#include	"radio-constants.h"
#include	<vector>

class	HilbertFilter;

class	basicFIR {
public:
		basicFIR	(int16_t);
		~basicFIR	();
std::complex<float>	Pass	(std::complex<float>);
float		Pass		(float);
void		setTaps		(int16_t len,
	                         float *itaps, float *qtaps);
	std::vector<std::complex<float>> filterKernel;
	std::vector<std::complex<float>> Buffer;
	int	filterSize;
	int	ip;
	int	sampleRate;
};

class	lowpassFIR : public basicFIR {
public:
			lowpassFIR (int16_t,	// order
	                            int32_t, 	// cutoff frequency
	                            int32_t	// samplerate
	                           );
			~lowpassFIR (void);
	std::complex<float>	*getKernel	(void);
	void		newKernel	(int32_t);	// cutoff
};
//
//	Both for lowpass band bandpass, we provide:
class	decimatingFIR: public basicFIR {
public:
		         decimatingFIR	(int16_t, int32_t, int32_t, int16_t);
	                 decimatingFIR	(int16_t, int32_t, int32_t,
	                                                   int32_t, int16_t);
			~decimatingFIR	(void);
	void		newKernel	(int32_t);
	void		newKernel	(int32_t, int32_t);
	bool		Pass	(std::complex<float>, std::complex<float> *);
	bool		Pass	(float, float *);
private:
	int16_t	decimationFactor;
	int16_t	decimationCounter;
};

class	highpassFIR: public basicFIR {
public:
			highpassFIR	(int16_t, int32_t, int32_t);
			~highpassFIR	(void);
	void		newKernel	(int32_t);
};

class	bandpassFIR: public basicFIR {
public:
			bandpassFIR	(int16_t, int32_t, int32_t, int32_t);
			~bandpassFIR	(void);
	std::complex<float>	*getKernel	(void);
	void		newKernel	(int32_t, int32_t);
private:
};

class	basicbandPass: public basicFIR {
public:
			basicbandPass	(int16_t, int32_t, int32_t, int32_t);
			~basicbandPass	(void);
	std::complex<float>	*getKernel	(void);
private:
};

class	adaptiveFilter {
public:
		adaptiveFilter	(int16_t, float);
		~adaptiveFilter	();
	std::complex<float>	Pass		(std::complex<float>);

private:
	int16_t		ip;
	float		err;
	float		mu;
	int16_t		firsize;
	float		*Kernel;
	std::complex<float>	*Buffer;
};

class HilbertFilter {
public:
		HilbertFilter	(int16_t, float, int32_t);
		~HilbertFilter	();
	std::complex<float>	Pass		(std::complex<float>);
	std::complex<float>	Pass		(float, float);
private:
	int16_t		ip;
	int16_t		firsize;
	int32_t		rate;
	float		*cosKernel;
	float		*sinKernel;
	std::complex<float>	*Buffer;
	void		adjustFilter	(float);
};


