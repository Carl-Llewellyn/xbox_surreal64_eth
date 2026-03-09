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

// Debugger.cpp : implementation file
//
#include "stdafx.h"
#include "_BldNum.h"


#ifdef _DEBUG

enum {
	LOG_DISABLE,
	LOG_TO_SCREEN,
	LOG_TO_FILE,
	LOG_TO_BOTH_SCREEN_AND_FILE,
};

char *logSettings[] =
{
	"Disable Log",
	"Log to Screen",
	"Log to File",
	"Log to Both",
};


bool debuggerWinOpened = FALSE;
bool debuggerDrawTexture = FALSE;
int debuggerDrawTextureNo = 0;
bool debuggerDrawTextureAlpha = false;

BOOL logUsedMux = FALSE;
BOOL logUnknownMux = TRUE;
BOOL logUnknownOpcode = TRUE;
BOOL logTriDetails = FALSE;
BOOL logTriDetailsWithVertexMtx = FALSE;
BOOL logTextureDetails = FALSE;
int  logSetting = LOG_TO_SCREEN;
FILE *logFp = NULL;

BOOL debuggerEnableTexture=TRUE;
BOOL debuggerEnableZBuffer=TRUE;
BOOL debuggerEnableCullFace=TRUE;
BOOL debuggerEnableTestTris=TRUE;
BOOL debuggerEnableAlphaTest=TRUE;
BOOL debuggerContinueWithUnknown=FALSE;
BOOL debuggerUpdateAtCIChange=FALSE;

bool debuggerPause = false;
bool pauseAtNext = false;
int  eventToPause = 0;
bool dlistDeassemble = false;
int  debuggerPauseCount = 0;
int  countToPause = 0;

bool debuggerDropCombiners=false;
bool debuggerDropDecodedMux=false;
bool debuggerDropCombinerInfos=false;

bool msgBufUpdated = false;
static HWND myDialogWnd = NULL;
extern HINSTANCE myhInst;
extern FiddledVtx * g_pVtxBase;
HWND hWndDlg = NULL;

bool debuggerWinActive = false;
CTexture *surfTlut = NULL;

u32 CachedTexIndex = 0;

char* otherNexts[] = {
	"DList",
	"Fake Frame Buffer",
	"FillRect",
	"Flush Tri",
	"Frame",
	"LoadTlut",
	"Matrix Cmd",
	"Mux",
	"ObjTxt Cmd",
	"Obj BG",
	"Set CImg",
	"Set Mode Cmd",
	"Set Prim Color",
	"Set Texture",
	"Sprite2D",
	"TextRect",
	"Texture Cmd",
	"Triangle",
	"Ucode",
	"Unknown Ops",
	"Vertex Cmd",
	"Scale Image",
	"New Texture",
	"Set Light",
	"Ucode Switching",
};
int numberOfNextOthers = sizeof(otherNexts)/sizeof(char*);

char* thingsToDump[] = {
	"Colors",
	"Content At",
	"Cur Mux",
	"Cur Texture",
	"Cur Texture to file",
	"Cur+1 Texture",
	"Cur+1 Texture to file",
	"Cur Texture Alpha",
	"Cur+1 Texture Alpha",
	"Cycle Type",
	"Frame Buffer",
	"Light Info",
	"Other Modes",
	"Simple Mux",
	"Texture0",
	"Texture1",
	"Texture2",
	"Texture3",
	"Texture4",
	"Texture5",
	"Texture6",
	"Texture7",
	"Tlut",
	"Obj Tlut",
	"Tile 0",
	"Tile 1",
	"Tile 2",
	"Tile 3",
	"Tile 4",
	"Tile 5",
	"Tile 6",
	"Tile 7",
	"Vertexes",
	"VI Regs",
	"Cached Texture",
	"Next Texture",
	"Prev Texture",
	"Dlist At",
	"Matrix At",
};
int numberOfThingsToDump = sizeof(thingsToDump)/sizeof(char*);

enum {
	DUMP_COLORS,
	DUMP_CONTENT_AT,
	DUMP_CUR_MUX,
	DUMP_CUR_TEXTURE,
	DUMP_CUR_TEXTURE_TO_FILE,
	DUMP_CUR_1_TEXTURE,
	DUMP_CUR_1_TEXTURE_TO_FILE,
	DUMP_CUR_TEXTURE_ALPHA,
	DUMP_CUR_1_TEXTURE_ALPHA,
	DUMP_CYCLE_TYPE,
	DUMP_FRAME_BUFFER,
	DUMP_LIGHT,
	DUMP_OTHER_MODE,
	DUMP_SIMPLE_MUX,
	DUMP_TEXTURE_0,
	DUMP_TEXTURE_1,
	DUMP_TEXTURE_2,
	DUMP_TEXTURE_3,
	DUMP_TEXTURE_4,
	DUMP_TEXTURE_5,
	DUMP_TEXTURE_6,
	DUMP_TEXTURE_7,
	DUMP_TLUT,
	DUMP_OBJ_TLUT,
	DUMP_TILE_0,
	DUMP_TILE_1,
	DUMP_TILE_2,
	DUMP_TILE_3,
	DUMP_TILE_4,
	DUMP_TILE_5,
	DUMP_TILE_6,
	DUMP_TILE_7,
	DUMP_VERTEXES,
	DUMP_VI_RGS,
	DUMP_CACHED_TEX,
	DUMP_NEXT_TEX,
	DUMP_PREV_TEX,
	DUMP_DLIST_AT,
	DUMP_MATRIX_AT,
};

//---------------------------------------------------------------------
void DumpVIRegisters(void)
{
	 
}

void DumpVertexArray(void)
{
	 
}

void DumpHex(DWORD rdramAddr, int count);
uint32 StrToHex(char *HexStr);

void DumpTileInfo(DWORD dwTile)
{
	 
}

void DumpTexture(int tex, bool alphaOnly=false)
{
	 
}

void DumpTextureToFile(int tex, bool alphaOnly=false)
{
	 
}

void DumpTlut(WORD* palAddr)
{
	 
}

extern LPCSTR g_szRDPInstrName_GBI2[256];
extern LPCSTR g_szRDPInstrName[256];
void DumpDlistAt(DWORD dwPC)
{
 
}

void DumpMatrixAt(DWORD dwPC)
{
	 
}

// High
 

void DumpOtherMode()
{
	 
}

void DumpCachedTexture(u32 index)
{
	 
}

extern uint32 gObjTlutAddr;
void DumpInfo(int thingToDump)
{
	 
}


void SetLogToFile(BOOL log)
{
 
}

//=====================================================================================
LRESULT APIENTRY DebuggerDialog(HWND hDlg, unsigned message, LONG wParam, LONG lParam)
{
 
	return(TRUE);
}

void OpenDialogBox()
{
	 
}

void CloseDialogBox()
{
	/*
	if( debuggerWinOpened )
	{
		DestroyWindow(hWndDlg);
		debuggerWinOpened = false;
	}
	*/
}


void __cdecl DebuggerAppendMsg(const char * Message, ...)
{
 
}


void DebuggerPause()
{
	 
}

void __cdecl DL_PF(LPCTSTR szFormat, ...)
{
	 
}

void DumpHex(DWORD rdramAddr, int count)
{
	 
}

uint32 StrToHex(char *HexStr)
{
	 
	return(0);
}

void DEBUGGER_PAUSE_COUNT_N(DWORD val)
{
	 
}

void DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(DWORD val)
{
	 
}

void DumpMatrix(const DaedalusMatrix &mat, const char* prompt)
{
 
}

#else
void __cdecl DebuggerAppendMsg(const char * Message, ...)
{
}

#endif
