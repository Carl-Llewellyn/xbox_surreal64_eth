/**********************************************************************************
Common Audio plugin spec, version #1.1 maintained by zilmar (zilmar@emulation64.com)
All questions or suggestions should go through the mailing list.
http://www.egroups.com/group/Plugin64-Dev
**********************************************************************************
Notes:
------
Setting the approprate bits in the MI_INTR_RG and calling CheckInterrupts which 
are both passed to the DLL in _AUDIO_InitiateAudio will generate an Interrupt from with in 
the plugin.
**********************************************************************************/
#pragma once
#include "Common.h"
#if defined(__cplusplus)
extern "C" {
#endif
/* Plugin types */
#define PLUGIN_TYPE_RSP			1
#define PLUGIN_TYPE_GFX			2
#define PLUGIN_TYPE_AUDIO		3
#define PLUGIN_TYPE_CONTROLLER	4
/***** Structures *****/
typedef struct
{
	WORD Version;        /* Set to 0x0102 */
	WORD Type;           /* Set to PLUGIN_TYPE_GFX */
	char Name[100];      /* Name of the DLL */
	/* If DLL supports memory these memory options then set them to TRUE or FALSE
	   if it does not support it */
	BOOL NormalMemory;    /* a normal BYTE array */ 
	BOOL MemoryBswaped;  /* a normal BYTE array where the memory has been pre
	                          bswap on a dword (32 bits) boundry */
} PLUGIN_INFO;

#define SYSTEM_NTSC					0 //1
#define SYSTEM_PAL					1 //0
#define SYSTEM_MPAL					2 //0

/***** Structures *****/
typedef struct 
{
	HWND hwnd;
	HINSTANCE hinst;
	BOOL MemoryBswaped;    // If this is set to TRUE, then the memory has been pre
	                       //   bswap on a dword (32 bits) boundry 
						   //	eg. the first 8 bytes are stored like this:
	                       //        4 3 2 1   8 7 6 5
	BYTE * HEADER;	// This is the rom header (first 40h bytes of the rom
					// This will be in the same memory format as the rest of the memory.
	BYTE * RDRAM;
	BYTE * DMEM;
	BYTE * IMEM;
	DWORD * MI_INTR_RG;
	DWORD * AI_DRAM_ADDR_RG;
	DWORD * AI_LEN_RG;
	DWORD * AI_CONTROL_RG;
	DWORD * AI_STATUS_RG;
	DWORD * AI_DACRATE_RG;
	DWORD * AI_BITRATE_RG;
	void (*CheckInterrupts)( void );
} AUDIO_INFO;
/* Note: BOOL, BYTE, WORD, DWORD, TRUE, FALSE are defined in windows.h */
/******************************************************************
  Function: _AUDIO_AiDacrateChanged
  Purpose:  This function is called to notify the dll that the
            AiDacrate registers value has been changed.
  input:    The System type:
               SYSTEM_NTSC	0
               SYSTEM_PAL	1
               SYSTEM_MPAL	2
  output:   none
*******************************************************************/ 

 void  _AUDIO_AZIMER_AiDacrateChanged (int  SystemType);
/******************************************************************
  Function: _AUDIO_AiLenChanged
  Purpose:  This function is called to notify the dll that the
            AiLen registers value has been changed.
  input:    none
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_AiLenChanged (void);
/******************************************************************
  Function: _AUDIO_AiReadLength
  Purpose:  This function is called to allow the dll to return the
            value that AI_LEN_RG should equal
  input:    none
  output:   The amount of bytes still left to play.
*******************************************************************/ 
 DWORD  _AUDIO_AZIMER_AiReadLength (void);

/******************************************************************
  Function: _AUDIO_AiUpdate
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
 void  _AUDIO_AZIMER_AiUpdate (BOOL Wait);

/******************************************************************
  Function: _AUDIO_CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_CloseDLL (void);
/******************************************************************
  Function: _AUDIO_DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_DllAbout ( HWND hParent );

/******************************************************************
  Function: _AUDIO_DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_DllConfig ( HWND hParent );
/******************************************************************
  Function: _AUDIO_DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_DllTest ( HWND hParent );

/******************************************************************
  Function: _AUDIO_GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_GetDllInfo ( PLUGIN_INFO * PluginInfo );

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
  To generate an interrupt set the appropriate bit in MI_INTR_RG
  and then call the function CheckInterrupts to tell the emulator
  that there is a waiting interrupt.
*******************************************************************/ 
 BOOL  _AUDIO_AZIMER_InitiateAudio (AUDIO_INFO Audio_Info);
/******************************************************************
  Function: _AUDIO_ProcessAList
  Purpose:  This function is called when there is a Alist to be
            processed. The Dll will have to work out all the info
			about the AList itself.
  input:    none
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_ProcessAList(void);

/******************************************************************
  Function: _AUDIO_RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
 void  _AUDIO_AZIMER_RomClosed (void); void _AUDIO_AZIMER_AudioBoost (BOOL Boost);
#if defined(__cplusplus)
}
#endif
extern AUDIO_INFO AudioInfo;

