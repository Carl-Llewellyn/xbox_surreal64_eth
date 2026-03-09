/*$T Dll_Audio.h GC 1.136 02/28/02 08:57:44 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#ifndef _DLL_AUDIO_H__1964_
#define _DLL_AUDIO_H__1964_

#include "../plugins.h"

BOOL			LoadAudioPlugin();
//void			CloseAudioPlugin(void);

extern void		AUDIO_GetDllInfo(PLUGIN_INFO *Plugin_Info);
extern BOOL		AUDIO_Initialize(AUDIO_INFO Audio_Info);
extern void		AUDIO_ProcessAList(void);
extern void		AUDIO_DllConfig(HWND);
extern void		AUDIO_Test(HWND);
extern void		AUDIO_About(HWND);
extern void		AUDIO_AiDacrateChanged(int);
extern void		AUDIO_AiLenChanged(void);
extern DWORD	AUDIO_AiReadLength(void);
extern void		AUDIO_AiUpdate(BOOL);
extern void		AUDIO_RomClosed(void);


/* Used when selecting plugin */
/*extern void		AUDIO_Under_Selecting_Test(HWND);
extern void		AUDIO_Under_Selecting_About(HWND);
extern void (__cdecl *_AUDIO_Under_Selecting_Test) (HWND);
extern void (__cdecl *_AUDIO_Under_Selecting_About) (HWND);*/

extern BOOL	CoreDoingAIUpdate;

// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
//extern BOOL g_bUseBasicAudio;
extern int g_iAudioPlugin;
extern BOOL g_bAudioBoost;


#endif
