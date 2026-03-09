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

#include "stdafx.h"
#include "_BldNum.h"

#ifdef _DEBUG

void DumpMatrix2(const Matrix &mtx, const char* prompt);

bool debuggerWinOpened = FALSE;
bool debuggerDrawTexture = FALSE;
int debuggerDrawTextureNo = 0;
bool debuggerDrawTextureAlpha = false;

BOOL logCombiners = FALSE;
BOOL logUnknownUcode = TRUE;
BOOL logTriangles = FALSE;
BOOL logMatrix = FALSE;
BOOL logVertex = FALSE;
BOOL logTextures = FALSE;
BOOL logTextureBuffer = FALSE;
BOOL logToScreen = TRUE;
BOOL logToFile = FALSE;
BOOL logUcodes = false;
BOOL logMicrocode = false;

FILE *logFp = NULL;

BOOL debuggerEnableTexture=TRUE;
BOOL debuggerEnableZBuffer=TRUE;
BOOL debuggerEnableCullFace=TRUE;
BOOL debuggerEnableTestTris=TRUE;
BOOL debuggerEnableAlphaTest=TRUE;
BOOL debuggerContinueWithUnknown=FALSE;

bool debuggerPause = false;
bool pauseAtNext = false;
int  eventToPause = 0;
int  debuggerPauseCount = 0;
int  countToPause = 0;

bool debuggerDropCombiners=false;
bool debuggerDropDecodedMux=false;
bool debuggerDropCombinerInfos=false;

char msgBuf[0x20000+2];
bool msgBufUpdated = false;
static HWND myDialogWnd = NULL;
extern FiddledVtx * g_pVtxBase;
HWND hWndDlg = NULL;

bool debuggerWinActive = false;
CTexture *surfTlut = NULL;

DWORD CachedTexIndex = 0;

char* otherNexts[] = {
	"Flush Tri",
	"TextRect",
	"Frame",
	"Set CImg",
	"ObjTxt Cmd",
	"Obj BG",
	"Sprite2D",
	"FillRect",
	"Triangle",
	"DList",
	"Ucode",
	"Texture Buffer",
	"Matrix Cmd",
	"Vertex Cmd",
	"New Texture",
	"Set Texture",
	"Mux",
	"Set Light",
	"Set Mode Cmd",
	"Set Prim Color",
	"Texture Cmd",
	"Unknown Ops",
	"Scale Image",
	"LoadTlut",
	"Ucode Switching",
};
int numberOfNextOthers = sizeof(otherNexts)/sizeof(char*);

char* thingsToDump[] = {
	"Cur Texture",
	"Cur+1 Texture",
	"Colors",
	"Content At",
	"Mux",
	"Simple Mux",
	"Other Modes",
	"Texture0",
	"Texture1",
	"Tile 0",
	"Tile 1",
	"VI Regs",
	"Cur Txt to file",
	"Cur+1 Txt to file",
	"Cur Txt Alpha",
	"Cur+1 Txt Alpha",
	"Cycle Type",
	"Frame Buffer",
	"Light Info",
	"Texture2",
	"Texture3",
	"Texture4",
	"Texture5",
	"Texture6",
	"Texture7",
	"Tlut",
	"Obj Tlut",
	"Tile 2",
	"Tile 3",
	"Tile 4",
	"Tile 5",
	"Tile 6",
	"Tile 7",
	"Vertexes",
	"Cached Texture",
	"Next Texture",
	"Prev Texture",
	"Dlist At",
	"Matrix At",
	"Combined Matrix",
	"World Top Matrix",
	"Projection Matrix",
	"World Matrix 0",
	"World Matrix 1",
	"World Matrix 2",
	"World Matrix 3",
	"World Matrix 4",
	"BackBuffer",
	"TexBuffer",
	"TexBuffer 0",
	"TexBuffer 1",
	"TexBuffer 2",
	"TexBuffer 3",
	"TexBuffer 4",
	"TexBuffer 5",
	"TexBuffer 6",
	"TexBuffer 7",
	"TexBuffer 8",
	"TexBuffer 9",
	"TexBuffer 10",
	"TexBuffer 11",
	"TexBuffer 12",
	"TexBuffer 13",
	"TexBuffer 14",
	"TexBuffer 15",
	"TexBuffer 16",
	"TexBuffer 17",
	"TexBuffer 18",
	"TexBuffer 19",
};
int numberOfThingsToDump = sizeof(thingsToDump)/sizeof(char*);

