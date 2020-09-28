#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM backend
 *
 *    DRM backend is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM backend is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM backend; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#ifndef	__AUDIO_PROCESSOR__
#define	__AUDIO_PROCESSOR__

#include	<QObject>
#include	"radio-constants.h"
#include	<cstring>
#include	"fir-filters.h"
#include	"aac-processor-faad.h"
//#include	"aac-processor-fdk.h"
#include	"celp-processor.h"
#include	"hvxc-processor.h"
//#include	"xheaac-processor.h"

class	drmDecoder;
class	stateDescriptor;

class	audioDispatcher: public QObject {
Q_OBJECT
public:
		audioDispatcher   (stateDescriptor *, drmDecoder *);
                ~audioDispatcher  (void);
	void    process_audio   (uint8_t *, int16_t,
                                 int16_t, int16_t, int16_t, int16_t);
private:
	stateDescriptor 	*theState;
        drmDecoder		*drmMaster;
//	aacProcessor_fdk	my_aacProcessor;
	aacProcessor_faad	my_aacProcessor;
	celpProcessor		my_celpProcessor;
	hvxcProcessor		my_hvxcProcessor;
signals:
	void			show_audioMode	(QString);
};
#endif

