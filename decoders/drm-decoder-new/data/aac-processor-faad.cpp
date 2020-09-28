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
#include        "drm-decoder.h"
#include        "state-descriptor.h"
#include        <stdio.h>
#include        <float.h>
#include        <math.h>
#include        "basics.h"
#include	"aac-processor-faad.h"

static	inline
uint16_t	get_MSCBits (uint8_t *v, int16_t offset, int16_t nr) {
int16_t		i;
uint16_t	res	= 0;

	for (i = 0; i < nr; i ++) 
	   res = (res << 1) | (v [offset + i] & 01);

	return res;
}
//
	aacProcessor_faad::aacProcessor_faad (stateDescriptor *theState,
	                                      drmDecoder *drm):
	                                      upFilter_24000 (5, 12000, 48000),
	                                      upFilter_12000 (5, 6000, 48000) {

	this	-> theState	= theState;
	this	-> drmMaster	= drm;
	this	-> theDecoder	= nullptr;
	this	-> SBR_flag	= false;
	this	-> audioMode	= 0x77;
	this	-> audioRate	= 0x77;
	connect (this, SIGNAL (show_audioMode (QString)),
	         drmMaster, SLOT (show_audioMode (QString)));
	connect (this, SIGNAL (putSample (float, float)),
	         drmMaster, SLOT (sampleOut (float, float)));
	connect (this, SIGNAL (faadSuccess (bool)),
	         drmMaster, SLOT (faadSuccess (bool)));
	connect (this, SIGNAL (aacData (QString)),
	         drm, SLOT (aacData (QString)));
	show_audioMode ("");
}

	aacProcessor_faad::~aacProcessor_faad	() {
	closeDecoder ();
}

void	aacProcessor_faad::process_aac (uint8_t *v, int16_t mscIndex,
	                                int16_t startHigh, int16_t lengthHigh,
	                                int16_t startLow,  int16_t lengthLow) {
	if (lengthHigh != 0) 
	   handle_uep_audio (v, mscIndex, startHigh, lengthHigh,
	                            startLow, lengthLow - 4);
	else 
	   handle_eep_audio (v, mscIndex,  startLow, lengthLow - 4);
}


static
int16_t outBuffer [8 * 960];
static
audioFrame f [20];
void	aacProcessor_faad::handle_uep_audio (uint8_t *v, int16_t mscIndex,
	                                     int16_t startHigh,
	                                     int16_t lengthHigh,
	                                     int16_t startLow,
	                                     int16_t lengthLow) {
int16_t	headerLength, i, j;
int16_t	usedLength	= 0;
int16_t	crcLength	= 1;
int16_t	payloadLength;

//	first the globals
	numFrames = theState -> streams [mscIndex]. audioSamplingRate == 1 ? 5 : 10;
	headerLength = numFrames == 10 ? (9 * 12 + 4) / 8 : (4 * 12) / 8;
	payloadLength = lengthLow + lengthHigh - headerLength - crcLength;
	
//	Then, read the numFrames - 1 "length"s from the header:
	for (i = 0; i < numFrames - 1; i ++) {
	   f [i]. length = get_MSCBits (v, startHigh * 8 + 12 * i, 12);
	   if (f [i]. length < 0)
	      return;
	   usedLength += f [i]. length;
	}
//	the length of the last segment is to be computed
	f [numFrames - 1]. length = payloadLength - usedLength;

//	OK, now it is getting complicated, we first load the part(s) from the
//	HP part. Length for all parts is equal, i.e. LengthHigh / numFrames
//	the audiopart is one byte shorter, due to the crc
	int16_t segmentinHP	= (lengthHigh - headerLength) / numFrames;
	int16_t audioinHP	= segmentinHP - 1;
	int16_t	entryinHP	= startHigh + headerLength;

	for (i = 0; i < numFrames; i ++) {
	   for (j = 0; j < audioinHP; j ++)
	      f [i]. audio [j] = get_MSCBits (v,
	                        (entryinHP ++) * 8, 8);
	   f [i]. aac_crc = get_MSCBits (v, 
	                        (entryinHP ++) * 8, 8);
	}

//	Now what is left is the part of the frame in the LP part

	int16_t entryinLP	= startLow;
	for (i = 0; i < numFrames; i ++) {
	   for (j = 0;
	        j < f [i]. length - audioinHP;
	        j ++)
	      if (entryinLP < startLow + lengthLow)
	         f [i]. audio [audioinHP + j] =
	                    get_MSCBits (v, (entryinLP++) * 8, 8);
	}
	playOut (mscIndex);
}

