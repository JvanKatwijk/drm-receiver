

-----------------------------------------------------------------------------
Redesign and re-implementation of the drm decoder
-----------------------------------------------------------------------------

DRM (Digital Radio Mondiale) is - as the name suggests - a form of digital
radio. 
In Europe drm is not very popular, most transmissions that started
in the first decade of the century already stopped. However,
Rumenia still has a regular DRM transmission.

![overview](/drm-receiver.png?raw=true)

The drm receiver is an experimental tool, a heavily reduced swradio,
with a single decoder, just for drm.
Here, it shows a test with two occurrences running on the same
frequency, just to test some loop antennas (ine running an RSP I,
the other running an RSP II).

-----------------------------------------------------------------------
New: rtlsdr and low frequencies
-----------------------------------------------------------------------

As is well known, using "dabsticks" for shortwave listening is asking
from trouble.
It is known that the current libraries for rtlsdr devices support
direct sampling for low frequencies.
The new rtlsdr interface shows a checkbox for selecting
direct sampling (or not).
Of course there is a price,
rather than IQ samples, samples with solely the I component
of samples are emitted. Since drm decoding needs the full IQ samples,
the rtlsdr interface is equipped with a filter transforming the
single I stream to an IQ stream. 
The IQ stream represents the so-called analytical frequencies, i.e.
the positive frequencies.

Two comments:
Currently I do not own a decent rtlsdr stick, the one I had was
mechanically very weak, and broke into two pieces, so the software
is NOT tested
Secondly, the direct sampling software requires a minor modification
to the older dabsticks, see a site as "rtlsdr.com"  for details

-----------------------------------------------------------------------
A short note on DRM
-------------------------------------------------------------------------

DRM, Digital Radio Mondiale, is a form of digital radio. Transmissions
are relatively scarce here in Western Europe, but is seems that elsewhere
there is an increasing interest.

For DRM signals there are different Modes and Spectra, here is Western
Europe transmissions can be received in Mode B, with a spectrum of 10 KHz.
Other spectra are 4.5, 5, 9, 18 and 20 KHz.

Different Modes are defined such that they perform optimal with given fading patterns.

Processing DRM requires sampling an analog input signal, usually with
12000 (complex) samples per second. Each complex sample then tells
about the amplitude and the phase of the signal at the sampling time.

Modes are defined in terms of symbols or words,
The most common mode, mode B has words
of 320 samples. For this mode, the first 64 samples of each word, a prefix, are a copy of the
last 64 samples. With other modes, the prefix length may differ.
Such a prefix, a copy of the last samples of the word, is very helpful in
determining both the first sample of a word as well
computing the frequency offset of the signal.

The 256 "useful" samples of the word, i.e. the samples following the prefix,
are fed into an FFT processor, mapping the samples as they were in the time domain,
to values (carriers) in the frequency domain.

These "words" with frequency domain values (carriers) are then the basis for
further processing. As always, not all carriers carry useful information,
for Mode B, 206 of the 256 carriers are the useful ones, the remaining ones, 25
on each end of the spectrum, are (supposed to be) zeros.
Given the bandwidth of 10 KHz, the frequency distance between successive carriers
is then about 46 Hz.

A group of these "words" forms a frame. For Mode B,
a frame is built up from 15 such words.
Such as frame then has 3840 cells with complex
values, with 3090 cells with useful values.
Note that in the time domain  such a frame takes 4800 samples, and with
a samplerate of 12000 samples a second, slightly less than 3 frames a second
are input.

In the frequency domain frame, a number of predefined locations
has predefined values. So, matching these predefined values
with the values that are actually found are used to reconstruct the signal
as it was at the transmitter side. The QAM4, QAM16 and QAM64 values require
coherent decoding, i.e. the value should be as precise as possible the 
value that was transmitted.

Such a frame contains three types of values

 * FAC, Fast Access Channel, values, containing QAM4 encoded bits with general
information, such as the mode and the spectrum, the number of services,
the way the other types are encoded etc.

 * SDC, Service Description Channel, containing detailed information on the
actual payload, the audio and data services that are
carried in the transmission. The bits for the SDC content are usually 
encoded in QAM16 (in DRM+ they are encoded QAM4)

 * MSC, Master Service Channel, containing the audio and data services. The bits
are often encoded in QAM64, sometimes in QAM16.

