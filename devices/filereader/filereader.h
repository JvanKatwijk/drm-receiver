#
/*
 *    Copyright (C) 2021
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM-receiver
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

#ifndef __FILEREADER__
#define	__FILEREADER__

#include	<QWidget>
#include	<QFrame>
#include	<QString>
#include	"device-handler.h"
#include	"ui_filereader-widget.h"
#include	"ringbuffer.h"

class	QLabel;
class	QSettings;
class	fileHulp;
class	RadioInterface;
/*
 */
class	fileReader: public deviceHandler, public Ui_filereader {
Q_OBJECT
public:
		fileReader		(RadioInterface *,
	                                 QSettings *,
	                                 RingBuffer<std::complex<float>> *);
		~fileReader		();
	int32_t	getRate			();

	bool	restartReader		();
	void	stopReader		();
	int16_t	bitDepth		();
	void	exit			();
	bool	isOK			();
protected:
	int32_t		setup_Device	();
	QFrame		myFrame;
	fileHulp	*myReader;
	QLabel		*indicator;
	QLabel		*fileDisplay;
	int32_t		lastFrequency;
	int32_t		theRate;
private slots:
	void		reset		(void);
	void		handle_progressBar (int);
	void		setProgress	(int, float, float);
};
#endif

