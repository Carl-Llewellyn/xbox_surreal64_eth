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

// This file should be only included in RDP_GRX.h, this is just a collection of related
// functions, not a global header file.

// ===========================================================================

// fix me, make me not depend on DirectX
#include "DXGraphicsContext.h"
#include "Render.h"
#include "D3DRender.h"

// 0 keeps the most recent CI info
// 1 keeps the frame buffer CI info which is being displayed now
// 2 keeps the older frame buffer CI info. This can be used if we are using triple buffer


RecentCIInfo g_RecentCIInfo[5];
RecentCIInfo *g_uRecentCIInfoPtrs[5] =
{
	&g_RecentCIInfo[0],
	&g_RecentCIInfo[1],
	&g_RecentCIInfo[2],
	&g_RecentCIInfo[3],
	&g_RecentCIInfo[4],
};

const numOfRecentCIInfos = 5;

RecentViOriginInfo g_RecentVIOriginInfo[numOfRecentCIInfos];
DWORD dwBackBufferSavedAtFrame=0;


// ===========================================================================
bool FrameBufferInRDRAMCheckCRC();

WORD ConvertRGBATo555(BYTE r, BYTE g, BYTE b, BYTE a)
{
	BYTE ar = a>=0x20?1:0;
	return ((r>>3)<<RGBA5551_RedShift) | ((g>>3)<<RGBA5551_GreenShift) | ((b>>3)<<RGBA5551_BlueShift) | ar;//(a>>7);
}

WORD ConvertRGBATo555(DWORD color32)
{
	return WORD((((color32>>19)&0x1F)<<RGBA5551_RedShift) | (((color32>>11)&0x1F)<<RGBA5551_GreenShift) | (((color32>>3)&0x1F)<<RGBA5551_BlueShift) | ((color32>>31)));;
}

void UpdateRecentCIAddr(SetImgInfo &ciinfo)
{
	if( ciinfo.dwAddr == g_uRecentCIInfoPtrs[0]->dwAddr )
		return;
	
	RecentCIInfo *temp;

	for( int i=1; i<numOfRecentCIInfos; i++ )
	{
		if( ciinfo.dwAddr == g_uRecentCIInfoPtrs[i]->dwAddr )
		{
			temp = g_uRecentCIInfoPtrs[i];

			for( int j=i; j>0; j-- )
			{
				g_uRecentCIInfoPtrs[j] = g_uRecentCIInfoPtrs[j-1];
			}
			break;
		}
	}

	if( i >= numOfRecentCIInfos )
	{
		temp = g_uRecentCIInfoPtrs[4];
		g_uRecentCIInfoPtrs[4] = g_uRecentCIInfoPtrs[3];
		g_uRecentCIInfoPtrs[3] = g_uRecentCIInfoPtrs[2];
		g_uRecentCIInfoPtrs[2] = g_uRecentCIInfoPtrs[1];
		g_uRecentCIInfoPtrs[1] = g_uRecentCIInfoPtrs[0];
		temp->dwCopiedAtFrame = 0;
		temp->bCopied = false;
	}

	g_uRecentCIInfoPtrs[0] = temp;

	// Fix me here for Mario Tennis
	temp->dwLastWidth = windowSetting.uViWidth;
	temp->dwLastHeight = windowSetting.uViHeight;

	temp->dwFormat = ciinfo.dwFormat;
	temp->dwAddr = ciinfo.dwAddr;
	temp->dwSize = ciinfo.dwSize;
	temp->dwWidth = ciinfo.dwWidth;
	temp->dwHeight = gRDP.scissor.bottom;
	temp->dwMemSize = (temp->dwWidth*temp->dwHeight/2)<<temp->dwSize;
	temp->bCopied = false;
	temp->lastUsedFrame = status.gDlistCount;
}


/************************************************************************/
/* Mark the ciinfo entry that the ciinfo is used by VI origin register  */
/* in another word, this is a real frame buffer, not a fake frame buffer*/
/* Fake frame buffers are never really used by VI origin				*/
/************************************************************************/
void SetAddrUsedByVIOrigin(DWORD addr)
{
	DWORD viwidth = *g_GraphicsInfo.VI_WIDTH_REG;
	addr &= (g_dwRamSize-1);
	for( int i=0; i<numOfRecentCIInfos; i++ )
	{
		if( g_uRecentCIInfoPtrs[i]->dwAddr+2*viwidth == addr )
		{
			g_uRecentCIInfoPtrs[i]->bUsedByVIAtFrame = status.gDlistCount;
		}
		else if( addr >= g_uRecentCIInfoPtrs[i]->dwAddr && addr < g_uRecentCIInfoPtrs[i]->dwAddr+0x1000 )
		{
			g_uRecentCIInfoPtrs[i]->bUsedByVIAtFrame = status.gDlistCount;
		}
	}

	for( i=0; i<numOfRecentCIInfos; i++ )
	{
		if( g_RecentVIOriginInfo[i].addr == addr )
		{
			g_RecentVIOriginInfo[i].FrameCount = status.gDlistCount;
			return;
		}
	}

	for( i=0; i<numOfRecentCIInfos; i++ )
	{
		if( g_RecentVIOriginInfo[i].addr == 0 )
		{
			// Never used
			g_RecentVIOriginInfo[i].addr = addr;
			g_RecentVIOriginInfo[i].FrameCount = status.gDlistCount;
			return;
		}
	}

	int index=0;
	DWORD minFrameCount = 0xffffffff;

	for( i=0; i<numOfRecentCIInfos; i++ )
	{
		if( g_RecentVIOriginInfo[i].FrameCount < minFrameCount )
		{
			index = i;
			minFrameCount = g_RecentVIOriginInfo[i].FrameCount;
		}
	}

	g_RecentVIOriginInfo[index].addr = addr;
	g_RecentVIOriginInfo[index].FrameCount = status.gDlistCount;
}

