#
/*
 *    Copyright (C) 2019
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDR-J (JSDR).
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#include	"state-descriptor.h"
#include	<stdio.h>
#include	<string.h>

	stateDescriptor::stateDescriptor (drmDecoder *mr,
	                                  uint8_t Mode, uint8_t Spectrum) {
	this    -> master       = mr;
	this	-> Mode		= Mode;
	this	-> Spectrum	= Spectrum;
	this	-> RMflag	= 0;
	this	-> sdcMode	= 0;
	this	-> numofStreams	= 0;
	this	-> set		= false;
	for (int i = 0; i < 3; i ++)
	   memset (&streams [i], 0, sizeof (streams [0]));
}

	stateDescriptor::~stateDescriptor	(void) {
}

//	return the actual protection level from prot, for stream nr
void	stateDescriptor::protLevel	(uint8_t prot, int16_t nr,
	                                 int16_t *RX, int16_t *RY) {

	if (this -> QAMMode == QAM16)
	   protLevel_qam16 (prot, nr, RX, RY);
	else
	if (this -> QAMMode == QAM64)
	   protLevel_qam64 (prot, nr, RX, RY);
}

void	stateDescriptor::protLevel_qam16 (uint8_t prot, int16_t nr,
	                                  int16_t *RX, int16_t *RY) {
	switch (prot){
	   case 0:
	      if (nr == 0) {
	         *RX	= 1; *RY = 3;
	      }
	      else {		// nr better be '1'
	         *RX = 2; *RY = 3;
	      }
	      return;
	   default:		// should not happen
	   case 1:
	      if (nr == 0) {
	         *RX = 1; *RY = 2;
	       }
	       else {	// nr better be '1'
	          *RX = 3; *RY = 4;
	       }
	       return;
	}
}

void	stateDescriptor::protLevel_qam64 (uint8_t prot, int16_t nr,
	                                  int16_t *RX, int16_t *RY) {
	switch (prot) {
	   default:		// should not happen
	   case 0:
	      if (nr == 0) {
	         *RX = 1; *RY = 4;
	      }
	      else 
	      if (nr == 1) {
	         *RX = 1; *RY = 2;
	      }
	      else {		// nr better be '2'
	         *RX = 3; *RY = 4;
	      }
	      return;

	   case 1:
	      if (nr == 0) {
	         *RX = 1; *RY = 3;
	      }
	      else 
	      if (nr == 1) {
	         *RX = 2; *RY = 3;
	      }
	      else {		// nr better be '2'
	         *RX = 4; *RY = 5;
	      }
	      return;

	   case 2:
	      if (nr == 0) {
	         *RX = 1; *RY = 2;
	      }
	      else 
	      if (nr == 1) {
	         *RX = 3; *RY = 4;
	      }
	      else {		// nr better be '2'
	         *RX = 7; *RY = 8;
	      }
	      return;

	   case 3:
	      if (nr == 0) {
	         *RX = 2; *RY = 3;
	      }
	      else 
	      if (nr == 1) {
	         *RX = 4; *RY = 5;
	      }
	      else {		// nr better be '2'
	         *RX = 8; *RY = 9;
	      }
	      return;
	}
}

float	stateDescriptor::getRp	(uint8_t prot, int16_t nr) {
int16_t	RX, RY;

	protLevel (prot, nr, &RX, &RY);
	return (float (RX))/RY;
}

int16_t	stateDescriptor::getRYlcm_64	(int16_t protLevel) {
	if (protLevel == 3)
	   return 45;
	if (protLevel == 2)
	   return 8;
	if (protLevel == 1)
	   return 15;
	return 4;
}

int16_t	stateDescriptor::getRYlcm_16	(int16_t protLevel) {
	return protLevel == 1 ? 4 : 3;
}

