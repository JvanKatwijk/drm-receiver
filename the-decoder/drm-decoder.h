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

#include	<QSettings>
#include	<QObject>
#include	<QFrame>
#include	<thread>
#include	<mutex>
#include	<atomic>
#include	<stdint.h>
#include	"radio.h"

//      for the payload we have
#include	"ringbuffer.h"
#include	"fir-filters.h"
#include	"utilities.h"
#include	"basics.h"
#include	"reader.h"
#include	"backend-controller.h"
#include	"state-descriptor.h"
#include	"my-array.h"
//	for the fdk-aac functionw we have
#ifdef	__WITH_FDK_AAC__
#ifdef	__MINGW32__
#include	"aac-handler.h"
#endif
#endif
class	EQDisplay;
class	IQDisplay;
class	QSettings;

#include	"ui_drmdecoder.h"

#define         WORKING_RATE    12000

class drmDecoder: public QObject, private Ui_drmdecoder {
Q_OBJECT
public:
	
		drmDecoder (RadioInterface *,
	                    QSettings *,
	                    RingBuffer<std::complex<float>> *);
	virtual ~drmDecoder ();
        void    processBuffer	(std::complex<float> *, int length);
        void    process		(std::complex<float>);

private:
	QFrame			myFrame;
	QSettings		*drmSettings;
	RingBuffer<std::complex<float>> *audioOut;
	std::thread		* m_worker;
	RingBuffer<std::complex<float>>	inputBuffer;
	RingBuffer<std::complex<float>> iqBuffer;
        RingBuffer<std::complex<float>> eqBuffer;
	Reader			my_Reader;   // single instance during life
#ifdef	__WITH_FDK_AAC__
#ifdef	__MINGW32__
	aacHandler		aacFunctions;
#endif
#endif
	backendController	my_backendController;
	stateDescriptor		theState;

	EQDisplay		*my_eqDisplay;
	IQDisplay		*my_iqDisplay;
	RadioInterface		*theRadio;
	std::mutex		m_lock;
	std::mutex	        locker;
	void			WorkerFunction	();
	void			processSample	(std::complex<float>);
	int			resample	(std::complex<float>,
                                                 std::complex<float> *);
	uint8_t			getSpectrum	(stateDescriptor *);
	void			getMode		(Reader *my_Reader,
	                                                 smodeInfo *m);

//
//
        std::atomic<bool>       running;

	bool		drmError;
	int		centerFrequency;
        int             VFOFRequency;
        int             selectedFrequency;
        int             Raw_Rate;
//
//
	int		scopeMode;
        int16_t         nSymbols;
        int32_t         sampleRate;
        int8_t          windowDepth;
        smodeInfo       modeInf;
	void		frequencySync		(Reader	*my_Reader,
	                                         smodeInfo *m);
	int16_t		sdcCells		(smodeInfo *);
	bool		isFACcell		(smodeInfo *, int16_t, int16_t);
	bool		isSDCcell		(smodeInfo *, int16_t, int16_t);
	bool		isDatacell		(smodeInfo *,
                                                 int16_t,
                                                 int16_t,
	                                         int16_t);

	void		set_sdcCells		(smodeInfo *);
	std::vector<sdcCell> sdcTable;
	bool		isFirstFrame		(stateDescriptor *);
	bool		isLastFrame		(stateDescriptor *);
	void		addtoSuperFrame		(smodeInfo *,
                                                 int16_t,
	                                         myArray<theSignal> *);
private slots:
	void		select_channel_1	();
	void		select_channel_2	();
	void            handle_strengthSelector (int);
        void            handle_f_cutSelector    (int);
        void            handle_modeSelector     (const QString &);
	void		handle_reset		();

public slots:
	void            set_faadSyncLabel	(bool);
	void		set_messageLabel	(const QString &);
	void		set_audioModeLabel	(const QString &);
	void		set_aacDataLabel	(const QString &);
	void		set_timeLabel		(const QString &);

	void		show_fineOffset		(float);
	void		show_coarseOffset	(float);
	void		executeTimeSync		(bool);
	void		executeFACSync		(bool);
	void		executeSDCSync		(bool);
	void		execute_showMode	(int);
	void		execute_showSpectrum	(int);

	void		show_fac_mer		(float);
	void            show_sdc_mer		(float);
	void		show_msc_mer		(float);

	void		set_datacoding		(const QString &);

	void		set_channel_1		(const QString &);
	void		set_channel_2		(const QString &);
	void		set_channel_3		(const QString &);
	void		set_channel_4		(const QString &);
	void		audioAvailable		();
	void		show_eqsymbol		(int);
	void		showIQ			(int);
signals:
	void		audioAvailable		(int, int);
	void		setTimeSync		(bool);
        void		setFACSync		(bool);
        void		setSDCSync		(bool);
        void		show_Mode		(int);
        void		show_Spectrum		(int);
};
