/**********************************************************************************
Common defines used in every plugin spec, 
version #1.0 maintained by zilmar (zilmar@emulation64.com)
All questions or suggestions should go through the mailing list.
http://www.egroups.com/group/Plugin64-Dev
***********************************************************************************/
#pragma once
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

// sometimes NTSC is 0 and PAL is 1, other times it's reverse - not a problem? (multiple files)
#define SYSTEM_NTSC					0 //1
#define SYSTEM_PAL					1 //0
#define SYSTEM_MPAL					2
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