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

#ifndef _1964_DLL_RSP_H
#define _1964_DLL_RSP_H

#include <xtl.h>
#include "../plugins.h"

/********** RSP DLL: Functions *********************/

void RSPCloseDLL( void );
void RSPDllAbout( HWND hWnd );
void RSPDllConfig( HWND hWnd );
void RSPRomClosed( void );
DWORD DoRspCycles( DWORD );
void InitiateRSP_1_1( RSP_INFO Rsp_Info, DWORD * Cycles);

//void CloseRSPPlugin (void);
BOOL LoadRSPPlugin();
void InitializeRSP (void);

extern BOOL rsp_plugin_is_loaded;

// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
//extern BOOL	g_bUseLLERspPlugin; // reinstate lle rsp
//extern int g_iAudioPlugin;
extern BOOL g_bUseRspAudio;
extern int g_iRspPlugin;

#endif