Actually, a group of three subsequent frames forms a so-called super frame,
the FAC contains information on whether the frame is the first or the
last of such a super frame. A superframe contains data, good for 1.2 seconds
of audio.

------------------------------------------------------------------------
Decoder issues
-----------------------------------------------------------------------

A decoder gets as input a sample stream, in our case the sample stream
has a rate of 12000 samples a second.
Of course, the first question that needs to be solved is what mode
and spectrum occupancy the transmission has and where do the "words" start.

The *Mode* can be determined by correlation, the length of the prefix,
mentioned before, is different for each mode. Mode detection is then simply by
correlating the prefix with the supposed end of word and finding the best
match.
In order to get a more reliable result, the process is done over a range
of words, say 15 to 20. 

Of course, by detecting the mode, the first sample of a word is
detected simultaneously.
A minor issue, though not unimportant is that there might be a clock offset
in the decoder compared to the clock in the transmitter.
This might imply that after the transmitter has sent a number samples, the
receiver may be one or two samples off.
So, a continuous monitoring that the time synchonization is still OK is a
must.

The width of the spectrum can be deduced from the words in the
fft output.
Looking at the strength of the carriers in the FFT output,
and noticing a strength difference is the way to go.
(Note that while the FFT was done over 256 elements (using mode B),
the outermost elements have (should have) an almost zero amplitude,
for mode B only the central 206 out of the 256 carriers contain
useful information.)

Happily, some values at predefined locations in the spectrum
have larger amplitudes and given phases.
By comparing the spectrum as computed
with the aspectrum as it should be, the
decoder can detect the frequency offset,
Furthermore, by comparing the phases of the values in the prefix
and the corresponding values at theend of the time domain words.
a more precise error can be computed and corrected.

Of course, if the frequency offset is larger than 48 Hz, the
carriers in the spectrum contain the wrong values and decoding is impossible.
Conversely, if decoding is possible, then obviously, the frequency correction
is more or less OK.

So, the decoder has to deal with

 * time synchronization to continuously detect the first sample of a word
in the time domain;

 * frequency synchronization, first of all to ensure that the frequency
offset is less than (half of) the carrier distance (the "coarse" offset),
secondly that small offsets within the carrier are also noted and dealt with.

 * the decoding of the FAC, since the FAC is encoded QAM4 it is easy is the frequency correction is OK. Note that if time and or frequency correction are
not done well, decoding of the FAC is impossible.

 * the decoding of the SDC, slightly more complex that decoding the FAC.
In general, if FAC decoding is impossible, so is SDC decoding.

 *  extracting the bits for the selected service, and decoding  these bits,
usually AAC encoded audio.

The decoder contains labels to show the correctness of the time, the FAC,
the SDC and the AAC decoding.

![overview](/drm-decoder.png?raw=true)

The picture of the decoder gives quite some information about the received
and decoded signal.

 * at the top left, the measured frequency offset, two numbers, one for the so-called coarse offset, i.e. the number of carriers off (here -281 Hz), the second number telling the fine offset, i.e. the offset in the carrier (here -18 Hz).
Since the FAC decosing is possible, the coarse frequency offset is compnsated
for, and the fine offset is sufficiently well handled.

 * the number 10:59 gives the time (UTC) derived from the transmission

 * 24000 stereo is the indication of the rate of the audio output

 * it can be seen that there is one service being transmitted (the software is able to handle 2 audio transmissions), here Radio kuwait

 * the numbers 1015 -2 give an indication of the time offset, here it states that after 1015 "words" (each 320 samples), a correction was needed with at most
 two samples, i.e. the next word should start 2 samples early

 * the number 0.974 tells that from the last 1000 audio frames, 974
cound be correctly translated into sound

 * the labels time sync etc speak for themselves

Below there is a row with 3, B, QAM64, AAC, which tells that the spectrum
is of type 3 (which just means a 10 KHz spectrum), the mode is Mode B, the
bits carrying the audio are encoded as QAM 64, and the audio itself is encoded as AAC.
The empty box then is available for text messages.

Below this, there are 3 numbers, giving some information on the quality of
the signal

 * the number 32.2 tells something about the quality of the signal elements
in which the FAC is encoded. The spectrum cells assigned to FAC contain values
encoded as QAM4, a value of 32 tells that the signal is reasonable.

 * the number 12.72 tells something about the quality of the signal elements
