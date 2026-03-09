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


// This file implements the S2DEX ucode, Yoshi story is using this ucodes

#include "Stdafx.h"

uObjTxtr *gObjTxtr;
uObjTxtrTLUT *gObjTlut;
uint32 gObjTlutAddr = 0;
uObjMtx *gObjMtx;
uObjSubMtx *gSubObjMtx;
uObjMtxReal gObjMtxReal = {1, 0, 0, 1, 0, 0, 0, 0};
DaedalusMatrix gD3DMtxReal(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
DaedalusMatrix gD3DObjOffset(1.0,0,0,0,  0,1.0,0,0, 0,0,0,1.0, 0,0,0,1.0);

u32 g_TxtLoadBy = CMD_LOAD_OBJ_TXTR;


// YoshiStory uses this - 0x02
void RDP_S2DEX_BG_COPY(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DP_Minimal16);
	DP_Timing(DP_Minimal16);

	DWORD dwAddr = RDPSegAddr(dwCmd1);
	uObjBg *sbgPtr = (uObjBg*)(g_pu8RamBase+dwAddr);
	CDaedalusRender::g_pRender->LoadObjBGCopy(*sbgPtr);
	CDaedalusRender::g_pRender->DrawObjBGCopy(*sbgPtr);
}

// YoshiStory uses this - 0x03
void RDP_S2DEX_OBJ_RECTANGLE(DWORD dwCmd0, DWORD dwCmd1)
{
	u32 dwAddress = RDPSegAddr(dwCmd1);
	uObjSprite *ptr = (uObjSprite*)(g_pu8RamBase+dwAddress);

	uObjTxSprite objtx;
	memcpy(&(objtx.txtr.block),&(gObjTxtr->block),sizeof(uObjTxtr));
	memcpy(&objtx.sprite,ptr,sizeof(uObjSprite));

	if( g_TxtLoadBy == CMD_LOAD_OBJ_TXTR )
		CDaedalusRender::g_pRender->LoadObjSprite(objtx,true);
	else
		SetupTextures();
	CDaedalusRender::g_pRender->DrawSprite(objtx, false);

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI)) || logTextureDetails )
	{	
		if( debuggerPauseCount > 0 ) 
			debuggerPauseCount--; 
		if( debuggerPauseCount == 0 )
		{
			eventToPause = false;
			debuggerPause = true;
			TRACE3("Paused at RDP_S2DEX_OBJ_RECTANGLE\nptr=%08X, img=%08X, Tmem=%08X",
				dwAddress,objtx.txtr.block.image, ptr->imageAdrs);
			CGraphicsContext::g_pGraphicsContext->SwapBuffer();
		}
	}
#endif
}

// YoshiStory uses this - 0x04
void RDP_S2DEX_OBJ_SPRITE(DWORD dwCmd0, DWORD dwCmd1)
{
	u32 dwAddress = RDPSegAddr(dwCmd1);
	uObjSprite *info = (uObjSprite*)(g_pu8RamBase+dwAddress);

	u32 dwTile	= gRSP.curTile;
	SetupTextures();
	
	//CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();

	uObjTxSprite drawinfo;
	memcpy( &(drawinfo.sprite), info, sizeof(uObjSprite));
	CDaedalusRender::g_pRender->DrawSpriteR(drawinfo, false, dwTile, 0, 0, drawinfo.sprite.imageW/32, drawinfo.sprite.imageH/32);


	/*
	static BOOL bWarned = FALSE;
	//if (!bWarned)
	{
		RDP_NOIMPL("RDP: RDP_S2DEX_OBJ_SPRITE (0x%08x 0x%08x)", dwCmd0, dwCmd1);
		bWarned = TRUE;
	}
	*/

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI)) || logTextureDetails )
	{	
		eventToPause = false;
		debuggerPause = true;
		TRACE0("Paused at RDP_S2DEX_OBJ_SPRITE");
		CGraphicsContext::g_pGraphicsContext->SwapBuffer();
	}
#endif
}

