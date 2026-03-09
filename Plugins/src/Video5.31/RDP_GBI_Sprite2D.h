/*
Copyright (C) 2002 Rice1964

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

// Sprite2D Ucodes


Sprite2DInfo g_Sprite2DInfo;
DWORD g_SavedUcode=1;
 
void DLParser_GBI_Sprite2DBase(DWORD dwCmd0, DWORD dwCmd1)
{
/*	Ucode 1 sprite2DBase ucode data
Pause after Sprite2DBase: Addr=003703D0
003703D0: 803742A0, 80380290, 00200020, 001E0200, 00000000, 00000000
003703E8: 803744C0, 80380180, 00200020, 001E0200, 00000000, 00000000
00370400: 803746E0, 80380070, 00200020, 001E0200, 00000000, 00000000
00370418: 80374900, 8037FF60, 00200020, 001E0200, 00000000, 00000000
00370430: 80374B20, 8037FE50, 00200020, 001E0200, 00000000, 00000000
00370448: 80374D40, 8037FD40, 00200020, 001E0200, 00000000, 00000000

Pause after Sprite2DScaleFlip
Pause after Sprite2DDraw
Pause after Sprite2DBase: Addr=003703E8
003703E8: 803744C0, 80380180, 00200020, 001E0200, 00000000, 00000000
00370400: 803746E0, 80380070, 00200020, 001E0200, 00000000, 00000000
00370418: 80374900, 8037FF60, 00200020, 001E0200, 00000000, 00000000
00370430: 80374B20, 8037FE50, 00200020, 001E0200, 00000000, 00000000
00370448: 80374D40, 8037FD40, 00200020, 001E0200, 00000000, 00000000
00370460: 80374F60, 8037FC30, 00200020, 001E0200, 00000000, 00000000


ucode 0 sprite2DBase ucode data	- NITRO64
Using the stride field differently
Pause after Sprite2DBase: Addr=00284620
00284620: 80121140, 80173D80, 02C00040, 01E00201, 00000000, 00000000
00284638: 80121180, 80173D80, 02C00100, 01E00201, 00000000, 00000000
00284650: 00000000, 00000000, 00000000, 00000000, 00000000, 00000000
00284668: 00000000, 00000000, 00000000, 00000000, 00000000, 00000000
00284680: 00000000, 00000000, 00000000, 00000000, 00000000, 00000000
00284698: 00000000, 00000000, 00000000, 00000000, 00000000, 00000000

Pause after Sprite2DBase: Addr=00282F28
00282F28: 80227960, 80225548, 0080000C, 001C0200, 008B0000, 00000000
00282F40: 80227960, 80225548, 00800006, 001C0200, 00580000, 00000000
00282F58: 80227960, 80225548, 0080000C, 001C0200, 008B0000, 00000000
00282F70: 80227960, 80225548, 0080000C, 001C0200, 00400000, 00000000
00282F88: 80227960, 80225548, 0080000C, 001C0200, 00BA0000, 00000000
00282FA0: 80227960, 80225548, 00800009, 001A0200, 004B0038, 00000000


*/
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	dwAddress &= (g_dwRamSize-1);

	//RDP_NOIMPL("RDP: Sprite2D (0x%08x 0x%08x)", dwCmd0, dwCmd1);

	g_Sprite2DInfo.spritePtr = (SpriteStruct *)(g_ps8RamBase+dwAddress);

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, {DebuggerAppendMsg("Pause after Sprite2DBase: Addr=%08X\n", dwAddress);});
}

