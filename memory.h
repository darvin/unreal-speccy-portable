#ifndef	__MEMORY_H__
#define	__MEMORY_H__

#include "device.h"

#pragma once

//*****************************************************************************
//	eRom
//-----------------------------------------------------------------------------
class eRom : public eDevice
{
public:
	virtual void Init();
	virtual void Reset();
};

//*****************************************************************************
//	eRam
//-----------------------------------------------------------------------------
class eRam : public eDevice
{
public:
	virtual void Reset();
};

//*****************************************************************************
//	eMemory
//-----------------------------------------------------------------------------
class eMemory
{
public:
	eMemory();
	virtual ~eMemory();
	byte Read(word addr) const;
	void Write(word addr, byte v);
	byte* Get(dword offset = 0) { return memory + offset; }

	enum ePage { P_ROM, P_RAM0, P_RAM1, P_RAM2, P_AMOUNT };
	void SetBank(int idx, ePage p);

	enum { LINEAR_PAGES = 4, PAGE_SIZE = 0x4000, SIZE = P_AMOUNT * PAGE_SIZE };
protected:
	byte* bank_read[LINEAR_PAGES];
	byte* bank_write[LINEAR_PAGES];
	byte* memory;
};

extern eMemory memory;

#endif//__MEMORY_H__