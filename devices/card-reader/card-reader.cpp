#
/*
 *    Copyright (C) 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the drm decoder
 *
 *    drm-decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm-decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm-decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QDebug>
#include	<QSettings>
#include	"ringbuffer.h"
#include	"card-reader.h"
#include	"radio.h"
/*
 *	The class describes the reading of
 *	data from the soundcard, storing it into the
 *	ringbuffer and making samples available for the user
 */

#define IandQ   0100
#define QandI   0101

	cardReader::cardReader 	(RadioInterface *mr,
	                         QSettings	*s,
	                         RingBuffer<std::complex<float>> *rb):
	                                      myFrame (nullptr) {
	this	-> inputRate	= 96000;
	theRadio		= mr;
	cardReaderSettings	= s;
	_I_Buffer		= rb;
	setupUi (&myFrame);
	myFrame. show ();
	portAudio		= false;
	readerRunning		= false;
	if (Pa_Initialize () != paNoError) {
	   qDebug ("PaInit failed for reader\n");
	   throw (20);
	}
	portAudio		= true;

	cardReaderSettings	-> beginGroup ("CARDREADER");
	QString IQMode_s	= cardReaderSettings -> value ("IandQ_Mode",
	                                                       "IandQ"). toString ();
	cardReaderSettings	-> endGroup ();
	int k = IandQSelector	-> findText (IQMode_s);
	if (k != -1)
	   IandQSelector  -> setCurrentIndex (k);
	IQMode		= IQMode_s == "IandQ" ? IandQ : QandI;
	qDebug ("Hostapis: %d\n", Pa_GetHostApiCount ());
	for (int i = 0; i < Pa_GetHostApiCount (); i ++)
	   qDebug ("Api %d is %s\n", i, Pa_GetHostApiInfo (i) -> name);

	numofDevices	= Pa_GetDeviceCount ();
	istream		= nullptr;
	int16_t		icnt	= 0;
	channelList -> addItem ("select stream");
//	look for appropriate devices
	for (int i = 0; i < numofDevices; i ++) {
	   QString si = inputChannelwithRate (i, inputRate);
	   if (si != "") {
              channelList -> addItem (si);	
	      channel temp;
	      temp.  channelIndex = i;
	      temp.  channelName = si;
	      inTable. push_back (temp);
	      icnt ++;
	   }
	}

	if (icnt == 0)		// no devices with 96000 rate detected
	   throw (21);
	connect (channelList, &QComboBox::textActivated,
	         this, &cardReader::setStreamSelector);
	connect (IandQSelector, &QComboBox::textActivated,
	         this, &cardReader::setIandQSelector);
}

	cardReader::~cardReader	() {
	if (!portAudio)
	   return;

	if ((istream != NULL) &&!Pa_IsStreamStopped (istream)) {
	   paCallbackReturn = paAbort;
	   (void) Pa_AbortStream (istream);
	   while (!Pa_IsStreamStopped (istream))
	      Pa_Sleep (1);
	   readerRunning = false;
	}

	if (istream != NULL)
	   Pa_CloseStream (istream);

	Pa_Terminate ();
}
//
//	Card readers do not set input freqyuencies
void	cardReader::setVFOFrequency	(int32_t freq) {
	lastFrequency	= freq;
}

int32_t	cardReader::getVFOFrequency	() {
	return lastFrequency;
}

bool	cardReader::restartReader () {
PaError	err;
	if (readerRunning ||!Pa_IsStreamStopped (istream)) // already running
	   return true;
	paCallbackReturn	= paContinue;
	err = Pa_StartStream (istream);
	if (err == paNoError)
	   readerRunning	= true;
	else {
	   const char *s	= Pa_GetErrorText (err);
	   fprintf (stderr, " Hier een Error %s\n", s);
	}
	return readerRunning;
}

void	cardReader::stopReader	() {
	if (!readerRunning || Pa_IsStreamStopped (istream))
	   return;

	paCallbackReturn = paAbort;
	(void) Pa_StopStream (istream);
	while (!Pa_IsStreamStopped (istream))
	   Pa_Sleep (1);
	readerRunning		= false;
}

void	cardReader::resetBuffer	() {
	_I_Buffer	->  FlushRingBuffer ();
}

int16_t	cardReader::bitDepth	() {	// just a guess
	return 16;
}

const QString	cardReader::inputChannelwithRate (int16_t ch, int32_t rate) {
const	PaDeviceInfo *deviceInfo;
QString name = "";

	if ((ch < 0) || (ch >= numofDevices))
	   return name;

	deviceInfo = Pa_GetDeviceInfo (ch);
	if (deviceInfo == NULL)
	   return name;

	if (deviceInfo -> maxInputChannels <= 0)
	   return name;

	if (InputrateIsSupported (ch, rate))
	   name = QString (deviceInfo -> name);
	return name;
}

