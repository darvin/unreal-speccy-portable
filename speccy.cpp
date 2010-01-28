#include "std.h"

#include "speccy.h"
#include "devices/device.h"
#include "z80/z80.h"
#include "devices/memory.h"
#include "devices/ula.h"
#include "devices/keyboard.h"
#include "devices/sound/beeper.h"
#include "devices/sound/ay.h"

//=============================================================================
//	eSpeccy::eSpeccy
//-----------------------------------------------------------------------------
eSpeccy::eSpeccy() : cpu(NULL), memory(NULL), devices(NULL), frame_tacts(0)
	, int_len(0), nmi_pending(0)
{
}
//=============================================================================
//	eSpeccy::~eSpeccy
//-----------------------------------------------------------------------------
eSpeccy::~eSpeccy()
{
	delete cpu;
	delete memory;
	delete devices;
}
//=============================================================================
//	eSpeccy::Init
//-----------------------------------------------------------------------------
void eSpeccy::Init()
{
	// pentagon timings
	frame_tacts = 71680;
	int_len = 32;

	memory = new eMemory;
	eRom* rom = new eRom(memory);
	eUla* ula = new eUla(memory);
	devices = new eDevices;
	cpu = new xZ80::eZ80(memory, rom, ula, devices, frame_tacts);

	devices->Add(rom, D_ROM);
	devices->Add(new eRam(memory), D_RAM);
	devices->Add(ula, D_ULA);
	devices->Add(new eKeyboard, D_KEYBOARD);
	devices->Add(new eBeeper(cpu), D_BEEPER);
	devices->Add(new eAY(cpu), D_AY);
}
//=============================================================================
//	eSpeccy::Reset
//-----------------------------------------------------------------------------
void eSpeccy::Reset()
{
	cpu->Reset();
	devices->Reset();
}
//=============================================================================
//	eSpeccy::Update
//-----------------------------------------------------------------------------
void eSpeccy::Update()
{
	Beeper()->StartFrame();
	AY()->StartFrame();
	cpu->Update(int_len, &nmi_pending);
	Ula()->Update();
	Beeper()->EndFrame(cpu->FrameTacts());
	AY()->EndFrame(cpu->FrameTacts());
}
//=============================================================================
//	eSpeccy::Keyboard
//-----------------------------------------------------------------------------
eKeyboard* eSpeccy::Keyboard() const
{
	return (eKeyboard*)devices->Item(D_KEYBOARD);
}
//=============================================================================
//	eSpeccy::Ula
//-----------------------------------------------------------------------------
eUla* eSpeccy::Ula() const
{
	return (eUla*)devices->Item(D_ULA);
}
//=============================================================================
//	eSpeccy::Beeper
//-----------------------------------------------------------------------------
eDeviceSound* eSpeccy::Beeper() const
{
	return (eDeviceSound*)devices->Item(D_BEEPER);
}
//=============================================================================
//	eSpeccy::AY
//-----------------------------------------------------------------------------
eDeviceSound* eSpeccy::AY() const
{
	return (eDeviceSound*)devices->Item(D_AY);
}
