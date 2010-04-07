#include "std.h"
#include "platform/platform.h"
#include "speccy.h"
#include "devices/ula.h"
#include "devices/input/keyboard.h"
#include "devices/input/kempston_joy.h"
#include "devices/input/kempston_mouse.h"
#include "devices/input/tape.h"
#include "devices/sound/ay.h"
#include "devices/sound/beeper.h"
#include "devices/fdd/wd1793.h"
#include "z80/z80.h"
#include "snapshot.h"

static struct eSpeccyHandler : public xPlatform::eHandler
{
	eSpeccyHandler() : speccy(NULL), video_paused(false), drive_for_open(0)	{}
	virtual ~eSpeccyHandler() { assert(!speccy); }
	virtual void OnInit()
	{
		assert(!speccy);
		speccy = new eSpeccy;
		sound_dev[0] = speccy->Device<eBeeper>();
		sound_dev[1] = speccy->Device<eAY>();
		sound_dev[2] = speccy->Device<eTape>();
	}
	virtual void OnDone()
	{
		SAFE_DELETE(speccy);
	}
	virtual void OnLoop()
	{
		if(!video_paused)
			speccy->Update();
	}
	virtual void* VideoData() { return speccy->Device<eUla>()->Screen(); }
	virtual const char* WindowCaption() { return "UnrealSpeccy portable"; }
	virtual void OnKey(char key, dword flags)
	{
		using namespace xPlatform;
		bool down = (flags&KF_DOWN) != 0;
		bool shift = (flags&KF_SHIFT) != 0;
		bool ctrl = (flags&KF_CTRL) != 0;
		bool alt = (flags&KF_ALT) != 0;
		if(flags&KF_CURSOR)
		{
			switch(key)
			{
			case 'l' : key = '5'; shift = down; break;
			case 'r' : key = '8'; shift = down; break;
			case 'u' : key = '7'; shift = down; break;
			case 'd' : key = '6'; shift = down; break;
			case 'f' : key = '0'; shift = false; break;
			}
		}
		if(flags&KF_QAOP)
		{
			switch(key)
			{
			case 'l' : key = 'O'; break;
			case 'r' : key = 'P'; break;
			case 'u' : key = 'Q'; break;
			case 'd' : key = 'A'; break;
			case 'f' : key = ' '; break;
			}
		}
		if(flags&KF_SINCLAIR2)
		{
			switch(key)
			{
			case 'l' : key = '6'; break;
			case 'r' : key = '7'; break;
			case 'u' : key = '9'; break;
			case 'd' : key = '8'; break;
			case 'f' : key = '0'; break;
			}
		}
		speccy->Device<eKeyboard>()->OnKey(key, down, shift, ctrl, alt);
		if(flags&KF_KEMPSTON)
		{
			speccy->Device<eKempstonJoy>()->OnKey(key, down);
		}
	}
	virtual void OnMouse(xPlatform::eMouseAction action, byte a, byte b)
	{
		using namespace xPlatform;
		switch(action)
		{
		case MA_MOVE: 	speccy->Device<eKempstonMouse>()->OnMouseMove(a, b); 	break;
		case MA_BUTTON:	speccy->Device<eKempstonMouse>()->OnMouseButton(a, b != 0);	break;
		default: break;
		}
	}

	virtual bool OnOpenFile(const char* name)
	{
		int l = strlen(name);
		if(l > 3)
		{
			const char* n = name + l - 4;
			if(!strcmp(n, ".trd") || !strcmp(n, ".TRD") || !strcmp(n, ".scl") || !strcmp(n, ".SCL"))
			{
				return speccy->Device<eWD1793>()->Open(name, drive_for_open);
			}
			else if(!strcmp(n, ".sna") || !strcmp(n, ".SNA"))
			{
				return xSnapshot::Load(speccy, name);
			}
			else if(!strcmp(n, ".tap") || !strcmp(n, ".TAP") ||
					!strcmp(n, ".csw") || !strcmp(n, ".CSW") ||
					!strcmp(n, ".tzx") || !strcmp(n, ".TZX")
				)
			{
				return speccy->Device<eTape>()->Open(name);
			}
		}
		return false;
	}
	virtual xPlatform::eActionResult OnAction(xPlatform::eAction action)
	{
		using namespace xPlatform;
		switch(action)
		{
		case A_RESET:
			speccy->Reset();
			return AR_RESET_OK;
		case A_TAPE_TOGGLE:
			{
				eTape* tape = speccy->Device<eTape>();
				if(!tape->Inserted())
					return AR_TAPE_NOT_INSERTED;
				if(!tape->Started())
					tape->Start();
				else
					tape->Stop();
				return tape->Started() ? AR_TAPE_STARTED : AR_TAPE_STOPPED;
			}
		case A_TAPE_FAST_TOGGLE:
			{
				eTape* tape = speccy->Device<eTape>();
				if(!tape->Inserted())
					return AR_TAPE_NOT_INSERTED;
				if(!speccy->CPU()->FastEmul())
					speccy->CPU()->FastEmul(FastTapeEmul);
				else
					speccy->CPU()->FastEmul(NULL);
				return speccy->CPU()->FastEmul() ? AR_TAPE_FAST_SET : AR_TAPE_FAST_RESET;
			}
		case A_DRIVE_NEXT:
			{
				switch(++drive_for_open)
				{
				case 4:	drive_for_open = 0; return AR_DRIVE_A;
				case 1:	return AR_DRIVE_B;
				case 2:	return AR_DRIVE_C;
				case 3:	return AR_DRIVE_D;
				}
				return AR_ERROR;
			}
		}
		return AR_ERROR;
	}

	virtual int	AudioSources() { return SOUND_DEV_COUNT; }
	virtual void* AudioData(int source) { return sound_dev[source]->AudioData(); }
	virtual dword AudioDataReady(int source) { return sound_dev[source]->AudioDataReady(); }
	virtual void AudioDataUse(int source, dword size) { sound_dev[source]->AudioDataUse(size); }
	virtual void VideoPaused(bool paused) {	video_paused = paused; }

	eSpeccy* speccy;
	bool video_paused;
	int drive_for_open;

	enum { SOUND_DEV_COUNT = 3 };
	eDeviceSound* sound_dev[SOUND_DEV_COUNT];
} sh;

// see platform-specific files for main() function