bool IsAddrUsedByVIorigin(DWORD addr, DWORD width)
{
	addr &= (g_dwRamSize-1);
	for( int i=0; i<numOfRecentCIInfos; i++ )
	{
		if( g_uRecentCIInfoPtrs[i]->dwAddr == 0 )
			continue;

		if( g_uRecentCIInfoPtrs[i]->dwAddr == addr )
		{
			if( status.gDlistCount-g_uRecentCIInfoPtrs[i]->bUsedByVIAtFrame < 20 )
			//if( g_uRecentCIInfoPtrs[i]->bUsedByVIAtFrame != 0 )
			{
				return true;
			}
			else
			{
				DEBUGGER_IF_DUMP((logTextureBuffer) ,{DebuggerAppendMsg("This is a new buffer address, the addr is never a displayed buffer");});
				return false;
			}
		}
	}

	for( i=0; i<numOfRecentCIInfos; i++ )
	{
		if( g_RecentVIOriginInfo[i].addr != 0 )
		{
			if( g_RecentVIOriginInfo[i].addr > addr && 
				(g_RecentVIOriginInfo[i].addr - addr)%width == 0 &&
				(g_RecentVIOriginInfo[i].addr - addr)/width <= 4)
			{
				if( status.gDlistCount-g_RecentVIOriginInfo[i].FrameCount < 20 )
				//if( g_RecentVIOriginInfo[i].FrameCount != 0 )
				{
					return true;
				}
				else
				{
				DEBUGGER_IF_DUMP((logTextureBuffer) ,{DebuggerAppendMsg("This is a new buffer address, the addr is never a displayed buffer");});
					return false;
				}
			}
		}
	}
	
	if( status.gDlistCount > 20 )
		return false;
	else
	{
		DEBUGGER_IF_DUMP((logTextureBuffer) ,{DebuggerAppendMsg("This is a new buffer address, the addr is never a displayed buffer");});
		return true;
	}
}

int FindRecentCIInfoIndex(DWORD addr)
{
	for( int i=0; i<numOfRecentCIInfos; i++ )
	{
		if( g_uRecentCIInfoPtrs[i]->dwAddr <= addr && addr < g_uRecentCIInfoPtrs[i]->dwAddr+g_uRecentCIInfoPtrs[i]->dwMemSize )
		{
			return i;
		}
	}
	return -1;
}

bool IsDIATextureBuffer()
{
	// Knowing g_CI and g_ZI

	//if( g_CI.dwWidth )

	bool foundSetScissor=false;
	bool foundFillRect=false;
	bool foundSetFillColor=false;
	bool foundSetCImg=false;
	bool foundTxtRect=false;
	int ucodeLength=10;
	int height;
	DWORD newFillColor;

	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	for( int i=0; i<10; i++ )
	{
		DWORD word0 = *(DWORD *)(g_pRDRAMu8 + dwPC + i*8);
		DWORD word1 = *(DWORD *)(g_pRDRAMu8 + dwPC + 4 + i*8);

		if( (word0>>24) == RDP_SETSCISSOR )
		{
			height   = ((word1>>0 )&0xFFF)/4;
			foundSetScissor = true;
			continue;
		}

		if( (word0>>24) == RDP_SETFILLCOLOR )
		{
			height   = ((word1>>0 )&0xFFF)/4;
			foundSetFillColor = true;
			newFillColor = word1;
			continue;
		}

		if( (word0>>24) == RDP_FILLRECT )
		{
			uint32 x0   = ((word1>>12)&0xFFF)/4;
			uint32 y0   = ((word1>>0 )&0xFFF)/4;
			uint32 x1   = ((word0>>12)&0xFFF)/4;
			uint32 y1   = ((word0>>0 )&0xFFF)/4;

			if( x0 == 0 && y0 == 0 )
			{
				if( x1 == g_CI.dwWidth )
				{
					height = y1;
					foundFillRect = true;
					continue;
				}

				if( x1 == g_CI.dwWidth - 1 )
				{
					height = y1+1;
					foundFillRect = true;
					continue;
				}
			}
		}	

		if( (word0>>24) == RDP_TEXRECT )
		{
			foundTxtRect = true;
			break;
		}

		if( (word0>>24) == RDP_SETCIMG )
		{
			foundSetCImg = true;
			break;
		}
	}

	/*
	bool foundSetScissor=false;
	bool foundFillRect=false;
	bool foundSetFillColor=false;
	bool foundSetCImg=false;
	bool foundTxtRect=false;
	int ucodeLength=10;
	DWORD newFillColor;
	*/

	if( foundFillRect )
	{
		if( foundSetFillColor )
		{
			if( newFillColor != 0xFFFCFFFC )
				return true;	// this is a texture buffer
			else
				return false;
		}

		if( gRDP.fillColor != 0x00FFFFF7 )
			return true;	// this is a texture buffer
		else
			return false;	// this is a normal ZImg
	}
	else
		return true;

	if( !foundSetCImg )
		return true;

	if( foundSetScissor )
		return true;
}