// YoshiStory uses this - 0xb0
void RDP_S2DEX_SELECT_DL(DWORD dwCmd0, DWORD dwCmd1)
{
	static BOOL bWarned = FALSE;
	//if (!bWarned)
	{
		RDP_NOIMPL("RDP: RDP_S2DEX_SELECT_DL (0x%08x 0x%08x)", dwCmd0, dwCmd1);
		bWarned = TRUE;
	}

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_OBJ_TXT_CMD, {DebuggerAppendMsg("Paused at RDP_S2DEX_SELECT_DL");});
}

void RDP_S2DEX_OBJ_RENDERMODE(DWORD dwCmd0, DWORD dwCmd1)
{
	/*
	static BOOL bWarned = FALSE;
	//if (!bWarned)
	{
	RDP_NOIMPL("RDP: RDP_S2DEX_OBJ_RENDERMODE (0x%08x 0x%08x)", dwCmd0, dwCmd1);
	bWarned = TRUE;
	}
	*/
}

// YoshiStory uses this - 0xb1
void DLParser_GBI1_Tri2(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_OBJ_RENDERMODE_2(DWORD dwCmd0, DWORD dwCmd1)
{
	if( (dwCmd0&0xFFFFFF) != 0 || (dwCmd1&0xFFFFFF00) != 0 )
	{
		// This is a TRI2 cmd
		DLParser_GBI1_Tri2(dwCmd0, dwCmd1);
		return;
	}

	RDP_S2DEX_OBJ_RENDERMODE(dwCmd0, dwCmd1);
}

#ifdef _DEBUG
void DumpBlockParameters(uObjTxtrBlock &ptr)
{
	/*
	typedef	struct	{	//Intel format
	  u32	type;		// G_OBJLT_TXTRBLOCK divided into types.                                
	  u64	*image;		// The texture source address on DRAM.       
  
	  u16	tsize;		// The Texture size.  Specified by the macro  GS_TB_TSIZE().            
	  u16	tmem;		// The  transferred TMEM word address.   (8byteWORD)  
  
	  u16	sid;		// STATE ID Multipled by 4.  Either one of  0,4,8 and 12.               
	  u16	tline;		// The width of the Texture 1-line. Specified by the macro GS_TB_TLINE()

	  u32	flag;		// STATE flag
	  u32	mask;		// STATE mask
	} uObjTxtrBlock;		// 24 bytes
	*/

	DebuggerAppendMsg("uObjTxtrBlock Header in RDRAM: 0x%08X", (uint32)&ptr-(uint32)g_pu8RamBase);
	DebuggerAppendMsg("ImgAddr=0x%08X(0x%08X), tsize=0x%X, \nTMEM=0x%X, sid=%d, tline=%d, flag=0x%X, mask=0x%X\n\n",
		RDPSegAddr(ptr.image), ptr.image, ptr.tsize, ptr.tmem, ptr.sid/4, ptr.tline, ptr.flag, ptr.mask);
}

void DumpSpriteParameters(uObjSprite &ptr)
{
	/*
	typedef struct {	// Intel format
	  u16  scaleW;		// Scaling of the u5.10 width direction.     
	  s16  objX;		// The x-coordinate of the upper-left end. s10.2 OBJ                
  
	  u16  paddingX;	// Unused.  Always 0.        
	  u16  imageW;		// The width of the u10.5 texture. (The length of the S-direction.) 
  
	  u16  scaleH;		// Scaling of the u5.10 height direction. 
	  s16  objY;		// The y-coordinate of the s10.2 OBJ upper-left end.                
  
	  u16  paddingY;	// Unused.  Always 0.              
	  u16  imageH;		// The height of the u10.5 texture. (The length of the T-direction.)
  
	  u16  imageAdrs;	// The texture header position in  TMEM.  (In units of 64bit word.)
	  u16  imageStride;	// The folding width of the texel.        (In units of 64bit word.) 

	  u8   imageFlags;	// The display flag.    G_OBJ_FLAG_FLIP*  
	  u8   imagePal;	//The pallet number.  0-7                        
	  u8   imageSiz;	// The size of the texel.         G_IM_SIZ_*       
	  u8   imageFmt;	// The format of the texel.   G_IM_FMT_*       
	} uObjSprite;		// 24 bytes 
	*/

	if( logTextureDetails || (pauseAtNext && eventToPause == NEXT_OBJ_TXT_CMD) )
	{
		DebuggerAppendMsg("uObjSprite Header in RDRAM: 0x%08X", (uint32)&ptr-(uint32)g_pu8RamBase);
		DebuggerAppendMsg("X=%d, Y=%d, W=%d, H=%d, scaleW=%f, scaleH=%f\n"
			"TAddr=0x%X, Stride=%d, Flag=0x%X, Pal=%d, Fmt=%s-%db\n\n", 
			ptr.objX/4, ptr.objY/4, ptr.imageW/32, ptr.imageH/32, ptr.scaleW/1024.0f, ptr.scaleH/1024.0f,
			ptr.imageAdrs, ptr.imageStride, ptr.imageFlags, ptr.imagePal, pszImgFormat[ptr.imageFmt], pnImgSize[ptr.imageSiz]);
	}
}

void DumpTileParameters(uObjTxtrTile &tile)
{
}

void DumpTlutParameters(uObjTxtrTLUT &tlut)
{
	/*
	typedef	struct	{	// Intel Format
	  u32	type;		// G_OBJLT_TLUT divided into types.                            
	  u32	image;
  
	  u16	pnum;		// The loading pallet number -1.   
	  u16	phead;		// The pallet number of the load header.  Between 256 and 511. 
  
	  u16	sid;		// STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.    
	  u16   zero;		// Assign 0 all the time.                                      
  
	  u32	flag;		// STATE flag  
	  u32	mask;		// STATE mask  
	} uObjTxtrTLUT;	// 24 bytes 
	*/
	DebuggerAppendMsg("ImgAddr=0x%08X(0x%08X), pnum=%d, phead=%d, sid=%d, flag=0x%X, mask=0x%X\n\n",
		RDPSegAddr(tlut.image), tlut.image, tlut.pnum+1, tlut.phead, tlut.sid/4, tlut.flag, tlut.mask);
}


void DumpTxtrInfo(uObjTxtr *ptr)
{
	if( logTextureDetails || (pauseAtNext && eventToPause == NEXT_OBJ_TXT_CMD) )
	{
		DebuggerAppendMsg("uObjTxtr Header in RDRAM: 0x%08X", (uint32)ptr-(uint32)g_pu8RamBase);
		switch( ptr->block.type )
		{
		case G_OBJLT_TXTRBLOCK:
			TRACE0("Loading ObjTxtr: type=BLOCK");
			DumpBlockParameters(ptr->block);
			break;
		case G_OBJLT_TXTRTILE:
			TRACE0("Loading ObjTxtr: type=TILE");
			DumpTileParameters(ptr->tile);
			break;
		case G_OBJLT_TLUT:
			TRACE0("Loading ObjTxtr: type=TLUT");
			DumpTlutParameters(ptr->tlut);
			break;
		}
	}
}

void DumpObjMtx(bool fullmtx = true)
{
	if( logTextureDetails || (pauseAtNext && eventToPause == NEXT_OBJ_TXT_CMD) )
	{
		if( fullmtx )
		DebuggerAppendMsg("A=%X, B=%X, C=%X, D=%X, X=%X, Y=%X, BaseX=%X, BaseY=%X",
			gObjMtx->A, gObjMtx->B, gObjMtx->C, gObjMtx->D, gObjMtx->X, gObjMtx->Y, gObjMtx->BaseScaleX, gObjMtx->BaseScaleY);
		else
			DebuggerAppendMsg("SubMatrix: X=%X, Y=%X, BaseX=%X, BaseY=%X", gSubObjMtx->X, gSubObjMtx->Y, gSubObjMtx->BaseScaleX, gSubObjMtx->BaseScaleY);
		
		DebuggerAppendMsg("A=%f, B=%f, C=%f, D=%f, X=%f, Y=%f, BaseX=%f, BaseY=%f",
			gObjMtxReal.A, gObjMtxReal.B, gObjMtxReal.C, gObjMtxReal.D, gObjMtxReal.X, gObjMtxReal.Y, gObjMtxReal.BaseScaleX, gObjMtxReal.BaseScaleY);
	}
}

#endif

void ObjMtxTranslate(float &x, float &y)
{
	float x1 = gObjMtxReal.A*x + gObjMtxReal.B*y + gObjMtxReal.X;
	float y1 = gObjMtxReal.C*x + gObjMtxReal.D*y + gObjMtxReal.Y;

	x = x1;
	y = y1;
}

void RDP_S2DEX_SPObjLoadTxtr(DWORD dwCmd0, DWORD dwCmd1)
{
	gObjTxtr = (uObjTxtr*)(g_pu8RamBase+(RDPSegAddr(dwCmd1)&(g_dwRamSize-1)));
	if( gObjTxtr->block.type == G_OBJLT_TLUT )
	{
		gObjTlut = (uObjTxtrTLUT*)gObjTxtr;
		gObjTlutAddr = (uint32)(RDPSegAddr(gObjTlut->image));
		
		// Copy tlut
		int size = gObjTlut->pnum+1;
		int offset = gObjTlut->phead-0x100;

		if( offset+size>0x100)
		{
			size = 0x100 - offset;
		}

		u32 addr = (gObjTlutAddr);//&0xFFFFFFFC);
		//if( addr & 3 ) addr = (addr&0xFFFFFFF0)+8;;
		WORD *srcPal = (WORD*)(g_pu8RamBase + (addr& (g_dwRamSize-1)) );

		for( int i=offset; i<offset+size; i++ )
		{
			//g_wRDPPal[i^1] = RDRAM_UHALF(addr);
			//addr += 2;
			//g_wRDPPal[i] = (*(u16 *)(addr+g_pu8RamBase));
			g_wRDPPal[i] = *(srcPal++);
		}
	}
	else
	{
		// Loading ObjSprite
		g_TxtLoadBy = CMD_LOAD_OBJ_TXTR;
	}

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
		{
			DumpTxtrInfo(gObjTxtr);
			TRACE0("Paused at RDP_S2DEX_SPObjLoadTxtr");
		}
	);
}

