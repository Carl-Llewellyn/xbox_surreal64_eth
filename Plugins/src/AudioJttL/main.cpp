/***************************************************************************
                          main.c  -  SDL Audio plugin for mupen64
                             -------------------
    begin                : Fri Oct 3 2003
    copyright            : (C) 2003 by Juha Luotio aka JttL
    email                : juha.luotio@porofarmi.net
    version              : 1.2
 ***************************************************************************/
/***************************************************************************
     This plug-in is originally based on Hactarux's "mupen audio plugin"
     and was modified by JttL. Actually there is no much original code
     left, but I think it's good to mention it anyway.
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 CHANGELOG:
 1.2:
 +Added possibility to swap channels
 +Some more optimizations
 +Calling RomOpen() is not required anymore. Plugin should now follow Zilmar's specs.
 +Added test functions.
 +Added support for config file

 1.1.1:
 +Fixed the bug that was causing terrible noise (thanks Law)
 +Much more debugging data appears now if DEBUG is defined
 +Few more error checks

 1.1:
 +Audio device is opened now with native byte ordering of the machine. Just
  for compatibility (thanks Flea).
 +Fixed possible double freeing bug (thanks Flea)
 +Optimizations in AiLenChanged
 +Fixed segmentation fault when changing rom.
 +Syncronization redone

 1.0.1.3:
 +Smarter versioning. No more betas.
 +More cleaning up done.
 +Buffer underrun and overflow messages appear now at stderr (if DEBUG is
  defined)
 +Many things are now precalculated (this should bring a small performance
  boost)
 +Buffer underrun bug fixed.
 +Segmentation fault when closing rom fixed (at least I think so)

 1.0 beta 2:
 +Makefile fixed to get rid of annoying warning messages
 +Cleaned up some old code
 +Default frequency set to 33600Hz (for Master Quest compatibility)
 +Better syncronization (needs some work still though)

 1.0 beta 1:
 +First public release


 ***************************************************************************/
/***************************************************************************
 TODO:
 +GUI for adjusting config file settings ;)

 ***************************************************************************/
/***************************************************************************
 KNOWN BUGS:

 ***************************************************************************/
#include <xtl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_thread.h"
#include "Audio.h"
#include "audiohle.h"

/* Current version number */
#define VERSION "1.2"

/* Size of primary buffer in bytes. This is the buffer where audio is loaded
   after it's extracted from n64's memory. */
#define PRIMARY_BUFFER_SIZE 65536

/* If buffer load goes under LOW_BUFFER_LOAD_LEVEL then game is speeded up to
   fill the buffer. If buffer load exeeds HIGH_BUFFER_LOAD_LEVEL then some
   extra slowdown is added to prevent buffer overflow (which is not supposed
   to happen in any circumstanses if syncronization is working but because
   computer's clock is such inaccurate (10ms) that might happen. I'm planning
   to add support for Real Time Clock for greater accuracy but we will see.

   The plugin tries to keep the buffer's load always between these values.
   So if you change only PRIMARY_BUFFER_SIZE, nothing changes. You have to
   adjust these values instead. You propably want to play with
   LOW_BUFFER_LOAD_LEVEL if you get dropouts. */
#define LOW_BUFFER_LOAD_LEVEL 16384
#define HIGH_BUFFER_LOAD_LEVEL 32768

/* Size of secondary buffer. This is actually SDL's hardware buffer. This is
   amount of samples, so final bufffer size is four times this. */
#define SECONDARY_BUFFER_SIZE 512

/* This sets default frequency what is used if rom doesn't want to change it.
   Popably only game that needs this is Zelda: Ocarina Of Time Master Quest */
#define DEFAULT_FREQUENCY 33600


/*--------------- VARIABLE DEFINITIONS ----------------*/

/* Read header for type definition */

// Ez0n3 - already declared in the Azimer/JttL lib :(
AUDIO_INFO AudioInfo;


