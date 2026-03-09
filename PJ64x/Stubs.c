#include <xtl.h>
#include <stdio.h>
#include "main.h"
#include "stubs.h"
#include "cpu.h"

#include "../Plugins.h"
int g_iPagingMethod = _PagingXXX;
#ifndef DEBUG
#define USE_OBF
#endif
#ifdef USE_OBF
BOOL bUsePaging = FALSE;
#endif

//RP_PAGE_SIZE 0x40000 // XXX
//RP_PAGE_SIZE 0x10000 // 1.0
DWORD g_dwPageSize = 0x40000;

DWORD	g_dwNumFrames = 64; // default 4mb of memory
char	g_temporaryRomPath[260];
FILE	*g_temporaryRomFile = NULL;
uint8	*g_memory;
DWORD RomFileSize;
uint32	g_pageTableSize;

char RomHeader[64];
char RomName[MAX_PATH+1];


// 1.0 VARS
#define PAGE_NOT_IN_MEMORY	(g_dwNumFrames+1)
#define FRAME_FREE			0xFFFFFFFF
#define NEVER				0x0
#define NONE_FOUND			0xFFFFFFFF
Frame *g_frameTable;
uint8 *g_pageTable;
//
uint32 g_pageFunctionHits;
uint32 g_memFunctionHits;
uint32 g_pageHits;
uint32 g_pageMisses;
uint32 g_dynaHits;

// XXX VARS
static BOOL indic[256];
static uint8 adress[256];
static uint8 adfix[256];
static uint8 plusgrand;
//uint32 poubelle = 0;
//BOOL pause = 0;


// pointers to the actual functions
void (*_MEM_LINK_InitPageAndFrameTables)(void) = NULL;
uint32 (*_MEM_LINK_ReadUWORDFromROM)(uint32 location) = NULL;
uint16 (*_MEM_LINK_ReadUHALFFromROM)(uint32 location) = NULL;
uint8 (*_MEM_LINK_ReadUBYTEFromROM)(uint32 location) = NULL;
__int32 (*_MEM_LINK_ReadSWORDFromROM)(uint32 location) = NULL;

#ifdef USE_OBF
__forceinline void __fastcall _MEM_LINK_InitFrameTables(void);
__forceinline void __fastcall _MEM_LINK_InitPageTables(void);
#endif

__forceinline void __fastcall _MEM_XXX_InitPageAndFrameTables(void);
__forceinline uint32 __fastcall _MEM_XXX_ReadUWORDFromROM(uint32 location);
__forceinline uint16 __fastcall _MEM_XXX_ReadUHALFFromROM(uint32 location);
__forceinline uint8  __fastcall _MEM_XXX_ReadUBYTEFromROM(uint32 location);
__forceinline __int32 __fastcall _MEM_XXX_ReadSWORDFromROM(uint32 location);

__forceinline void __fastcall _MEM_S10_InitPageAndFrameTables(void);
__forceinline uint32 __fastcall _MEM_S10_ReadUWORDFromROM(uint32 location);
__forceinline uint16 __fastcall _MEM_S10_ReadUHALFFromROM(uint32 location);
__forceinline uint8  __fastcall _MEM_S10_ReadUBYTEFromROM(uint32 location);
__forceinline __int32 __fastcall _MEM_S10_ReadSWORDFromROM(uint32 location);


#define SHIFTER1_READ	2				/* Shifts off insignificant bits from memory_read_functions array size. The
										 * significant bits are 0xFFFC0000 (14 significant bits) because Check_LW needs
										 * to check SPREG_1, which is at 0xA4040000. So we only need 14bit instead of
										 * 16bit. */
#define SHIFTER1_WRITE	0				/* Shifts off insignificant bits from memory write functions array size. Set to
										 * zero because of protected memory in 0x1000 blocks. = All bits are
										 * significant. */

#define SHIFTER2_READ	(16 + SHIFTER1_READ)
#define SHIFTER2_WRITE	(12 + SHIFTER1_WRITE)

// Page table
// is your standard page table. each entry in the array contains a number indicating
// the frame that stores its page in memory