void	aacProcessor_faad::handle_eep_audio (uint8_t *v,
	                                     int16_t mscIndex,
	                                     int16_t startLow,
	                                     int16_t lengthLow) {
int16_t		i, j;
int16_t		headerLength;
int16_t		crc_start;
int16_t		payLoad_start;
int16_t		payLoad_length = 0;

	numFrames = theState -> streams [mscIndex]. audioSamplingRate == 1 ? 5 : 10;
	headerLength = numFrames == 10 ? (9 * 12 + 4) / 8 : (4 * 12) / 8;

//	startLow in bytes!!
	f [0]. startPos = 0;
	for (i = 1; i < numFrames; i ++) {
	   f [i]. startPos = get_MSCBits (v,
	                                  startLow * 8 + 12 * (i - 1), 12);
	}
	for (i = 1; i < numFrames; i ++) {
	   f [i - 1]. length = f [i]. startPos - f [i - 1]. startPos;
	   if (f [i - 1]. length < 0 ||
	       f [i - 1]. length >= lengthLow) {
	      faadSuccess (false);
	      return;
	   }
	   payLoad_length += f [i - 1]. length;
	}

	f [numFrames - 1]. length = lengthLow - 
	                            (headerLength + numFrames) -
	                            payLoad_length;
	if (f [numFrames - 1]. length < 0) {
	   faadSuccess (false);
	   return;
	}
//
//	crc_start in bytes
	crc_start	= startLow + headerLength;
	for (i = 0; i < numFrames; i ++)
	   f [i]. aac_crc = get_MSCBits (v, (crc_start + i) * 8, 8);

//	The actual audiobits (bytes) starts at crc_start + numFrames
	payLoad_start	= crc_start + numFrames; // the crc's

	for (i = 0; i < numFrames; i ++) {
	   for (j = 0; j < f [i]. length; j ++) {
	      int16_t in2 = (payLoad_start + f [i]. startPos + j);
	      f [i]. audio [j] = get_MSCBits (v, in2 * 8, 8);
	   }
	}
	playOut (mscIndex);
}

void	aacProcessor_faad::playOut (int16_t	mscIndex) {
int16_t	i;
//uint8_t	audioSamplingRate	= theState -> streams [mscIndex].
//	                                                   audioSamplingRate;
//uint8_t	SBR_flag		= theState -> streams [mscIndex].
//	                                                   SBR_flag;
//uint8_t	audioMode		= theState -> streams [mscIndex].
//	                                                   audioMode;

std::vector<uint8_t>audioDescriptor =
	getAudioInformation (theState, mscIndex);

	if (!reinit (audioDescriptor, mscIndex)) {
	   faadSuccess (false);
	   return;
	}

	for (i = 0; i < numFrames; i ++) {
	   int16_t	index = i;
	   bool		convOK;
	   int16_t	cnt;
	   int32_t	rate;
	   if (f [index]. length < 0)
              continue;
#if 0
           fprintf (stderr, "Frame %d (numFrames %d) length %d\n",
                                  index, numFrames, f [index]. length + 1);
#endif
	   decodeFrame ((uint8_t *)(&f [index]. aac_crc),
	                            f [index]. length + 1,
	                            &convOK,
	                            outBuffer,
	                            &cnt, &rate);
	   if (convOK) {
	      faadSuccess (true);
	      writeOut (outBuffer, cnt, rate);
	   }
	   else
	      faadSuccess (false);
	}
}

