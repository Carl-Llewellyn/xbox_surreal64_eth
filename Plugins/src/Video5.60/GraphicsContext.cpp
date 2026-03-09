/*

  Copyright (C) 2003 Rice1964

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

TextureBufferInfo gTextureBufferInfos[20];
const int numOfTxtBufInfos = sizeof(gTextureBufferInfos)/sizeof(TextureBufferInfo);
TextureBufferInfo *g_pTextureBufferInfo = NULL;

CGraphicsContext* CGraphicsContext::g_pGraphicsContext = NULL;
bool CGraphicsContext::m_deviceCapsIsInitialized = false;
bool CGraphicsContext::needCleanScene = false;
int	CGraphicsContext::m_maxFSAA = 16;
int	CGraphicsContext::m_maxAnisotropy = 16;
UINT CGraphicsContext::m_FullScreenRefreshRates[40] = {	0, 50, 55, 60, 65, 70, 72, 75, 80, 85, 90, 95, 100, 110, 120};
int CGraphicsContext::m_FullScreenResolutions[40][2] = {
	{320,200}, {400,300}, {480,360}, {512,384}, {640,480}, 
	{800,600}, {1024,768}, {1152,864}, {1280,960}, 
	{1400,1050}, {1600,1200}, {1920,1440}, {2048,1536}};
int CGraphicsContext::m_numOfResolutions = 0;
UINT CGraphicsContext::m_ColorBufferDepths[4] = {16, 32, 0, 0};

CGraphicsContext * CGraphicsContext::Get(void)
{	
	return CGraphicsContext::g_pGraphicsContext;
}
	
CGraphicsContext::CGraphicsContext() :
	m_bReady(false), 
	m_bActive(false),
	m_bWindowed(true),
	m_supportTextureMirror(false),
	m_backBufferIsSaved(true),
	m_isRenderingToTexture(false),
	m_lastTextureBufferIndex(-1),
	m_curBackBufferIndex(-1),
	m_curTextureBufferIndex(-1)
{
	memset(&m_textureColorBufferInfo,0,sizeof(TextureBufferShortInfo));
	memset(&m_textureDepthBufferInfo,0,sizeof(TextureBufferShortInfo));
}
CGraphicsContext::~CGraphicsContext()
{
	for( int i=0; i<numOfTxtBufInfos; i++ )
	{
		SAFE_DELETE(gTextureBufferInfos[i].pTxtBuffer);
	}
}

HWND		CGraphicsContext::m_hWnd=NULL;
HWND		CGraphicsContext::m_hWndStatus=NULL;
HWND		CGraphicsContext::m_hWndToolbar=NULL;
HMENU		CGraphicsContext::m_hMenu=NULL;
DWORD		CGraphicsContext::m_dwWindowStyle=0;     // Saved window style for mode switches
DWORD		CGraphicsContext::m_dwWindowExStyle=0;   // Saved window style for mode switches
DWORD		CGraphicsContext::m_dwStatusWindowStyle=0;     // Saved window style for mode switches
 

void CGraphicsContext::InitWindowInfo()
{
 
}


bool CGraphicsContext::Initialize(HWND hWnd, HWND hWndStatus, DWORD dwWidth, DWORD dwHeight, BOOL bWindowed )
{
 	/*if( windowSetting.bDisplayFullscreen )
	{
		windowSetting.uDisplayWidth = windowSetting.uFullScreenDisplayWidth;
		windowSetting.uDisplayHeight = windowSetting.uFullScreenDisplayHeight;
	}
	else
	{
		windowSetting.uDisplayWidth = windowSetting.uWindowDisplayWidth;
		windowSetting.uDisplayHeight= windowSetting.uWindowDisplayHeight;
	}*/
	windowSetting.uDisplayWidth = (unsigned short)dwWidth;
	windowSetting.uDisplayHeight = (unsigned short)dwHeight;
 
	
	return true;
}

void CGraphicsContext::CleanUp()
{
    m_bActive = false;
    m_bReady  = false;

 
}

bool FrameBufferInRDRAMCheckCRC();
bool ProcessFrameWriteRecord();
extern RECT frameWriteByCPURect;
extern std::vector<RECT> frameWriteByCPURects;
extern RECT frameWriteByCPURectArray[20][20];
extern bool frameWriteByCPURectFlag[20][20];
#define FRAMEBUFFER_IN_BLOCK