// YoshiStory uses this - 0xc2
void RDP_S2DEX_SPObjLoadTxSprite(DWORD dwCmd0, DWORD dwCmd1)
{
	uObjTxSprite* ptr = (uObjTxSprite*)(g_pu8RamBase+(RDPSegAddr(dwCmd1)&(g_dwRamSize-1)));
	gObjTxtr = (uObjTxtr*)ptr;
	
	//Now draw the sprite
	CDaedalusRender::g_pRender->LoadObjSprite(*ptr);
	CDaedalusRender::g_pRender->DrawSpriteR(*ptr);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
		{
			DumpTxtrInfo(gObjTxtr);
			DumpSpriteParameters(ptr->sprite);
			TRACE0("Paused at RDP_S2DEX_SPObjLoadTxSprite");
		}
	);
}


// YoshiStory uses this - 0xc3
void RDP_S2DEX_SPObjLoadTxRect(DWORD dwCmd0, DWORD dwCmd1)
{
	uObjTxSprite* ptr = (uObjTxSprite*)(g_pu8RamBase+(RDPSegAddr(dwCmd1)&(g_dwRamSize-1)));
	gObjTxtr = (uObjTxtr*)ptr;
	
	//Now draw the sprite
	CDaedalusRender::g_pRender->LoadObjSprite(*ptr);
	CDaedalusRender::g_pRender->DrawSprite(*ptr, false);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
		{
			DumpTxtrInfo(gObjTxtr);
			DumpSpriteParameters(ptr->sprite);
			TRACE0("Paused at RDP_S2DEX_SPObjLoadTxRect");
		}
	);
}