in which the SDC is encoded. The spectrum cells assigned to SDC contain values
encoded as QAM16, a values 12 tells that the signal is reasonable.

 * the number 4.375 tells that the quality of the signal in which the audio data is encoded is not very good (MSC data is encoded as QAM64);

The scopes at the bottom give some visual information about the restored signal.
The scope shows the correction that is to be applied to the incoming data
in the frequency domain: the red line tells the correction to the amplitude,
the blue line the correction to8 the phase. It shows that the signal here
is not all too best.

The scope right gives the constellation of the restored signal. Ideally
for QAM64, there are 64 dots evenly distributed over the screen.


--------------------------------------------------------------------------
A note on building an executable and equalization
--------------------------------------------------------------------------

For building an executable the - more or less - standard libraries
are supposed to be installed.
Qt, Qwt, fftw3, libsamplerate and libsndfile and of course, the
support libraries for the devices one chooses.

There is one thing that might be slightly more complex, it has to do
with preparation for the equalisation.
Input to the equalizer is an estimate of the channel,  in the implementation
there are some choices, represented by different versions of 
the "estimator"s in the equalizer directory.

The estimator computes the difference between the pilot values as
seen in the data and the pilot values as they should be.

	
	F_p is initialized with the precomputed values and is
	matrix filled with the (pilot, tap) combinations, where for the
	pilots, their carrier values (relative to 0) are relevant
	Basically the FFT transformer, with positions related to
	the positions of the taps

	The approach for channel estimation is as follows:
	we call the channel in the time domain h_td,
	in the frequency domain that will then be h_fd = F x h_td
 	we observe the values of the pilots at the recoever
 	side, X_p, It is known that they were transmittes as s_p
 	We know then that
	X_p = diag (s_p) * f, where f = F_p * h_td
	so, X_p = S_p * F_p * h_td
	with S_p is diag (s_p), and F_p is the FFT transform of the channel
	S_p en F_p can be computed, since they are known, regardless
	A_p = S_p * F_p
	the equation then is X_p = A_p * h_td
	we solve that as h_td = A_p ^ -1 x X_p
	recall that h_td is the chaannel in the time domain, so
	h_fd = F_p * h_td is the solution

estimator_1 is the simplest one, it works though.

estimator_2 has three different versions, the simplest one
is estimator_jan_2.cpp, is limited in the computations, not all "taps" are
taken into account.

estimator_arma_2 uses the armadillo library for the matrix computations,
estimator_eigen_2 uses the "eigen" template library .

----------------------------------------------------------------------------
----------------------------------------------------------------------------

A note on the implementation

The implementation is partially reimplemented from the implementation of the drm decoder
at the sw radio. The software was restructured though and limited in functionality: this software is restricted to handling audio streams.

----------------------------------------------------------------------------
----------------------------------------------------------------------------

For Windows an installer is available, for Linux an AppImage.
The AAC decoder for the Windows version is the fdk-aac library,
for the Linux one needs to install  the libfdk-aac library is used.

Of course, one may create an executable. Note that to keep things simple
the only path is using qmake/make.

By setting some parameters in the ".pro" file, one may choose between
aac decoding using the "faad_drm" library or the "fdk-aac" library.

The latter has the (potential) advantage of being able to decode xHE-AAC
as well

-----------------------------------------------------------------------------
Note on using the linfdk-aac library
----------------------------------------------------------------------------

The software is tested with the libfdk-aac library that was created from
https://github.com/mstorsjo/fdk-aac

Under Ubuntu the software did not work properly with the fdk-aac library
as provided by the repository

-------------------------------------------------------------------------------
Supported devices
-------------------------------------------------------------------------------

Since DRM is (mainly) transmitted on shortwave, there is support for
devices with which shortwave reception is possible

 * the SDRplay,  in the drm-receiver.pro file one may choose between using  the "old"
2.13 interface library, or the 3.0X library;

 * the Hackrf. 

 * the DABsticks, for RT820 based sticcks with a driver that allows selecting frequencies
as low as 13 MHz. Note that direct sampling gives real rather than complex samples and is not useable.

-----------------------------------------------------------------------------------------
Copyright
----------------------------------------------------------------------------------------

 Copyright

        Copyright (C)  2019, 2020
        Jan van Katwijk (J.vanKatwijk@gmail.com)
        Lazy Chair Computing

        The drm receiver software is made available under the GPL-2.0. and
        is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

