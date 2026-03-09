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
 *
 * 1964 modified for rom paging and paging algorithms by 
 * oDD buttza@hotmail.com July 2003
 */

#include <xtl.h>
#include <stdio.h>
#include <malloc.h>

#include "globals.h"
#include "rompaging.h"

#include "win32/wingui.h"
#include "fileio.h"
#include "r4300i.h"
#include "n64rcp.h"

//weinerschnitzel - lets do this for rom paging now
// Ez0n3 - determine if the current phys ram is greater than 100MB
//extern int RAM_IS_128;
//extern BOOL PhysRam128(); //May need this here for clean paging

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
uint32	g_romSize;
uint32	g_pageTableSize;


// 1.0 VARS
#define PAGE_NOT_IN_MEMORY	(g_dwNumFrames+1)
#define FRAME_FREE			0xFFFFFFFF
#define NEVER				0x0
#define NONE_FOUND			0xFFFFFFFF
Frame *g_frameTable;
uint8 *g_pageTable;

// XXX VARS
static BOOL indic[256];
static uint8 adress[256];
static uint8 adfix[256];
static uint8 plusgrand;

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
	#ifdef USE_OBF
	_MEM_LINK_InitPageTables();
	#endif
	InitMemoryLookupTables();
	InitTLB();
	
	Is_Reading_Rom_File = FALSE;
	
	//strcpy(g_temporaryRomPath, "Z:\\TemporaryRom.dat");

	// open the temporary file for reading
	g_temporaryRomFile = fopen(g_temporaryRomPath, "rb");

	// get the size of the rom file
	rewind(g_temporaryRomFile);
	fseek(g_temporaryRomFile, 0, SEEK_END);
	g_romSize = ftell(g_temporaryRomFile);
	
	// read in the rom header
	fseek(g_temporaryRomFile, 0, SEEK_SET);
	fread((uint8 *) &rominfo.validation, sizeof(uint8), 0x40, g_temporaryRomFile);
	SwapRomHeader((uint8 *) &rominfo.validation);

	// read in boot code to SP_DMEM
	fseek(g_temporaryRomFile, 0, SEEK_SET);
	fread((uint8 *) &SP_DMEM, sizeof(uint8), 0x1000, g_temporaryRomFile);
	
	// read in rom name
	fseek(g_temporaryRomFile, 0x20, SEEK_SET);
	fread(rominfo.name, sizeof(uint8), 20, g_temporaryRomFile);
	SwapRomName(rominfo.name);
	
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
	
	g_pageTableSize = (g_romSize / g_dwPageSize); // get the size of the page table
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

	if (location > g_romSize)
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
	
	if (location > g_romSize)
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

	if (location > g_romSize)
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

__forceinline __int32 __fastcall _MEM_S10_ReadSWORDFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;
	uint32 retVal;

	g_pageFunctionHits++;
	location &= 0x7ffffff;

	if (location > g_romSize)
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

