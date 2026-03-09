/*$T Dll_Audio.c GC 1.136 03/09/02 17:41:09 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Audio plugin interface functions
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*
 * 1964 Copyright (C) 1999-2002 Joel Middendorf, <schibo@emulation64.com> This
 * program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: schibo@emulation64.com, rice1964@yahoo.com
 */

#include <xtl.h>

#include "../globals.h"
#include "../n64rcp.h"
#include "../emulator.h"
#include "registry.h"
#include "DLL_Audio.h"
#include "DLL_Rsp.h"

AUDIO_INFO	Audio_Info;

BOOL	CoreDoingAIUpdate = TRUE;

//BOOL	g_bUseBasicAudio = FALSE; //use iAudioPlugin instead to determine if basic audio is used

#if defined(USE_MUSYX)
int g_iAudioPlugin = _AudioPluginMusyX; // set default to musyx
#elif defined(USE_M64PAUDIO)
int g_iAudioPlugin = _AudioPluginM64p; // set default to m64p
#else
int g_iAudioPlugin = _AudioPluginJttl; // set default to jttl
#endif

BOOL g_bAudioBoost = FALSE;

/* Used when selecting plugins */
//void (__cdecl *_AUDIO_Under_Selecting_About) (HWND) = NULL;
//void (__cdecl *_AUDIO_Under_Selecting_Test) (HWND) = NULL;


void (*_AUDIO_LINK_AiDacrateChanged)(int SystemType) = NULL;
void (*_AUDIO_LINK_AiLenChanged)(void) = NULL;
DWORD (*_AUDIO_LINK_AiReadLength)(void) = NULL;
void (*_AUDIO_LINK_AiUpdate)(BOOL Wait) = NULL;
void (*_AUDIO_LINK_CloseDLL)(void) = NULL;
void (*_AUDIO_LINK_DllAbout)(HWND hParent) = NULL;
void (*_AUDIO_LINK_DllConfig)(HWND hParent) = NULL;
void (*_AUDIO_LINK_DllTest)(HWND hParent) = NULL;
void (*_AUDIO_LINK_GetDllInfo)(PLUGIN_INFO *PluginInfo) = NULL;
BOOL (*_AUDIO_LINK_InitiateAudio)(AUDIO_INFO Audio_Info) = NULL;
void (*_AUDIO_LINK_ProcessAList)(void) = NULL;
void (*_AUDIO_LINK_RomClosed)(void) = NULL;
//DWORD (*_AUDIO_LINK_ProcessAListCountCycles)(void) = NULL;

