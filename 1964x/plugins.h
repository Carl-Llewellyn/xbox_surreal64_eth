/*$T plugins.h GC 1.136 02/28/02 09:02:31 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#pragma once

//#include <Common.h>
// original 1964
//controls tv sys only - similar to rice
#define TV_SYSTEM_NTSC		1
#define TV_SYSTEM_PAL		0
#define TV_SYSTEM_MPAL		0

// pj64 and all audio plugins
#define SYSTEM_NTSC			0
#define SYSTEM_PAL			1
#define SYSTEM_MPAL			2

#include <RSP.h>

#if defined(USE_MUSYX)
#include <AudioMusyX.h>
#elif defined(USE_M64PAUDIO)
#include <AudioM64Plus.h>
#else

#include <AudioJttL.h>
#include <AudioBasic.h>
#include <AudioNone.h>
#include <AudioAzimer.h>
#endif

#include "../Plugins.h"

#include <Input.h>
#include <Video.h>

extern GFX_INFO		Gfx_Info;
extern AUDIO_INFO	Audio_Info;
extern CONTROL		Controls[4];

