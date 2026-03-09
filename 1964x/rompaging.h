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

#include "memory.h"

//#define RP_PAGE_SIZE		0x10000		// 64k pages
//#define RP_PAGE_SIZE		0x40000		// 64k pages

extern DWORD g_dwPageSize;
extern DWORD g_dwNumFrames;
extern int g_iPagingMethod;

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

//extern Frame *g_frameTable;
//extern uint8 *g_memory;

BOOL __fastcall InitVirtualRomData(char *rompath);
void __fastcall LoadVirtualRomData();
void __fastcall CloseVirtualRomData();

void __fastcall InitPageAndFrameTables();

uint32 __fastcall ReadUWORDFromROM(uint32 location);
uint16 __fastcall ReadUHALFFromROM(uint32 location);
uint8  __fastcall ReadUBYTEFromROM(uint32 location);

__int32 __fastcall ReadSWORDFromROM(uint32 location);

BOOL __fastcall CheckIfInRom(uint32 location);

static __forceinline __int32 ReadSWORDFromROM_EAXOnly(uint32 location)
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

extern char g_temporaryRomPath[260];

extern uint32 g_pageFunctionHits;
extern uint32 g_memFunctionHits;
extern uint32 g_pageHits;
extern uint32 g_pageMisses;
extern uint32 g_dynaHits;
