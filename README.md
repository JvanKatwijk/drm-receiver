

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

-----------------------------------------------------------------------
A short nore on DRM
-------------------------------------------------------------------------

DRM, Digital Radio Mondiale, is a form of digital radio. Transmissions
are relatively scarce here in Western Europe, but is seems that elsewhere
there is an increasing interest.

For DRM signals there are different Modes and Spectra, here is Western
Europe transmissions can be received in Mode B, with a spectrum of 10 KHz.
Other spectra are 4.5, 5, 9, 18 and 20 KHz.

Different Modes have different measures against fading in the signal.

Processing DRM requires sampling an analog input signal, usually with
12000 (complex) samples per second. Each complex sample then tells
aboaut the amplitude and the phase of the signal.

The most common mode, mode B, is defined in terms of symbols or words,
each of 320 samples. The first 64 samples of each word are a copy of the
last 64 samples. This is very helpful in determining frequency
offsets of the signal.

The 256 "useful" samples of a word are fed into an FFT processor, mapping
the samples as they were in the time domain, to values in the frequency
domain.

These values are then the basis for further processing.
A group of these "words" form a frame. For Mode B, 15 such words
form a frame. Such as frame then has 3840 cells with complex
values.
Note that in the time domain  such a frame takes 4800 samples, and with
a samplerate of 12000 samples a second, slightly less than 3 frames a second
are input.

In such a frame, predefined locations
have predefined values. So, matching these predefined values
with the values that are actually found help in reconstructing the signal
as it was at the transmitter side.

Such a frame contains three types of values

 * FAC, Fast Access Channel, values, containing QAM4 encoded bits with general
information, such as the mode and the spectrum, the number of services,
the way the other types are encoded etc.

 * SDC, Service Description Channel, containing detailed information on the
actual payload, the audio and data services that are
carried in the transmission. The bits for the SDC content are usually 
encoded in QAM16 (in DRM+ they are encoded QAM4)

 *MSC, Master Service Channel, containing the audio and data services. The bits
are often encoded in QAM64, sometimes in QAM16.

=========================================================================
Decoder issues
=========================================================================

A decoder gets as input a sample stream.
Of course, the first question is what mode and spectrum occupancy there are.
The Mode can be determined by correlation, the length of the prefix,
mentioned before, is different for each mode.

Of course, by detecting the mode, the first sample of a word can be
detected.
A minor issue, though not unimportant is that there might be a clock offset
in the decoder, compared to the clock in the transmitter.
This implies that after the transmitter has sent N * 320 samples, the
receiver may be one or two samples off.
So, a continuous monitoring that the time synchonization is still OK is a
must.

The spectrum can be deduced from the words in the frequency domain, 
after all, looking at the strength of the carriers in the FFT output,
and noticing a strength difference is the way to go.

One of the issues with the spectrum is that a spectrum based on a
frequency error may be worthless. If the frequency error is too large,
the values in the spectrum are useless for further processing.

Happily, some values in the spectrum are predefined in a way that the
decoder can correct the frequency.
Furthermore, by comparing the phases of the values in the prefix
and the corresponding values at theend of the time domain words.
a more precise error can be computed and corrected.


![overview](/drm-decoder.png?raw=true)

The picture of the decoder gives quite some information about the received
and decoded signal.

 * at the top left, the measured frequency offset, two numbers, one for the so-called coarse offset, i.e. the number of carriers off (here -281 Hz), the second number telling the fine offset, i.e. the offset in the carrier (here -21.3 Hz)

 * the number 12:51 gives the time (UTC) derived from the transmission

 * 24000 stereo is the indiction of the rate of the audio output

 * it can be seen that there is one service being transmitted (the software is able to hanfle 2 audio transmissions), here Radio kuwait

 * the numbers 544 -2 give an indication of the time offset, here it states that after 544 "words" (each 320 samples), a correction was needed with at most
 two samples

 * the number 0.986 tells that from the 1000 audio frames, 986 were correct

 * the labels time sync etc speak for themselves

Below there is a row with 3, B, QAM64, AAC, which tells that the spectrum
is of type 3 (which just means 1 10 KHz spectrum), the mode is Mode B, the
bits carrying the audio are encoded as QAM 64, and the audio is encoded as AAC.
The empty box then is available for text messages.

Below this, there are 4 numbers, giving some information on the quality of
the signal

 * the number 68.59 tells that the quality of the signal in which the FAC is encoded is very good (FAC data is encoded as QAM4);

 * the number 17.52 tells that the quality of the signal in which the SDC is
encoded is good (SDC data is encoded as QAM16);

 * the number 8.215 tells that the quality of the signal in which the audio data is encoded is fairly good (MSC data is encoded as QAM64);

The scopes at the bottom give some visual information about the restored signal.
The scope left tells the correction that is to be applied to the data
in the frequency domain, the red line tells the correction on the amplitude,
the blue line the correction on the phase. It shows that the signal here
is fairly reasonable.

The scope right gives the constellation of the restored signal. Ideally
there are 64 dots evenly distributed over the screen.


----------------------------------------------------------------------------
----------------------------------------------------------------------------

A note on the implementation

The implementation is derived from the implementation of the drm decoder
at the sw radio. The software was restructured though and limited in functionality: this software is restricted to handling audio streams.

----------------------------------------------------------------------------
----------------------------------------------------------------------------

For windows an installer is available
For Linux, one has to create an executable. Note that to keep things simple
the only path is using qmake/make.

By setting some parameters in the ".pro" file, one may choose between
aac decoding using the "faad_drm" library or the "fdk-aac" library.

The latter has the (potential) advantage of being able to decode xHE-AAC
as well

