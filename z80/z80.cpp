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

#include "../std.h"
#include "../devices/memory.h"
#include "../devices/ula.h"
#include "../devices/device.h"
#include "../speccy.h"

#include "z80.h"

namespace xZ80
{

byte even_m1 = 0;
bool unstable_databus = false;

//=============================================================================
//	eZ80::eZ80
//-----------------------------------------------------------------------------
eZ80::eZ80(eMemory* _m, eDevices* _d, dword _frame_tacts)
	: memory(_m), rom(_d->Get<eRom>()), ula(_d->Get<eUla>()), devices(_d)
	, fast_emul(NULL)
	, t(0), im(0), eipos(0), haltpos(0)
	, frame_tacts(_frame_tacts)
	, last_branch(0)
{
	pc = sp = ir = memptr = ix = iy = 0;
	bc = de = hl = af = alt.bc = alt.de = alt.hl = alt.af = 0;
	int_flags = 0;

	InitOpNoPrefix();
	InitOpCB();
	InitOpDD();
	InitOpED();
	InitOpFD();
	InitOpDDCB();

	// offsets to b,c,d,e,h,l,<unused>,a  from cpu.c
	const REGP r_offset[] =
	{
		&eZ80::b, &eZ80::c, &eZ80::d, &eZ80::e,
		&eZ80::h, &eZ80::l, &eZ80::reg_unused, &eZ80::a
	};
	memcpy(reg_offset, r_offset, sizeof(r_offset));
}
//=============================================================================
//	eZ80::Reset
//-----------------------------------------------------------------------------
void eZ80::Reset()
{
	int_flags = 0;
	ir = 0;
	im = 0;
	pc = 0;
	last_branch = 0;
}
//=============================================================================
//	eZ80::Reset
//-----------------------------------------------------------------------------
void eZ80::Update(int int_len, int* nmi_pending)
{
	haltpos = 0;
	// INT check separated from main Z80 loop to improve emulation speed
	while(t < int_len)
	{
		if(iff1 && t != eipos) // int enabled in CPU not issued after EI
		{
			Int();
			break;
		}
		Step();
	}
	eipos = -1;
	while(t < frame_tacts)
	{
		Step();
		if(*nmi_pending)
		{
			--*nmi_pending;
			if(pc >= 0x4000)
			{
				Nmi();
				*nmi_pending = 0;
			}
		}
	}
	t -= frame_tacts;
	eipos -= frame_tacts;
}
//=============================================================================
//	eZ80::Fetch
//-----------------------------------------------------------------------------
byte eZ80::Fetch()
{
	r_low++;// = (cpu->r & 0x80) + ((cpu->r+1) & 0x7F);
	t += 4;
	return Read(pc++);
}
//=============================================================================
//	eZ80::IoWrite
//-----------------------------------------------------------------------------
void eZ80::IoWrite(word port, byte v)
{
	devices->IoWrite(port, v, t);
}
//=============================================================================
//	eZ80::IoRead
//-----------------------------------------------------------------------------
byte eZ80::IoRead(word port) const
{
	return devices->IoRead(port, t);
}
//=============================================================================
//	eZ80::Write
//-----------------------------------------------------------------------------
void eZ80::Write(word addr, byte v)
{
	ula->Write(addr, v, t);
	memory->Write(addr, v);
}
//=============================================================================
//	eZ80::Read
//-----------------------------------------------------------------------------
byte eZ80::Read(word addr) const
{
	return memory->Read(addr);
}
//=============================================================================
//	eZ80::Int
//-----------------------------------------------------------------------------
void eZ80::Int()
{
	byte vector = 0xff; //unstable_databus ? (byte)rdtsc() : 0xFF;
	word intad = 0x38;
	if(im >= 2) // im2
	{ 
		word vec = vector + i*0x100;
		intad = Read(vec) + 0x100*Read(vec+1);
	}
	t += (im < 2) ? 13 : 19;
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = intad;
	memptr = intad;
	halted = 0;
	iff1 = iff2 = 0;
}
//=============================================================================
//	eZ80::Nmi
//-----------------------------------------------------------------------------
void eZ80::Nmi()
{
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x66;
	iff1 = halted = 0;
}
//=============================================================================
//	eZ80::Step
//-----------------------------------------------------------------------------
void eZ80::Step()
{
	rom->Read(pc);
	if(fast_emul)
		fast_emul(this);
	if(pc_h & even_m1) //wait
	{
		t += t & 1;
	}
	byte opcode = Fetch();
	(this->*normal_opcodes[opcode])();
}
//=============================================================================
//	eZ80::inc8
//-----------------------------------------------------------------------------
void eZ80::inc8(byte& x)
{
	f = incf[x] | (f & CF);
	x++;
}
//=============================================================================
//	eZ80::dec8
//-----------------------------------------------------------------------------
void eZ80::dec8(byte& x)
{
	f = decf[x] | (f & CF);
	x--;
}
//=============================================================================
//	eZ80::add8
//-----------------------------------------------------------------------------
void eZ80::add8(byte src)
{
	f = adcf[a + src*0x100];
	a += src;
}
//=============================================================================
//	eZ80::adc8
//-----------------------------------------------------------------------------
void eZ80::adc8(byte src)
{
	byte carry = f & CF;
	f = adcf[a + src*0x100 + 0x10000*carry];
	a += src + carry;
}
//=============================================================================
//	eZ80::sub8
//-----------------------------------------------------------------------------
void eZ80::sub8(byte src)
{
	f = sbcf[a*0x100 + src];
	a -= src;
}
//=============================================================================
//	eZ80::sbc8
//-----------------------------------------------------------------------------
void eZ80::sbc8(byte src)
{
	byte carry = f & CF;
	f = sbcf[a*0x100 + src + 0x10000*carry];
	a -= src + carry;
}
//=============================================================================
//	eZ80::and8
//-----------------------------------------------------------------------------
void eZ80::and8(byte src)
{
	a &= src;
	f = log_f[a] | HF;
}
//=============================================================================
//	eZ80::or8
//-----------------------------------------------------------------------------
void eZ80::or8(byte src)
{
	a |= src;
	f = log_f[a];
}
//=============================================================================
//	eZ80::xor8
//-----------------------------------------------------------------------------
void eZ80::xor8(byte src)
{
	a ^= src;
	f = log_f[a];
}
//=============================================================================
//	eZ80::cp8
//-----------------------------------------------------------------------------
void eZ80::cp8(byte src)
{
	f = cpf[a*0x100 + src];
}
//=============================================================================
//	eZ80::bit
//-----------------------------------------------------------------------------
void eZ80::bit(byte src, byte bit)
{
	f = log_f[src & (1 << bit)] | HF | (f & CF) | (src & (F3|F5));
}
//=============================================================================
//	eZ80::bitmem
//-----------------------------------------------------------------------------
void eZ80::bitmem(byte src, byte bit)
{
	f = log_f[src & (1 << bit)] | HF | (f & CF);
	f = (f & ~(F3|F5)) | (mem_h & (F3|F5));
}
//=============================================================================
//	eZ80::res
//-----------------------------------------------------------------------------
void eZ80::res(byte& src, byte bit) const
{
	src &= ~(1 << bit);
}
//=============================================================================
//	eZ80::resbyte
//-----------------------------------------------------------------------------
byte eZ80::resbyte(byte src, byte bit) const
{
	return src & ~(1 << bit);
}
//=============================================================================
//	eZ80::set
//-----------------------------------------------------------------------------
void eZ80::set(byte& src, byte bit) const
{
	src |= (1 << bit);
}
//=============================================================================
//	eZ80::setbyte
//-----------------------------------------------------------------------------
byte eZ80::setbyte(byte src, byte bit) const
{
	return src | (1 << bit);
}

}//namespace xZ80