void CGraphicsContext::UpdateFrameBufferBeforeUpdateFrame()
{
	if( (frameBufferOptions.bProcessCPUWrite && status.frameWriteByCPU ) ||
		(frameBufferOptions.bLoadBackBufFromRDRAM && !FrameBufferInRDRAMCheckCRC() ) )		
		// Checks if frame buffer has been modified by CPU
		// Only happens to Dr. Mario
	{
		if( frameBufferOptions.bProcessCPUWrite )
		{
			if( ProcessFrameWriteRecord() )
			{
#ifdef FRAMEBUFFER_IN_BLOCK
				int i,j;
				for( i=0; i<20; i++)
				{
					for( j=0; j<20; j++ )
					{
						if( frameWriteByCPURectFlag[i][j] )
						{
							CRender::GetRender()->DrawFrameBuffer(false, frameWriteByCPURectArray[i][j].left, frameWriteByCPURectArray[i][j].top,
								frameWriteByCPURectArray[i][j].right-frameWriteByCPURectArray[i][j].left+1, frameWriteByCPURectArray[i][j].bottom-frameWriteByCPURectArray[i][j].top+1);
						}
					}
				}
				for( i=0; i<20; i++)
				{
					for( j=0; j<20; j++ )
					{
						if( frameWriteByCPURectFlag[i][j] )
						{
							ClearFrameBufferToBlack(frameWriteByCPURectArray[i][j].left, frameWriteByCPURectArray[i][j].top,
								frameWriteByCPURectArray[i][j].right-frameWriteByCPURectArray[i][j].left+1, frameWriteByCPURectArray[i][j].bottom-frameWriteByCPURectArray[i][j].top+1);
							frameWriteByCPURectFlag[i][j] = false;
						}
					}
				}
				//memset(frameWriteByCPURectArray, 0, sizeof(frameWriteByCPURectArray));
				//memset(frameWriteByCPURectFlag, 0, sizeof(frameWriteByCPURectFlag));
#else
				CRender::GetRender()->DrawFrameBuffer(false, frameWriteByCPURect.left, frameWriteByCPURect.top,
					frameWriteByCPURect.right-frameWriteByCPURect.left, frameWriteByCPURect.bottom-frameWriteByCPURect.top);
				ClearFrameBufferToBlack(frameWriteByCPURect.left, frameWriteByCPURect.top,
					frameWriteByCPURect.right-frameWriteByCPURect.left+1, frameWriteByCPURect.bottom-frameWriteByCPURect.top+1);

				/*
				int size = frameWriteByCPURects.size();
				for( int i=0; i<size; i++)
				{
					CRender::GetRender()->DrawFrameBuffer(false, frameWriteByCPURects[i].left, frameWriteByCPURects[i].top,
						frameWriteByCPURects[i].right-frameWriteByCPURects[i].left, frameWriteByCPURects[i].bottom-frameWriteByCPURects[i].top);
					ClearFrameBufferToBlack(frameWriteByCPURects[i].left, frameWriteByCPURects[i].top,
						frameWriteByCPURects[i].right-frameWriteByCPURects[i].left+1, frameWriteByCPURects[i].bottom-frameWriteByCPURects[i].top+1);
				}
				frameWriteByCPURects.clear();
				*/
#endif
			}
			status.frameWriteByCPU = FALSE;
		}
		else
		{
			extern RecentCIInfo *g_uRecentCIInfoPtrs[3];
			RecentCIInfo &p = *(g_uRecentCIInfoPtrs[0]);
			CRender::GetRender()->DrawFrameBuffer(false, 0,0,p.dwWidth,p.dwHeight);
			ClearFrameBufferToBlack();
		}
	}
}