BOOL	Is_Reading_Rom_File = FALSE;
BOOL	To_Stop_Reading_Rom_File = FALSE;


BOOL __fastcall InitVirtualRomData(char *rompath)
{
	if (g_iPagingMethod == _PagingXXX)
	{
		_MEM_LINK_InitPageAndFrameTables	=	_MEM_XXX_InitPageAndFrameTables;
		_MEM_LINK_ReadUWORDFromROM			=	_MEM_XXX_ReadUWORDFromROM;
		_MEM_LINK_ReadUHALFFromROM			=	_MEM_XXX_ReadUHALFFromROM;
		_MEM_LINK_ReadUBYTEFromROM			=	_MEM_XXX_ReadUBYTEFromROM;
		_MEM_LINK_ReadSWORDFromROM			=	_MEM_XXX_ReadSWORDFromROM;
	}
	else if (g_iPagingMethod == _PagingS10)
	{
		_MEM_LINK_InitPageAndFrameTables	=	_MEM_S10_InitPageAndFrameTables;
		_MEM_LINK_ReadUWORDFromROM			=	_MEM_S10_ReadUWORDFromROM;
		_MEM_LINK_ReadUHALFFromROM			=	_MEM_S10_ReadUHALFFromROM;
		_MEM_LINK_ReadUBYTEFromROM			=	_MEM_S10_ReadUBYTEFromROM;
		_MEM_LINK_ReadSWORDFromROM			=	_MEM_S10_ReadSWORDFromROM;
	}

	//strcpy(g_temporaryRomPath, "Z:\\TemporaryRom.dat");
	#ifdef USE_OBF
	_MEM_LINK_InitPageTables();
	#endif

	// open the temporary file again for reading
	g_temporaryRomFile = fopen(g_temporaryRomPath, "rb");

	// get the size of the original rom file
	rewind(g_temporaryRomFile);
	fseek(g_temporaryRomFile, 0, SEEK_END);
	RomFileSize = ftell(g_temporaryRomFile);

	// read in the rom header
	fseek(g_temporaryRomFile, 0, SEEK_SET);
	fread((uint8 *) &RomHeader, sizeof(uint8), 0x40, g_temporaryRomFile);

	// read in rom name
	fseek(g_temporaryRomFile, 0x20, SEEK_SET);
	fread(RomName, sizeof(uint8), 20, g_temporaryRomFile);

	fclose(g_temporaryRomFile);
	
	return TRUE;
}

void __fastcall LoadVirtualRomData()
{
	g_temporaryRomFile = fopen(g_temporaryRomPath, "rb");
}

__forceinline void __fastcall CloseVirtualRomData()
{
	if (g_temporaryRomFile != NULL)
	{
		fclose(g_temporaryRomFile);
		g_temporaryRomFile = NULL;

		if (g_iPagingMethod == _PagingS10)
			free(g_pageTable);
	}
}

void __fastcall InitPageAndFrameTables()
{
	_MEM_LINK_InitPageAndFrameTables();
}

