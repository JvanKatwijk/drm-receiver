//#
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%                                                                         %
//%%  University of Kaiserslautern, Institute of Communications Engineering  %
//%%  Copyright (C) 2004 Andreas Dittrich                                    %
//%%                                                                         %
//%%  Author(s)    : Andreas Dittrich (dittrich@eit.uni-kl.de)               %
//%%  Project start: 27.05.2004                                              %
//%%  Last change: 02.05.2005, 11:30                                         %
//%%  Changes      : |                                                       %
//%%                                                                         %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/*************************************************************************
*
*                           PA0MBO
*
*    COPYRIGHT (C)  2009  M.Bos 
*
*    This file is part of the distribution package RXAMADRM
*
*    This package is free software and you can redistribute is
*    and/or modify it under the terms of the GNU General Public License
*
*    More details can be found in the accompanying file COPYING
*************************************************************************/
/*
 *    Copyright (C) 2015
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
 *
 *	The Wiener algorithm used in this implementation is written 
 *	in C++  and is a translation of the algorithm as given in
 *	diorama 1.1, and inspired by the translitteration
 *	of the Matlab specification of that algorithm as done in RXAMADRM
 */
#
#include	"referenceframe.h"
#include	"basics.h"
#include	"equalizer-1.h"
#include	"estimator-2.h"
#include	"matrix2.h"
#include	"drm-decoder.h"

#define	realSym(x)	((x + symbolsinFrame)% symbolsinFrame)

		equalizer_1::equalizer_1 (drmDecoder	*parent,
	                              uint8_t		Mode,
	                              uint8_t		Spectrum,
	                              RingBuffer<std::complex<float>> *b):
	                                     equalizer_base (Mode, Spectrum) {
float	sigmaq_noise_list [] = {16.0, 14.0, 14.0, 12.0};
	sigmaq_noise	= pow (10.0, - sigmaq_noise_list [Mode - Mode_A] / 10.0);
	this	-> scopeMode	= SHOW_PILOTS;
	this	-> eqBuffer	= b;
	connect (this, SIGNAL (show_eqsymbol (int)),
                 parent, SLOT (show_eqsymbol (int)));


//	Based on table 92 ETSI ES 201980

//	Just for experimentation, we added some alternatives
int16_t		symbols_per_window_list []	= {10, 6, 8, 6};
//
//	first shorthands 
	symbols_per_window	= symbols_per_window_list [Mode - Mode_A];
	symbols_to_delay	= symbols_per_window / 2;
	periodforSymbols	= groupsperFrame (Mode);
	periodforPilots		= pilotDistance (Mode);
	windowsinFrame		= groupsperFrame (Mode);

	Ts			= Ts_of (Mode);
	Tu			= Tu_of (Mode);
	Tg			= Tg_of	(Mode);

	scopeMode		= SHOW_PILOTS;
//	we kunnen het aantal trainers redelijk schatten door
//	het aantal pilots per symbol te benaderen (carriers / afstand)
//	en te vermenigvuldigen met  het aantal symbols per window

	theTrainers		. resize (windowsinFrame);
	pilotEstimates		. resize (symbolsinFrame);
	for (int i = 0; i < symbolsinFrame; i ++)
	   pilotEstimates. at (i). resize (carriersinSymbol);

//	precompute 2-D-Wiener filter-matrix w.r.t. power boost
//	Reference: Peter Hoeher, Stefan Kaiser, Patrick Robertson:
//	"Two-Dimensional Pilot-Symbol-Aided Channel Estimation By
//	Wiener Filtering",
//	ISIT 1997, Ulm, Germany, June 29 - July 4
//	PHI	= auto-covariance-matrix
//	THETA	= cross-covariance-vector
//	f_cut_t	% two-sided maximum doppler frequency
//	                       (normalized w.r.t symbol duration Ts) 
//	f_cut_k % two-sided maximum echo delay
//	                       (normalized w.r.t useful symbol duration Tu)
//
//	values taken from diorama
	f_cut_t = 0.0675 / symbols_to_delay;
	f_cut_k = 1.75 * (float) Tg / (float) Tu;
//
//	This code is based on the diorama Matlab code, and a
//	(complete)rewrite of the C translation of this Matlab code by Ties Bos.
//
//
//	"Trainers" are formed by the regular gain cells for each window
//	We build up the "trainers" as relative addresses of the pilots
	for (int window = 0; window < windowsinFrame; window ++)  {
	   buildTrainers (window);
	   trainers_per_window [window] = theTrainers [window]. size ();
	}
//
//	The idea is to create a 3D matrix with for 
//	* each window
//	* and for each carrier in such a window,
//	  a vector with size "trainers" (from that window)
//	  with elements from Phi * Theta
//	* thereby is Phi a 2 dimensional matrix, one built for each window and
//	  Theta a once dimensional vector dependent, one built
//	  for each window and each carrier
//
	for (int window = 0; window < windowsinFrame; window ++) {
	   
	   W_symbol_blk [window] = new double *[carriersinSymbol];
	   for (int cs = 0; cs < carriersinSymbol; cs ++)
	      W_symbol_blk [window][cs] =
	                new double [theTrainers [window]. size ()];
	   int trainersInWindow		= trainers_per_window [window];
//	allocate a Phi and Theta
	   float *Theta = new float [trainersInWindow];
	   float **Phi	= new float *[trainersInWindow];
	   for (int Phi_el = 0;
	          Phi_el < trainersInWindow; Phi_el ++)
	      Phi [Phi_el] = new float [trainersInWindow];
//
//	as said, we create a Phi for each window
	   makePhi (Phi, window, f_cut_k, f_cut_t);

	   for (int carrier = K_min; carrier <= K_max; carrier ++) {	   
	      if (carrier == 0)
	         continue;

//	      and a Theta for each window and carrier
	      makeTheta (Theta, window, carrier, f_cut_k, f_cut_t);
//
//	   Given the Phi and Theta, we can fill one the vectors
//	   in the W_symbol_bl matrix.	
//	   W_symbol_blk [window, carrier] = Theta * Phi;
	      for (int j = 0; j < trainers_per_window [window]; j ++) {
	         W_symbol_blk [window][indexFor (carrier)][j] = 0;
	         for (int k = 0; k < trainers_per_window [window]; k ++)
	            W_symbol_blk [window][indexFor (carrier)][j] +=
	                                     Theta [k] * Phi [k][j];
	      }
	   }	// end carrier, W_symbol_blk done for this carrier

	   delete [] Theta;
	   for (int k = 0; k < trainers_per_window [window]; k ++)
	      delete [] Phi [k];
	   delete [] Phi;
	}	// end of loop over windows
//	The W_symbol_blk filters are ready now
//
//	and finally, the estimators
	estimators	= new estimator_2 *[symbolsinFrame];
	for (int i = 0; i < symbolsinFrame; i ++)
	   estimators [i] = new estimator_2 (refFrame, Mode, Spectrum, i);
	estimator_channel = new estimator_2 (refFrame, Mode, Spectrum, 1);
}

		equalizer_1::~equalizer_1 () {
int16_t	i;
	for (i = 0; i < symbolsinFrame; i ++)
	   delete estimators [i];
	delete [] estimators;
	delete estimator_channel;
//
//	W_symbol_blk is a matrix with three dimensions
	for (int window = 0; window < windowsinFrame; window ++) { 
	   for (i = 0; i < carriersinSymbol; i ++)
	      delete [] W_symbol_blk [window][i];
	   delete [] W_symbol_blk [window];
	}
}

