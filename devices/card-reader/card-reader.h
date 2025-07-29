#
/*
 *    Copyright (C) 2016 .. 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm receiver
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

#include	<portaudio.h>
#include	"device-handler.h"
#include	"radio-constants.h"
#include	"ringbuffer.h"
#include	<QObject>
#include	<QFrame>
#include	"ui_cardreader-widget.h"

class	RadioInterface;
class	QSettings;

class  cardReader: public deviceHandler, 
	                         public Ui_cardreaderWidget {
Q_OBJECT
public:
	 		cardReader	(RadioInterface *,
	                                 QSettings	*,
	                                 RingBuffer<std::complex<float>> *);

			~cardReader	();
//	for setting up
	void		setVFOFrequency	(int32_t);
	int32_t		getVFOFrequency	();
	bool		restartReader	();
	void		stopReader	();
	void		resetBuffer	();
	int16_t		bitDepth	();
private:
	QFrame		myFrame;
	RadioInterface	*theRadio;
	QSettings	*cardReaderSettings;
	RingBuffer<std::complex<float>>	*_I_Buffer;
	bool		InputrateIsSupported	(int16_t, int32_t);
const	QString		inputChannelwithRate	(int16_t, int32_t);
	bool		selectDevice		(int);
	int		inputRate;
	int32_t		portAudio;
	int16_t		numofDevices;
	int		paCallbackReturn;
	int16_t		bufSize;
	PaStream	*istream;

	uint8_t		IQMode;
	int16_t		invalidDevice		();
	bool		isValidDevice		(int16_t);
	PaStreamParameters	inputParameters;
	bool		readerRunning;
	struct  channel {
	   int16_t	channelIndex;
	   QString	channelName;
	};
	std::vector<channel> inTable;
static	int	paCallback_i	(const void *input,
	                         void	    *output,
	                         unsigned long framesperBuffer,
	                         const PaStreamCallbackTimeInfo *timeInfo,
	                         PaStreamCallbackFlags statusFlags,
	                         void	    *userData);
public slots:
	void	setStreamSelector	(const QString &);
	void	setIandQSelector	(const QString &);
};

