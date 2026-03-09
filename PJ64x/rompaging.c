
// NOT USED!!!

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

#include "main.h"
#include "cpu.h"
#include "memory.h"
#include "debugger.h"
#include "plugin.h"
#include "unzip.h"
#include "resource.h" 

#define uint8  unsigned __int8
#define uint16 unsigned __int16
#define uint32 unsigned __int32
#define uint64 unsigned __int64

#include "rompaging.h"
 

#define PAGE_SIZE		0x10000		// 64k pages
#define NO_FRAMES		128			// 64k*128 = 8mb of memory

#define PAGE_NOT_IN_MEMORY	(NO_FRAMES+1)
#define FRAME_FREE			0xFFFFFFFF
#define NEVER				0x0
#define NONE_FOUND			0xFFFFFFFF		

// Frames
// the 8meg memory block is made up of 128 frames, each able to hold 64k
// the frame table index is the same that index's memory, i wanted to keep
// memory sequential so i kept them seperate
// the frame table stores the page number of the page its storing in memory
// and the last time the page was accessed
typedef struct
{
	LARGE_INTEGER	lastUsed;
	uint32			pageNum;
} Frame;

Frame g_frameTable[NO_FRAMES];

// Memory
uint8 g_memory[PAGE_SIZE*NO_FRAMES];

// Page table
// is your standard page table. each entry in the array contains a number indicating
// the frame that stores its page in memory
uint8 *g_pageTable;
uint32 g_pageTableSize;

// Rom file
// the original rom file is loaded, byte swapped and written to a temporary file pointed
// to below. the file is open for the duration of play and seeked and read when needed.
FILE	*g_temporaryRomFile = NULL;
char    g_temporaryRomPath[260];
uint32	g_romSize;

BOOL InitVirtualRomData(char *rompath)
{
	FILE *originalFile = NULL;
	FILE *temporaryFile = NULL;
	char tempPath[_MAX_PATH];

	uint32  i;
	uint8	*bswapBuf;
	uint8	bswapType = 0;

	// check to see if the rom is zipped
	//if(stricmp(&rompath[strlen(rompath) - 4], ".zip") == 0)
	//{
	//	return InitVirtualZippedRomData(rompath);
	//}

	// open the rom
	originalFile = fopen(rompath, "rb");

	if(originalFile == NULL)
	{
		OutputDebugString("Error opening this file. Please refresh the ROM list.\n");
		return FALSE;
	}

	strcat(tempPath, "T:\\1964TemporaryRom.dat");

	// open the temporary file for writing
	temporaryFile = fopen(tempPath, "wb");

	if(temporaryFile == NULL)
	{
		OutputDebugString("Error creating a temporary rom file for paging. Please make sure you have write access to your temporary directory.\n");
		return FALSE;
	}

	// get the size of the original rom file
	rewind(originalFile);
	fseek(originalFile, 0, SEEK_END);
	g_romSize = ftell(originalFile);
	//gAllocationLength = g_romSize;

	// point back to the start of the file
	fseek(originalFile, 0, SEEK_SET);

	// start reading
	{
		InitMemoryLookupTables();
		SetupTLB();

		Is_Reading_Rom_File = TRUE;
		To_Stop_Reading_Rom_File = FALSE;

		// allocate 1 meg for bswapping and storing
		bswapBuf = (uint8 *)malloc(0x100000);

		// load the rom 1 meg at a time, byteswapping that 1meg and then storing it
		// in our temporary file
		for(i = 0; i < g_romSize && To_Stop_Reading_Rom_File == FALSE; i += 0x100000)
		{
			 
			if (To_Stop_Reading_Rom_File == TRUE)
			{
				CloseROM();
				To_Stop_Reading_Rom_File = FALSE;
				Is_Reading_Rom_File = FALSE;

				free(bswapBuf);

				fclose(originalFile);
				fclose(temporaryFile);
			
				return FALSE;
			}

			// seek to the required position and read in 1 meg
			fseek(originalFile, i, SEEK_SET);
			fread(bswapBuf, sizeof(uint8), 0x100000, originalFile);

			// check how the bytes in the rom are organised so we know what
			// to use to byte swap each meg
			if (i == 0)
			{
				if(bswapBuf[0] == 0x80 && bswapBuf[1] == 0x37)
				{
					bswapType = 1;
				}
				else if (bswapBuf[0] == 0x37 && bswapBuf[1] == 0x80)
				{
					bswapType = 2;
				}
				else if(bswapBuf[2] == 0x37 && bswapBuf[3] == 0x80)
				{
					bswapType = 0;
				}
				else
				{
					OutputDebugString("Not a rom file!\n");

					CloseROM();
					To_Stop_Reading_Rom_File = FALSE;
					Is_Reading_Rom_File = FALSE;

					free(bswapBuf);

					fclose(originalFile);
					fclose(temporaryFile);

					return FALSE;
				}
			}

			// byte swap the meg
			ByteSwap2(0x100000, bswapBuf, bswapType);

			// write it out to the temporary file
			fwrite(bswapBuf, sizeof(uint8), 0x100000, temporaryFile);
		}

		free(bswapBuf);

		Set_Ready_Message();
		Is_Reading_Rom_File = FALSE;
	}

	fclose(originalFile);
	fclose(temporaryFile);

	// open the temporary file again for reading
	temporaryFile = fopen(tempPath, "rb");
	
	// read in the rom header
	fseek(temporaryFile, 0, SEEK_SET);
	fread((uint8 *) &rominfo.validation, sizeof(uint8), 0x40, temporaryFile);
	SwapRomHeader((uint8 *) &rominfo.validation);

	// read in boot code to SP_DMEM
	fseek(temporaryFile, 0, SEEK_SET);
	fread((uint8 *) &SP_DMEM, sizeof(uint8), 0x1000, temporaryFile);
	
	// read in rom name
	fseek(temporaryFile, 0x20, SEEK_SET);
	fread(rominfo.name, sizeof(uint8), 20, temporaryFile);
	SwapRomName(rominfo.name);

	fclose(temporaryFile);

	strcpy(g_temporaryRomPath, tempPath);
	
	return TRUE;
}

