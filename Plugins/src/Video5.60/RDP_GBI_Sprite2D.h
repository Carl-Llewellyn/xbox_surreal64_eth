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
uint32 g_SavedUcode=1;
 
void RSP_GBI_Sprite2DBase(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	dwAddr &= (g_dwRamSize-1);

	//RSP_RDP_NOIMPL("RDP: Sprite2D (0x%08x 0x%08x)", word0, word1);

	g_Sprite2DInfo.spritePtr = (SpriteStruct *)(g_pRDRAMs8+dwAddr);

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, {DebuggerAppendMsg("Pause after Sprite2DBase: Addr=%08X\n", dwAddr);});
}

void RSP_GBI_Sprite2D_PuzzleMaster64(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	dwAddr &= (g_dwRamSize-1);

	g_Sprite2DInfo.spritePtr = (SpriteStruct *)(g_pRDRAMs8+dwAddr);
	g_Sprite2DInfo.scaleX = 1;
	g_Sprite2DInfo.scaleY = 1;

	g_Sprite2DInfo.flipX = 0;
	g_Sprite2DInfo.flipY = 0;
	g_Sprite2DInfo.px = 0;
	g_Sprite2DInfo.py = 0;

	CRender::g_pRender->DrawSprite2D(g_Sprite2DInfo, 1);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, {DebuggerAppendMsg("Pause after Sprite2DBase: Addr=%08X\n", dwAddr);});
}


void RSP_GBI1_Sprite2DDraw(uint32 word0, uint32 word1)
{
	// This ucode is shared by PopMtx and gSPSprite2DDraw
	g_Sprite2DInfo.px = (short)((word1>>16)&0xFFFF)/4;
	g_Sprite2DInfo.py = (short)(word1&0xFFFF)/4;

	//RSP_RDP_NOIMPL("gSPSprite2DDraw is not implemented", word0, word1);
	CRender::g_pRender->DrawSprite2D(g_Sprite2DInfo, 1);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, 
		{DebuggerAppendMsg("Pause after Sprite2DDraw at (%d, %d)\n", g_Sprite2DInfo.px, g_Sprite2DInfo.py);});

	LoadedUcodeMap[RSP_SPRITE2D_SCALEFLIP] = &RSP_GBI1_CullDL;
	LoadedUcodeMap[RSP_SPRITE2D_DRAW] = &RSP_GBI1_PopMtx;
	LoadedUcodeMap[RSP_SPRITE2D_BASE] = &RSP_GBI1_Sprite2DBase;

}

void RSP_GBI0_Sprite2DDraw(uint32 word0, uint32 word1)
{
	// This ucode is shared by PopMtx and gSPSprite2DDraw
	g_Sprite2DInfo.px = (short)((word1>>16)&0xFFFF)/4;
	g_Sprite2DInfo.py = (short)(word1&0xFFFF)/4;

	//RSP_RDP_NOIMPL("gSPSprite2DDraw is not implemented", word0, word1);
	CRender::g_pRender->DrawSprite2D(g_Sprite2DInfo, 0);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, {TRACE0("Pause after Sprite2DDraw\n");});
}


void RSP_GBI1_Sprite2DScaleFlip(uint32 word0, uint32 word1)
{
	g_Sprite2DInfo.scaleX = ((word1>>16)&0xFFFF)/1024.0f;
	g_Sprite2DInfo.scaleY = (word1&0xFFFF)/1024.0f;

	if( (word1&0xFFFF) < 0x100 )
	{
		g_Sprite2DInfo.scaleY = g_Sprite2DInfo.scaleX;
	}

	g_Sprite2DInfo.flipX = (BYTE)((word0>>8)&0xFF);
	g_Sprite2DInfo.flipY = (BYTE)(word0&0xFF);
	//RSP_RDP_NOIMPL("RSP_SPRITE2D_SCALEFLIP is not implemented", word0, word1);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SPRITE_2D, 
		{DebuggerAppendMsg("Pause after Sprite2DScaleFlip, Flip (%d,%d), Scale (%f, %f)\n", g_Sprite2DInfo.flipX, g_Sprite2DInfo.flipY,
			g_Sprite2DInfo.scaleX, g_Sprite2DInfo.scaleY);});
}



void RSP_GBI1_Sprite2DBase(uint32 word0, uint32 word1)
{
	if( !status.bUseModifiedUcodeMap )
	{
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
		status.bUseModifiedUcodeMap = true;
	}

	LoadedUcodeMap[RSP_SPRITE2D_BASE] = &RSP_GBI_Sprite2DBase;
	LoadedUcodeMap[RSP_SPRITE2D_SCALEFLIP] = &RSP_GBI1_Sprite2DScaleFlip;
	LoadedUcodeMap[RSP_SPRITE2D_DRAW] = &RSP_GBI1_Sprite2DDraw;

	TRACE0("Adding Sprite2D ucodes to ucode 1");
	RSP_GBI_Sprite2DBase(word0, word1);
}



void RSP_GBI0_Sprite2DBase(uint32 word0, uint32 word1)
{
	//Weired, this ucode 0 game is using ucode 1, but sprite2D cmd is working differently from
	//normal ucode1 sprite2D game

	TRACE0("Ucode 0 game is using Sprite2D, and using ucode 1 codes, create a new ucode for me");

	RSP_GBI_Sprite2DBase(word0, word1);
}