// YoshiStory uses this - 0xc4
void RDP_S2DEX_SPObjLoadTxRectR(DWORD dwCmd0, DWORD dwCmd1)
{
	uObjTxSprite* ptr = (uObjTxSprite*)(g_pu8RamBase+(RDPSegAddr(dwCmd1)&(g_dwRamSize-1)));
	gObjTxtr = (uObjTxtr*)ptr;
	
	//Now draw the sprite
	CDaedalusRender::g_pRender->LoadObjSprite(*ptr);
	CDaedalusRender::g_pRender->DrawSprite(*ptr, true);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
		{
			DumpTxtrInfo(gObjTxtr);
			DumpSpriteParameters(ptr->sprite);
			TRACE0("Paused at RDP_S2DEX_SPObjLoadTxRect");
		}
	);
}

extern DList	g_dwPCStack[MAX_DL_STACK_SIZE];
extern int		g_dwPCindex;
void DLParser_TexRect(DWORD dwCmd0, DWORD dwCmd1);
// YoshiStory uses this - 0xe4
void RDP_S2DEX_RDPHALF_0(DWORD dwCmd0, DWORD dwCmd1)
{
	//RDP: RDP_S2DEX_RDPHALF_0 (0xe449c0a8 0x003b40a4)
	//0x001d3c88: e449c0a8 003b40a4 G_TEXRECT 
	//0x001d3c90: b4000000 00000000 G_RDPHALF_1
	//0x001d3c98: b3000000 04000400 G_RDPHALF_2

	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	DWORD dwNextUcode = *(DWORD *)(g_pu8RamBase + dwPC);

	if( (dwNextUcode>>24) != G_SELECT_DL )
	{
		// Pokemom Puzzle League
		if( (dwNextUcode>>24) == 0xB4 )
		{
			DLParser_TexRect(dwCmd0, dwCmd1);
		}
		else
		{
			RDP_NOIMPL("RDP: RDP_S2DEX_RDPHALF_0 (0x%08x 0x%08x)", dwCmd0, dwCmd1);
		}
	}
	else
	{
		RDP_NOIMPL("RDP: RDP_S2DEX_RDPHALF_0 (0x%08x 0x%08x)", dwCmd0, dwCmd1);
		DEBUGGER_PAUSE_COUNT_N(NEXT_OBJ_TXT_CMD);
	}
}