/* The hardware specifications we are using */
static SDL_AudioSpec *hardware_spec;
/* Pointer to the primary audio buffer */
static Uint8 *buffer = NULL;
/* Position in buffer array where next audio chunk should be placed */
static unsigned int buffer_pos = 0;
/* Audio frequency, this is usually obtained from the game, but for compatibility we set default value */
static int frequency = DEFAULT_FREQUENCY;
/* This is for syncronization, it's ticks saved just before AiLenChanged() returns. */
static Uint32 last_ticks = 0;
/* This is amount of ticks that are needed for previous audio chunk to be played */
static Uint32 expected_ticks = 0;
/* AI_LEN_RG at previous round */
static DWORD prev_len_reg = 0;
/* If this is true then left and right channels are swapped */
static BOOL SwapChannels = FALSE;
/* */
static Uint32 PrimaryBufferSize = PRIMARY_BUFFER_SIZE;
/* */
static Uint32 SecondaryBufferSize = SECONDARY_BUFFER_SIZE;
/* */
static Uint32 LowBufferLoadLevel = LOW_BUFFER_LOAD_LEVEL;
/* */
static Uint32 HighBufferLoadLevel = HIGH_BUFFER_LOAD_LEVEL;


/* ----------- FUNCTIONS ------------- */
/* This function closes the audio device and reinitializes it with requested frequency */

// Ez0n3 - already declared in the Azimer/JttL lib :(
void InitializeAudio(int freq);

void InitializeSDL();

// Ez0n3 - already declared in the Azimer/JttL lib :(
void my_audio_callback(void *userdata, Uint8 *stream, int len);

void _AUDIO_JTTL_AiDacrateChanged( int SystemType )
{
	int f = frequency;

	switch (SystemType)
	{
        case SYSTEM_NTSC:
			f = 48681812 / (*AudioInfo.AI_DACRATE_RG + 1);
			break;
        case SYSTEM_PAL:
			f = 49656530 / (*AudioInfo.AI_DACRATE_RG + 1);
			break;
        case SYSTEM_MPAL:
			f = 48628316 / (*AudioInfo.AI_DACRATE_RG + 1);
			break;
	}

	InitializeAudio(f);
}

void  _AUDIO_JTTL_AiUpdate(BOOL Wait)
{
}

void _AUDIO_JTTL_AiLenChanged( void )
{
    /* Time that should be sleeped to keep game in sync */
    int wait_time = 0;
    DWORD LenReg = *AudioInfo.AI_LEN_RG;
    Uint8 *p = (Uint8*)(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_RG & 0xFFFFFF));

	if(buffer_pos + LenReg  < PrimaryBufferSize)
	{
		register unsigned int i;
		SDL_LockAudio();

		for ( i = 0 ; i < LenReg ; i += 4 )
		{
			if(SwapChannels == FALSE) 
			{
				/* Left channel */
				buffer[ buffer_pos + i ] = p[ i + 2 ];
				buffer[ buffer_pos + i + 1 ] = p[ i + 3 ];

				/* Right channel */
				buffer[ buffer_pos + i + 2 ] = p[ i ];
				buffer[ buffer_pos + i + 3 ] = p[ i + 1 ];
				} else {
				/* Left channel */
				buffer[ buffer_pos + i ] = p[ i ];
				buffer[ buffer_pos + i + 1 ] = p[ i + 1 ];

				/* Right channel */
				buffer[ buffer_pos + i + 2 ] = p[ i + 2];
				buffer[ buffer_pos + i + 3 ] = p[ i + 3 ];
			}
		}

		SDL_UnlockAudio();
		buffer_pos += i;
	}

	/* And then syncronization */

	/* If buffer is running slow we speed up the game a bit. Actually we skip the syncronization. */
	if(buffer_pos < LowBufferLoadLevel)
	{
		//wait_time -= (LOW_BUFFER_LOAD_LEVEL - buffer_pos);
		wait_time = -1;
	}

	if(wait_time != -1) 
	{
		/* If for some reason game is runnin extremely fast and there is risk buffer is going to
			overflow, we slow down the game a bit to keep sound smooth. The overspeed is caused
			by inaccuracy in machines clock. */
		if(buffer_pos > HighBufferLoadLevel)
		{
			wait_time += (int)((float)(buffer_pos - HIGH_BUFFER_LOAD_LEVEL) / (float)(frequency / 250));
		}

		expected_ticks = (Uint32)((float)(prev_len_reg) / (float)(frequency / 250));

		if(last_ticks + expected_ticks > SDL_GetTicks()) 
		{
			wait_time += (last_ticks + expected_ticks) - SDL_GetTicks();
			SDL_Delay(wait_time);
		}
	}
	
	last_ticks = SDL_GetTicks();
	prev_len_reg = LenReg;
}

