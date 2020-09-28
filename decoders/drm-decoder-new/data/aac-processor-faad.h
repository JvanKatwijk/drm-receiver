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
#ifndef	__AAC_PROCESSOR_FAAD__
#define	__AAC_PROCESSOR_FAAD__

#include	<QObject>
#include	<neaacdec.h>
#include	<stdio.h>
#include	<stdint.h>
#include	<cstring>
#include	"radio-constants.h"
#include	"fir-filters.h"
class	drmDecoder;
class	stateDescriptor;

typedef	struct frame {
	int16_t length, startPos;
	uint8_t	aac_crc;
	uint8_t audio [512];
} audioFrame; 

class	aacProcessor_faad: public QObject {
Q_OBJECT
public:
		aacProcessor_faad   (stateDescriptor *, drmDecoder *);
                ~aacProcessor_faad  (void);

        void	process_aac	(uint8_t *, int16_t, int16_t,
                                         int16_t, int16_t, int16_t);
private:
	stateDescriptor *theState;
        drmDecoder      *drmMaster;
//	DRM_aacDecoder  my_aacDecoder;
        lowpassFIR      upFilter_24000;
        lowpassFIR      upFilter_12000;
        int16_t         numFrames;
        int16_t         selectedAudioService;

        void		handle_uep_audio        (uint8_t *, int16_t,
	                                 int16_t, int16_t, int16_t, int16_t);
        void		handle_eep_audio (uint8_t *, int16_t, int16_t, int16_t);
        void		writeOut        (int16_t *, int16_t, int32_t);
        void		toOutput        (float *, int16_t);
        void		playOut         (int16_t);
//
//	added to support inclusion of the last phase
	bool		reinit		(std::vector<uint8_t>, int);
	bool		initDecoder	(int16_t	audioSampleRate,
                                         bool		SBR_used,
                                         uint8_t	audioMode);
        void		decodeFrame     (uint8_t *, int16_t,
                                         bool *, int16_t *,    // buffer
                                         int16_t *, int32_t *);
        void    closeDecoder    (void);

	uint8_t         aac_isInit;
        uint8_t         prev_audioSamplingRate;
        uint8_t         prevSBR_flag;
        uint8_t         prev_audioMode;
        int16_t         audioChannel;
//
//	added to support inclusion of the last phase
	NeAACDecHandle  theDecoder;
        bool            SBR_flag;
        uint8_t         audioMode;
        int16_t         audioRate;
	std::vector<uint8_t>
	                getAudioInformation (stateDescriptor *, int);
signals:
        void            show_audioMode  (QString);
        void            putSample       (float, float);
        void            faadSuccess     (bool);
//
//	added to support inclusion of the phase 
	void		aacData		(QString);
};
#endif