// Return -1 if the addr is not in recent frame buffer addr
// Return 1 if the addr is in the frame buffer which is currently being displayed
// Return 2 if the addr is in the 3rd frame buffer if we are using triple buffer
// this function will not return 0 which means the addr is within the current rendering frame buffer
//     this should not happen
extern BufferSettingInfo DirectXRenderBufferSettings[];
int IsAddrInRecentFrameBuffers(DWORD addr)
{
	if( addr >= g_uRecentCIInfoPtrs[1]->dwAddr && addr < g_uRecentCIInfoPtrs[1]->dwAddr+g_uRecentCIInfoPtrs[1]->dwMemSize )
		return 1;
	else if( DirectXRenderBufferSettings[options.RenderBufferSetting].number > 1 &&
		addr >= g_uRecentCIInfoPtrs[2]->dwAddr && addr < g_uRecentCIInfoPtrs[2]->dwAddr+g_uRecentCIInfoPtrs[1]->dwMemSize )
	{
		return 2;
	}
	else if( addr >= g_uRecentCIInfoPtrs[0]->dwAddr && addr < g_uRecentCIInfoPtrs[0]->dwAddr+g_uRecentCIInfoPtrs[0]->dwMemSize && status.bHandleN64TextureBuffer )
	{
		return 1;
	}
	{
		return -1;
	}
}

int CheckAndSaveBackBuffer(DWORD addr, DWORD memsize, bool copyToRDRAM = false);
int CheckAndSaveBackBuffer(DWORD addr, DWORD memsize, bool copyToRDRAM)
{
	if( CDeviceBuilder::GetGeneralDeviceType() != DIRECTX_DEVICE ) 
		return -1;

	int r = FindRecentCIInfoIndex(addr);
	if( r >= 0 && status.gDlistCount - g_uRecentCIInfoPtrs[r]->lastUsedFrame <= 3  && g_uRecentCIInfoPtrs[r]->bCopied == false )
	{
		DEBUGGER_IF_DUMP((logTextureBuffer&&r==1),TRACE2("Hit current front buffer at %08X, size=0x%X", addr, memsize));
		DEBUGGER_IF_DUMP((logTextureBuffer&&r==0),TRACE2("Hit current back buffer at %08X, size=0x%X", addr, memsize));
		DEBUGGER_IF_DUMP((logTextureBuffer&&r>=1),TRACE2("Hit old back buffer at %08X, size=0x%X", addr, memsize));

		CGraphicsContext::g_pGraphicsContext->SaveBackBuffer(r);
	}		

	return r;
}


BYTE CIFindIndex(WORD val)
{
	for( int i=0; i<=0xFF; i++ )
	{
		if( val == g_wRDPTlut[i] )
		{
			return (BYTE)i;
		}
	}
	return 0;
}



extern DWORD ComputeCImgHeight(SetImgInfo &info, DWORD &height);