DWORD ComputeCImgHeight(SetImgInfo &info, DWORD &height)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	for( int i=0; i<10; i++ )
	{
		DWORD word1 = *(DWORD *)(g_pRDRAMu8 + dwPC + i*8);
		DWORD word2 = *(DWORD *)(g_pRDRAMu8 + dwPC + 4 + i*8);

		if( (word1>>24) == RDP_SETSCISSOR )
		{
			height   = ((word2>>0 )&0xFFF)/4;
			DEBUGGER_IF_DUMP((logTextureBuffer), {TRACE1("buffer height = %d", height);});
			return RDP_SETSCISSOR;
		}

		if( (word1>>24) == RDP_FILLRECT )
		{
			uint32 x0   = ((word2>>12)&0xFFF)/4;
			uint32 y0   = ((word2>>0 )&0xFFF)/4;
			uint32 x1   = ((word1>>12)&0xFFF)/4;
			uint32 y1   = ((word1>>0 )&0xFFF)/4;

			if( x0 == 0 && y0 == 0 )
			{
				if( x1 == info.dwWidth )
				{
					height = y1;
					DEBUGGER_IF_DUMP((logTextureBuffer), {TRACE1("buffer height = %d", height);});
					return RDP_FILLRECT;
				}

				if( x1 == info.dwWidth - 1 )
				{
					height = y1+1;
					DEBUGGER_IF_DUMP((logTextureBuffer), {TRACE1("buffer height = %d", height);});
					return RDP_FILLRECT;
				}
			}
		}	

		if( (word1>>24) == RDP_SETCIMG )
		{
			goto step2;
		}

		if( (word1>>24) == RDP_SETCIMG )
		{
			goto step2;
		}
	}

	if( gRDP.scissor.left == 0 && gRDP.scissor.top == 0 && gRDP.scissor.right == info.dwWidth )
	{
		height = gRDP.scissor.bottom;
		DEBUGGER_IF_DUMP((logTextureBuffer), {TRACE1("buffer height = %d", height);});
		return RDP_SETSCISSOR+1;
	}

step2:
	DEBUGGER_IF_DUMP((logTextureBuffer), {TRACE0("Not sure about buffer height");});

	height = info.dwWidth*3/4;
	if( status.dwTvSystem == TV_SYSTEM_PAL )
	{
		height = info.dwWidth*9/11;
	}

	if( gRDP.scissor.bottom < (int)height && gRDP.scissor.bottom != 0 )
	{
		height = gRDP.scissor.bottom;
	}

	if( info.dwAddr + height*info.dwWidth*info.dwSize >= g_dwRamSize )
	{
		height = info.dwWidth*3/4;
		if( status.dwTvSystem == TV_SYSTEM_PAL )
		{
			height = info.dwWidth*9/11;
		}

		if( gRDP.scissor.bottom < (int)height && gRDP.scissor.bottom != 0 )
		{
			height = gRDP.scissor.bottom;
		}

		if( info.dwAddr + height*info.dwWidth*info.dwSize >= g_dwRamSize )
		{
			height = ( g_dwRamSize - info.dwAddr ) / info.dwWidth;
		}
	}

	DEBUGGER_IF_DUMP((logTextureBuffer), {TRACE1("texture buffer height = %d", height);});
	return 0;
}

int CGraphicsContext::CheckTxtrBufsWithNewCI(SetImgInfo &CIinfo, DWORD height, bool byNewTxtrBuf)
{
	int matchidx = -1;
	uint32 memsize = ((height*CIinfo.dwWidth)>>1)<<CIinfo.dwSize;

	for( int i=0; i<numOfTxtBufInfos; i++ )
	{
		TextureBufferInfo &info = gTextureBufferInfos[i];
		if( !info.isUsed )	continue;

		bool covered = false;

		if( info.CI_Info.dwAddr == CIinfo.dwAddr )
		{
			if( info.CI_Info.dwSize == CIinfo.dwSize &&
				info.CI_Info.dwWidth == CIinfo.dwWidth &&
				info.CI_Info.dwFormat == CIinfo.dwFormat &&
				info.N64Height == height )
			{
				// This is the same texture at the same address
				if( byNewTxtrBuf )
				{
				matchidx = i;
				break;
			}
			}

				// At the same address, but not the same size
				//SAFE_DELETE(info.psurf);
			covered = true;
		}

		if( !covered )
		{
		uint32 memsize2 = ((info.N64Height*info.N64Width)>>1)<<info.CI_Info.dwSize;

		if( info.CI_Info.dwAddr > CIinfo.dwAddr && info.CI_Info.dwAddr < CIinfo.dwAddr + memsize)
			covered = true;
		else if( info.CI_Info.dwAddr+memsize2 > CIinfo.dwAddr && info.CI_Info.dwAddr+memsize2 < CIinfo.dwAddr + memsize)
			covered = true;
		else if( CIinfo.dwAddr > info.CI_Info.dwAddr && CIinfo.dwAddr < info.CI_Info.dwAddr + memsize2 )
			covered = true;
		else if( CIinfo.dwAddr+ memsize > info.CI_Info.dwAddr && CIinfo.dwAddr+ memsize < info.CI_Info.dwAddr + memsize2 )
			covered = true;
		}

		if( covered )
		{
			//SAFE_DELETE(info.psurf);
			if( info.pTxtBuffer->IsBeingRendered() )
			{
				TRACE0("Error, covering a texture buffer which is being rendered");
				TRACE3("New addrr=%08X, width=%d, height=%d", CIinfo.dwAddr, CIinfo.dwWidth, height );
				TRACE3("Old addrr=%08X, width=%d, height=%d", info.CI_Info.dwAddr, info.N64Width, info.N64Height );
			}
			info.isUsed = false;
			DEBUGGER_IF_DUMP((logTextureBuffer),TRACE5("Delete txtr buf %d at %08X, covered by new CI at %08X, Width=%d, Height=%d", 
				i, info.CI_Info.dwAddr, CIinfo.dwAddr, CIinfo.dwWidth, height ));
			SAFE_DELETE(info.pTxtBuffer);
			info.txtEntry.pTexture = NULL;
			continue;
		}
	}

	return matchidx;
}