void DLParser_GBI1_Sprite2DDraw(DWORD dwCmd0, DWORD dwCmd1)
{
	// This ucode is shared by PopMtx and gSPSprite2DDraw
	g_Sprite2DInfo.px = (s16)((dwCmd1>>16)&0xFFFF)/4;
	g_Sprite2DInfo.py = (s16)(dwCmd1&0xFFFF)/4;

	//RDP_NOIMPL("gSPSprite2DDraw is not implemented", dwCmd0, dwCmd1);
	CDaedalusRender::g_pRender->DrawSprite2D(g_Sprite2DInfo, 1);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, 
		{DebuggerAppendMsg("Pause after Sprite2DDraw at (%d, %d)\n", g_Sprite2DInfo.px, g_Sprite2DInfo.py);});

	LoadedUcodeMap[G_SPRITE2D_SCALEFLIP] = &DLParser_GBI1_CullDL;
	LoadedUcodeMap[G_SPRITE2D_DRAW] = &DLParser_GBI1_PopMtx;
	LoadedUcodeMap[G_SPRITE2D_BASE] = &DLParser_GBI1_Sprite2DBase;

}

void DLParser_GBI0_Sprite2DDraw(DWORD dwCmd0, DWORD dwCmd1)
{
	// This ucode is shared by PopMtx and gSPSprite2DDraw
	g_Sprite2DInfo.px = (s16)((dwCmd1>>16)&0xFFFF)/4;
	g_Sprite2DInfo.py = (s16)(dwCmd1&0xFFFF)/4;

	//RDP_NOIMPL("gSPSprite2DDraw is not implemented", dwCmd0, dwCmd1);
	CDaedalusRender::g_pRender->DrawSprite2D(g_Sprite2DInfo, 0);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, {TRACE0("Pause after Sprite2DDraw\n");});
}


void DLParser_GBI1_Sprite2DScaleFlip(DWORD dwCmd0, DWORD dwCmd1)
{
	g_Sprite2DInfo.scaleX = ((dwCmd1>>16)&0xFFFF)/1024.0f;
	g_Sprite2DInfo.scaleY = (dwCmd1&0xFFFF)/1024.0f;

	if( (dwCmd1&0xFFFF) < 0x100 )
	{
		g_Sprite2DInfo.scaleY = g_Sprite2DInfo.scaleX;
	}

	g_Sprite2DInfo.flipX = (BYTE)((dwCmd0>>8)&0xFF);
	g_Sprite2DInfo.flipY = (BYTE)(dwCmd0&0xFF);
	//RDP_NOIMPL("G_SPRITE2D_SCALEFLIP is not implemented", dwCmd0, dwCmd1);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, 
		{DebuggerAppendMsg("Pause after Sprite2DScaleFlip, Flip (%d,%d), Scale (%f, %f)\n", g_Sprite2DInfo.flipX, g_Sprite2DInfo.flipY,
			g_Sprite2DInfo.scaleX, g_Sprite2DInfo.scaleY);});
}



void DLParser_GBI1_Sprite2DBase(DWORD dwCmd0, DWORD dwCmd1)
{
	if( !status.useModifiedMap )
	{
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
		status.useModifiedMap = true;
	}

	LoadedUcodeMap[G_SPRITE2D_BASE] = &DLParser_GBI_Sprite2DBase;
	LoadedUcodeMap[G_SPRITE2D_SCALEFLIP] = &DLParser_GBI1_Sprite2DScaleFlip;
	LoadedUcodeMap[G_SPRITE2D_DRAW] = &DLParser_GBI1_Sprite2DDraw;
	/*
	status.useModifiedMap = true;
	memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
	
	g_SavedUcode = gRSP.ucode;
	*/
	TRACE0("Adding Sprite2D ucodes to ucode 1");
	DLParser_GBI_Sprite2DBase(dwCmd0, dwCmd1);
}



void DLParser_GBI0_Sprite2DBase(DWORD dwCmd0, DWORD dwCmd1)
{
	//Weired, this ucode 0 game is using ucode 1, but sprite2D cmd is working differently from
	//normal ucode1 sprite2D game

	TRACE0("Ucode 0 game is using Sprite2D, and using ucode 1 codes, create a new ucode for me");

	DLParser_GBI_Sprite2DBase(dwCmd0, dwCmd1);
}