DWORD dwOldCIAddr;
bool LastCIIsNewCI=false;
SetImgInfo g_LastCI = { TXT_FMT_RGBA, TXT_SIZE_16b, 1, 0 };
#define STORE_CI	{g_CI.dwAddr = dwNewAddr;g_CI.dwFormat = dwFmt;g_CI.dwSize = dwSiz;g_CI.dwWidth = dwWidth;g_CI.bpl=dwBpl;}
void DLParser_SetCImg(uint32 word0, uint32 word1)
{
	DWORD dwFmt		= (word0>>21)&0x7;
	DWORD dwSiz		= (word0>>19)&0x3;
	DWORD dwWidth	= (word0&0x0FFF) + 1;
	DWORD dwNewAddr = RSPSegmentAddr(word1) & 0x00FFFFFF;
	DWORD dwBpl		= dwWidth << dwSiz >> 1;

	{
		DEBUGGER_IF_DUMP((logTextureBuffer) ,
		{DebuggerAppendMsg("SetCImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
		dwNewAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);}
		);

		if( dwFmt == TXT_FMT_YUV || dwFmt == TXT_FMT_IA )
		{
			DebuggerAppendMsg("Check me:  SetCImg Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
		}

		LOG_DL("    Image: 0x%08x", RSPSegmentAddr(word1));
		LOG_DL("    Fmt: %s Size: %s Width: %d",
			pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
	}

	if( g_CI.dwAddr == dwNewAddr && g_CI.dwFormat == dwFmt && g_CI.dwSize == dwSiz && g_CI.dwWidth == dwWidth )
	{
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
			{
				TRACE0("Set CIMG to the same address, no change, skipped");
				DebuggerAppendMsg("Pause after SetCImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
					g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
			}
		);
		return;
	}

	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1ST_CI_CHANGE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->UpdateFrame();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st CI change");});
	}

	if( !frameBufferOptions.bUpdateCIInfo )
	{
		dwOldCIAddr = g_CI.dwAddr;
		STORE_CI;
		status.bCIBufferIsRendered = false;
		status.bN64IsDrawingTextureBuffer = false;

		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
		{
			DebuggerAppendMsg("Pause after SetCImg : Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
		}
		);
		return;
	}

	if( dwNewAddr == g_ZI.dwAddr )
	{
		DWORD height;
		if( ComputeCImgHeight(g_CI, height) != RDP_SETSCISSOR )
		{
			// This is a render-to-texture buffer
			dwOldCIAddr = g_CI.dwAddr;
			STORE_CI;
			//UpdateRecentCIAddr(g_CI);
			status.bCIBufferIsRendered = false;
			status.leftRendered = status.topRendered = status.rightRendered = status.bottomRendered = -1;

			DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
			{
				DebuggerAppendMsg("Pause after SetCImg == ZImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
					g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
			}
			);
			return;
		}

	}

	if( options.enableHackForGames == HACK_FOR_SUPER_BOWLING )
	{
		if( dwNewAddr%0x100 == 0 )
		{
			if( dwWidth < 320 )
			{
				// Left half screen
				gRDP.scissor.left = 0;
				gRDP.scissor.right = 160;
				CRender::g_pRender->SetViewport(0, 0, 160, 240, 0xFFFF);
				CRender::g_pRender->UpdateClipRectangle();
				CRender::g_pRender->UpdateScissor();
			}
			else
			{
				gRDP.scissor.left = 0;
				gRDP.scissor.right = 320;
				CRender::g_pRender->SetViewport(0, 0, 320, 240, 0xFFFF);
				CRender::g_pRender->UpdateClipRectangle();
				CRender::g_pRender->UpdateScissor();
			}
		}
		else
		{
			// right half screen
			gRDP.scissor.left = 160;
			gRDP.scissor.right = 320;
			gRSP.nVPLeftN = 160;
			gRSP.nVPRightN = 320;
			CRender::g_pRender->UpdateClipRectangle();
			CRender::g_pRender->UpdateScissor();
			CRender::g_pRender->SetViewport(160, 0, 320, 240, 0xFFFF);
		}
	}

	bool wasDrawingTextureBuffer = status.bN64IsDrawingTextureBuffer;
	status.bN64IsDrawingTextureBuffer = ( dwSiz != TXT_SIZE_16b || dwFmt != TXT_FMT_RGBA || dwWidth < 200 || ( !IsAddrUsedByVIorigin(dwNewAddr, dwWidth) && dwWidth != 512 && dwNewAddr != g_ZI.dwAddr) );
	status.bN64FrameBufferIsUsed = status.bN64IsDrawingTextureBuffer;

	if( !wasDrawingTextureBuffer && g_CI.dwAddr == g_ZI.dwAddr && status.bCIBufferIsRendered )
	{
		DEBUGGER_IF_DUMP((logTextureBuffer),TRACE0("ZI is rendered"));

		if( options.enableHackForGames != HACK_FOR_CONKER && g_uRecentCIInfoPtrs[0]->bCopied == false )
		{
			// Conker is not actually using a backbuffer
			UpdateRecentCIAddr(g_CI);
			if( status.leftRendered != -1 && status.topRendered != -1 && status.rightRendered != -1 && status.bottomRendered != -1 )
			{
				RECT rect={status.leftRendered,status.topRendered,status.rightRendered,status.bottomRendered};
				CGraphicsContext::g_pGraphicsContext->SaveBackBuffer(0,&rect);
			}
			else
			{
				CGraphicsContext::g_pGraphicsContext->SaveBackBuffer(0,NULL);
			}
		}
	}

	frameBufferOptions.bFillRectNextTextureBuffer = false;
	if( g_CI.dwAddr == dwNewAddr && status.bHandleN64TextureBuffer && (g_CI.dwFormat != dwFmt || g_CI.dwSize != dwSiz || g_CI.dwWidth != dwWidth ) )
	{
		// Mario Tennis player shadow
		CGraphicsContext::g_pGraphicsContext->CloseTextureBuffer(true);
		if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS )
			frameBufferOptions.bFillRectNextTextureBuffer = true;	// Hack for Mario Tennis
	}

	dwOldCIAddr = g_CI.dwAddr;
	STORE_CI;
	memcpy(&g_LastCI, &g_CI, sizeof(g_CI));

	if( g_CI.dwAddr == g_ZI.dwAddr && !status.bN64IsDrawingTextureBuffer )
	{
		if( IsDIATextureBuffer() )
		{
			status.bN64IsDrawingTextureBuffer = true;
			status.bN64FrameBufferIsUsed = status.bN64IsDrawingTextureBuffer;
		}
	}

	status.bCIBufferIsRendered = false;
	status.leftRendered = status.topRendered = status.rightRendered = status.bottomRendered = -1;

	if( currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_CI_CHANGE && !status.bN64IsDrawingTextureBuffer )
	{
		if( status.curRenderBuffer == NULL )
		{
			status.curRenderBuffer = g_CI.dwAddr;
		}
		else if( status.curRenderBuffer != g_CI.dwAddr )
		{
			status.curDisplayBuffer = status.curRenderBuffer;
			CGraphicsContext::Get()->UpdateFrame();
			status.curRenderBuffer = g_CI.dwAddr;
			DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Screen Update because CI change to %08X, Display Buf=%08X", status.curRenderBuffer, status.curDisplayBuffer);});
		}
	}

	if( frameBufferOptions.bAtEachFrameUpdate && !status.bHandleN64TextureBuffer )
	{
		if( status.curRenderBuffer != g_CI.dwAddr )
		{
			if( status.gDlistCount%(currentRomOptions.N64FrameBufferWriteBackControl+1) == 0 )
			{
				CDXGraphicsContext::g_pGraphicsContext->CopyBackToRDRAM(status.curRenderBuffer, 
					dwFmt, dwSiz, windowSetting.uViWidth, windowSetting.uViHeight,
					windowSetting.uDisplayWidth, windowSetting.uDisplayHeight);
			}
		}

		//status.curDisplayBuffer = status.curRenderBuffer;
		status.curRenderBuffer = g_CI.dwAddr;
	}


	switch( currentRomOptions.N64RenderToTextureEmuType )
	{
	case TXT_BUF_NONE:
		if( status.bHandleN64TextureBuffer )
			CGraphicsContext::g_pGraphicsContext->CloseTextureBuffer(false);
		status.bHandleN64TextureBuffer = false;	// Don't handle N64 texture buffer stuffs
		if( !status.bN64IsDrawingTextureBuffer )
			UpdateRecentCIAddr(g_CI);
		break;
	default:
		if( status.bHandleN64TextureBuffer )
		{
#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_TEXTURE_BUFFER )
			{
				pauseAtNext = TRUE;
				eventToPause = NEXT_TEXTURE_BUFFER;
			}
#endif
			CGraphicsContext::g_pGraphicsContext->CloseTextureBuffer(true);
		}

		status.bHandleN64TextureBuffer = status.bN64IsDrawingTextureBuffer;
		if( status.bHandleN64TextureBuffer )
		{
			if( options.enableHackForGames != HACK_FOR_BANJO_TOOIE )
			{
				CGraphicsContext::g_pGraphicsContext->SetTextureBuffer(g_CI);
			}
		}
		else
		{
#ifdef _DEBUG
			if( g_CI.dwWidth == 512 && pauseAtNext && (eventToPause==NEXT_OBJ_BG || eventToPause==NEXT_SET_CIMG) )
			{
				DebuggerAppendMsg("Warning SetCImg: new Addr=0x%08X, fmt:%s size=%sb, Width=%d\n", 
					g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
			}
#endif
			UpdateRecentCIAddr(g_CI);
			CGraphicsContext::g_pGraphicsContext->CheckTxtrBufsWithNewCI(g_CI,g_uRecentCIInfoPtrs[0]->dwHeight,false);
		}
		break;
	}

	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
	{
		DebuggerAppendMsg("Pause after SetCImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
			g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
	}
	);
}


void TexRectToFrameBuffer_8b(DWORD dwXL, DWORD dwYL, DWORD dwXH, DWORD dwYH, float t0u0, float t0v0, float t0u1, float t0v1, DWORD dwTile)
{
	// Copy the texture into the N64 framebuffer memory
	// Used in Yoshi

	/*
	DWORD maxW = g_pTextureBufferInfo->CI_Info.dwWidth;
	DWORD maxH = maxW*3/4;
	if( status.dwTvSystem == TV_SYSTEM_PAL )
	{
		maxH = maxW*9/11;
	}
	*/

	DWORD maxW = g_pTextureBufferInfo->N64Width;
	DWORD maxH = g_pTextureBufferInfo->N64Height;

	DWORD maxOff = maxW*maxH;
	
	TMEMLoadMapInfo &info = g_tmemLoadAddrMap[gRDP.tiles[dwTile].dwTMem];
	DWORD dwWidth = dwXH-dwXL;
	DWORD dwHeight = dwYH-dwYL;

	float xScale = (t0u1-t0u0)/dwWidth;
	float yScale = (t0v1-t0v0)/dwHeight;

	BYTE* dwSrc = g_pRDRAMu8 + info.dwLoadAddress;
	BYTE* dwDst = g_pRDRAMu8 + g_pTextureBufferInfo->CI_Info.dwAddr;

	DWORD dwSrcPitch = gRDP.tiles[dwTile].dwPitch;
	DWORD dwDstPitch = g_pTextureBufferInfo->CI_Info.dwWidth;

	DWORD dwSrcOffX = gRDP.tiles[dwTile].hilite_sl;
	DWORD dwSrcOffY = gRDP.tiles[dwTile].hilite_tl;

	DWORD dwLeft = dwXL;
	DWORD dwTop = dwYL;

	dwWidth = min(dwWidth,maxW-dwLeft);
	dwHeight = min(dwHeight, maxH-dwTop);
	if( maxH <= dwTop )	return;

	for (DWORD y = 0; y < dwHeight; y++)
	{
		DWORD dwByteOffset = (DWORD)(((y*yScale+dwSrcOffY) * dwSrcPitch) + dwSrcOffX);
		
		for (DWORD x = 0; x < dwWidth; x++)
		{
			if( (((y+dwTop)*dwDstPitch+x+dwLeft)^0x3) > maxOff )
			{
#ifdef _DEBUG
				TRACE0("Warning: Offset exceeds limit");
#endif
				continue;
			}
			dwDst[((y+dwTop)*dwDstPitch+x+dwLeft)^0x3] = dwSrc[(DWORD)(dwByteOffset+x*xScale) ^ 0x3];
		}
	}
	
	DEBUGGER_IF_DUMP((logTextureBuffer),
		{
			DebuggerAppendMsg("TexRect To FrameBuffer: X0=%d, Y0=%d, X1=%d, Y1=%d,\n\t\tfS0=%f, fT0=%f, fS1=%f, fT1=%f ",
			dwXL, dwYL, dwXH, dwYH, t0v0, t0v0, t0u1, t0v1);
		}
	);
}

void TexRectToN64FrameBuffer_16b(DWORD x0, DWORD y0, DWORD width, DWORD height, DWORD dwTile)
{
	// Copy the texture into the N64 RDRAM framebuffer memory

	DrawInfo srcInfo;	
	if( g_textures[dwTile].m_pCTexture->StartUpdate(&srcInfo) == false )
	{
		DebuggerAppendMsg("Fail to lock texture:TexRectToN64FrameBuffer_16b" );
		return;
	}

	DWORD n64CIaddr = g_CI.dwAddr;
	DWORD n64CIwidth = g_CI.dwWidth;

	for (DWORD y = 0; y < height; y++)
	{
		DWORD* pSrc = (DWORD*)((BYTE*)srcInfo.lpSurface + y * srcInfo.lPitch);
		WORD* pN64Buffer = (WORD*)(g_pRDRAMu8+(n64CIaddr&(g_dwRamSize-1)))+(y+y0)*n64CIwidth;
		
		for (DWORD x = 0; x < width; x++)
		{
			pN64Buffer[x+x0] = ConvertRGBATo555(pSrc[x]);
		}
	}

	g_textures[dwTile].m_pCTexture->EndUpdate(&srcInfo);
}

/*
DWORD CalculateRDRAMCRC(DWORD addr, DWORD width, DWORD height, DWORD size, DWORD pitch)
{
	DWORD crc=0;

	DWORD len = height*pitch*size;
	if( size == TXT_SIZE_4b ) len = (height*pitch)>>1;

	len >>= 2;

	DWORD *frameBufferBase = (DWORD*)(g_pRDRAMu8+addr);


	for( DWORD i=0; i<len; i++ )
	{
		crc += frameBufferBase[i];
	}

	return crc;
}
*/

#define FAST_CRC_CHECKING_INC_X	13
#define FAST_CRC_CHECKING_INC_Y	11
#define FAST_CRC_MIN_Y_INC		2
#define FAST_CRC_MIN_X_INC		2
#define FAST_CRC_MAX_X_INC		7
#define FAST_CRC_MAX_Y_INC		3
extern DWORD dwAsmHeight;
extern DWORD dwAsmPitch;
extern DWORD dwAsmdwBytesPerLine;
extern DWORD dwAsmCRC;
extern BYTE* pAsmStart;

DWORD CalculateRDRAMCRC(void *pPhysicalAddress, DWORD left, DWORD top, DWORD width, DWORD height, DWORD size, DWORD pitchInBytes )
{
	dwAsmCRC = 0;
	dwAsmdwBytesPerLine = ((width<<size)+1)/2;

	if( currentRomOptions.bFastTexCRC && (height>=32 || (dwAsmdwBytesPerLine>>2)>=16))
	{
		DWORD realWidthInDWORD = dwAsmdwBytesPerLine>>2;
		DWORD xinc = realWidthInDWORD / FAST_CRC_CHECKING_INC_X;	
		if( xinc < FAST_CRC_MIN_X_INC )
		{
			xinc = min(FAST_CRC_MIN_X_INC, width);
		}
		if( xinc > FAST_CRC_MAX_X_INC )
		{
			xinc = FAST_CRC_MAX_X_INC;
		}

		DWORD yinc = height / FAST_CRC_CHECKING_INC_Y;	
		if( yinc < FAST_CRC_MIN_Y_INC ) 
		{
			yinc = min(FAST_CRC_MIN_Y_INC, height);
		}
		if( yinc > FAST_CRC_MAX_Y_INC )
		{
			yinc = FAST_CRC_MAX_Y_INC;
		}

		DWORD pitch = pitchInBytes>>2;
		register DWORD *pStart = (DWORD*)(pPhysicalAddress);
		pStart += (top * pitch) + (((left<<size)+1)>>3);

		/*
		DWORD x,y;
		for (y = 0; y < height; y+=yinc)		// Do every nth line?
		{
		for (x = 0; x < realWidthInDWORD; x+=xinc)
		{
		dwAsmCRC += *(pStart+x);
		dwAsmCRC ^= x;
		}
		pStart += pitch;
		dwAsmCRC ^= y;
		}
		*/


		__asm
		{
			push	esi;
			mov		esi, DWORD PTR [xinc]; 
			mov		ebx, DWORD PTR [pStart];
			mov		eax,0;	// EAX = the CRC
			mov		edx,0x0;
loop1:
			cmp		edx, height;
			jae		endloop1;
			mov		ecx, 0x0;
loop2:
			add		eax, ecx;
			cmp		ecx, DWORD PTR [realWidthInDWORD]
			jae		endloop2;

			add		eax, DWORD PTR [ebx][ecx*4];

			add		ecx, esi;
			jmp		loop2;
endloop2:
			xor		eax, edx;
			add		edx, DWORD PTR [yinc];
			add		ebx, DWORD PTR [pitch];
			jmp		loop1;
endloop1:
			mov		DWORD PTR [dwAsmCRC], eax;
			pop		esi;
		}
	}
	else
	{
		try{
			dwAsmdwBytesPerLine = ((width<<size)+1)/2;

			pAsmStart = (BYTE*)(pPhysicalAddress);
			pAsmStart += (top * pitchInBytes) + (((left<<size)+1)>>1);

			dwAsmHeight = height - 1;
			dwAsmPitch = pitchInBytes - 1;
			__asm 
			{
				push eax
					push ebx
					push ecx
					push edx
					push esi

					mov  ecx, pAsmStart;	// = pStart
				mov  edx, 0			// The CRC
					mov  eax, dwAsmHeight	// = y
l2:	mov	 ebx, dwAsmdwBytesPerLine	// = x
l1:	mov esi, [ecx+ebx]
				xor esi, ebx
					xor esi, eax
					add edx, esi
					sub	ebx, 4
					jge l1
					add ecx, dwAsmPitch
					dec eax
					jge l2

					mov	dwAsmCRC, edx

					pop esi
					pop edx
					pop ecx
					pop ebx
					pop	eax
			}

			/*
			DWORD x,y;
			for (y = 0; y < height; y++)		// Do every nth line?
			{
			// Byte fiddling won't work, but this probably doesn't matter
			// Now process 4 bytes at a time
			for (x = 0; x < dwBytesPerLine; x+=4)
			{
			dwCRC += (((*(DWORD*)(pStart+x))^y)^x);
			//dwCRC += x;
			}
			pStart += pitchInBytes;
			}
			*/
		}
		catch(...)
		{
			TRACE0("Exception in texture CRC calculation");
		}
	}
	return dwAsmCRC;
}


bool FrameBufferInRDRAMCheckCRC()
{
	RecentCIInfo &p = *(g_uRecentCIInfoPtrs[0]);
	BYTE *pFrameBufferBase = (BYTE*)(g_pRDRAMu8+p.dwAddr);
	DWORD pitch = (p.dwWidth << p.dwSize ) >> 1;
	DWORD crc = CalculateRDRAMCRC(pFrameBufferBase, 0, 0, p.dwWidth, p.dwHeight, p.dwSize, pitch);
	if( crc != p.dwCRC )
	{
		p.dwCRC = crc;
		TRACE0("Frame Buffer CRC mismitch, it is modified by CPU");
		return false;
	}
	else
	{
		return true;
	}
}

extern std::vector<DWORD> frameWriteRecord;
void FrameBufferWriteByCPU(DWORD addr, DWORD size)
{
	if( !frameBufferOptions.bProcessCPUWrite )	return;
	//DebuggerAppendMsg("Frame Buffer Write, addr=%08X, CI Addr=%08X", addr, g_CI.dwAddr);
	status.frameWriteByCPU = TRUE;
	frameWriteRecord.push_back(addr&(g_dwRamSize-1));
}

extern RECT frameWriteByCPURect;
extern std::vector<RECT> frameWriteByCPURects;
extern RECT frameWriteByCPURectArray[20][20];
extern bool frameWriteByCPURectFlag[20][20];
#define FRAMEBUFFER_IN_BLOCK
bool ProcessFrameWriteRecord()
{
	int size = frameWriteRecord.size();
	if( size == 0 ) return false;

	int index = FindRecentCIInfoIndex(frameWriteRecord[0]);
	if( index == -1 )
	{
		DEBUGGER_IF_DUMP( logTextures, {TRACE1("Frame Buffer Write to non-record addr = %08X", frameWriteRecord[0])});
		frameWriteRecord.clear();
		return false;
	}
	else
	{
		DWORD base = g_uRecentCIInfoPtrs[index]->dwAddr;
		DWORD uwidth = g_uRecentCIInfoPtrs[index]->dwWidth;
		DWORD uheight = g_uRecentCIInfoPtrs[index]->dwHeight;
		DWORD upitch = uwidth<<1;

		frameWriteByCPURect.left=uwidth-1;
		frameWriteByCPURect.top = uheight-1;

		frameWriteByCPURect.right=0;
		frameWriteByCPURect.bottom = 0;

		int x, y, off;

		for( int i=0; i<size; i++ )
		{
			off = frameWriteRecord[i]-base;
			if( off < (int)g_uRecentCIInfoPtrs[index]->dwMemSize )
			{
				y = off/upitch;
				x = (off - y*upitch)>>1;

#ifdef FRAMEBUFFER_IN_BLOCK
				int xidx=x/32;
				int yidx=y/24;

				RECT &rect = frameWriteByCPURectArray[xidx][yidx];

				if( !frameWriteByCPURectFlag[xidx][yidx] )
				{
					rect.left=rect.right=x;
					rect.top=rect.bottom=y;
					frameWriteByCPURectFlag[xidx][yidx]=true;
				}
				else
				{
					if( x < rect.left )	rect.left = x;
					if( x > rect.right ) rect.right = x;
					if( y < rect.top )	rect.top = y;
					if( y > rect.bottom ) rect.bottom = y;
				}
#else

				/*
				int index = -1;
				int rectsize = frameWriteByCPURects.size();

				if( rectsize == 0 )
				{
					RECT rect;
					rect.left=rect.right=x;
					rect.top=rect.bottom=y;
					frameWriteByCPURects.push_back(rect);
					continue;
				}

				for( int j=0; j<rectsize; j++ )
				{
					if( ( (x >= frameWriteByCPURects[j].left && (x<=frameWriteByCPURects[j].right || x-frameWriteByCPURects[j].left<=30)) ||
						  (x < frameWriteByCPURects[j].left && frameWriteByCPURects[j].right-x <= 30) ) &&
						( (y >= frameWriteByCPURects[j].top && (x<=frameWriteByCPURects[j].bottom || x-frameWriteByCPURects[j].top<=30)) ||
						  (y < frameWriteByCPURects[j].top && frameWriteByCPURects[j].bottom-y <= 30) ) )
					{
						index = j;
						break;
					}
				}

				if( index < 0 )
				{
					RECT rect;
					rect.left=rect.right=x;
					rect.top=rect.bottom=y;
					frameWriteByCPURects.push_back(rect);
					continue;
				}

				RECT &rect = frameWriteByCPURects[index];
				if( x < rect.left )	rect.left = x;
				if( x > rect.right ) rect.right = x;
				if( y < rect.top )	rect.top = y;
				if( y > rect.bottom ) rect.bottom = y;
				*/

				if( x < frameWriteByCPURect.left )	frameWriteByCPURect.left = x;
				if( x > frameWriteByCPURect.right ) frameWriteByCPURect.right = x;
				if( y < frameWriteByCPURect.top )	frameWriteByCPURect.top = y;
				if( y > frameWriteByCPURect.bottom ) frameWriteByCPURect.bottom = y;
#endif
			}
		}

		frameWriteRecord.clear();
		DebuggerAppendMsg("Frame Buffer Write: Left=%d, Top=%d, Right=%d, Bottom=%d", frameWriteByCPURect.left,
			frameWriteByCPURect.top, frameWriteByCPURect.right, frameWriteByCPURect.bottom);
		return true;
	}
}

void FrameBufferReadByCPU( DWORD addr )
{
	///return;	// it does not work very well anyway

	if( !frameBufferOptions.bProcessCPURead )	return;
	
	addr &= (g_dwRamSize-1);
	int index = FindRecentCIInfoIndex(addr);
	if( index == -1 ) 
	{
		// Check if this is the depth buffer
		DWORD size = 2*g_RecentCIInfo[0].dwWidth*g_RecentCIInfo[0].dwHeight;
		addr &= 0x3FFFFFFF;

		if( addr >= g_ZI.dwAddr && addr < g_ZI.dwAddr + size )
		{
			DEBUGGER_IF_DUMP( logTextures, {TRACE1("Depth Buffer read, reported by emulator, addr=%08X", addr)});
		}
		else
		{
			return;
		}
	}

	if( status.gDlistCount - g_uRecentCIInfoPtrs[index]->lastUsedFrame > 3 )
	{
		// Ok, we don't have this frame anymore
		return;
	}

	if( g_uRecentCIInfoPtrs[index]->bCopied )	return;

	if( status.frameReadByCPU == FALSE )
	{
		TRACE1("Frame Buffer read, reported by emulator, addr=%08X", addr);
		DWORD size = 0x1000 - addr%0x1000;
		CheckAndSaveBackBuffer(addr, size, true);

		DEBUGGER_IF_DUMP(pauseAtNext,{TRACE0("Frame Buffer read");});
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_TEXTURE_BUFFER, 
			{DebuggerAppendMsg("Paused after setting Frame Buffer read:\n Cur CI Addr: 0x%08x, Fmt: %s Size: %s Width: %d",
				g_CI.dwAddr, pszImgFormat[g_CI.dwFormat], pszImgSize[g_CI.dwSize], g_CI.dwWidth);});
	}
}