extern RecentCIInfo *g_uRecentCIInfoPtrs[5];
int CGraphicsContext::SetTextureBuffer(SetImgInfo &CIinfo, int ciInfoIdx, bool toSaveBackBuffer)
{
	TextureUsage usage = AS_RENDER_TARGET;

	if(!toSaveBackBuffer)
		status.bHandleN64TextureBuffer = true;

	TextureBufferInfo tempInfo;
	memcpy(&(tempInfo.CI_Info), &CIinfo, sizeof(SetImgInfo));

	if( toSaveBackBuffer )
	{
		tempInfo.N64Width = g_uRecentCIInfoPtrs[ciInfoIdx]->dwLastWidth;
		tempInfo.N64Height = g_uRecentCIInfoPtrs[ciInfoIdx]->dwLastHeight;
		tempInfo.knownHeight = true;
		usage = AS_BACK_BUFFER_SAVE;
	}
	else
	{
		tempInfo.N64Width = tempInfo.CI_Info.dwWidth;
		tempInfo.knownHeight = ComputeCImgHeight(CIinfo, tempInfo.N64Height);
	}
	tempInfo.maxUsedHeight = 0;

	if( !toSaveBackBuffer )
	{
		if( defaultRomOptions.bInN64Resolution )
		{
			tempInfo.bufferWidth = tempInfo.N64Width;
			tempInfo.bufferHeight = tempInfo.N64Height;
		}
		else if( defaultRomOptions.bDoubleSizeForSmallTxtrBuf && tempInfo.N64Width<=128 && tempInfo.N64Height<=128)
		{
			tempInfo.bufferWidth = tempInfo.N64Width*2;
			tempInfo.bufferHeight = tempInfo.N64Height*2;
		}
		else
		{
			tempInfo.bufferWidth = tempInfo.N64Width;
			tempInfo.bufferHeight = tempInfo.N64Height;
		}
	}
	else
	{
		tempInfo.bufferWidth = windowSetting.uDisplayWidth;
		tempInfo.bufferHeight = windowSetting.uDisplayHeight;
	}

	tempInfo.scaleX = tempInfo.bufferWidth / float(tempInfo.N64Width);
	tempInfo.scaleY = tempInfo.bufferHeight / float(tempInfo.N64Height);

	status.bFrameBufferIsDrawn = false;
	status.bFrameBufferDrawnByTriangles = false;

	tempInfo.updateAtFrame = status.gDlistCount;
	tempInfo.updateAtUcodeCount = status.gUcodeCount;

	// Checking against previous texture buffer infos
	int matchidx = -1;

	uint32 memsize = ((tempInfo.N64Height*tempInfo.N64Width)>>1)<<tempInfo.CI_Info.dwSize;

	matchidx = CheckTxtrBufsWithNewCI(CIinfo,tempInfo.N64Height,true);

	int idxToUse=-1;
	if( matchidx >= 0 )
	{
		// Reuse the matched slot
		idxToUse = matchidx;
		if( gTextureBufferInfos[matchidx].pTxtBuffer == NULL )
		{
			if( tempInfo.knownHeight == RDP_SETSCISSOR && tempInfo.CI_Info.dwAddr == g_ZI.dwAddr )
			{
				gTextureBufferInfos[matchidx].pTxtBuffer = 
					new CDXTextureBuffer(gRDP.scissor.right, tempInfo.bufferHeight, &gTextureBufferInfos[matchidx], usage);
			}
			else
			{
				gTextureBufferInfos[matchidx].pTxtBuffer = 
					new CDXTextureBuffer(tempInfo.bufferWidth, tempInfo.bufferHeight, &gTextureBufferInfos[matchidx], usage);
			}
		}
	}
	else
	{
		// Find an empty slot
		bool found = false;
		for( int i=0; i<numOfTxtBufInfos; i++ )
		{
			if( !gTextureBufferInfos[i].isUsed )
			{
				found = true;
				idxToUse = i;
				break;
			}
		}

		// If cannot find an empty slot, find the oldest slot and reuse the slot
		if( !found )
		{
			uint32 oldestCount=0xFFFFFFFF;
			uint32 oldestIdx = 0;
			for( int i=0; i<numOfTxtBufInfos; i++ )
			{
				if( gTextureBufferInfos[i].updateAtUcodeCount < oldestCount )
				{
					oldestCount = gTextureBufferInfos[i].updateAtUcodeCount;
					oldestIdx = i;
				}
			}

			idxToUse = oldestIdx;
		}

		DEBUGGER_IF_DUMP((logTextureBuffer && gTextureBufferInfos[idxToUse].pTxtBuffer ),TRACE2("Delete txtr buf %d at %08X, to reuse it.", idxToUse, gTextureBufferInfos[idxToUse].CI_Info.dwAddr ));
		SAFE_DELETE(gTextureBufferInfos[idxToUse].pTxtBuffer) ;

		// After get the slot
		// create a new texture buffer and assign it to this slot
		if( tempInfo.knownHeight == RDP_SETSCISSOR && tempInfo.CI_Info.dwAddr == g_ZI.dwAddr )
		{
			gTextureBufferInfos[idxToUse].pTxtBuffer = 
				new CDXTextureBuffer(gRDP.scissor.right, tempInfo.bufferHeight, &gTextureBufferInfos[idxToUse], usage);
		}
		else
		{
			gTextureBufferInfos[idxToUse].pTxtBuffer = 
				new CDXTextureBuffer(tempInfo.bufferWidth, tempInfo.bufferHeight, &gTextureBufferInfos[idxToUse], usage); 
		}
	}

	// Need to set all variables for gTextureBufferInfos[idxToUse]
	CTextureBuffer *pTxtBuffer = gTextureBufferInfos[idxToUse].pTxtBuffer;
	memcpy(&gTextureBufferInfos[idxToUse], &tempInfo, sizeof(TextureBufferInfo) );
	gTextureBufferInfos[idxToUse].pTxtBuffer = pTxtBuffer;
	gTextureBufferInfos[idxToUse].isUsed = true;
	gTextureBufferInfos[idxToUse].txtEntry.pTexture = pTxtBuffer->m_pTexture;
	gTextureBufferInfos[idxToUse].txtEntry.txtrBufIdx = idxToUse+1;

	if( !toSaveBackBuffer )
	{
		g_pTextureBufferInfo = &gTextureBufferInfos[idxToUse];

		// Active the texture buffer
		if( m_curTextureBufferIndex >= 0 && gTextureBufferInfos[m_curTextureBufferIndex].isUsed && gTextureBufferInfos[m_curTextureBufferIndex].pTxtBuffer )
		{
			gTextureBufferInfos[m_curTextureBufferIndex].pTxtBuffer->SetAsRenderTarget(false);
			m_isRenderingToTexture = false;
		}

		if( gTextureBufferInfos[idxToUse].pTxtBuffer->SetAsRenderTarget(true) )
		{
			m_isRenderingToTexture = true;

			//Clear(CLEAR_COLOR_AND_DEPTH_BUFFER,0x80808080,1.0f);
			if( frameBufferOptions.bFillRectNextTextureBuffer )
				Clear(CLEAR_COLOR_BUFFER,gRDP.fillColor,1.0f);
			else if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS && g_pTextureBufferInfo->N64Width > 64 && g_pTextureBufferInfo->N64Width < 300 )
			{
				Clear(CLEAR_COLOR_BUFFER,0,1.0f);
			}
			else if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS && g_pTextureBufferInfo->N64Width < 64 && g_pTextureBufferInfo->N64Width > 32 )
			{
				Clear(CLEAR_COLOR_BUFFER,0,1.0f);
			}

			m_curTextureBufferIndex = idxToUse;

			status.bDirectWriteIntoRDRAM = false;

			//SetScreenMult(1, 1);
			SetScreenMult(gTextureBufferInfos[m_curTextureBufferIndex].scaleX, gTextureBufferInfos[m_curTextureBufferIndex].scaleY);
			CRender::g_pRender->UpdateClipRectangle();
			D3DVIEWPORT8 vp = {0,0,gTextureBufferInfos[idxToUse].bufferWidth,gTextureBufferInfos[idxToUse].bufferHeight};
			g_pD3DDev->SetViewport(&vp);

			// If needed, draw RDRAM into the texture buffer
			//if( frameBufferOptions.bLoadRDRAMIntoTxtBuf )
			//{
			//	CRender::GetRender()->LoadTxtrBufFromRDRAM();
			//}
		}
		else
		{
			TRACE1("Error to set Render Target: %d", idxToUse);
			TRACE1("Addr = %08X", gTextureBufferInfos[idxToUse].CI_Info.dwAddr);
			TRACE2("Width = %d, Height=%d", gTextureBufferInfos[idxToUse].N64Width, gTextureBufferInfos[idxToUse].N64Height);
		}	
	}


	DEBUGGER_IF_DUMP((logTextureBuffer),{TRACE2("Set texture buffer %d, addr=%08X", idxToUse, CIinfo.dwAddr);});
	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_TEXTURE_BUFFER, 
	{DebuggerAppendMsg("Paused after setting texture buffer:\nAddr: 0x%08x, Fmt: %s Size: %s Width: %d, Height:%d",
	CIinfo.dwAddr, pszImgFormat[CIinfo.dwFormat], pszImgSize[CIinfo.dwSize], CIinfo.dwWidth, g_pTextureBufferInfo->N64Height);});

	return idxToUse;
}

