#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-wspr decoder
 *
 *    qt-wspr is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    qt-wspr is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-wspr; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"bandplan.h"

	bandPlan::bandPlan (QString fileName) {
	labelTable. resize (0);
	loadPlan (fileName);
	
}

	bandPlan::~bandPlan () {
}

const
QString	bandPlan::getFrequencyLabel (uint32_t Frequency) {
int	freq	= Frequency / 1000;

	for (int i = 0; i < labelTable. size (); i ++)
	   if ((labelTable. at (i). low <= freq) &&
	       (freq <= labelTable. at (i). high))
	      return QString (labelTable. at (i). label);

	return QString ("");
}


bool	bandPlan::loadPlan (QString fileName) {
QDomDocument xmlBOM;
QFile f (fileName);

	this	-> fileName = fileName;
	if (!f. open (QIODevice::ReadOnly)) 
	   return false;

	xmlBOM. setContent (&f);
	f. close ();
	QDomElement root	= xmlBOM. documentElement ();
	QDomElement component	= root. firstChild (). toElement ();
	while (!component. isNull ()) {
	   if (component. tagName () == "Band") {
	      bandElement bd;
	      bd. low		= component. attribute ("Low", "0"). toInt ();
	      bd. high		= component. attribute ("High", "0"). toInt ();
	      bd. label		= component. attribute ("Label", "");
	      labelTable. push_back (bd);
	   }
	   component = component. nextSibling (). toElement ();
	}
	return true;
}