/*
#define	GS_CALC_DXT(line)	(((1<< G_TX_DXT_FRAC)-1)/(line)+1)
#define	GS_PIX2TMEM(pix, siz)	((pix)>>(4-(siz)))
#define	GS_PIX2DXT(pix, siz)	GS_CALC_DXT(GS_PIX2TMEM((pix), (siz)))
#define	GS_TB_TSIZE(pix,siz)	(GS_PIX2TMEM((pix),(siz))-1)
#define	GS_TB_TLINE(pix,siz)	(GS_CALC_DXT(GS_PIX2TMEM((pix),(siz))))
u16	tsize;		// The Texture size.  Specified by the macro  GS_TB_TSIZE().            
u16	tline;		// The width of the Texture 1-line. Specified by the macro GS_TB_TLINE()
*/

// YoshiStory uses this - 0x05
void RDP_S2DEX_OBJ_MOVEMEM(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwCommand = (dwCmd0>>16)&0xFF;
	DWORD dwLength  = (dwCmd0)    &0xFFFF;
	DWORD dwAddress = RDPSegAddr(dwCmd1);

	if( dwAddress >= g_dwRamSize )
	{
		TRACE0("ObjMtx: memory ptr is invalid");
	}

	if( dwLength == 0 && dwCommand == 23 )
	{
		gObjMtx = (uObjMtx *)(dwAddress+g_pu8RamBase);
		gObjMtxReal.A = gObjMtx->A/65536.0f;
		gObjMtxReal.B = gObjMtx->B/65536.0f;
		gObjMtxReal.C = gObjMtx->C/65536.0f;
		gObjMtxReal.D = gObjMtx->D/65536.0f;
		gObjMtxReal.X = float(gObjMtx->X>>2);
		gObjMtxReal.Y = float(gObjMtx->Y>>2);
		gObjMtxReal.BaseScaleX = gObjMtx->BaseScaleX/1024.0f;
		gObjMtxReal.BaseScaleY = gObjMtx->BaseScaleY/1024.0f;

#ifdef _DEBUG
		DumpObjMtx();
#endif
	}
	else if( dwLength == 2 && dwCommand == 7 )
	{
		gSubObjMtx = (uObjSubMtx*)(dwAddress+g_pu8RamBase);
		gObjMtxReal.X = float(gSubObjMtx->X>>2);
		gObjMtxReal.Y = float(gSubObjMtx->Y>>2);
		gObjMtxReal.BaseScaleX = gSubObjMtx->BaseScaleX/1024.0f;
		gObjMtxReal.BaseScaleY = gSubObjMtx->BaseScaleY/1024.0f;

#ifdef _DEBUG
		DumpObjMtx(false);
#endif
	}

	gD3DMtxReal._11 = gObjMtxReal.A;
	gD3DMtxReal._12 = gObjMtxReal.C;
	gD3DMtxReal._13 = 0;
	gD3DMtxReal._14 = 0;//gObjMtxReal.X;

	gD3DMtxReal._21 = gObjMtxReal.B;
	gD3DMtxReal._22 = gObjMtxReal.D;
	gD3DMtxReal._23 = 0;
	gD3DMtxReal._24 = 0;//gObjMtxReal.Y;

	gD3DMtxReal._31 = 0;
	gD3DMtxReal._32 = 0;
	gD3DMtxReal._33 = 1.0;
	gD3DMtxReal._34 = 0;

	gD3DMtxReal._41 = gObjMtxReal.X;
	gD3DMtxReal._42 = gObjMtxReal.Y;
	gD3DMtxReal._43 = 0;
	gD3DMtxReal._44 = 1.0;

	DEBUGGER_PAUSE_COUNT_N(NEXT_OBJ_TXT_CMD);
}