void CGraphicsContext::CloseTextureBuffer(bool toSave)
{
	status.bHandleN64TextureBuffer = false;
	if( status.bDirectWriteIntoRDRAM )
	{
	}
	else 
	{
		if( !toSave || !status.bFrameBufferIsDrawn || !status.bFrameBufferDrawnByTriangles )
		{
			RestoreNormalBackBuffer();

			DEBUGGER_IF_DUMP((logTextureBuffer) ,{DebuggerAppendMsg("Closing texture buffer without save");});
		}
		else
		{
			RestoreNormalBackBuffer();
			StoreTextureBufferToRDRAM();
		}
		//g_pTextureBufferInfo->pTxtBuffer->m_pTexture->RestoreAlphaChannel();

		g_pTextureBufferInfo->crcInRDRAM = ComputeTextureBufferCRCInRDRAM(m_curTextureBufferIndex);
		g_pTextureBufferInfo->crcCheckedAtFrame = status.gDlistCount;
	}

	SetScreenMult(windowSetting.uDisplayWidth/windowSetting.fViWidth, windowSetting.uDisplayHeight/windowSetting.fViHeight);
	CRender::g_pRender->UpdateClipRectangle();
	CRender::g_pRender->ApplyScissorWithClipRatio();

	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_TEXTURE_BUFFER, 
	{
		DebuggerAppendMsg("Paused after saving texture buffer %d:\nAddr: 0x%08x, Fmt: %s Size: %s Width: %d", m_curTextureBufferIndex,
			g_pTextureBufferInfo->CI_Info.dwAddr, pszImgFormat[g_pTextureBufferInfo->CI_Info.dwFormat], pszImgSize[g_pTextureBufferInfo->CI_Info.dwSize], g_pTextureBufferInfo->CI_Info.dwWidth);
	});
}

