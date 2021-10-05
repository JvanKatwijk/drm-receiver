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

#ifndef __DECIMATOR_25_H
#define __DECIMATOR_25_H

#include	"basics.h"
#include	<stdint.h>
#include	<complex>
#include	<vector>
#include	"decimator.h"

//
class	decimator_25 {
public:
	                 decimator_25	(int32_t);
			~decimator_25	();
	bool		Pass	(std::complex<DRM_FLOAT>, std::complex<DRM_FLOAT> *);
private:
	int		counter;
	int		amount;
	std::complex<DRM_FLOAT>	iValue;
};

#endif