void	aacProcessor_faad::toOutput (float *b, int16_t cnt) {
int16_t	i;
	if (cnt == 0)
	   return;

	for (i = 0; i < cnt / 2; i ++)
	   putSample (b [2 * i], b [2 * i + 1]);
}

void	aacProcessor_faad::writeOut (int16_t *buffer, int16_t cnt,
	                             int32_t pcmRate) {
int16_t	i;
	if (pcmRate == 48000) {
	   float lbuffer [cnt];
	   for (i = 0; i < cnt / 2; i ++) {
	      lbuffer [2 * i]     = float (buffer [2 * i] / 32767.0);
	      lbuffer [2 * i + 1] = float (buffer [2 * i + 1] / 32767.0);
	   }
	   toOutput (lbuffer, cnt);
	   return;
	}

	if (pcmRate == 12000) {
	   float lbuffer [4 * cnt];
	   for (i = 0; i < cnt / 2; i ++) {
	      DSPCOMPLEX help =
	           upFilter_12000. Pass (DSPCOMPLEX (buffer [2 * i] / 32767.0,
	                                           buffer [2 * i + 1] / 32767.0));
	      lbuffer [8 * i + 0] = real (help);
	      lbuffer [8 * i + 1] = imag (help);
	      help = upFilter_12000. Pass (DSPCOMPLEX (0, 0));
	      lbuffer [8 * i + 2] = real (help);
	      lbuffer [8 * i + 3] = imag (help);
	      help = upFilter_12000. Pass (DSPCOMPLEX (0, 0));
	      lbuffer [8 * i + 4] = real (help);
	      lbuffer [8 * i + 5] = imag (help);
	      help = upFilter_12000. Pass (DSPCOMPLEX (0, 0));
	      lbuffer [8 * i + 6] = real (help);
	      lbuffer [8 * i + 7] = imag (help);
	   }
	   toOutput (lbuffer, 4 * cnt);
	   return;
	}
	if (pcmRate == 24000) {
	   float lbuffer [2 * cnt];
	   for (i = 0; i < cnt / 2; i ++) {
	      DSPCOMPLEX help =
	           upFilter_24000. Pass (DSPCOMPLEX (buffer [2 * i] / 32767.0,
	                                           buffer [2 * i + 1] / 32767.0));
	      lbuffer [4 * i + 0] = real (help);
	      lbuffer [4 * i + 1] = imag (help);
	      help = upFilter_24000. Pass (DSPCOMPLEX (0, 0));
	      lbuffer [4 * i + 2] = real (help);
	      lbuffer [4 * i + 3] = imag (help);
	   }
	   toOutput (lbuffer, 2 * cnt);
	   return;
	}
}
//

static
std::vector <uint8_t> theDescriptor;

bool	aacProcessor_faad::reinit (std::vector<uint8_t> audioD, int streamId) {
uint8_t audioRate		= theState -> streams [streamId].
                                                           audioSamplingRate;
uint8_t SBR_flag                = theState -> streams [streamId].
                                                           SBR_flag;
uint8_t audioMode               = theState -> streams [streamId].
                                                           audioMode;
	if (theDecoder == nullptr)
	   theDecoder = NeAACDecOpen ();

	if (theDescriptor. size () != audioD. size ()) {
	   theDescriptor = audioD;
	   return initDecoder (audioRate, SBR_flag, audioMode);
	}

	for (uint16_t i = 0; i < theDescriptor. size (); i ++) {
	   if (theDescriptor. at (i) != audioD. at (i)) {
	      theDescriptor = audioD;
	      return initDecoder (audioRate, SBR_flag, audioMode);
	   }
	}
}