int FindRecentCIInfoIndex (DWORD addr);

void CGraphicsContext::ClearFrameBufferToBlack(DWORD left, DWORD top, DWORD width, DWORD height)
{
	RecentCIInfo &p = *(g_uRecentCIInfoPtrs[0]);
	WORD *frameBufferBase = (WORD*)(g_pRDRAMu8+p.dwAddr);
	DWORD pitch = p.dwWidth;

	if( width == 0 || height == 0 )
	{
		DWORD len = p.dwHeight*p.dwWidth*p.dwSize;
		if( p.dwSize == TXT_SIZE_4b ) len = (p.dwHeight*p.dwWidth)>>1;
		memset(frameBufferBase, 0, len);
	}
	else
	{
		for( DWORD y=0; y<height; y++)
		{
			for( DWORD x=0; x<width; x++ )
			{
				*(frameBufferBase+(y+top)*pitch+x+left) = 0;
			}
		}
	}
}

BYTE RevTlutTable[0x10000];
bool RevTlutTableNeedUpdate = false;
void InitTlutReverseLookup(void)
{
	if( RevTlutTableNeedUpdate )
	{
		memset(RevTlutTable, 0, 0x10000);
		for( int i=0; i<=0xFF; i++ )
		{
			RevTlutTable[g_wRDPTlut[i]] = BYTE(i);
		}

		RevTlutTableNeedUpdate = false;
	}
}


