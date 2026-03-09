/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - Audio_1.2.h                                             *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2007-2008 Richard42 Ebenblues                           *
 *   Copyright (C) 2002 Zilmar                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**********************************************************************************
Notes:
------

Setting the approprate bits in the MI_INTR_REG and calling CheckInterrupts which
are both passed to the DLL in InitiateAudio will generate an Interrupt from with in
the plugin.

**********************************************************************************/

#pragma once

#include "Common.h"
#if defined(__cplusplus)
extern "C" {
#endif

/***** Structures *****/
//AUDIO_INFO struct -> common.h

/******************************************************************
  Function: AiDacrateChanged
  Purpose:  This function is called to notify the dll that the
            AiDacrate registers value has been changed.
  input:    The System type:
               SYSTEM_NTSC  0
               SYSTEM_PAL   1
               SYSTEM_MPAL  2
  output:   none
*******************************************************************/
void _AUDIO_M64P_AiDacrateChanged (int SystemType);

/******************************************************************
  Function: AiLenChanged
  Purpose:  This function is called to notify the dll that the
            AiLen registers value has been changed.
  input:    none
  output:   none
*******************************************************************/
void _AUDIO_M64P_AiLenChanged (void);

/******************************************************************
  Function: AiReadLength
  Purpose:  This function is called to allow the dll to return the
            value that AI_LEN_REG should equal
  input:    none
  output:   The amount of bytes still left to play.
*******************************************************************/
DWORD _AUDIO_M64P_AiReadLength (void);

/******************************************************************
  Function: AiUpdate
  Purpose:  This function is called to allow the dll to update
            things on a regular basis (check how long to sound to
            go, copy more stuff to the buffer, anyhting you like).
            The function is designed to go in to the message loop
            of the main window ... but can be placed anywhere you
            like.
  input:    if Wait is set to true, then this function should wait
            till there is a messgae in the its message queue.
  output:   none
*******************************************************************/
void _AUDIO_M64P_AiUpdate (BOOL Wait);

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/
void _AUDIO_M64P_CloseDLL (void);

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
void _AUDIO_M64P_DllAbout ( HWND hParent );

/******************************************************************
  Function: DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
void _AUDIO_M64P_DllConfig ( HWND hParent );

/******************************************************************
  Function: DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
void _AUDIO_M64P_DllTest ( HWND hParent );

/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/
void _AUDIO_M64P_GetDllInfo ( PLUGIN_INFO * PluginInfo );

/******************************************************************
  Function: InitiateSound
  Purpose:  This function is called when the DLL is started to give
            information from the emulator that the n64 audio
            interface needs
  Input:    Audio_Info is passed to this function which is defined
            above.
  Output:   TRUE on success
            FALSE on failure to initialise

  ** note on interrupts **:
  To generate an interrupt set the appropriate bit in MI_INTR_REG
  and then call the function CheckInterrupts to tell the emulator
  that there is a waiting interrupt.
*******************************************************************/
BOOL _AUDIO_M64P_InitiateAudio (AUDIO_INFO Audio_Info);

/******************************************************************
  Function: ProcessAList
  Purpose:  This function is called when there is a Alist to be
            processed. The Dll will have to work out all the info
            about the AList itself.
  input:    none
  output:   none
*******************************************************************/
void _AUDIO_M64P_ProcessAList(void);

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/
void _AUDIO_M64P_RomClosed (void);

/******************************************************************
  Function: PauseAudio
  Purpose:  This function is called when plugin should pause or
            re-enable sound. Audio should be enabled by default.
  input:    TRUE when audio should be paused.
            FALSE when audio sould continue playing.
  output:   TRUE when audio is paused.
            FALSE when audio plays.
*******************************************************************/
BOOL _AUDIO_M64P_PauseAudio (BOOL Pause);

//weinerschnitzel - we just use zilmar's spec, no need for this
/******************************************************************
   NOTE: THIS HAS BEEN ADDED FOR MUPEN64PLUS AND IS NOT PART OF THE
         ORIGINAL SPEC
  Function: SetConfigDir
  Purpose:  To pass the location where config files should be read/
            written to.
  input:    path to config directory
  output:   none
*******************************************************************/
//void _AUDIO_M64P_SetConfigDir( char *configDir );

/******************************************************************
   NOTE: THIS HAS BEEN ADDED FOR MUPEN64PLUS AND IS NOT PART OF THE
         ORIGINAL SPEC
  Function: VolumeUp
  Purpose:  To increase the audio volume
  input:    none
  output:   none
*******************************************************************/
//void _AUDIO_M64P_VolumeUp(void);

/******************************************************************
   NOTE: THIS HAS BEEN ADDED FOR MUPEN64PLUS AND IS NOT PART OF THE
         ORIGINAL SPEC
  Function: VolumeDown
  Purpose:  To decrease the audio volume
  input:    none
  output:   none
*******************************************************************/
//void _AUDIO_M64P_VolumeDown(void);

/******************************************************************
   NOTE: THIS HAS BEEN ADDED FOR MUPEN64PLUS AND IS NOT PART OF THE
         ORIGINAL SPEC
  Function: VolumeMute
  Purpose:  Toggles between muted and not muted
  input:    none
  output:   none
*******************************************************************/
//void _AUDIO_M64P_VolumeMute(void);

/******************************************************************
   NOTE: THIS HAS BEEN ADDED FOR MUPEN64PLUS AND IS NOT PART OF THE
         ORIGINAL SPEC
  Function: VolumeGet
  Purpose:  Return the current volume level
  input:    none
  output:   string containing volume level state (percentage or mute)
*******************************************************************/
//const char * _AUDIO_M64P_VolumeGetString(void);

void _AUDIO_M64P_AudioBoost (BOOL Boost);

#if defined(__cplusplus)
}
#endif