#
/*
 *    Copyright (C) 2013 .. 2017
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
//
//	just some very general stuff
#ifndef	__DRM_BASICS__
#define	__DRM_BASICS__

#include	<stdio.h>
#include	<stdint.h>
#include	<complex>

enum    {
        QAM4, QAM16, QAM64
};

typedef	double JAN;

static inline
std::complex<JAN> cmul (std::complex<JAN> a, JAN b) {
	return std::complex<JAN> (real (a) * b, imag (a) * b);
}

static inline
std::complex<JAN> cdiv (std::complex<JAN> a, JAN b) {
	return std::complex<JAN> (real (a) / b, imag (a) / b);
}

struct	modeinfo {
	int16_t		Mode;
	uint8_t		Spectrum;
	JAN		sampleRate_offset;
//	JAN		timeOffset;
	JAN		freqOffset_fractional;
	int		freqOffset_integer;
	int16_t		timeOffset_integer;
	JAN		timeOffset_fractional;
};

typedef	struct modeinfo smodeInfo;

typedef struct {
        int     symbol;
        int     carrier;
} sdcCell;

typedef struct	ourSignal {
	std::complex<JAN>	signalValue;
	double		rTrans;
} theSignal;

struct metrics_struct {
	JAN	rTow0;
	JAN	rTow1;
};

typedef struct metrics_struct metrics;

//
//	rChan was computed by the equalizer,
//	in the current setting, the best results are
//	with a plain rDist (or squared)
static inline
JAN computeMetric (const JAN rDist, const JAN rChan) {
	return rDist * rDist * rChan;
//	return rDist * rChan;
//	return rDist * rDist;
	return rDist;
#ifdef USE_MAX_LOG_MAP
/* | r / h - s | ^ 2 * | h | ^ 2 */
	return rDist * rDist * rChan;
#else
/* | r / h - s | * | h | */
	return rDist * rChan;
#endif
}

static inline
JAN Minimum1 (const JAN rA, const JAN rB, const JAN rChan) {
//	The minimum in case of only one parameter is trivial 
	return computeMetric (fabs(rA - rB), rChan);
}

static inline
JAN	Minimum2 (const JAN rA,	// value to consider
	          const JAN rB1,	// reference 1
	          const JAN rB2,	// reference 2
	          const JAN rChan) {
        /* First, calculate all distances */
const JAN rResult1 = fabs (rA - rB1);
const JAN rResult2 = fabs (rA - rB2);

/* Return smallest one */
        return  (rResult1 < rResult2) ?
	              computeMetric (rResult1, rChan) :
	              computeMetric (rResult2, rChan);
}

static inline
JAN	Minimum2 (const JAN rA,
	          const JAN rX0,
	          const JAN rX1,
                  const JAN rChan,
	          const JAN rLVal0) {
        /* X0: L0 < 0
           X1: L0 > 0 */

/* First, calculate all distances */
JAN rResult1	= computeMetric (fabs (rA - rX0), rChan);
JAN rResult2	= computeMetric (fabs (rA - rX1), rChan);

/* Add L-value to metrics which to not correspond to correct hard decision */
	if (rLVal0 > 0.0)
            rResult1 += rLVal0;
        else
            rResult2 -= rLVal0;

/* Return smallest one */
        if (rResult1 < rResult2)
            return rResult1;
        else
            return rResult2;
}

static inline 
JAN	Minimum4 (const JAN rA, const JAN rB1, const JAN rB2,
	          const JAN rB3, JAN rB4, const JAN rChan) {
/* First, calculate all distances */
const JAN rResult1 = fabs (rA - rB1);
const JAN rResult2 = fabs (rA - rB2);
const JAN rResult3 = fabs (rA - rB3);
const JAN rResult4 = fabs (rA - rB4);

        /* Search for smallest one */
JAN rReturn = rResult1;
        if (rResult2 < rReturn)
            rReturn = rResult2;
        if (rResult3 < rReturn)
            rReturn = rResult3;
        if (rResult4 < rReturn)
            rReturn = rResult4;

        return computeMetric (rReturn, rChan);
}

#define		BITSPERBYTE	8

#define	Mode_A	1
#define	Mode_B	2
#define	Mode_C	3
#define	Mode_D	4

int16_t		Ts_of		(uint8_t);
int16_t		Tu_of		(uint8_t);
int16_t		Tg_of		(uint8_t);
int16_t		symbolsperFrame	(uint8_t);
int16_t		groupsperFrame	(uint8_t);
int16_t		pilotDistance	(uint8_t);
int16_t		symbolsperGroup	(uint8_t);
int16_t		Kmin		(uint8_t, uint8_t);
int16_t		Kmax		(uint8_t, uint8_t);
int16_t		ususedCarriers	(uint8_t);
JAN		sinc		(JAN);
#endif

