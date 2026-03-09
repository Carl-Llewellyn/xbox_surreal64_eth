
#include <xtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Audio.h"

//#define MI_INTR_AI			0x04			/* Bit 2: AI intr */
//AUDIO_INFO AudioInfo; // <- already defined in jttl


void _AUDIO_NONE_AiDacrateChanged( int SystemType ) {
}

void _AUDIO_NONE_AiLenChanged( void ) {
}

DWORD _AUDIO_NONE_AiReadLength( void )
{
   return 0;
}

void  _AUDIO_NONE_AiUpdate(BOOL Wait) {
}

void _AUDIO_NONE_CloseDLL( void ) {
}

void _AUDIO_NONE_DllAbout( HWND hParent ) {
}

void _AUDIO_NONE_GetDllInfo( PLUGIN_INFO * PluginInfo ) {
	PluginInfo->Version = 0x0101;
	PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
	sprintf(PluginInfo->Name,"No Sound");
	PluginInfo->NormalMemory  = TRUE;
	PluginInfo->MemoryBswaped = TRUE;
}

BOOL _AUDIO_NONE_InitiateAudio( AUDIO_INFO Audio_Info )
{
	AudioInfo = Audio_Info;
	return TRUE;
}

void _AUDIO_NONE_ProcessAList(void) {
}


void _AUDIO_NONE_DllConfig ( HWND hParent ) {
}

void _AUDIO_NONE_DllTest ( HWND hParent ) {
}

void _AUDIO_NONE_RomClosed(void) {
}

/*
void RomOpen() {
}

void InitializeAudio(int freq) {
}
*/


