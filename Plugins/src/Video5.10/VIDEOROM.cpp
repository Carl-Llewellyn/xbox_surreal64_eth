/*
Copyright (C) 2001 StrmnNrmn

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

static u32 ROM_CrcBootCode(u8 * pRomBase);
static void ROM_CheckSumMario();
static void ROM_CheckSumZelda();

RomInfo g_curRomInfo;

static BOOL ROM_ByteSwap(u8 * pBytes, DWORD dwLen);
static void ROM_ByteSwap_2301(void *v, DWORD numBytes);

inline DWORD SwapEndian(DWORD x)
{
	return ((x >> 24)&0x000000FF) |
		   ((x >> 8 )&0x0000FF00) |
		   ((x << 8 )&0x00FF0000) |
		   ((x << 24)&0xFF000000);
}


/*
-mario    = Mario-Type CIC Bootchip. (CIC-6102 or CIC-7101) 
-starf    = Starf-Type CIC Bootchip. (CIC-6101)
-lylat    = Lylat-Type CIC Bootchip. (CIC-7102)
-diddy    = Diddy-Type CIC Bootchip. (CIC-6103 or CIC-7103)
-yoshi    = Yoshi-Type CIC Bootchip. (CIC-6106 or CIC-7106)
-zelda    = Zelda-Type CIC Bootchip. (CIC-6105 or CIC-7105)
*/

#define MARIO_BOOT_CRC 0xb9c47dc8		// CIC-6102 or CIC-7101
#define STARF_BOOT_CRC 0xb4086651		// CIC-6101
#define LYLAT_BOOT_CRC 0xb3d6a525		// CIC-7102				// Lylat - Entyrpoint = 0x80000480
#define BANJO_BOOT_CRC 0xedce5ad9		// CIC-6103 or CIC-7103 // Diddy - Entrypoint + 0x00100000 
#define YOSHI_BOOT_CRC 0x06d8ed9c		// CIC-6106 or CIC-7106 // Yoshi - Entrypoint + 0x00200000 
#define ZELDA_BOOT_CRC 0xb53c588a		// CIC-6105 or CIC-7105

// Rom uses Unknown boot (0x51ae9f98)
// Rom uses Unknown boot (0x8dbba989)

#define MARIO_BOOT_CIC 0x3f
#define STARF_BOOT_CIC 0x3f // Incorrect
#define LYLAT_BOOT_CIC 0x3f // Incorrect
#define BANJO_BOOT_CIC 0x78
#define YOSHI_BOOT_CIC 0x85				// Same as FZero???
#define ZELDA_BOOT_CIC 0x91


BOOL ROM_ByteSwap(u8 * pBytes, DWORD dwLen)
{
	u32 nSig;

	nSig = *(u32*)pBytes;

	switch (nSig)
	{
	case 0x80371240:
		// Pre byteswapped - no need to do anything
		break;
	case 0x40123780:
		ROM_ByteSwap_3210(pBytes, dwLen);
		break;
	case 0x12408037:
		ROM_ByteSwap_2301(pBytes, dwLen);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}


// Swap bytes from 37 80 40 12
// to              40 12 37 80
void ROM_ByteSwap_2301(void *v, DWORD dwLen)
{
	__asm
	{
		mov		esi, v
		mov		edi, v
		mov		ecx, dwLen

		add		edi, ecx

	top:
		mov		al, byte ptr [esi + 0]
		mov		bl, byte ptr [esi + 1]
		mov		cl, byte ptr [esi + 2]
		mov		dl, byte ptr [esi + 3]

		mov		byte ptr [esi + 0], cl		//2
		mov		byte ptr [esi + 1], dl		//3
		mov		byte ptr [esi + 2], al		//0
		mov		byte ptr [esi + 3], bl		//1

		add		esi, 4
		cmp		esi, edi
		jne		top

	}

}


// Swap bytes from 80 37 12 40
// to              40 12 37 80
void ROM_ByteSwap_3210(void *v, DWORD dwLen)
{
	__asm
	{
		mov		esi, v
		mov		edi, v
		mov		ecx, dwLen

		add		edi, ecx

	top:
		mov		al, byte ptr [esi + 0]
		mov		bl, byte ptr [esi + 1]
		mov		cl, byte ptr [esi + 2]
		mov		dl, byte ptr [esi + 3]

		mov		byte ptr [esi + 0], dl		//3
		mov		byte ptr [esi + 1], cl		//2
		mov		byte ptr [esi + 2], bl		//1
		mov		byte ptr [esi + 3], al		//0

		add		esi, 4
		cmp		esi, edi
		jne		top

	}
}


void ROM_GetRomNameFromHeader(TCHAR * szName, ROMHeader * pHdr)
{
	TCHAR * p;

	memcpy(szName, pHdr->szName, 20);
	szName[20] = '\0';

	p = szName + (lstrlen(szName) -1);		// -1 to skip null
	while (p >= szName && *p == ' ')
	{
		*p = 0;
		p--;
	}
}


DWORD CountryCodeToTVSystem(DWORD countryCode)
{
	DWORD system;
	switch(countryCode)
	{
	/* Demo */
	case 0:
		system = TV_SYSTEM_NTSC;
		break;

	case '7':
		system = TV_SYSTEM_NTSC;
		break;

	case 0x41:
		system = TV_SYSTEM_NTSC;
		break;

	/* Germany */
	case 0x44:
		system = TV_SYSTEM_PAL;
		break;

	/* USA */
	case 0x45:
		system = TV_SYSTEM_NTSC;
		break;

	/* France */
	case 0x46:
		system = TV_SYSTEM_PAL;
		break;

	/* Italy */
	case 'I':
		system = TV_SYSTEM_PAL;
		break;

	/* Japan */
	case 0x4A:
		system = TV_SYSTEM_NTSC;
		break;

	/* Europe - PAL */
	case 0x50:
		system = TV_SYSTEM_PAL;
		break;

	case 'S':	/* Spain */
		system = TV_SYSTEM_PAL;
		break;

	/* Australia */
	case 0x55:
		system = TV_SYSTEM_PAL;
		break;

	case 0x58:
		system = TV_SYSTEM_PAL;
		break;

	/* Australia */
	case 0x59:
		system = TV_SYSTEM_PAL;
		break;

	case 0x20:
	case 0x21:
	case 0x38:
	case 0x70:
		system = TV_SYSTEM_PAL;
		break;

	/* ??? */
	default:
		system = TV_SYSTEM_PAL;
		break;
	}

	return system;
}
