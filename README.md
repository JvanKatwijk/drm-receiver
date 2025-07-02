
Redesign and re-implementation of the drm decoder
-----------------------------------------------------------------------------

The new version supports, next to the SDRplay and HackRf the spyServer.
In my environment SW signals suffer from pollution by solar panels etc, and
while my antenna equipment is less than optimal, I added support for the
spyServer.
Now recall that most drm transmissions are in SW, varying from 6 to 15 Khz,
frequencies not supported by regular AIRsoy devices or by V3 DABsticks.
It turns out that there are a few spyServers connected to an AirspyHF,
which obviously does support the frequencies used for DRM.


-------------------------------------------------------------------------------
Note that the ".pro" file now is for use with Qt6
----------------------------------------------------------------------------:


**DRM (Digital Radio Mondiale)** is - as the name suggests - a form of digital
radio. 

In Europe, DRM is not very popular, most stations that have started a DRM 
transmission in the first decade of the century already have stopped. However, 
Romania still has a number of regular DRM transmissions that I can receive, 
the website http://www.hfcc.org/drm/ gives an overview.

![overview](/drm-receiver.png?raw=true)

The DRM receiver is an experimental tool, a heavily reduced swradio, with a 
single decoder, just for DRM. The decoder is a copy of the decoder in the 
SW receiver software.

The decoder uses the FDK_AAC library for AAC and xHE-AAC decoding.


The decoder
-----------------------------------------------------------------------

The decoder takes a samplerate of 12000 samples/second as input. 

The incoming sample stream is best seen as a sequence of **words**,
for mode B, such a word contains just over 300 samples.

Apart from the **time synchronization**, i.e. finding in the input stream
where the **words** start, all processing is done in the **frequency domain**.
The incoming **words** are processed by an FFT processor,
and each incoming word is eventually translated into a word
containing just over 200 carriers (The FFT is done on segments of 256 samples).
These translated words are grouped into **frames** (for mode B a frame
contains 15 words) and each group of 3 frames is grouped into a **superframe**.

The **layout** of frames and superframes is defined in the standards,
i.e. it is defined which carriers are used for synchronization, which carriers
belong to the FAC (Fast Access Channel), which carriers belong to the
SDR (Service Description Channel) and which carriers contain the payload.

![overview](/drm-decoder-1.png?raw=true)

The decoder gives quite some information on the state of decoding. The 4 boxes 
top right tell about the synchronization reached, from top to bottom

 * time sync tells whether or not the software thinks the input blocks
in the input sample stream are identified;
 * FAC sync tells whether or not decoding the FAC is successful.
The FAC contains specified carriers, with data encoded as QAM4, and
it gives general information on the structure of the transmission;
 * SDC sync tells whether or not decoding the SDC is successful.
The SDC contains - as the name suggests - data telling where to find
the service data and how to interpret it.
SDC data is encoded as QAM4 or QAM16;
 * aacSync tells about the decoding of the selected service. While the previous
ones dealt with the whole content of the transmission, the selected service
may be just one of 4. The payload of the transmission is encoded in QAM16
or QAM64 format. The audio services are encoded as AAC or xHE-AAC
streams, that is why this software uses the FDK_AAC library, since that
is able to handle both formats.

Note that in previous versions, there was a bug, a buffer overflow with
one of the output samplerates of the xHE-AAC streams, causing the software
to crash.

The boxes, here labeled "3" and "B", tell that in the transmission the
mode "B" was detected and the spectrum layout "3" (the latter just indicating
a 10 kHz wide signal).

The box with "QAM16" tells that the audio content is encoded in QAM16, i.e
each complex carrier takes one of 16 positions (which can be decoded into 
4 bits), and the box with "AAC" tells that the audio was encoded in AAC format
(the other format is xHE-AAC).

The 3 numbers in the middle of the widget give an indication of the quality 
of the decoding.

Top left of the widget the frequency offset is indicated,
the total offset can be obtained by adding the two numbers.

The label tells that the picture is taken from a transmission
of RadioRomaniaIntl.

The "scope" with the yellow arrows gives an impression on the amount
with which the signal has to be corrected. In the frequency domain
signal, some carriers are known to have a predefined phase and amplitude,
i.e. pilots. The scope tells the correction to be applied on these
pilots, information that can be used to transform all carriers.

![overview](/drm-decoder-2.png?raw=true)

The second picture shows - from the same recording a few minutes later - the
so-called **channel**. On the way from transmitter to receiver the signal
is malformed, in the picture the impulse response of the channel, i.e.
the way the channel filters the transmission, is shown.

![overview](/drm-decoder-3.png?raw=true)

The third picture shows, again from the same transmission, the measured 
errors in the pilots after an attempt to correct.

The computation of the channel is based on an NLE approach, the actual
equalization is done using the corrected pilots.

Just for illustration, the equalization done by interpolation
of the corrected pilots can be selected by setting the "linear" checkbox.
Note however, that then a reset of the decoder is required.


A note on building an executable and equalization
--------------------------------------------------------------------------

For building an executable the - more or less - standard libraries
are supposed to be installed.
`Qt`, `Qwt`, `fftw3`, `libsamplerate` and `libsndfile` and of course, the
support libraries for the devices one chooses.

The estimator computes the difference between the pilot values as
seen in the data and the pilot values as they should be.
	
`F_p` is initialized with the precomputed values and is
matrix filled with the `(pilot, tap)` combinations, where for the
pilots, their carrier values (relative to 0) are relevant. 
Basically the FFT transformer, with positions related to
the positions of the taps

The approach for channel estimation is as follows:

We call the channel in the time domain `h_td`,
in the frequency domain that will then be `h_fd = F x h_td`
we observe the values of the pilots at the receiver
side, `X_p`. It is known that they were transmitted as `s_p`. 
We know then that `X_p = diag (s_p) * f`, where `f = F_p * h_td`, 
so `X_p = S_p * F_p * h_td` with `S_p` is `diag (s_p)`, 
and `F_p` is the FFT transform of the channel.

`S_p` and `F_p` can be computed, since they are known, regardless
`A_p = S_p * F_p` the equation then is `X_p = A_p * h_td`
we solve that as `h_td = A_p ^ -1 x X_p`
recall that `h_td` is the channel in the time domain, so
`h_fd = F_p * h_td` is the solution.

The estimator uses the own library (all rights reserved) to
estimate the channel.

The equalizer itself is indirectly derived from the one in diorama, which 
was a Matlab implementation, and was translated (translitterated) by M. Bos
in his package **RXADADRM**.


Installer
----------------------------------------------------------------------------

For Windows an installer is available, for Linux an AppImage.
The AAC decoder for the Windows version is the fdk-aac library,
for Linux one needs to install the libfdk-aac library.

Of course, one may create an executable. Note that to keep things simple
the only path is using `qmake`/`make`.

By setting some parameters in the `.pro` file, one may choose between
aac decoding using the `faad_drm` library or the `fdk-aac` library.

The latter has the (potential) advantage of being able to decode xHE-AAC
as well


Supported devices
-------------------------------------------------------------------------------

Since DRM is (mainly) transmitted on shortwave, there is support for
devices with which shortwave reception is possible

 * the SDRplay,  in the `drm-receiver.pro` file one may choose between using  the "old"
2.13 interface library, or the 3.0x library;

 * the Hackrf. 

 * connecting to a spyServer with a spyServer client

Copyright
-------------------------------------------------------------------------------

 Copyright

        Copyright (C)  2019, 2025
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing

        The drm receiver software is made available under the GPL-2.0. and
        is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

