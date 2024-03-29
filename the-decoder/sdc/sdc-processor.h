#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDRunoPlugin_drm
 *
 *    drm plugin is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm plugin is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm plugin; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
/*
 *	SDC processor for QAM4/QAM16 encoded SDC's
 */
#ifndef	__SDC_PROCESSOR
#define	__SDC_PROCESSOR

#include	<QObject>
#include	<stdint.h>
#include	<vector>
#include	"basics.h"
#include	"checkcrc.h"
#include	"my-array.h"
#include	"mapper.h"

class	stateDescriptor;
class	prbs;
class	SDC_streamer;
class	qam4_metrics;
class	qam16_metrics;
class	drmDecoder;

class	sdcProcessor: public QObject {
Q_OBJECT
public:
		sdcProcessor	(drmDecoder	*,
	                         smodeInfo	*,
	                         std::vector<sdcCell> &,
	                         stateDescriptor	*);
		~sdcProcessor	();
	bool	processSDC	        (myArray<theSignal>  *);
private:
	bool	processSDC_QAM4		(std::vector<theSignal> &v);
	bool	processSDC_QAM16	(std::vector<theSignal> &v);
	void	interpretSDC		(uint8_t *v, int16_t size,
                                          stateDescriptor *theState);
	void	set_SDCData		(stateDescriptor *theState,
                                         uint8_t *v, uint8_t afs,
                                         uint8_t dataType,
                                         uint8_t versionFlag,
                                         int8_t lengthofBody);
        std::vector<sdcCell> *sdcTable;
	drmDecoder	*m_form;
	bool		sdcCorrect;
	qam16_metrics	*my_qam16_metrics;
	qam4_metrics	*my_qam4_metrics;
	SDC_streamer	*stream_0;
	SDC_streamer	*stream_1;
	checkCRC	theCRC;
	Mapper		Y13Mapper;
	Mapper		Y21Mapper;
	uint8_t		Mode;
	uint8_t		Spectrum;
	stateDescriptor		*theState;
	int16_t		nrCells;
	int16_t		lengthofSDC;
	prbs		*thePRBS;
	uint8_t		qammode;
	uint8_t		rmFlag;
	uint8_t		SDCmode;
signals:
	void		show_sdc_mer	(float);
	void		set_timeLabel	(const QString &);
	void		set_channel_1	(const QString &);
	void		set_channel_2	(const QString &);
	void		set_channel_3	(const QString &);
	void		set_channel_4	(const QString &);
};
#endif
