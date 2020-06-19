#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the swradio
 *
 *    swradio is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    swradio is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with swradio; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __FIR_FILTERS__
#define __FIR_FILTERS__

#include	"radio-constants.h"
#include	<vector>

class	HilbertFilter;

class	Basic_FIR {
public:
		Basic_FIR	(int16_t);
		~Basic_FIR	();
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

class	LowPassFIR : public Basic_FIR {
public:
			LowPassFIR (int16_t,	// order
	                            int32_t, 	// cutoff frequency
	                            int32_t	// samplerate
	                           );
			~LowPassFIR (void);
	DSPCOMPLEX	*getKernel	(void);
	void		newKernel	(int32_t);	// cutoff
};
//
//	Both for lowpass band bandpass, we provide:
class	decimatingFIR: public Basic_FIR {
public:
		         decimatingFIR	(int16_t, int32_t, int32_t, int16_t);
	                 decimatingFIR	(int16_t, int32_t, int32_t,
	                                                   int32_t, int16_t);
			~decimatingFIR	(void);
	void		newKernel	(int32_t);
	void		newKernel	(int32_t, int32_t);
	bool		Pass	(DSPCOMPLEX, DSPCOMPLEX *);
	bool		Pass	(float, float *);
private:
	int16_t	decimationFactor;
	int16_t	decimationCounter;
};

class	HighPassFIR: public Basic_FIR {
public:
			HighPassFIR	(int16_t, int32_t, int32_t);
			~HighPassFIR	(void);
	void		newKernel	(int32_t);
};

class	bandpassFIR: public Basic_FIR {
public:
			bandpassFIR	(int16_t, int32_t, int32_t, int32_t);
			~bandpassFIR	(void);
	DSPCOMPLEX	*getKernel	(void);
	void		newKernel	(int32_t, int32_t);
private:
};

class	BasicBandPass: public Basic_FIR {
public:
			BasicBandPass	(int16_t, int32_t, int32_t, int32_t);
			~BasicBandPass	(void);
	DSPCOMPLEX	*getKernel	(void);
private:
};

class	adaptiveFilter {
public:
		adaptiveFilter	(int16_t, float);
		~adaptiveFilter	();
	DSPCOMPLEX	Pass		(DSPCOMPLEX);

private:
	int16_t		ip;
	float		err;
	float		mu;
	int16_t		firsize;
	float	*Kernel;
	DSPCOMPLEX	*Buffer;
};

class HilbertFilter {
public:
		HilbertFilter	(int16_t, float, int32_t);
		~HilbertFilter	();
	DSPCOMPLEX	Pass		(DSPCOMPLEX);
	DSPCOMPLEX	Pass		(float, float);
private:
	int16_t		ip;
	int16_t		firsize;
	int32_t		rate;
	float	*cosKernel;
	float	*sinKernel;
	DSPCOMPLEX	*Buffer;
	void		adjustFilter	(float);
};

#endif