//
//	We create one dimensional vector for theta, one
//	for each combination of window and carrier
void equalizer_1::makeTheta (float *Theta, int window,
	                        int carrier, float f_cut_k, float f_cut_t) {
trainer *currentTrainers = theTrainers [window]. data ();

	for (int trainer_1 = 0;
	            trainer_1 < theTrainers [window]. size (); trainer_1 ++) {
	   int16_t pilotSymbol	= currentTrainers [trainer_1]. symbol;
	   int16_t pilotCarrier	= currentTrainers [trainer_1]. carrier;
	   Theta [trainer_1] = sinc ((carrier - pilotCarrier) * f_cut_k)
                            * sinc ((symbols_to_delay - pilotSymbol) * f_cut_t);
	}
}

//	with PHI we create a 2 dim matrix over the trainter/trainer pairs
void	equalizer_1::makePhi (float **Phi, int window,
	                               float f_cut_k, float f_cut_t) {
int numOfTrainers	= theTrainers [window]. size ();
float *Phi_temp [numOfTrainers];
	for (int i = 0; i < numOfTrainers; i ++) 
	      Phi_temp  [i] = new float [numOfTrainers];

	trainer	*currentTrainers = theTrainers [window]. data ();
	for (int trainer_1 = 0; trainer_1 < numOfTrainers; trainer_1 ++) {
	   int16_t sym_1	= currentTrainers [trainer_1]. symbol;
	   int16_t car_1	= currentTrainers [trainer_1]. carrier;
	   for (int trainer_2 = 0; trainer_2 < numOfTrainers; trainer_2 ++) {
	      int16_t sym_2 = currentTrainers [trainer_2]. symbol;
	      int16_t car_2 = currentTrainers [trainer_2]. carrier;
	      Phi_temp [trainer_1][trainer_2] =
	                       sinc ((car_1 - car_2) * f_cut_k) *
	                       sinc ((sym_1 - sym_2) * f_cut_t);
	   }
	   std::complex<float> v = getPilotValue (Mode, Spectrum,
	                                             window + sym_1, car_1);
	   float amp = real (v * conj (v));
	   Phi_temp [trainer_1][trainer_1] += sigmaq_noise * 2.0 / amp;
	}
	gjinv (Phi_temp, numOfTrainers, Phi);

	for (int i = 0; i < theTrainers [window]. size (); i ++)
	   delete [] Phi_temp [i];
}
//	The "trainers" are built over the "regular" pilots, i.e.
//	those pilots that appear in the regular pilot pattern.
//	"Trainers" are encoded as a pair, with the symbol relative
//	to the start of the window
int16_t		equalizer_1::buildTrainers (int16_t window) {
int16_t symbol, carrier;
int16_t	myCount	= 0;
	theTrainers. at (window). resize (0);
	for (symbol = window;
	     symbol < window + symbols_per_window; symbol ++) {
	   for (carrier = K_min; carrier <= K_max; carrier ++) {
	      if (isPilotCell (Mode, symbol, carrier)) {
	         trainer temp;
	         temp. symbol	= symbol - window;
	         temp. carrier	= carrier;
	         theTrainers. at (window). push_back (temp);
	         myCount ++;
	      }
	   }
	}
	return myCount;
}

