#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm-receiver
 *
 *    drm-receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm-receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm-receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	<stdio.h>
#include	"radio-constants.h"
#include        <fdk-aac/aacdecoder_lib.h>

#ifdef __MINGW32__
#define GETPROCADDRESS  GetProcAddress
#else
#define GETPROCADDRESS  dlsym
#endif

//	functions used from the libfdk-aac.so and dll

typedef LINKSPEC_H HANDLE_AACDECODER 
	 (*pfn_aacDecoder_Open) (TRANSPORT_TYPE transportFmt, UINT nrOfLayers);

typedef LINKSPEC_H void
	(*pfn_aacDecoder_Close)(HANDLE_AACDECODER self);

typedef LINKSPEC_H AAC_DECODER_ERROR
	(*pfn_aacDecoder_ConfigRaw)(HANDLE_AACDECODER self,
                                                  UCHAR *conf[],
                                                  const UINT length[]);

typedef LINKSPEC_H CStreamInfo *
	(*pfn_aacDecoder_GetStreamInfo)(HANDLE_AACDECODER self);

typedef LINKSPEC_H AAC_DECODER_ERROR
	(*pfn_aacDecoder_Fill)(HANDLE_AACDECODER self,
                                             UCHAR *pBuffer[],
                                             const UINT bufferSize[],
                                             UINT *bytesValid);

typedef LINKSPEC_H AAC_DECODER_ERROR
	(*pfn_aacDecoder_DecodeFrame)(HANDLE_AACDECODER self,
                                                    INT_PCM *pTimeData,
                                                    const INT timeDataSize,
                                                    const UINT flags);


class	aacHandler {
public:
		aacHandler		(int);
		~aacHandler		();
	

	pfn_aacDecoder_Open		aacDecoder_Open;
	pfn_aacDecoder_Close		aacDecoder_Close;
	pfn_aacDecoder_ConfigRaw	aacDecoder_ConfigRaw;
	pfn_aacDecoder_GetStreamInfo	aacDecoder_GetStreamInfo;
	pfn_aacDecoder_Fill		aacDecoder_Fill;
	pfn_aacDecoder_DecodeFrame	aacDecoder_DecodeFrame;
private:	
	HINSTANCE			Handle;
};