void CGraphicsContext::CopyBackToFrameBufferIfReadByCPU(DWORD addr)
{
	int i = FindRecentCIInfoIndex(addr);
	if( i != -1 )
	{
		//if( i == 0 ) CGraphicsContext::Get()->UpdateFrame();
		RecentCIInfo *info = g_uRecentCIInfoPtrs[i];
		CopyBackToRDRAM( info->dwAddr, info->dwFormat, info->dwSize, info->dwWidth, info->dwHeight, 
			windowSetting.uDisplayWidth, windowSetting.uDisplayHeight, addr, 0x1000-addr%0x1000);
		TRACE1("Copy back for CI Addr=%08X", info->dwAddr);
	}
}
void CGraphicsContext::CheckTxtrBufsCRCInRDRAM(void)
{
	for( int i=0; i<numOfTxtBufInfos; i++ )
	{
		if( !gTextureBufferInfos[i].isUsed )	
			continue;

		if( gTextureBufferInfos[i].pTxtBuffer->IsBeingRendered() )
			continue;

		if( gTextureBufferInfos[i].crcCheckedAtFrame < status.gDlistCount )
		{
			uint32 crc = ComputeTextureBufferCRCInRDRAM(i);
			if( gTextureBufferInfos[i].crcInRDRAM != crc )
			{
				// RDRAM has been modified by CPU core
				DEBUGGER_IF_DUMP((logTextureBuffer),TRACE2("Delete txtr buf %d at %08X, CRC in RDRAM changed", i, gTextureBufferInfos[i].CI_Info.dwAddr ));
				SAFE_DELETE(gTextureBufferInfos[i].pTxtBuffer);
				gTextureBufferInfos[i].isUsed = false;
				continue;
			}
			else
			{
				gTextureBufferInfos[i].crcCheckedAtFrame = status.gDlistCount;
			}
		}
	}
}

int	CGraphicsContext::CheckAddrInTxtrBufs(TextureInfo &ti)
{
	for( int i=0; i<numOfTxtBufInfos; i++ )
	{
		if( !gTextureBufferInfos[i].isUsed )	
			continue;

		if( gTextureBufferInfos[i].pTxtBuffer->IsBeingRendered() )
			continue;

		uint32 bufHeight = gTextureBufferInfos[i].knownHeight ? gTextureBufferInfos[i].N64Height : gTextureBufferInfos[i].maxUsedHeight;
		DWORD bufMemSize = gTextureBufferInfos[i].CI_Info.dwSize*gTextureBufferInfos[i].N64Width*bufHeight;
		if( ti.Address >=gTextureBufferInfos[i].CI_Info.dwAddr && ti.Address < gTextureBufferInfos[i].CI_Info.dwAddr+bufMemSize)
		{
			// Check the CRC in RDRAM
			if( gTextureBufferInfos[i].crcCheckedAtFrame < status.gDlistCount )
			{
				uint32 crc = ComputeTextureBufferCRCInRDRAM(i);
				if( gTextureBufferInfos[i].crcInRDRAM != crc )
				{
					// RDRAM has been modified by CPU core
					DEBUGGER_IF_DUMP((logTextureBuffer),TRACE2("Delete txtr buf %d at %08X, crcInRDRAM failed.", i, gTextureBufferInfos[i].CI_Info.dwAddr ));
					SAFE_DELETE(gTextureBufferInfos[i].pTxtBuffer);
					gTextureBufferInfos[i].isUsed = false;
					continue;
				}
				else
				{
					gTextureBufferInfos[i].crcCheckedAtFrame = status.gDlistCount;
				}
			}

			DEBUGGER_IF_DUMP((logTextureBuffer),TRACE2("Loading texture addr = %08X from txtr buf %d", ti.Address, i));
			return i;
		}
	}

	return -1;
}

