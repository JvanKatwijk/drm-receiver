#
/*
 *    Copyright (C)  2020 .. 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of drm receiver
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
#include	<unistd.h>
#include	<QSettings>
#include	<QDebug>
#include	<QDateTime>
#include	<QObject>
#include	<QDir>
#include	<QColor>
#include	<QMessageBox>
#include	<QFileDialog>
#include	"radio.h"
#include	"fft-scope.h"
#include	"up-converter.h"
#include        "audiosink.h"
#include        "popup-keypad.h"
#include        "program-list.h"
#include	"bandplan.h"
//
//      devices
#include        "device-handler.h"
#include        "filereader.h"
#ifdef	HAVE_SDRPLAY_V2
#include        "sdrplay-handler-v2.h"
#endif
#ifdef	HAVE_SDRPLAY_V3
#include        "sdrplay-handler-v3.h"
#endif
#ifdef	HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SPYSERVER
#include	"spyserver-client.h"
#endif
//
#include	"deviceselect.h"
#include	"drm-decoder.h"

#define	D_SDRPLAY_V2	"sdrplay_v2"
#define	D_SDRPLAY_V3	"sdrplay"
#define	D_RTL_TCP	"rtl_tcp"
#define	D_HACKRF	"hackrf"
#define	D_RTLSDR	"dabstick"
#define	D_SPYSERVER	"spyserver_client"
static 
const char *deviceTable [] = {
#ifdef	HAVE_SDRPLAY_V2
	D_SDRPLAY_V2,
#endif
#ifdef	HAVE_SDRPLAY_V3
	D_SDRPLAY_V3,
#endif
#ifdef	HAVE_HACKRF
	D_HACKRF,
#endif
#ifdef	HAVE_RTLSDR
	D_RTLSDR,
#endif
#ifdef	HAVE_RTL_TCP
	D_RTL_TCP,
#endif
#ifdef	HAVE_SPYSERVER
	D_SPYSERVER,
#endif
	nullptr
};

static int startKnop;
static	QTimer	*starter;

static inline
int twoPower (int a) {
int	res	= 1;
	while (--a >= 0)
	   res <<= 1;
	return res;
}

QString	FrequencytoString (int32_t freq) {
	if (freq < 10)
	   return QString ('0' + (uint8_t)(freq % 10));
	return 
	   FrequencytoString (freq / 10). append (QString ('0' + (uint8_t)(freq % 10)));
}

	RadioInterface::RadioInterface (QSettings	*sI,
	                                QString		stationList,
	                                bandPlan	*my_bandPlan,
	                                QWidget		*parent):
	                                    QMainWindow (parent),
	                                    inputData	(1024 * 1024),
	                                    hfShifter	(96000),
	                                    hfFilter	(2048, 377),
	                                    audioData	(48000),
	                                    theDecimator (96000, 12000) {

	this	-> settings	= sI;
	this	-> my_bandPlan	= my_bandPlan;
	this	-> inputRate	= 96000;
	setupUi (this);
//      and some buffers
	audioRate		= 48000;

//	and the decoders
	displaySize		= 1024;
	scopeWidth		= inputRate;
	centerFrequency		= KHz (14070);
	selectedFrequency	= KHz (14070);

	theBand. currentOffset	= 0;
	theBand. lowF		= -5000;
	theBand. highF		= 5000;
	mouseIncrement		= 5;
//	scope and settings
	hfScopeSlider	-> setValue (50);
        hfScope		= new fftScope (hfSpectrum,
                                        displaySize,
                                        kHz (1),        // scale
                                        inputRate,
                                        hfScopeSlider -> value (),
                                        8);
	hfScope		-> set_bitDepth (12);	// default
	centerFrequency		= KHz (14070);	// default
	selectedFrequency	= KHz (14070);	// default
	hfScope	-> setScope (centerFrequency, selectedFrequency - centerFrequency);
        connect (hfScope,
                 SIGNAL (clickedwithLeft (int)),
                 this,
                 SLOT (adjustFrequency_khz (int)));
	connect (hfScope,
	         SIGNAL (clickedwithRight (int)),
	         this, SLOT (switch_hfViewMode (int)));
        connect (hfScopeSlider, SIGNAL (valueChanged (int)),
                 this, SLOT (set_hfscopeLevel (int)));

//	output device
        audioHandler            = new audioSink (this -> audioRate, 16384);
        outTable                = new int16_t
                                     [audioHandler -> numberofDevices () + 1];
        for (int i = 0; i < audioHandler -> numberofDevices (); i ++)
           outTable [i] = -1;

        try {
           setupSoundOut (streamOutSelector, audioHandler,
                          audioRate, outTable);
        } catch (int e) {
           QMessageBox::warning (this, tr ("sdr"),
                                       tr ("Opening audio failed\n"));
           abort ();
	}
	audioHandler -> selectDefaultDevice ();

        mykeyPad                = new keyPad (this);
        connect (freqButton, SIGNAL (clicked (void)),
                 this, SLOT (handle_freqButton (void)));

	connect (mouse_Inc, SIGNAL (valueChanged (int)),
	         this, SLOT (set_mouseIncrement (int)));

        connect (streamOutSelector, SIGNAL (activated (int)),
                 this, SLOT (setStreamOutSelector (int)));

	connect (freqSave, SIGNAL (clicked (void)),
                 this, SLOT (set_freqSave (void)));

        myList  = new programList (this, stationList);
        myList  -> show ();
	myLine	= NULL;
        audioHandler    -> selectDefaultDevice ();

	theDevice		= nullptr;
	delayCount		= 0;
	dumpfilePointer		= nullptr;

	connect (dumpButton, SIGNAL (clicked (void)),
	         this, SLOT (set_dumpButton (void)));
	secondsTimer. setInterval (1000);
	connect (&secondsTimer, SIGNAL (timeout (void)),
                 this, SLOT (updateTime (void)));
        secondsTimer. start (1000);

//	and off we go
	theDecoder	= new drmDecoder (this, settings,  &audioData);
//	bind the signals in the decoder to some slots
	connect (theDecoder, SIGNAL (audioAvailable (int, int)),
	         this, SLOT (processAudio (int, int)));
//
//	and now for the device
	QString device =
	            settings -> value ("device", "no device").toString ();
	int k = -1;
	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   if (deviceTable [i] == device) {
	      k = i;
	      break;
	}
	if (k != -1) {
	   starter	= new QTimer;
	   startKnop	= k;
	   starter -> setSingleShot (true);
	   starter -> setInterval (500);
	   connect (starter, SIGNAL (timeout ()), 
	            this, SLOT (quickStart ()));
	   starter -> start (500);
	}
	else {
	   startKnop	= 0;
	   theDevice 	= setDevice (settings, &inputData);
	   if (theDevice != nullptr) {
	      hfScope	 -> set_bitDepth (theDevice -> bitDepth ());
	      connect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (sampleHandler (int)));
	      fprintf (stderr, "Device hier?\n");
	      theDevice	-> restartReader ();
	      fprintf (stderr, "Hier??\n");
	   }
	   else
	      throw (24);
	}
	fprintf (stderr, "end van radio start\n");
}

void    RadioInterface::quickStart () {
        disconnect (starter, SIGNAL (timeout ()),
                    this, SLOT (quickStart ()));
        fprintf (stderr, "going for quickStart\n");
        delete starter;
        if (getDevice (deviceTable [startKnop], settings, &inputData) == nullptr)
           if (setDevice (settings, &inputData) == nullptr) {
	      hfScope	 -> set_bitDepth (theDevice -> bitDepth ());
	      connect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (sampleHandler (int)));
	      theDevice	-> restartReader ();
	   }
	   else
	      throw (31);
}

//      The end of all
        RadioInterface::~RadioInterface () {
}
//
//	If the user quits before selecting a device ....
void	RadioInterface::handle_quitButton	(void) {
	if (theDevice != NULL) {
	   theDevice	-> stopReader ();
	   disconnect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (sampleHandler (int)));
	   delete  theDevice;
	}
	fprintf (stderr, "device is gone, go\n");
	fprintf (stderr, "device is deleted\n");
	myList          -> saveTable ();
	myList		-> hide ();

	fprintf (stderr, "we kappen ermee\n");
	delete		theDecoder;
	secondsTimer. stop ();
        delete		myList;
	delete []	outTable;
	delete		hfScope;
        delete		mykeyPad;
}

deviceHandler	*RadioInterface::
	            setDevice (QSettings *s,
	                       RingBuffer<std::complex<float>> *hfBuffer) {
deviceSelect	deviceSelect;
deviceHandler	*theDevice	= nullptr;
QStringList devices;

	for (int i = 0; deviceTable [i] != nullptr; i ++)
	   devices += deviceTable [i];
	devices	+= "filereader";
	devices	+= "quit";
	deviceSelect. addList (devices);
	int theIndex = -1;
	while (theDevice == nullptr) {
	   theIndex = deviceSelect. QDialog::exec ();
	   if (theIndex < 0)
	      continue;
	   QString s = devices. at (theIndex);
	   if (s == "quit")
	      return nullptr;
	   fprintf (stderr, "we go for %s\n", s. toLatin1 (). data ());
	   theDevice	= getDevice (s, settings, hfBuffer);
	   theDevice -> restartReader ();
	   fprintf (stderr, "Hebben we een device?\n");
	}
	return theDevice;
}

deviceHandler	*RadioInterface::
	                      getDevice (const QString &s,
	                                 QSettings *settings,
	                                 RingBuffer<std::complex<float>> *b) {
#ifdef HAVE_SDRPLAY_V2
	if (s == D_SDRPLAY_V2) {
	   try {
	      return  new sdrplayHandler_v2 (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_SDRPLAY_V3
	if (s == D_SDRPLAY_V3) {
	   try {
	      return new sdrplayHandler_v3 (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_HACKRF
	if (s == D_HACKRF) {
	   try {
	      return  new hackrfHandler (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_RTLSDR
	if (s == D_RTLSDR) {
	   fprintf (stderr, "going for the stick\n");
	   try {
	      return new rtlsdrHandler (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
#endif
#ifdef HAVE_SPYSERVER
	if (s == D_SPYSERVER) {
	   fprintf (stderr, "going for the spyServer client\n");
	   try {
	      deviceHandler *tt =  new spyServer_client (this, settings, b);
	      tt -> restartReader ();
	      return tt;
	   } catch (int e) {
	   }
	}
	else
#endif
	if (s == "filereader") {
	   try {
	      return new fileReader (this, settings, b);
	   } catch (int e) {
	   }
	}
	else
	QMessageBox::warning (this, tr ("sdr"),
	                               tr ("loading device failed"));
	return nullptr;
}
//
//	
void    RadioInterface::handle_freqButton (void) {
        if (mykeyPad -> isVisible ())
           mykeyPad -> hidePad ();
        else
           mykeyPad     -> showPad ();
}

//	setFrequency is called from the frequency panel
//	as well as from inside to change VFO and offset
void	RadioInterface::setFrequency (int32_t frequency) {
	centerFrequency		= frequency;
	selectedFrequency	= frequency;
	theDevice	-> setVFOFrequency (centerFrequency);
	hfScope		-> setScope  (centerFrequency,
	                              centerFrequency - selectedFrequency);
	hfFilter. setBand (theBand. currentOffset + theBand. lowF,
                           theBand. currentOffset + theBand. highF,
                                                  inputRate);
	fprintf (stderr, "Low %d high %d\n", theBand. lowF, theBand. highF);
	QString ff	= FrequencytoString (selectedFrequency);
	frequencyDisplay	-> display (ff);
	bandLabel		-> setText (my_bandPlan -> getFrequencyLabel (selectedFrequency));
}
//
//	adjustFrequency is called whenever clicking the mouse
void	RadioInterface::adjustFrequency_khz (int32_t n) {
	adjustFrequency_hz (1000 * n);
	fprintf (stderr, "adjusting %d Hz\n", n * 1000);
}

void	RadioInterface::adjustFrequency_hz (int32_t n) {
int	lowF	= theBand. lowF;
int	highF	= theBand. highF;
int	currOff	= theBand. currentOffset;

	if ((currOff + highF + n >= scopeWidth / 2 - scopeWidth / 20) ||
	    (currOff + lowF + n <= - scopeWidth / 2 + scopeWidth / 20) ) {
	   quint64 newFreq = theDevice -> getVFOFrequency () +
	                                   theBand. currentOffset + n;
	   setFrequency ((quint64)newFreq);
	   return;
	}
	else {
	   theBand. currentOffset += n;
	   hfScope -> setScope (theDevice -> getVFOFrequency (),
	                                   theBand. currentOffset);
	   hfFilter. setBand (theBand. currentOffset + theBand. lowF,
	                      theBand. currentOffset + theBand. highF,
	                            inputRate);
	}

	int freq		= theDevice -> getVFOFrequency () +
	                                    theBand. currentOffset;
	QString ff = FrequencytoString ((quint64)freq);
	frequencyDisplay	-> display (ff);
//	bandLabel	-> setAutoFillBackground (true);
//	bandLabel	-> setPalette (labelPalette);
	bandLabel	-> setText (my_bandPlan -> getFrequencyLabel (freq));
}


//      just a convenience button
void    RadioInterface::set_inMiddle () {
        quint64 newFreq = theDevice -> getVFOFrequency () +
                                            theBand. currentOffset;
        setFrequency (newFreq);
}

int32_t	RadioInterface::get_selectedFrequency	() {
	return selectedFrequency;
}

int32_t	RadioInterface::get_centerFrequency	() {
	return centerFrequency;
}
//
void    RadioInterface::set_freqSave    () {
        if (myLine == NULL)
           myLine  = new QLineEdit ();
        myLine  -> show ();
        connect (myLine, SIGNAL (returnPressed (void)),
                 this, SLOT (handle_myLine (void)));
}

void    RadioInterface::handle_myLine () {
QString programName     = myLine -> text ();
        myList  -> addRow (programName, QString::number (selectedFrequency / Khz (1)));
        disconnect (myLine, SIGNAL (returnPressed (void)),
                    this, SLOT (handle_myLine (void)));
        myLine  -> hide ();
}

void	RadioInterface::set_mouseIncrement (int inc) {
	mouseIncrement = inc;
}

void	RadioInterface::wheelEvent (QWheelEvent *e) {
	adjustFrequency_hz ((e -> delta () > 0) ?
	                        mouseIncrement : -mouseIncrement);
}

//////////////////////////////////////////////////////////////////
//
void	RadioInterface::sampleHandler (int amount) {
std::complex<float>   buffer [theDecimator. inSize ()]; 
std::complex<float> ifBuffer [theDecimator. outSize ()];
float dumpBuffer [2 * 512];
int	i, j;

	(void)amount;
	while (inputData. GetRingBufferReadAvailable () > 512) {
	   inputData. getDataFromBuffer (buffer, 512);
	   hfScope	-> addElements (buffer, 512);
	   if (dumpfilePointer != NULL) {
	      for (i = 0; i < 512; i ++) {
                 dumpBuffer [2 * i]	= real (buffer [i]);
                 dumpBuffer [2 * i + 1] = imag (buffer [i]);
              }
              sf_writef_float (dumpfilePointer, dumpBuffer, 512);
           }

	   for (i = 0; i < 512; i ++) {
              float agcGain;
              std::complex<float> temp = hfFilter. Pass (buffer [i]);
              temp = hfShifter. do_shift (temp, theBand. currentOffset);
              if (theDecimator. add (temp, ifBuffer)) {
                 for (j = 0; j < theDecimator. outSize (); j ++) {
                    theDecoder -> process (ifBuffer [j]);
                 }
              }
           }
        }
}
//
//
void    RadioInterface::set_hfscopeLevel (int level) {
        hfScope -> setLevel (level);
}

//	do not forget that ocnt starts with 1, due
//	to Qt list conventions
void	RadioInterface::setupSoundOut (QComboBox	*streamOutSelector,
	                               audioSink	*our_audioSink,
	                               int32_t		cardRate,
	                               int16_t		*table) {
uint16_t	ocnt	= 1;
uint16_t	i;

	for (i = 0; i < our_audioSink -> numberofDevices (); i ++) {
	   const char *so =
	             our_audioSink -> outputChannelwithRate (i, cardRate);

	   if (so != NULL) {
	      streamOutSelector -> insertItem (ocnt, so, QVariant (i));
	      table [ocnt] = i;
	      ocnt ++;
	   }
	}

	qDebug () << "added items to combobox";
	if (ocnt == 1)
	   throw (22);
}

void	RadioInterface::setStreamOutSelector (int idx) {
int16_t	outputDevice;

	if (idx == 0)
	   return;

	outputDevice = outTable [idx];
	if (!audioHandler -> isValidDevice (outputDevice)) 
	   return;

	audioHandler	-> stop	();
	if (!audioHandler -> selectDevice (outputDevice)) {
	   QMessageBox::warning (this, tr ("sdr"),
	                               tr ("Selecting  output stream failed\n"));
	   return;
	}

	qWarning () << "selected output device " << idx << outputDevice;
	audioHandler	-> restart ();
}

void    RadioInterface::processAudio  (int amount, int rate) {
DSPCOMPLEX buffer [rate / 10];

        (void)amount;
	usleep (1000);
        while (audioData. GetRingBufferReadAvailable () >
                                                 (uint32_t)rate / 10) {
           audioData. getDataFromBuffer (buffer, rate / 10);
	   audioHandler      -> putSamples (buffer, rate / 10);
        }
}

void	RadioInterface::switch_hfViewMode	(int d) {
	(void)d;
	hfScope	-> switch_viewMode ();
}

void	RadioInterface::updateTime		(void) {
QDateTime currentTime = QDateTime::currentDateTime ();

	timeDisplay     -> setText (currentTime.
                                    toString (QString ("dd.MM.yy:hh:mm:ss")));
}

void	RadioInterface::set_dumpButton (void) {
SF_INFO	*sf_info	= (SF_INFO *)alloca (sizeof (SF_INFO));

	if (dumpfilePointer != NULL) {
	   sf_close (dumpfilePointer);
	   dumpfilePointer	= NULL;
	   dumpButton	-> setText ("dump");
	   return;
	}

	QString file = QFileDialog::getSaveFileName (this,
	                                        tr ("Save file ..."),
	                                        QDir::homePath (),
	                                        tr ("PCM wave file (*.wav)"));
	if (file == QString (""))
	   return;
	file		= QDir::toNativeSeparators (file);
	if (!file.endsWith (".wav", Qt::CaseInsensitive))
	   file.append (".wav");
	sf_info		-> samplerate	= inputRate;
	sf_info		-> channels	= 2;
	sf_info		-> format	= SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	dumpfilePointer	= sf_open (file. toUtf8 (). data (),
	                                   SFM_WRITE, sf_info);
	if (dumpfilePointer == NULL) {
	   qDebug () << "Cannot open " << file. toUtf8 (). data ();
	   return;
	}

	dumpButton		-> setText ("WRITING");
}

#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

        QMessageBox::StandardButton resultButton =
                        QMessageBox::question (this, "dabRadio",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
        if (resultButton != QMessageBox::Yes) {
           event -> ignore();
        } else {
           handle_quitButton ();
           event -> accept ();
        }
}