void LoadVirtualRomData()
{
	g_temporaryRomFile = fopen(g_temporaryRomPath, "rb");
}

void CloseVirtualRomData()
{
	if (g_temporaryRomFile != NULL)
	{
		fclose(g_temporaryRomFile);
		g_temporaryRomFile = NULL;

		free(g_pageTable);
	}
}

void InitPageAndFrameTables()
{
	uint32 i;

	// get the size of the page table
	g_pageTableSize = g_romSize / PAGE_SIZE;

	// allocate memory for the page table
	g_pageTable = (uint8 *)malloc(g_pageTableSize);

	// initialize it
	for (i = 0; i < g_pageTableSize; i++)
	{
		g_pageTable[i] = PAGE_NOT_IN_MEMORY;
	}

	// initialize the frame table
	for (i = 0; i < NO_FRAMES; i++)
	{
		g_frameTable[i].pageNum				= FRAME_FREE;
		g_frameTable[i].lastUsed.QuadPart	= NEVER;
	}
}

uint32 ReadUWORDFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > g_romSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint32)))
		{
			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
}

uint16 ReadUHALFFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > g_romSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint16)))
		{
			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
}

uint8 ReadUBYTEFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > g_romSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint8)))
		{
			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
}

__int32 ReadSWORDFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;
	uint32 retVal;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > g_romSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint16)))
		{
			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}

	return retVal;
}

BOOL CheckIfInRom(uint32 location)
{
	//return (((addr)&0x1FFFFFFF) >= MEMORY_START_ROM_IMAGE && ((addr)&0x1FFFFFFF) <= 0x14000000);
	register uint32 locShifted = location >> SHIFTER2_READ;

	return ((locShifted >= 0x2400 && locShifted <= 0x24FF) \
		 || (locShifted >= 0x2C00 && locShifted <= 0x2CFF));
}