DWORD _AUDIO_JTTL_AiReadLength( void )
{
   return 0;
}

void _AUDIO_JTTL_CloseDLL( void )
{
}

void _AUDIO_JTTL_DllAbout( HWND hParent )
{
}

void _AUDIO_JTTL_DllConfig ( HWND hParent )
{
}

void _AUDIO_JTTL_DllTest ( HWND hParent )
{
}

void _AUDIO_JTTL_GetDllInfo( PLUGIN_INFO * PluginInfo )
{
	PluginInfo->Version = 0x0101;
	PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
	sprintf(PluginInfo->Name,"JttL's SDL plugin %s", VERSION);
	PluginInfo->NormalMemory  = TRUE;
	PluginInfo->MemoryBswaped = TRUE;
}

BOOL _AUDIO_JTTL_InitiateAudio( AUDIO_INFO Audio_Info )
{
	ChangeABI(0);
	AudioInfo = Audio_Info;
	return TRUE;
}

// Ez0n3 - already declared in the Azimer/JttL lib :(
void my_audio_callback(void *userdata, Uint8 *stream, int len)
{
    if(buffer_pos > (unsigned int)len)
    {
        memcpy(stream, buffer, len);
        memmove(buffer, &buffer[ len ], buffer_pos  - len);

        buffer_pos = buffer_pos - len;
    }
    else
    {
        memcpy(stream, buffer, buffer_pos );
        buffer_pos = 0;
    }
}

void RomOpen()
{
	/* This function is for compatibility with mupen. */
	InitializeAudio(frequency);

}
void InitializeSDL()
{
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
	{
		OutputDebugString("Failed to initialize SDL audio subsystem!");
	}
}

// Ez0n3 - already declared in the Azimer/JttL lib :(
void InitializeAudio(int freq)
{
	SDL_AudioSpec *desired, *obtained;

	if(SDL_WasInit(SDL_INIT_AUDIO|SDL_INIT_TIMER) != (SDL_INIT_AUDIO|SDL_INIT_TIMER)) 
	{
		InitializeSDL();
	}

	frequency = freq; //This is important for the sync
        
	if(hardware_spec != NULL) 
		free(hardware_spec);
        
	SDL_PauseAudio(1);
    SDL_CloseAudio();
    
    // Allocate a desired SDL_AudioSpec 
    desired = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    // Allocate space for the obtained SDL_AudioSpec 
    obtained = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    // 22050Hz - FM Radio quality 
    desired->freq=freq;

    // 16-bit signed audio 
    desired->format=AUDIO_S16SYS;

    // Stereo 
    desired->channels=2;
    // Large audio buffer reduces risk of dropouts but increases response time 
    desired->samples=SecondaryBufferSize;

    // Our callback function 
    desired->callback=my_audio_callback;
    desired->userdata=NULL;

    // Open the audio device 
    if (SDL_OpenAudio(desired, obtained) < 0)
	{
        OutputDebugString("Couldn't open audio!");
    }

    // desired spec is no longer needed 
    if(desired->format != obtained->format)
    {
		OutputDebugString("Obtained audio format differs from requested!");
    }
    if(desired->freq != obtained->freq)
    {
		OutputDebugString("Obtained frequency differs from requested!");
    }

    free(desired);
    hardware_spec=obtained;

    SDL_PauseAudio(0);

	if(buffer == NULL)
    {
        buffer = (Uint8*)malloc(PrimaryBufferSize*sizeof(Uint8));
        SDL_PauseAudio(0);
    }
}

void _AUDIO_JTTL_RomClosed(void)
{
   SDL_PauseAudio(1);

   if(buffer != NULL) 
	   free(buffer);

   if(hardware_spec != NULL) 
	   free(hardware_spec);

   hardware_spec = NULL;
   buffer = NULL;
   SDL_Quit();
}

void _AUDIO_JTTL_ProcessAList(void)
{
	if (AudioInfo.DMEM)
		HLEStart();
}

//BOOL bAudioBoostJttL = FALSE;
void _AUDIO_JTTL_AudioBoost (BOOL Boost)
{
	//bAudioBoostJttL = Boost;
	SDL_AudioBoost(Boost ? 1 : 0);
}