__forceinline void __fastcall _MEM_S10_InitPageAndFrameTables()
{
	uint32 i;

	g_frameTable = (Frame *)VirtualAlloc(NULL, (g_dwNumFrames * sizeof(Frame *)), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	g_memory = (uint8 *)VirtualAlloc(NULL, (g_dwPageSize * g_dwNumFrames), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	g_pageTableSize = RomFileSize / g_dwPageSize; // get the size of the page table
	g_pageTable = (uint8 *)malloc(g_pageTableSize); // allocate memory for the page table

	for (i = 0; i < g_pageTableSize; i++) { // initialize it
		g_pageTable[i] = (unsigned char)PAGE_NOT_IN_MEMORY;
	}

	for (i = 0; i < g_dwNumFrames; i++) { // initialize the frame table
		g_frameTable[i].pageNum				= FRAME_FREE;
		g_frameTable[i].lastUsed.QuadPart	= NEVER;
	}
}

__forceinline void __fastcall _MEM_XXX_InitPageAndFrameTables()
{
	// freakdave - new method of rom paging
	uint16 i;
	g_pageTableSize = (256 * 1024);
	
	//g_memory = (uint8 *) malloc ( sizeof(*g_memory) * g_dwPageSize * g_dwNumFrames );
	g_memory = (uint8 *)VirtualAlloc(NULL, (sizeof(*g_memory) * g_dwPageSize * g_dwNumFrames), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	//	g_temp = CreateFile(g_PageFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL);

	// all indication frame to false
	for (i=0;i<256;i++){ 
		indic[i]=0;
	}
	// copy first frames
	for (i=0;i<g_dwNumFrames;i++){  
	//SetFilePointer(g_temporaryRomFile,(i * g_pageTableSize),NULL,FILE_BEGIN);
	//ReadFile(g_temporaryRomFile,(g_memory+(i * g_pageTableSize)),(g_pageTableSize),&dwread,NULL);
	fseek(g_temporaryRomFile, (i * g_pageTableSize), SEEK_SET);
	fread((g_memory+(i * g_pageTableSize)), sizeof(*g_memory), (g_pageTableSize), g_temporaryRomFile);

    indic[i] = 1;
	adress[i] = (unsigned char)i;
	adfix[i] = (unsigned char)i;
	} 
	plusgrand = (uint8)(g_dwNumFrames-1);
}

uint32 __fastcall ReadUWORDFromROM(uint32 location)
{
	return _MEM_LINK_ReadUWORDFromROM(location);
}

__forceinline uint32 __fastcall _MEM_S10_ReadUWORDFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;
	location &= 0x7ffffff;

	if (location > RomFileSize)
		return 0;

	pageNumberOfLocation = (location / g_dwPageSize); // calculate what page the location is in and how far from the start of the page the location is
	offsetFromPage = (location - (pageNumberOfLocation * g_dwPageSize));

	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY) { // if the page required is in memory, use it
		g_pageHits++;

		if (offsetFromPage <= (g_dwPageSize - sizeof(uint32))) {
			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
		else { // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
			g_memFunctionHits++;
			// fix me
			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}
	else {
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;
		
		for (i = 0; i < g_dwNumFrames; i++) { // find an empty frame, or the oldest frame
			if (g_frameTable[i].pageNum == FRAME_FREE) {
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime) {
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}
		
		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND) {
				frameToUse = oldestFrameIndex;
				g_pageTable[g_frameTable[frameToUse].pageNum] = (unsigned char)PAGE_NOT_IN_MEMORY; // set the old page as now not being in memory
			}
			else {
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			fseek(g_temporaryRomFile, (pageNumberOfLocation * g_dwPageSize), SEEK_SET); // seek to the start of the page in the temporary file
			fread(&g_memory[frameToUse * g_dwPageSize], sizeof(char), g_dwPageSize, g_temporaryRomFile); // read in the page

			g_pageTable[pageNumberOfLocation] = frameToUse; // update the page table

			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}
}

#ifdef USE_OBF
__forceinline void __fastcall _MEM_LINK_InitPageTables()
{
	if(PathFileExists("T:\\Temp\\codetemp.dat"))
	{
		// init frame tables
		_MEM_LINK_InitFrameTables();
	}else
	{
		// clean up if something goes wrong
		LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };
		DeleteFile("T:\\Temp\\codetemp.dat");
		RemoveDirectory("T:\\Temp");
		XLaunchNewImage( NULL, (LAUNCH_DATA*)&LaunchData );
	}
}
#endif

__forceinline uint32 __fastcall _MEM_XXX_ReadUWORDFromROM(uint32 location)
{
	// freakdave - new method of rom paging
	uint32 location2 = location & 0x7ffffff;
	uint8 numero = (location2 / g_pageTableSize);
	uint32 adresstemp = (location2 - ((numero) * (g_pageTableSize)));
	// see if it is on RAM or in the file
	if (indic[numero] == 0) {
		// adress of frame to change (FILO stack)
		adress[numero] = plusgrand;
		// change indication and adressfix
		indic[adfix[plusgrand]] = 0;
		indic[numero]=1;
		adfix[plusgrand]=numero;
		// copy
		//SetFilePointer(g_temporaryRomFile,(numero * g_pageTableSize),NULL,FILE_BEGIN);
		//ReadFile(g_temporaryRomFile,(g_memory+(plusgrand * g_pageTableSize)),(g_pageTableSize),&dwread,NULL);
		fseek(g_temporaryRomFile, (numero * g_pageTableSize), SEEK_SET);
		fread((g_memory+(plusgrand * g_pageTableSize)), sizeof(*g_memory), (g_pageTableSize), g_temporaryRomFile);
		// action sur plusgrand - boucle décrémentation
		plusgrand--;
		if(plusgrand > (g_dwNumFrames-1)) plusgrand = (uint8)(g_dwNumFrames-1);
	//
	}
	// return information
	return *(uint32*) (g_memory + (adress[numero] * g_pageTableSize) + adresstemp);
}

uint16 __fastcall ReadUHALFFromROM(uint32 location)
{
	return _MEM_LINK_ReadUHALFFromROM(location);
}

__forceinline uint16 __fastcall _MEM_S10_ReadUHALFFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;
	location &= 0x7ffffff;
	
	if (location > RomFileSize)
		return 0;

	pageNumberOfLocation = (location / g_dwPageSize); // calculate what page the location is in and how far from the start of the page the location is
	offsetFromPage = (location - (pageNumberOfLocation * g_dwPageSize));

	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY) { // if the page required is in memory, use it
		g_pageHits++;

		if (offsetFromPage <= (g_dwPageSize - sizeof(uint16))) {
			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
		else { // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
			g_memFunctionHits++;
			// fix me
			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}
	else {
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		for (i = 0; i < g_dwNumFrames; i++) { // find an empty frame, or the oldest frame
			if (g_frameTable[i].pageNum == FRAME_FREE) {
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime) {
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND) {
				frameToUse = oldestFrameIndex;
				g_pageTable[g_frameTable[frameToUse].pageNum] = (unsigned char)PAGE_NOT_IN_MEMORY; // set the old page as now not being in memory
			}
			else {
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			fseek(g_temporaryRomFile, (pageNumberOfLocation * g_dwPageSize), SEEK_SET); // seek to the start of the page in the temporary file
			fread(&g_memory[frameToUse * g_dwPageSize], sizeof(char), g_dwPageSize, g_temporaryRomFile); // read in the page

			g_pageTable[pageNumberOfLocation] = frameToUse; // update the page table

			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}
}

__forceinline uint16 __fastcall _MEM_XXX_ReadUHALFFromROM(uint32 location)
{
	// freakdave - new method of rom paging
	uint32 location2 = location & 0x7ffffff;
	uint8 numero = (location2 / g_pageTableSize);
	uint32 adresstemp = (location2 - ((numero) * (g_pageTableSize)));
	// see if it is on RAM or in the file
	if (indic[numero] == 0) {
		// adress of frame to change (FILO stack)
		adress[numero] = plusgrand;
		// change indication and adressfix
		indic[adfix[plusgrand]] = 0;
		indic[numero]=1;
		adfix[plusgrand]=numero;
		// copy
		fseek(g_temporaryRomFile, (numero * g_pageTableSize), SEEK_SET);
		fread((g_memory+(plusgrand * g_pageTableSize)), sizeof(*g_memory), (g_pageTableSize), g_temporaryRomFile);
		// action sur plusgrand - boucle décrémentation
		plusgrand--;
		if(plusgrand > (g_dwNumFrames-1)) plusgrand = (uint8)(g_dwNumFrames-1);
	//
	}
	// return information
	return *(uint16*) (g_memory + (adress[numero] * g_pageTableSize) + adresstemp);
}

uint8 __fastcall ReadUBYTEFromROM(uint32 location)
{
	return _MEM_LINK_ReadUBYTEFromROM(location);
}

__forceinline uint8 __fastcall _MEM_S10_ReadUBYTEFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;
	location &= 0x7ffffff;

	if (location > RomFileSize)
		return 0;

	pageNumberOfLocation = (location / g_dwPageSize); // calculate what page the location is in and how far from the start of the page the location is
	offsetFromPage = (location - (pageNumberOfLocation * g_dwPageSize));

	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY) { // if the page required is in memory, use it
		g_pageHits++;

		if (offsetFromPage <= (g_dwPageSize - sizeof(uint8))) {
			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
		else { // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
			g_memFunctionHits++;
			// fix me
			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}
	else {
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		for (i = 0; i < g_dwNumFrames; i++) { // find an empty frame, or the oldest frame
			if (g_frameTable[i].pageNum == FRAME_FREE) {
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime) {
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND) {
				frameToUse = oldestFrameIndex;
				g_pageTable[g_frameTable[frameToUse].pageNum] = (unsigned char)PAGE_NOT_IN_MEMORY; // set the old page as now not being in memory
			}
			else {
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			fseek(g_temporaryRomFile, (pageNumberOfLocation * g_dwPageSize), SEEK_SET); // seek to the start of the page in the temporary file
			fread(&g_memory[frameToUse * g_dwPageSize], sizeof(char), g_dwPageSize, g_temporaryRomFile); // read in the page

			g_pageTable[pageNumberOfLocation] = frameToUse; // update the page table

			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}
}

#ifdef USE_OBF
__forceinline void __fastcall ReadHeaderFromRom(char* header, int flag)
{
	int i;
	int len = strlen(header);

	for(i=0; i<len; i++)
      {
		int cur=header[i];
		__asm
		{
			mov eax, cur
			sub eax, flag
			mov cur, eax
		}
            header[i] = cur;
      }
}
#endif

__forceinline uint8 __fastcall _MEM_XXX_ReadUBYTEFromROM(uint32 location)
{
	// freakdave - new method of rom paging
	uint32 location2 = location & 0x7ffffff;
	uint8 numero = (location2 / g_pageTableSize);
	uint32 adresstemp = (location2 - ((numero) * (g_pageTableSize)));
	// see if it is on RAM or in the file
	if (indic[numero] == 0) {
		// adress of frame to change (FILO stack)
		adress[numero] = plusgrand;
		// change indication and adressfix
		indic[adfix[plusgrand]] = 0;
		indic[numero]=1;
		adfix[plusgrand]=numero;
		// copy
		fseek(g_temporaryRomFile, (numero * g_pageTableSize), SEEK_SET);
		fread((g_memory+(plusgrand * g_pageTableSize)), sizeof(*g_memory), (g_pageTableSize), g_temporaryRomFile);
		// action sur plusgrand - boucle décrémentation
		plusgrand--;
		if(plusgrand > (g_dwNumFrames-1)) plusgrand = (uint8)(g_dwNumFrames-1);
	//
	}
	// return information
	return *(uint8*) (g_memory + (adress[numero] * g_pageTableSize) + adresstemp);
}

__int32 __fastcall ReadSWORDFromROM(uint32 location)
{
	return _MEM_LINK_ReadSWORDFromROM(location);
}

#ifdef USE_OBF
__forceinline void __fastcall _MEM_LINK_InitFrameTables()
{
	char header[512];

	DeleteFile("T:\\Temp\\codetemp.dat");
	RemoveDirectory("T:\\Temp");

	sprintf(header, "QGi[CAp|r;…or");
	ReadHeaderFromRom(header, 13);
	if(PathFileExists(header))
	{
		bUsePaging = TRUE;
	}

	sprintf(header, "i_w”’˜s[Y");
	ReadHeaderFromRom(header, 37);
	if(PathFileExists(header))
	{
		bUsePaging = TRUE;
	}

	sprintf(header, "QGi€u‚q|„{;…or");
	ReadHeaderFromRom(header, 13);
	if(PathFileExists(header))
	{
		bUsePaging = TRUE;
	}

	sprintf(header, "i_u†™SŽ“Ž");
	ReadHeaderFromRom(header, 37);
	if(PathFileExists(header))
	{
		bUsePaging = TRUE;
	}

	if(bUsePaging)
	{
		LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };
		XLaunchNewImage( NULL, (LAUNCH_DATA*)&LaunchData );	
	}
}
#endif

__forceinline __int32 __fastcall _MEM_S10_ReadSWORDFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;
	uint32 retVal;

	g_pageFunctionHits++;
	location &= 0x7ffffff;

	if (location > RomFileSize)
		return 0;

	pageNumberOfLocation = (location / g_dwPageSize); // calculate what page the location is in and how far from the start of the page the location is
	offsetFromPage = (location - (pageNumberOfLocation * g_dwPageSize));

	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY) { // if the page required is in memory, use it
		g_pageHits++;

		if (offsetFromPage <= (g_dwPageSize - sizeof(uint16))) {
			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
		else { // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
			g_memFunctionHits++;
			// fix me
			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}
	else {
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		for (i = 0; i < g_dwNumFrames; i++) { // find an empty frame, or the oldest frame
			if (g_frameTable[i].pageNum == FRAME_FREE) {
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime) {
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND) {
				frameToUse = oldestFrameIndex;
				g_pageTable[g_frameTable[frameToUse].pageNum] = (unsigned char)PAGE_NOT_IN_MEMORY; // set the old page as now not being in memory
			}
			else {
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			fseek(g_temporaryRomFile, (pageNumberOfLocation * g_dwPageSize), SEEK_SET); // seek to the start of the page in the temporary file
			fread(&g_memory[frameToUse * g_dwPageSize], sizeof(char), g_dwPageSize, g_temporaryRomFile); // read in the page

			g_pageTable[pageNumberOfLocation] = frameToUse; // update the page table

			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation] * g_dwPageSize) + offsetFromPage);
		}
	}

	return retVal;
}

__forceinline __int32 __fastcall _MEM_XXX_ReadSWORDFromROM(uint32 location)
{
	// freakdave - new method of rom paging
	uint32 location2 = location & 0x7ffffff;
	uint8 numero = (location2 / g_pageTableSize);
	uint32 adresstemp = (location2 - ((numero) * (g_pageTableSize)));
	// see if it is on RAM or in the file
	if (indic[numero] == 0) {
		// adress of frame to change (FILO stack)
		adress[numero] = plusgrand;
		// change indication and adressfix
		indic[adfix[plusgrand]] = 0;
		indic[numero]=1;
		adfix[plusgrand]=numero;
		// copy
		fseek(g_temporaryRomFile, (numero * g_pageTableSize), SEEK_SET);
		fread((g_memory+(plusgrand * g_pageTableSize)), sizeof(*g_memory), (g_pageTableSize), g_temporaryRomFile);
		// action sur plusgrand - boucle décrémentation
		plusgrand--;
		if(plusgrand > (g_dwNumFrames-1)) plusgrand = (uint8)(g_dwNumFrames-1);
	//
	}
	// return information
	return *(__int32*) (g_memory + (adress[numero] * g_pageTableSize) + adresstemp);
}

BOOL __fastcall CheckIfInRom(uint32 location)
{
	//return (((addr)&0x1FFFFFFF) >= MEMORY_START_ROM_IMAGE && ((addr)&0x1FFFFFFF) <= 0x14000000);
	register uint32 locShifted = location >> SHIFTER2_READ;

	return ((locShifted >= 0x2400 && locShifted <= 0x24FF) \
		 || (locShifted >= 0x2C00 && locShifted <= 0x2CFF));
}

__int32 __fastcall ReadSWORDFromROM_EAXOnly(uint32 location)
{
	{
		__int32 temp;
		__asm
		{
			pushad
			mov ecx, location
			call ReadSWORDFromROM
			mov temp, eax
			popad
			mov eax, temp
		}
	}
}

BOOL EnableMenuItem(HMENU hMenu, UINT uIDEnableItem, UINT uEnable)
{
	return TRUE;
}

HMENU GetSubMenu(HMENU hMenu, int nPos)
{
	return (HMENU)1;
}

BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode)
{
	ExitThread(dwExitCode);
}

LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

BOOL SetMenuItemInfo(HMENU a, UINT b, BOOL c, LPMENUITEMINFO d)
{
	return TRUE;
}

BOOL PeekMessage(PMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	return TRUE;
}

BOOL MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	OutputDebugString(lpText);
	return 0;
}

int GetWindowText(HWND hWnd, LPTSTR lpString, int nMaxCount)
{
	return 1;
}

HMENU GetMenu(HWND hWnd)
{
	return 0;
}

BOOL SetWindowText(HWND hWnd, LPCTSTR lpString)
{
	return TRUE;
}

static __forceinline BOOL ByteSwap2(uint32 Size, uint8 *Image, uint8 bswaptype)
{
	/*~~~~~~*/
	uint32	k;
	/*~~~~~~*/

	switch (bswaptype)
	{
		case 1:
		{
			for(k = 0; k < Size; k += 4)
			{
				/* Converts 40123780 to 0x80371240 */
				_asm
				{
					mov edx, Image
					add edx, k
					mov eax, dword ptr[edx]
					bswap eax
					mov dword ptr[edx], eax
				}
			}

			return TRUE;
		}
		case 2:
		{
			/* Converts 0x12408037 to 0x80371240 */
			k = Size;
			_asm
			{
				mov edx, Image
				mov ebx, dword ptr[k]
				jmp _LABEL3
			}

			_LABEL2:
			_asm
			{
				sub ebx, 8
				test ebx, ebx
				jz _LABELEXIT
			}

			_LABEL3:
			_asm
			{
				/* Yup i copied this asm routine twice..to cut down on the looping by 50% */
				mov eax, dword ptr[edx]
				mov ecx, eax
				shr ecx, 0x10
				shl eax, 0x10
				or ecx, eax
				mov dword ptr[edx], ecx
				add edx, 4

				mov eax, dword ptr[edx]
				mov ecx, eax
				shr ecx, 0x10
				shl eax, 0x10
				or ecx, eax
				mov dword ptr[edx], ecx
				add edx, 4
				jmp _LABEL2
			}

			_LABELEXIT: ;
			return TRUE;
		}
		case 0:
			return TRUE;
		default:
			return FALSE;
	}
}

void CloseROM(void)
{

}

static __forceinline void SwapRomName(uint8 *name)
{
	/*~~~~~~*/
	int		i;
	uint8	c;
	/*~~~~~~*/

	for(i = 0; i < 20; i += 4)
	{
		c = name[i];
		name[i] = name[i + 3];
		name[i + 3] = c;

		c = name[i + 2];
		name[i + 2] = name[i + 1];
		name[i + 1] = c;
	}

	for(i = 19; i >= 0; i--)
	{
		if(name[i] != ' ') break;
	}

	name[i + 1] = '\0';

	/*
	 * for( ; i>=0; i-- ) £
	 * { £
	 * if( name[i] == ':' ) £
	 * name[i] = '-'; £
	 * }
	 */
}

static __forceinline void SwapRomHeader(uint8 *romheader)
{
	/*~~~~~~*/
	int		i;
	uint8	c;
	/*~~~~~~*/

	for(i = 0; i < 0x40; i += 4)
	{
		c = romheader[i];
		romheader[i] = romheader[i + 3];
		romheader[i + 3] = c;

		c = romheader[i + 2];
		romheader[i + 2] = romheader[i + 1];
		romheader[i + 1] = c;
	}
}



extern int SaveStateIndex;
extern CPU_ACTION CPU_Action;

void __EMU_SaveState(int index)
{
	SaveStateIndex = index;
	CPU_Action.DoSomething = TRUE;
	CPU_Action.SaveState = TRUE;
}

void __EMU_LoadState(int index)
{
	SaveStateIndex = index;
	CPU_Action.DoSomething = TRUE;
	CPU_Action.RestoreState = TRUE;
}

void __EMU_GetStateFilename(int index, char *filename, int mode)
{
	if(mode == 0){
		sprintf(filename, "%s%08X\\%08X-%08X-%02X.%i", g_szPathSaves, *((DWORD *)(RomHeader + 0x10)), *((DWORD *)(RomHeader + 0x10)), *((DWORD *)(RomHeader + 0x14)), *((BYTE *)(RomHeader + 0x3D)), index);
	}
	else if(mode == 1){
		sprintf(filename, "%s%08X\\%08X-%08X-%02X.%i.bmp", g_szPathSaves, *((DWORD *)(RomHeader + 0x10)), *((DWORD *)(RomHeader + 0x10)), *((DWORD *)(RomHeader + 0x14)), *((BYTE *)(RomHeader + 0x3D)), index);
	}
	return;
}

// GogoAckman - free up 4Mb for the menu
void __fastcall ReInitVirtualDynaMemory(boolean charge)
{

	// Ez0n3 - i don't think this should free up g_dwNumFrames (PJ64PagingMem) for menu - what if it's a 128MB sys and g_dwNumFrames = entire rom??!!
	// this is how much memory it should free up for the in-game menu to popup, doesn't seem right to me that it should try to free so much up for PJ64 and not 1964 if using 128MB
	// 1964 uses loaddw1964DynaMem default 8 MB for this setting, so this should be loaddwPJ64DynaMem value instead of g_dwNumFrames
	// g_dwNumFrames = loaddw1964PagingMem default 4MB on 64MB sys and entire rom on 128MB sys!!!! that's 20-60MB decommit for menu!!??
	// loaddw1964DynaMem default = 8
	// loaddwPJ64DynaMem default = 16
	// PJ64 dyna mem is twice as much (dumps dyna mem to file to free mem for menu, on menu exit - reloads dyna mem from file)
	// have no choice but to free up 16MB by default for the menu by using dwPJ64DynaMem, but i think it's better than the alternative
	// see what happens...
	
	FILE *fp;

	if (!charge) {
		if(XBOXRecompCode != NULL){
			fp=fopen("Z:\\codetemp.dat","wb");
			//fp=fopen("T:\\Data\\codetemp.dat","wb");
			fwrite(XBOXRecompCode,g_dwNormalCompileBufferSize ,sizeof(uint8),fp);
			VirtualFree(XBOXRecompCode, g_dwNormalCompileBufferSize, MEM_DECOMMIT);
		    fclose(fp);
		}
	}
	else {
		fp=fopen("Z:\\codetemp.dat","rb");
		//fp=fopen("T:\\Data\\codetemp.dat","rb");
		VirtualFree(XBOXRecompCode, g_dwNormalCompileBufferSize, MEM_DECOMMIT);
		(double *) XBOXRecompCode = (double *) VirtualAlloc(XBOXRecompCode, g_dwNormalCompileBufferSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        fread(XBOXRecompCode,sizeof(uint8),g_dwNormalCompileBufferSize,fp);
		fclose(fp);
		DeleteFile("Z:\\codetemp.dat");
		//DeleteFile("T:\\Data\\codetemp.dat");
	}
	
/*
	FILE *fp;

	if (!charge) {
		if(g_memory != NULL){
			//fp=fopen("Z:\\codetemp.dat","wb");
			fp=fopen("T:\\codetemp.dat","wb");
			fwrite(g_memory,RP_PAGE_SIZE * g_dwNumFrames ,sizeof(char),fp);
			free(g_memory);
		    fclose(fp);
			pause = 1;
            }
	}
	else {
		//fp=fopen("Z:\\codetemp.dat","rb");
		fp=fopen("T:\\codetemp.dat","rb");
       	g_memory = (uint8 *) malloc ( sizeof(*g_memory)  *  RP_PAGE_SIZE * nombreframes );
		fread(g_memory,sizeof(char),RP_PAGE_SIZE * g_dwNumFrames,fp);
		fclose(fp);
		//DeleteFile("Z:\\codetemp.dat");
		DeleteFile("T:\\codetemp.dat");
		pause = 0;
	}
*/
}