enum {
	DUMP_CUR_TEXTURE,
	DUMP_CUR_1_TEXTURE,
	DUMP_COLORS,
	DUMP_CONTENT_AT,
	DUMP_CUR_MUX,
	DUMP_SIMPLE_MUX,
	DUMP_OTHER_MODE,
	DUMP_TEXTURE_0,
	DUMP_TEXTURE_1,
	DUMP_TILE_0,
	DUMP_TILE_1,
	DUMP_VI_REGS,
	DUMP_CUR_TEXTURE_TO_FILE,
	DUMP_CUR_1_TEXTURE_TO_FILE,
	DUMP_CUR_TEXTURE_ALPHA,
	DUMP_CUR_1_TEXTURE_ALPHA,
	DUMP_CYCLE_TYPE,
	DUMP_FRAME_BUFFER,
	DUMP_LIGHT,
	DUMP_TEXTURE_2,
	DUMP_TEXTURE_3,
	DUMP_TEXTURE_4,
	DUMP_TEXTURE_5,
	DUMP_TEXTURE_6,
	DUMP_TEXTURE_7,
	DUMP_TLUT,
	DUMP_OBJ_TLUT,
	DUMP_TILE_2,
	DUMP_TILE_3,
	DUMP_TILE_4,
	DUMP_TILE_5,
	DUMP_TILE_6,
	DUMP_TILE_7,
	DUMP_VERTEXES,
	DUMP_CACHED_TEX,
	DUMP_NEXT_TEX,
	DUMP_PREV_TEX,
	DUMP_DLIST_AT,
	DUMP_MATRIX_AT,
	DUMP_COMBINED_MATRIX,
	DUMP_WORLD_TOP_MATRIX,
	DUMP_PROJECTION_MATRIX,
	DUMP_WORLD_MATRIX_0,
	DUMP_WORLD_MATRIX_1,
	DUMP_WORLD_MATRIX_2,
	DUMP_WORLD_MATRIX_3,
	DUMP_WORLD_MATRIX_4,
	DUMP_BACKBUFFER,
	DUMP_TEXBUFFER,
	DUMP_TEXBUFFER_0,
	DUMP_TEXBUFFER_1,
	DUMP_TEXBUFFER_2,
	DUMP_TEXBUFFER_3,
	DUMP_TEXBUFFER_4,
	DUMP_TEXBUFFER_5,
	DUMP_TEXBUFFER_6,
	DUMP_TEXBUFFER_7,
	DUMP_TEXBUFFER_8,
	DUMP_TEXBUFFER_9,
	DUMP_TEXBUFFER_10,
	DUMP_TEXBUFFER_11,
	DUMP_TEXBUFFER_12,
	DUMP_TEXBUFFER_13,
	DUMP_TEXBUFFER_14,
	DUMP_TEXBUFFER_15,
	DUMP_TEXBUFFER_16,
	DUMP_TEXBUFFER_17,
	DUMP_TEXBUFFER_18,
	DUMP_TEXBUFFER_19,
};

//---------------------------------------------------------------------
void DumpVIRegisters(void)
{
	 
}

void DumpVertexArray(void)
{
 
}

void DumpHex(DWORD rdramAddr, int count);
DWORD StrToHex(char *HexStr);

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

void DumpDlistAt(DWORD dwPC)
{
 
}

void DumpMatrixAt(DWORD dwPC)
{
	 
}

// High
static const char *alphadithertypes[4]	= {"Pattern", "NotPattern", "Noise", "Disable"};
static const char *rgbdithertype[4]		= {"MagicSQ", "Bayer", "Noise", "Disable"};
static const char *convtype[8]			= {"Conv", "?", "?", "?",   "?", "FiltConv", "Filt", "?"};
static const char *filtertype[4]		= {"Point", "?", "Bilinear", "Average"};
static const char *cycletype[4]			= {"1Cycle", "2Cycle", "Copy", "Fill"};
static const char *detailtype[4]		= {"Clamp", "Sharpen", "Detail", "?"};
static const char *alphacomptype[4]		= {"None", "Threshold", "?", "Dither"};
static const char * szCvgDstMode[4]		= { "Clamp", "Wrap", "Full", "Save" };
static const char * szZMode[4]			= { "Opa", "Inter", "XLU", "Decal" };
static const char * szZSrcSel[2]		= { "Pixel", "Primitive" };
static const char * sc_szBlClr[4]		= { "In", "Mem", "Bl", "Fog" };
static const char * sc_szBlA1[4]		= { "AIn", "AFog", "AShade", "0" };
static const char * sc_szBlA2[4]		= { "1-A", "AMem", "1", "0" };

void DumpOtherMode()
{
	 
}

void DumpCachedTexture(DWORD index)
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

void __cdecl LOG_DL(LPCTSTR szFormat, ...)
{
	 
}

void DumpHex(DWORD rdramAddr, int count)
{
	 
}

DWORD StrToHex(char *HexStr)
{
	 
	return(0);
}

void DEBUGGER_PAUSE_COUNT_N(DWORD val)
{
	 
}

void DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(DWORD val)
{
 
}

void DumpMatrix2(const Matrix &mat, const char* prompt)
{
	 
}

void DumpMatrix(const Matrix &mat, const char* prompt)
{
	 
}

#else
void __cdecl DebuggerAppendMsg(const char * Message, ...)
{
}

#endif