extern bool g_bFakeCIUpdated;


// YoshiStory uses this - 0x01
extern void DLParser_GBI0_Mtx(DWORD dwCmd0, DWORD dwCmd1);
extern bool g_bFakeCIUpdated;

void RDP_S2DEX_BG_1CYC(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DP_Minimal16);
	DP_Timing(DP_Minimal16);

	DWORD dwAddr = RDPSegAddr(dwCmd1);
	uObjScaleBg *sbgPtr = (uObjScaleBg *)(dwAddr+g_pu8RamBase);
	CDaedalusRender::g_pRender->LoadObjBG1CYC(*sbgPtr);
	CDaedalusRender::g_pRender->DrawObjBG1CYC(*sbgPtr);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI||eventToPause == NEXT_OBJ_BG),
		{
			DebuggerAppendMsg("S2DEX BG 1CYC: %08X-%08X\n", dwCmd0, dwCmd1 );		
			TRACE0("Paused at RDP_S2DEX_BG_1CYC");
		}
	);
}

void RDP_S2DEX_BG_1CYC_2(DWORD dwCmd0, DWORD dwCmd1)
{
	if( (dwCmd0&0x00FFFFFF) != 0 )
	{
		DLParser_GBI0_Mtx(dwCmd0, dwCmd1);
		return;
	}

	RDP_S2DEX_BG_1CYC(dwCmd0, dwCmd1);
}


// YoshiStory uses this - 0xb2
void RDP_S2DEX_OBJ_RECTANGLE_R(DWORD dwCmd0, DWORD dwCmd1)
{
	u32 dwAddress = RDPSegAddr(dwCmd1);
	uObjSprite *ptr = (uObjSprite*)(g_pu8RamBase+dwAddress);

	uObjTxSprite objtx;
	memcpy(&(objtx.txtr.block),&(gObjTxtr->block),sizeof(uObjTxtr));
	memcpy(&objtx.sprite,ptr,sizeof(uObjSprite));


	//uObjTxSprite* ptr = (uObjTxSprite*)(g_pu8RamBase+(RDPSegAddr(dwCmd1)&(g_dwRamSize-1)));
	//gObjTxtr = (uObjTxtr*)ptr;
	
	//Now draw the sprite
	if( g_TxtLoadBy == CMD_LOAD_OBJ_TXTR )
		//CDaedalusRender::g_pRender->LoadObjSprite(*ptr,true);
		CDaedalusRender::g_pRender->LoadObjSprite(objtx,true);
	else
		SetupTextures();
	//CDaedalusRender::g_pRender->DrawSprite(*ptr, true);
	CDaedalusRender::g_pRender->DrawSprite(objtx, true);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI),
		{
			DumpTxtrInfo(gObjTxtr);
			DumpSpriteParameters(*ptr);
			TRACE0("Paused at RDP_S2DEX_OBJ_RECTANGLE_R");
		}
	);
}