bool	aacProcessor_faad::initDecoder (int16_t	audioSampleRate,
	                                 bool	SBR_used,
	                                 uint8_t	audioMode) {
int16_t aacRate = 12000;
uint8_t	aacMode	= DRMCH_SBR_PS_STEREO;
QString	text;

	if (theDecoder == nullptr)
	   theDecoder = NeAACDecOpen ();

// Only 12 kHz and 24 kHz with DRM
	aacRate = audioSampleRate == 01 ? 12000 : 24000;
	text = QString::number (aacRate);
	text. append (" ");

// Number of channels for AAC: Mono, PStereo, Stereo 
	switch (audioMode) {
	   case 0:		// audioMode == 0, MONO
	      if (SBR_used) 
	         aacMode = DRMCH_SBR_MONO;
	      else 
	         aacMode = DRMCH_MONO;
	      text. append ("mono");
	      break;

	   case 1:
/* Low-complexity stereo only defined in SBR mode */
	      aacMode = DRMCH_SBR_PS_STEREO;
	      text. append ("stereo");
	      break;
	
	   default:
	   case 2:
	      if (SBR_used) 
	         aacMode = DRMCH_SBR_STEREO;
	      else
	         aacMode = DRMCH_STEREO;
	      text. append ("stereo");
	      break;
	}

	aacData (text);
	int s = NeAACDecInitDRM (&theDecoder, aacRate, (uint8_t)aacMode);
	return s >= 0;
}
//
//	Later on, we'll change this such that the return
//	vector is for further processing
void	aacProcessor_faad::decodeFrame (uint8_t	*AudioFrame,
	                                int16_t	frameSize,
	                                bool	*conversionOK,
	                                int16_t	*buffer,
	                                int16_t	*samples,
	                                int32_t	*pcmRate) {
int16_t* outBuf = nullptr;
NeAACDecFrameInfo hInfo;
uint16_t	i;
	hInfo.channels	= 1;
	hInfo.error	= 1;
//	ensure we have a decoder:
	if (theDecoder == nullptr) 
	   return;
	outBuf = (int16_t*) NeAACDecDecode (theDecoder,
	                                    &hInfo,
	                                    &AudioFrame[0],
	                                    frameSize);

	*pcmRate	= hInfo. samplerate;
	*conversionOK	= !hInfo. error;

	if (hInfo. error != 0) {
	   fprintf (stderr, "Warning %s\n",
	                     faacDecGetErrorMessage (hInfo. error));
	   *samples	= 0;
	   return;
	}

	if (hInfo. channels == 2)
	   for (i = 0; i < hInfo. samples; i ++)
//	   for (i = 0; i < 2 * hInfo. samples; i ++)
	      buffer [i] = outBuf [i];
	else
	if (hInfo. channels == 1)
	   for (i = 0; i < hInfo. samples; i ++) {
	      buffer [2 * i]	= ((int16_t *)outBuf) [i];
	      buffer [2 * i + 1] = buffer [2 * i];
	   }
	*samples = hInfo. samples;
	return;
}

void	aacProcessor_faad::closeDecoder () {
	if (theDecoder != NULL) {
           NeAACDecClose (theDecoder);
           theDecoder = NULL;
        }

}

std::vector<uint8_t>
	aacProcessor_faad::getAudioInformation (stateDescriptor *theState,
	                                             int streamId) {
std::vector<uint8_t> temp;
//streamParameters *sp = &(theState -> theStreams [streamId]);
uint8_t	xxx	= 0;

	xxx	=  theState -> streams [streamId]. audioCoding << 6;
	xxx	|= theState -> streams [streamId]. SBR_flag << 5;
	xxx	|= theState -> streams [streamId]. audioMode << 3;
	xxx	|= theState -> streams [streamId]. audioSamplingRate;
	temp. push_back (xxx);
	xxx	= 0;
	xxx	=  theState -> streams [streamId]. textFlag << 7;
	xxx	|= theState -> streams [streamId]. enhancementFlag << 6;
	xxx	|= theState -> streams [streamId]. coderField << 1;
	temp. push_back (xxx);
//	for (int i = 0; i < sp -> xHE_AAC. size (); i ++)
//	   temp. push_back (sp -> xHE_AAC. at (i));
	return temp;
}