int16_t	cardReader::invalidDevice	() {
	return numofDevices + 128;
}

bool	cardReader::isValidDevice	(int16_t dev) {
	if (dev < 0)
	   return false;
	return (dev < numofDevices);
}

bool	cardReader::selectDevice	(int idev) {
PaError	err;

	if (!isValidDevice (idev))
	   return false;
//
//	If already running. dtop previois operation
	if ((istream != NULL) &&!Pa_IsStreamStopped (istream)) {
	   paCallbackReturn = paAbort;
	   (void) Pa_StopStream (istream);
	   while (!Pa_IsStreamStopped (istream))
	      Pa_Sleep (1);
	   readerRunning = false;
	}

	if (istream != NULL)
	   Pa_CloseStream (istream);

        inputParameters. device = idev;
	inputParameters. channelCount		= 2;	/* I and Q	*/
	inputParameters. sampleFormat		= paFloat32;
	inputParameters. suggestedLatency	= 
	                 Pa_GetDeviceInfo (idev) -> 
	                       defaultLowInputLatency;
	inputParameters. hostApiSpecificStreamInfo = nullptr;

	bufSize	= (int)((float)inputParameters. suggestedLatency * 
	                                        (float)inputRate);
	paCallbackReturn = paContinue;
	err = Pa_OpenStream (
	             &istream,
	             &inputParameters,
	             NULL,
	             inputRate,
	             bufSize,
	             0,
	             this	-> paCallback_i,
	             this
	      );
	if (err != paNoError) {
	   qDebug ("Open istream error\n");
	   return false;
	}
	restartReader ();
	readerRunning = true;
	return true;
}
//
//	helper
bool	cardReader::InputrateIsSupported (int16_t device, int32_t Rate) {
PaStreamParameters *inputParameters =
	           (PaStreamParameters *)alloca (sizeof (PaStreamParameters)); 

	inputParameters	-> device		= device;
	inputParameters -> channelCount		= 2;	/* I and Q	*/
	inputParameters -> sampleFormat		= paFloat32;
	inputParameters -> suggestedLatency	= 0;
	inputParameters -> hostApiSpecificStreamInfo = nullptr;

	bool res = Pa_IsFormatSupported (inputParameters, nullptr, Rate) ==
	                                          paFormatIsSupported;
	fprintf (stderr, "device %d accepts parameters %s\n", device,
	                                           res ? "" : "niet");
	return res;
}

int	cardReader::paCallback_i (
		const void*			inputBuffer,
                void*				outputBuffer,
		unsigned long			framesPerBuffer,
		const PaStreamCallbackTimeInfo	*timeInfo,
	        PaStreamCallbackFlags		statusFlags,
	        void				*userData) {
RingBuffer<std::complex<float>>	*inB;
float *inp	= (float *)inputBuffer;
cardReader	*ud	= reinterpret_cast <cardReader *>(userData);
uint8_t		IQMode  = ud -> IQMode;
	(void) outputBuffer;
	(void) timeInfo;
	(void) statusFlags;

	if (inputBuffer != NULL) {
	   if (ud -> paCallbackReturn == paContinue) {
	      inB =  ud -> _I_Buffer;
	      for (int i = 0; i <  (int)framesPerBuffer; i ++) {
	         std::complex<float> temp;
	         if (IQMode == IandQ)
	            temp = std::complex <float> (inp [2 * i], inp [2 * i + 1]);
	         else
	            temp = std::complex <float> (inp [2 * i + 1], inp [2 * i]);
	         (void) ((inB) -> putDataIntoBuffer (&temp, 1));
	         if ((inB) -> GetRingBufferReadAvailable () >
	                      (uint32_t)(ud -> inputRate / 10)) {
	            emit ud ->  dataAvailable (ud -> inputRate / 10);
	         }
	      }
	   }
	}
	return ud -> paCallbackReturn;
}
//
//	Called from the control widget
void	cardReader::setStreamSelector (const QString &channelName) {
int16_t	inputDevice = -1;

	for (auto &ch : inTable) {
	   if (ch. channelName == channelName) {
	      inputDevice = ch. channelIndex;
	      break;
	   }
	}
	if (!isValidDevice (inputDevice)) {
	   return;
	}

	stopReader	();	// may be superfluous
	if (!selectDevice (inputDevice)) {
	   fprintf (stderr, "selecting %s (%d) failed\n",
	                                  channelName. toLatin1 (). data (),
	                                  inputDevice);
	   return;
	}
	qWarning () << "selected input device " << channelName << inputDevice;
}

void	cardReader::setIandQSelector (const QString &s) {
	IQMode = s == "IandQ" ? IandQ : QandI;
	cardReaderSettings	-> beginGroup ("CARDREADER");
	cardReaderSettings	-> setValue ("IandQ_Mode", s);
	cardReaderSettings	-> endGroup ();
}