void (*_AUDIO_LINK_AudioBoost)(BOOL Boost) = NULL;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BOOL LoadAudioPlugin()
{ 
	// plugin selection to replace bools: g_bUseBasicAudio, g_bUseLLERspPlugin;
#if defined(USE_MUSYX)
	//if (g_iAudioPlugin == _AudioPluginMusyX)
	{
		// freakdave - new MusyX Audio plugin
		_AUDIO_LINK_AiDacrateChanged 	= _AUDIO_MUSYX_AiDacrateChanged;
		_AUDIO_LINK_AiLenChanged	 	= _AUDIO_MUSYX_AiLenChanged;
		_AUDIO_LINK_AiReadLength	 	= _AUDIO_MUSYX_AiReadLength;
		_AUDIO_LINK_AiUpdate		 	= _AUDIO_MUSYX_AiUpdate;
		_AUDIO_LINK_CloseDLL		 	= _AUDIO_MUSYX_CloseDLL;
		_AUDIO_LINK_DllAbout		 	= _AUDIO_MUSYX_DllAbout;
		_AUDIO_LINK_DllConfig		 	= _AUDIO_MUSYX_DllConfig;
		_AUDIO_LINK_DllTest			 	= _AUDIO_MUSYX_DllTest;
		_AUDIO_LINK_GetDllInfo		 	= _AUDIO_MUSYX_GetDllInfo;
		_AUDIO_LINK_InitiateAudio	 	= _AUDIO_MUSYX_InitiateAudio;
		_AUDIO_LINK_ProcessAList	 	= _AUDIO_MUSYX_ProcessAList;
		_AUDIO_LINK_RomClosed		 	= _AUDIO_MUSYX_RomClosed;
		//_AUDIO_LINK_ProcessAListCountCycles = _AUDIO_MUSYX_ProcessAListCountCycles;
		
		_AUDIO_LINK_AudioBoost		 	= _AUDIO_MUSYX_AudioBoost;
	}
#elif defined(USE_M64PAUDIO)
	//if (g_iAudioPlugin == _AudioPluginM64p) // so it doesn't break for now
	{
		// Mupen 1.5 Audio plugin
		_AUDIO_LINK_AiDacrateChanged 	= _AUDIO_M64P_AiDacrateChanged;
		_AUDIO_LINK_AiLenChanged	 	= _AUDIO_M64P_AiLenChanged;
		_AUDIO_LINK_AiReadLength	 	= _AUDIO_M64P_AiReadLength;
		_AUDIO_LINK_AiUpdate		 	= _AUDIO_M64P_AiUpdate;
		_AUDIO_LINK_CloseDLL		 	= _AUDIO_M64P_CloseDLL;
		_AUDIO_LINK_DllAbout		 	= _AUDIO_M64P_DllAbout;
		_AUDIO_LINK_DllConfig		 	= _AUDIO_M64P_DllConfig;
		_AUDIO_LINK_DllTest			 	= _AUDIO_M64P_DllTest;
		_AUDIO_LINK_GetDllInfo		 	= _AUDIO_M64P_GetDllInfo;
		_AUDIO_LINK_InitiateAudio	 	= _AUDIO_M64P_InitiateAudio;
		_AUDIO_LINK_ProcessAList	 	= _AUDIO_M64P_ProcessAList;
		_AUDIO_LINK_RomClosed		 	= _AUDIO_M64P_RomClosed;
		//_AUDIO_LINK_ProcessAListCountCycles = _AUDIO_MUSYX_ProcessAListCountCycles;
		
		_AUDIO_LINK_AudioBoost		 	= _AUDIO_M64P_AudioBoost;
	}
#else

	if (g_iAudioPlugin == _AudioPluginNone)
	{
		// Ez0n3 - No Audio (declared in plugin.h include)
		_AUDIO_LINK_AiDacrateChanged 	= _AUDIO_NONE_AiDacrateChanged;
		_AUDIO_LINK_AiLenChanged	 	= _AUDIO_NONE_AiLenChanged;
		_AUDIO_LINK_AiReadLength	 	= _AUDIO_NONE_AiReadLength;
		_AUDIO_LINK_AiUpdate		 	= _AUDIO_NONE_AiUpdate;
		_AUDIO_LINK_CloseDLL		 	= _AUDIO_NONE_CloseDLL;
		_AUDIO_LINK_DllAbout		 	= _AUDIO_NONE_DllAbout;
		_AUDIO_LINK_DllConfig		 	= _AUDIO_NONE_DllConfig;
		_AUDIO_LINK_DllTest			 	= _AUDIO_NONE_DllTest;
		_AUDIO_LINK_GetDllInfo		 	= _AUDIO_NONE_GetDllInfo;
		_AUDIO_LINK_InitiateAudio	 	= _AUDIO_NONE_InitiateAudio;
		_AUDIO_LINK_ProcessAList	 	= _AUDIO_NONE_ProcessAList;
		_AUDIO_LINK_RomClosed		 	= _AUDIO_NONE_RomClosed;
	}
	/*else if (g_iAudioPlugin == _AudioPluginLleRsp)
	{
		// deprecated
	}*/
	else if (g_iAudioPlugin == _AudioPluginBasic)
	{
		_AUDIO_LINK_AiDacrateChanged 	= _AUDIO_BASIC_AiDacrateChanged;
		_AUDIO_LINK_AiLenChanged	 	= _AUDIO_BASIC_AiLenChanged;
		_AUDIO_LINK_AiReadLength	 	= _AUDIO_BASIC_AiReadLength;
		_AUDIO_LINK_AiUpdate		 	= _AUDIO_BASIC_AiUpdate;
		_AUDIO_LINK_CloseDLL		 	= _AUDIO_BASIC_CloseDLL;
		_AUDIO_LINK_DllAbout		 	= _AUDIO_BASIC_DllAbout;
		_AUDIO_LINK_DllConfig		 	= _AUDIO_BASIC_DllConfig;
		_AUDIO_LINK_DllTest			 	= _AUDIO_BASIC_DllTest;
		_AUDIO_LINK_GetDllInfo		 	= _AUDIO_BASIC_GetDllInfo;
		_AUDIO_LINK_InitiateAudio	 	= _AUDIO_BASIC_InitiateAudio;
		_AUDIO_LINK_ProcessAList	 	= _AUDIO_BASIC_ProcessAList;
		_AUDIO_LINK_RomClosed		 	= _AUDIO_BASIC_RomClosed;
	}
	else if (g_iAudioPlugin == _AudioPluginJttl)
	{
		//freakdave - JttL 1.2
		_AUDIO_LINK_AiDacrateChanged 	= _AUDIO_JTTL_AiDacrateChanged;
		_AUDIO_LINK_AiLenChanged	 	= _AUDIO_JTTL_AiLenChanged;
		_AUDIO_LINK_AiReadLength	 	= _AUDIO_JTTL_AiReadLength;
		_AUDIO_LINK_AiUpdate		 	= _AUDIO_JTTL_AiUpdate;
		_AUDIO_LINK_CloseDLL		 	= _AUDIO_JTTL_CloseDLL;
		_AUDIO_LINK_DllAbout		 	= _AUDIO_JTTL_DllAbout;
		_AUDIO_LINK_DllConfig		 	= _AUDIO_JTTL_DllConfig;
		_AUDIO_LINK_DllTest			 	= _AUDIO_JTTL_DllTest;
		_AUDIO_LINK_GetDllInfo		 	= _AUDIO_JTTL_GetDllInfo;
		_AUDIO_LINK_InitiateAudio	 	= _AUDIO_JTTL_InitiateAudio;
		_AUDIO_LINK_ProcessAList	 	= _AUDIO_JTTL_ProcessAList;
		_AUDIO_LINK_RomClosed		 	= _AUDIO_JTTL_RomClosed;
		
		_AUDIO_LINK_AudioBoost		 	= _AUDIO_JTTL_AudioBoost;
	}
	else if (g_iAudioPlugin == _AudioPluginAzimer)
	{
		//freakdave - readded Azimer plugin from Surreal 1.0 and FDB
		_AUDIO_LINK_AiDacrateChanged 	= _AUDIO_AZIMER_AiDacrateChanged;
		_AUDIO_LINK_AiLenChanged	 	= _AUDIO_AZIMER_AiLenChanged;
		_AUDIO_LINK_AiReadLength	 	= _AUDIO_AZIMER_AiReadLength;
		_AUDIO_LINK_AiUpdate		 	= _AUDIO_AZIMER_AiUpdate;
		_AUDIO_LINK_CloseDLL		 	= _AUDIO_AZIMER_CloseDLL;
		_AUDIO_LINK_DllAbout		 	= _AUDIO_AZIMER_DllAbout;
		_AUDIO_LINK_DllConfig		 	= _AUDIO_AZIMER_DllConfig;
		_AUDIO_LINK_DllTest			 	= _AUDIO_AZIMER_DllTest;
		_AUDIO_LINK_GetDllInfo		 	= _AUDIO_AZIMER_GetDllInfo;
		_AUDIO_LINK_InitiateAudio	 	= _AUDIO_AZIMER_InitiateAudio;
		_AUDIO_LINK_ProcessAList	 	= _AUDIO_AZIMER_ProcessAList;
		_AUDIO_LINK_RomClosed		 	= _AUDIO_AZIMER_RomClosed;
		
		_AUDIO_LINK_AudioBoost		 	= _AUDIO_AZIMER_AudioBoost;
	}
#endif

	__try
	{
		_AUDIO_LINK_AudioBoost(g_bAudioBoost);
		_AUDIO_LINK_AudioBoost = NULL;
	}
	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return TRUE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_GetDllInfo(PLUGIN_INFO *Plugin_Info)
{
	__try
	{
		_AUDIO_LINK_GetDllInfo(Plugin_Info);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BOOL AUDIO_Initialize(AUDIO_INFO Audio_Info)
{ 
	__try
	{
		_AUDIO_LINK_InitiateAudio(Audio_Info);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		/* Some people won't have a soud card. No error. */
	}

	return(1);	/* for now.. */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_ProcessAList(void)
{
	__try
	{
		_AUDIO_LINK_ProcessAList();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		/* Some people won't have a soud card. No error. */
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_RomClosed(void)
{
	__try
	{
		_AUDIO_LINK_RomClosed();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_DllClose(void)
{
	__try
	{
		_AUDIO_LINK_CloseDLL();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*void CloseAudioPlugin(void)
{
	// AUDIO_End();
	AUDIO_RomClosed();
	AUDIO_DllClose();
}*/

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_DllConfig(HWND hParent)
{
	//_AUDIO_LINK_DllConfig(hParent);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_About(HWND hParent)
{
	_AUDIO_LINK_DllAbout(hParent);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_Test(HWND hParent)
{
	//_AUDIO_LINK_DllTest(hParent);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_AiDacrateChanged(int SystemType)
{
	_AUDIO_LINK_AiDacrateChanged(SystemType);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_AiLenChanged(void)
{
	_AUDIO_LINK_AiLenChanged();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
DWORD AUDIO_AiReadLength(void)
{
	return _AUDIO_LINK_AiReadLength();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AUDIO_AiUpdate(BOOL update)
{
	_AUDIO_LINK_AiUpdate(update);
}
/*
DWORD AUDIO_ProcessAListCountCycles(void)
{
	_AUDIO_LINK_ProcessAListCountCycles();
}
*/
/*
 =======================================================================================================================
    Used when selecting plugins
 =======================================================================================================================
 */
/*void AUDIO_Under_Selecting_About(HWND hParent)
{
	if(_AUDIO_LINK_Under_Selecting_About != NULL)
	{
		_AUDIO_LINK_Under_Selecting_About(hParent);
	}
	else
	{

	}
}*/

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*void AUDIO_Under_Selecting_Test(HWND hParent)
{
	if(_AUDIO_LINK_Under_Selecting_Test != NULL)
	{
		_AUDIO_LINK_Under_Selecting_Test(hParent);
	}
	else
	{
	}
}*/
