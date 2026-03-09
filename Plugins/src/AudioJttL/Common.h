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

#define SYSTEM_NTSC					0 //1
#define SYSTEM_PAL					1 //0
#define SYSTEM_MPAL					2