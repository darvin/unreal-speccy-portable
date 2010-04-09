/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DEVICE_SOUND_H__
#define __DEVICE_SOUND_H__

#include "../device.h"

#pragma once

union SNDSAMPLE
{
	dword sample; // left/right channels in low/high WORDs
	struct { word left, right; } ch; // or left/right separately
};

struct SNDOUT
{
	dword timestamp; // in 'system clock' ticks
	SNDSAMPLE newvalue;
};

typedef SNDSAMPLE* bufptr_t;

const dword SNDR_DEFAULT_SYSTICK_RATE = 3500000; // ZX-Spectrum Z80 clock
const dword SNDR_DEFAULT_SAMPLE_RATE = 44100;

//=============================================================================
//	eDeviceSound
//-----------------------------------------------------------------------------
class eDeviceSound : public eDevice
{
public:
	eDeviceSound();
	void SetTimings(dword clock_rate, dword sample_rate);

	virtual void FrameStart();
	virtual void FrameEnd(dword tacts);
	virtual void Update(dword tact, dword l, dword r);

	enum { BUFFER_LEN = 16384 };

	void* AudioData();
	dword AudioDataReady();
	void AudioDataUse(dword size);

protected:
	// 'render' is a function that converts array of DAC inputs into PCM-buffer
	void Render(SNDOUT *src, dword srclen, dword clk_ticks, bufptr_t dst);

	dword mix_l, mix_r;
	bufptr_t dstpos, dst_start;
	dword clock_rate, sample_rate; //Alone Coder

	SNDSAMPLE buffer[BUFFER_LEN];

private:
	dword tick, base_tick;
	dword s1_l, s1_r;
	dword s2_l, s2_r;
	dword firstsmp; //Alone Coder
	int oldleft,useleft,olduseleft,oldfrmleft; //Alone Coder
	int oldright,useright,olduseright,oldfrmright; //Alone Coder

	//   dword clock_rate, sample_rate;
	qword passed_clk_ticks, passed_snd_ticks;
	dword mult_const;

	void Flush(dword endtick);
};

#endif//__DEVICE_SOUND_H__