bool	equalizer_1::equalize (std::complex<float> *testRow,
	                       int16_t	newSymbol,
	                       myArray<theSignal>*outFrame,
	                       float	*offset_fractional,
	                       float	*delta_freq_offset,
	                       float	*sampleclockOffset,
	                       std::vector<std::complex<float>> &v) {
int16_t	carrier;
int16_t	symbol_to_process;
int16_t	i;

//	First, we copy the incoming vector to the appropriate vector
//	in the testFrame. Next we compute the estimates for the 
//	channels of the pilots.
//
//	Tracking the freqency offset is done by looking at the
//	phase difference of frequency pilots in subsequent words
	std::complex<float>	offs1	= std::complex<float> (0, 0);
	std::complex<float>	offs2	= std::complex<float> (0, 0);
	float		offsa	= 0;
	int		offs3	= 0;
	int		offsb	= 0;
	std::complex<float>	offs7	= std::complex<float> (0, 0);
	
	for (carrier = K_min; carrier <= K_max; carrier ++) {
	   if (carrier == 0)
	      continue;
	   std::complex<float> oldValue	= 
	                  testFrame [newSymbol][indexFor (carrier)];
	   testFrame [newSymbol][indexFor (carrier)] = 
	                                 testRow [indexFor (carrier)];
//
//	apply formula 5.40 from the Tsai book to get the SCO
	   if (isPilotCell (Mode, newSymbol, carrier)) {
	      offsa	+= arg (oldValue * conj (testRow [indexFor (carrier)]))  / symbols_to_delay * indexFor (carrier);
	      offsb	+= indexFor (carrier) * indexFor (carrier); 
	   }
//
//	For an estimate of the residual frequency offset, we
//	look at the average phase difference in the
//	frequency pilots of the last N symbols
	   if (isFreqCell (Mode, newSymbol, carrier)) {
	      for (i = 1; i < symbolsinFrame; i ++) {
	         offs1 += conj (testFrame [realSym (newSymbol - 1)]
	                                             [indexFor (carrier)]) *
	                   (testFrame [realSym (newSymbol)]
	                                             [indexFor (carrier)]);
	      }
	   }
//
//	alternatively, use the phase differences between successive
//	symbols with the same pilot layout
	   if (isPilotCell (Mode, newSymbol, carrier)) {
	      int16_t helpme = realSym (newSymbol - periodforSymbols);
	      std::complex<float> f1 =
	               testFrame [newSymbol][indexFor (carrier)] *
	                   conj (getPilotValue (Mode, Spectrum, newSymbol, carrier));
	      std::complex<float> f2 =
	               testFrame [helpme][indexFor (carrier)] *
	                   conj (getPilotValue (Mode, Spectrum, helpme, carrier));
	      offs7 += f1 * conj (f2);
	   }
	}

//	For an estimate of the residual sample time offset (includes
//	the phase offset of the LO), we look at the average of the
//	phase offsets of the subsequent pilots in the current symbol
	std::complex<float> prev_1 = std::complex<float> (0, 0);
	std::complex<float> prev_2 = std::complex<float> (0, 0);
	for (carrier = K_min; carrier <= K_max; carrier ++) {
	   if (isPilotCell (Mode, newSymbol, carrier)) {
//	Formula 5.26 (page 99, Tsai et al), average phase offset
	      if (offs3 > 0) 
	         offs2 += (testRow [indexFor (carrier)] * 
	                      conj (getPilotValue (Mode, Spectrum, newSymbol, carrier))) *
	                   conj (prev_1 * conj (prev_2));
	         
	      offs3 += 1;
	      prev_1 = testRow [indexFor (carrier)];
	      prev_2 = getPilotValue (Mode, Spectrum, newSymbol, carrier);
	   }
	}
//
//	the SCO is then
//	arg (offsa) / symbolsinFrame / (2 * M_PI * Ts / Tu * offsb)) * Ts;
//	The measured offset is in radials
	*sampleclockOffset = offsa / (2 * M_PI * (float (Ts) / Tu) * offsb);

//	still wondering about the scale
	*offset_fractional	= arg (offs2) / (2 * M_PI * periodforPilots);
//	the frequency error we measure in radials
//	we may choose here between two ways of computing
//	offs1 means using the frequency pilots over N symbols
//	offs7 means using all pilots over two near symbols with the same
//	pilot layout
	*delta_freq_offset	=  arg (offs1) / (3 * (symbolsinFrame - 1));
	*delta_freq_offset	=  arg (offs7) / periodforSymbols;
//	*delta_freq_offset	= (arg (offs1) + arg (offs7) / periodforSymbols) / 2;

//
//	If asked for we show (input/reference)
//
//	It took a while, but here it really begins
	estimators [newSymbol] ->
	              estimate (testFrame [newSymbol],
	                        pilotEstimates [newSymbol]. data ());

//	For equalizing symbol X, we need the pilotvalues
//	from the symbols X - symbols_to_delay .. X + symbols_to_delay - 1
//
//	We added the symbol at loc newSymbol, so we can equalize
//	the symbol "symbols_to_delay" back.
//	
	symbol_to_process = realSym (newSymbol - symbols_to_delay);
	
	if ((symbol_to_process == 1) && (scopeMode == SHOW_CHANNEL)) {
	   std::complex<float> result [K_max - K_min + 1];
	   for (int i = 0; i < K_max - K_min + 1; i ++)
	      result [i] = 0;
	   estimator_channel -> estimate_2 (testFrame [newSymbol], result);
	   eqBuffer -> putDataIntoBuffer (result, K_max - K_min + 1);
	   show_eqsymbol (K_max - K_min + 1);
	}

	processSymbol (symbol_to_process,
	               outFrame -> element (symbol_to_process),
	               v);

	if ((symbol_to_process == 3)  && (scopeMode == SHOW_ERROR)) {
	   std::complex<float> xx [K_max - K_min + 1];
	   int teller = 0;
	   for (int carrier = K_min; carrier <= K_max; carrier ++) {
	      if ((carrier != 0) &&
	          (isPilotCell (Mode, symbol_to_process, carrier))) {
	         std::complex<float> nom =
                    outFrame -> element (symbol_to_process)
	                                  [indexFor (carrier)]. signalValue;
	         std::complex<float>denom =
	                  getPilotValue (Mode, Spectrum, symbol_to_process, carrier);
	         xx [carrier - K_min] = (nom * conj (denom));
	         teller ++;
	      }
	      else
	         xx [carrier - K_min] = 0;
	   }
	   eqBuffer -> putDataIntoBuffer (xx, K_max - K_min + 1);
	   show_eqsymbol (K_max - K_min + 1);
	}

//	If we have a frame full of output: return true
	return symbol_to_process == symbolsinFrame - 1;
}
//
bool	equalizer_1::equalize (std::complex<float> *testRow,
	                       int16_t newSymbol,
	                       myArray<theSignal>*outFrame,
	                       std::vector<std::complex<float>> &v) {
//	Copy the incoming vector to the appropriate vector
//	in the testFrame. and compute the channel estimated for
//	the pilots.
	for (int carrier = K_min; carrier <= K_max; carrier ++) {
	   if (carrier == 0)
	      continue;
	   testFrame [newSymbol][indexFor (carrier)] =
	                         testRow [indexFor (carrier)];
	   if (isPilotCell (Mode, newSymbol, carrier))
	      pilotEstimates [newSymbol][indexFor (carrier)] =
	                  testRow [indexFor (carrier)] /
	                    getPilotValue (Mode, Spectrum, newSymbol, carrier);
	}
//
//	For equalizing symbol X, we need the pilotvalues
//	from the symbols X - symbols_to_delay .. X + symbols_to_delay - 1
//
//	We added the symbol at loc newSymbol, so we can equalize
//	the symbol "symbols_to_delay - 1" back.
//	
	int symbol_to_process = realSym (newSymbol - symbols_to_delay);
	processSymbol (symbol_to_process,
	               outFrame -> element (symbol_to_process), v);
//	If we have a frame full of output: return true
	return symbol_to_process == symbolsinFrame - 1;
}