void CGraphicsContext::LoadTextureFromTextureBuffer(TextureEntry* pEntry, int infoIdx)
{
	if( infoIdx < 0 || infoIdx >= numOfTxtBufInfos )
	{
		infoIdx = CheckAddrInTxtrBufs(pEntry->ti);
	}

	if( infoIdx >= 0 && gTextureBufferInfos[infoIdx].isUsed && gTextureBufferInfos[infoIdx].pTxtBuffer )
	{
		DEBUGGER_IF_DUMP((logTextureBuffer), 
		{TRACE1("Loading from texture buffer %d", infoIdx);});
		gTextureBufferInfos[infoIdx].pTxtBuffer->LoadTexture(pEntry);
	}
}

void CGraphicsContext::RestoreNormalBackBuffer()
{
	if( m_curTextureBufferIndex >= 0 && m_curTextureBufferIndex < numOfTxtBufInfos )
	{
		gTextureBufferInfos[m_curTextureBufferIndex].pTxtBuffer->SetAsRenderTarget(false);
		m_isRenderingToTexture = false;
		m_lastTextureBufferIndex = m_curTextureBufferIndex;
	}

	if( !status.bFrameBufferIsDrawn || !status.bFrameBufferDrawnByTriangles )
	{
		gTextureBufferInfos[m_curTextureBufferIndex].isUsed = false;
		DEBUGGER_IF_DUMP((logTextureBuffer),TRACE2("Delete txtr buf %d at %08X, it is never rendered", m_curTextureBufferIndex, gTextureBufferInfos[m_curTextureBufferIndex].CI_Info.dwAddr ));
		SAFE_DELETE(gTextureBufferInfos[m_curTextureBufferIndex].pTxtBuffer);
	}
}

extern DWORD CalculateRDRAMCRC(void *pAddr, DWORD left, DWORD top, DWORD width, DWORD height, DWORD size, DWORD pitchInBytes );
uint32 CGraphicsContext::ComputeTextureBufferCRCInRDRAM(int infoIdx)
{
	if( infoIdx >= numOfTxtBufInfos || infoIdx < 0 || !gTextureBufferInfos[infoIdx].isUsed )
		return 0;

	TextureBufferInfo &info = gTextureBufferInfos[infoIdx];
	uint32 height = info.knownHeight ? info.N64Height : info.maxUsedHeight;
	BYTE *pAddr = (BYTE*)(g_pRDRAMu8+info.CI_Info.dwAddr);
	DWORD pitch = (info.N64Width << info.CI_Info.dwSize ) >> 1;
	return CalculateRDRAMCRC(pAddr, 0, 0, info.N64Width, height, info.CI_Info.dwSize, pitch);
}

int _cdecl SortFrequenciesCallback( const VOID* arg1, const VOID* arg2 )
{
	UINT* p1 = (UINT*)arg1;
	UINT* p2 = (UINT*)arg2;

	if( *p1 < *p2 )   
		return -1;
	else if( *p1 > *p2 )   
		return 1;
	else 
		return 0;
}
int _cdecl SortResolutionsCallback( const VOID* arg1, const VOID* arg2 )
{
	UINT* p1 = (UINT*)arg1;
	UINT* p2 = (UINT*)arg2;

	if( *p1 < *p2 )   
		return -1;
	else if( *p1 > *p2 )   
		return 1;
	else 
	{
		if( p1[1] < p2[1] )   
			return -1;
		else if( p1[1] > p2[1] )   
			return 1;
		else
			return 0;
	}
}

// This is a static function, will be called when the plugin DLL is initialized
void CGraphicsContext::InitDeviceParameters(void)
{
 
}