//      Build a corrector for carrier "carrier" in the real Window

std::complex<float> equalizer_1::build_corrector (int symbol, int carrier,
	                                    int windowBase, int realWindow) {
trainer *trainers = theTrainers [windowBase]. data ();
std::complex<float>   res     = 0;
float	sum	= 0;
        for (int currentTrainer = 0;
             currentTrainer < theTrainers [windowBase]. size ();  currentTrainer ++) {
           int16_t relSym       = trainers [currentTrainer]. symbol;
           int16_t actualSymbol = realSym (relSym + realWindow);
           int16_t pilotCarrier = trainers [currentTrainer]. carrier;

           res += cmul (pilotEstimates [actualSymbol][indexFor (pilotCarrier)],
                        W_symbol_blk [windowBase][indexFor (carrier)]
                                                            [currentTrainer]);
           sum += W_symbol_blk [windowBase][indexFor (carrier)][currentTrainer];
        }  
        return cdiv (res, sum);
}

//	we have a symbol number (i.e. an index in the DRM frame)
//	process symbol "symbol", 
void	equalizer_1::processSymbol (int16_t symbol,
	                            theSignal *outVector,
	                            std::vector<std::complex<float>> &v) {
//	"ntwee" will indicate the "model" of the window, we deal with
//	while windowBase indicates the REAL window, i.e. the
//	the first symbol of the window as appearing in the frame.
int16_t	windowBase		= realSym (symbol +
	                                   symbols_to_delay -
	                                   symbols_per_window + 1);
int16_t	ntwee			= windowBase % windowsinFrame;
int16_t	nrTrainers		= trainers_per_window [ntwee];
trainer *currentTrainers	= theTrainers [ntwee]. data ();

//	The trainers are over N subsequent symbols in the frame, starting
//	at windowBase. The "model" is to be found at ntwee.
//	we determine the REAL address in the frame by first computing the
//	relative address, and then adding the base of the window over
//	the frame we are looking at

//	The outer loop refers to the targets for the
//	filtering,
//	the inner loop loops over the pilots (aka trainers) for
//	the reference window
	for (int carrier = K_min; carrier <= K_max; carrier ++) {
	   double sum = 0;
	   if (carrier == 0)
	      continue;
	   refFrame [symbol][indexFor (carrier)] =
	                build_corrector (symbol, carrier, ntwee, windowBase);
	}

	if ((symbol == 4) && (scopeMode == SHOW_PILOTS)) {
	   std::complex<float> xx [K_max - K_min + 1];
	   for (int carrier = K_min; carrier <= K_max; carrier ++)
	      if (isPilotCell (Mode, symbol, carrier)) 
	         xx [carrier - K_min] = refFrame [symbol][carrier - K_min] *
	                    conj (getPilotValue (Mode, Spectrum, symbol, carrier));
	      else
	         xx [carrier - K_min] = std::complex<float> (0, 0);
	   eqBuffer -> putDataIntoBuffer (xx, K_max - K_min + 1);
	   show_eqsymbol (K_max - K_min + 1);
	}

//	The transfer function is now there, stored in the appropriate
//	entry in the refFrame, so let us equalize
	for (int carrier = K_min; carrier <= K_max; carrier ++) {
	   std::complex<float> temp	=
	                   refFrame [symbol] [indexFor (carrier)];
	   if (carrier == 0)
	      outVector [indexFor (0)]. signalValue = 
	                                     std::complex<float> (0, 0);
	   else {
	      std::complex<float> qq =
	                 testFrame [symbol][indexFor (carrier)] / temp;
	      outVector [indexFor (carrier)] . signalValue = qq;
	   }
	   outVector [indexFor (carrier)]. rTrans = abs (temp);
	}
}

void	equalizer_1::set_scopeMode	(int mode) {
	scopeMode	= mode;
}

