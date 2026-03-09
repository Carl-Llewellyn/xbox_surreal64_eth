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
#include "DirectXGraphicsContext.h"
#include "DaedalusRender.h"
#include "D3DRender.h"
#define DIRECTX_ONLY	{if( CDeviceBuilder::GetGeneralDeviceType() != DIRECTX_DEVICE) return;}


FakeFrameBufferInfo g_FakeFrameBufferInfo = 
{ 
	NULL,
	{G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 },
};

bool g_bUsingFakeCI = false;
bool g_bFakeCIUpdated = false;

// 0 keeps the most recent CI info
// 1 keeps the frame buffer CI info which is being displayed now
// 2 keeps the older frame buffer CI info. This can be used if we are using triple buffer


RecentCIInfo g_RecentCIInfo[3];
RecentCIInfo *g_uRecentCIInfoPtrs[3] =
{
	&g_RecentCIInfo[0],
	&g_RecentCIInfo[1],
	&g_RecentCIInfo[2],
};

RecentViOriginInfo g_RecentVIOriginInfo[3];


// ===========================================================================
bool FrameBufferInRDRAMCheckCRC();


inline u32 GetNextPowerOf2( u32 x )
{
	u32 n = 1;
	while ( n < x )
	{
		n = n<<1;
	}

	return n;
}

inline WORD ConvertRGBATo555(BYTE r, BYTE g, BYTE b, BYTE a)
{
	return ((r>>3)<<RGBA5551_RedShift) | ((g>>3)<<RGBA5551_GreenShift) | ((b>>3)<<RGBA5551_BlueShift) | 1;//(a>>7);
}

inline WORD ConvertRGBATo555(DWORD color32)
{
	return WORD((((color32>>19)&0x1F)<<RGBA5551_RedShift) | (((color32>>11)&0x1F)<<RGBA5551_GreenShift) | (((color32>>3)&0x1F)<<RGBA5551_BlueShift) | ((color32>>31)));;
}

void UpdateRecentCIAddr(SetImgInfo &ciinfo)
{
	if( ciinfo.dwAddr == g_uRecentCIInfoPtrs[0]->dwAddr )
		return;
	
	RecentCIInfo *temp = g_uRecentCIInfoPtrs[2];
	g_uRecentCIInfoPtrs[2] = g_uRecentCIInfoPtrs[1];
	g_uRecentCIInfoPtrs[1] = g_uRecentCIInfoPtrs[0];

	g_uRecentCIInfoPtrs[0] = temp;
	temp->dwFormat = ciinfo.dwFormat;
	temp->dwAddr = ciinfo.dwAddr;
	temp->dwSize = ciinfo.dwSize;
	temp->dwWidth = ciinfo.dwWidth;
	temp->dwHeight = gRDP.scissor.bottom;
	temp->dwMemSize = (temp->dwWidth*temp->dwHeight/2)<<temp->dwSize;
	temp->bCopied = false;
	temp->lastUsedFrame = status.gRDPFrame;
}


/************************************************************************/
/* Mark the ciinfo entry that the ciinfo is used by VI origin register  */
/* in another word, this is a real frame buffer, not a fake frame buffer*/
/* Fake frame buffers are never really used by VI origin				*/
/************************************************************************/
void SetAddrUsedByVIOrigin(u32 addr)
{
	u32 viwidth = *g_GraphicsInfo.VI_WIDTH_RG;
	addr &= (g_dwRamSize-1);
	for( int i=0; i<3; i++ )
	{
		if( g_uRecentCIInfoPtrs[i]->dwAddr+2*viwidth == addr )
		{
			g_uRecentCIInfoPtrs[i]->bUsedByVIAtFrame = status.gRDPFrame;
		}
		else if( addr >= g_uRecentCIInfoPtrs[i]->dwAddr && addr < g_uRecentCIInfoPtrs[i]->dwAddr+0x1000 )
		{
			g_uRecentCIInfoPtrs[i]->bUsedByVIAtFrame = status.gRDPFrame;
		}
	}

	for( i=0; i<3; i++ )
	{
		if( g_RecentVIOriginInfo[i].addr == addr )
		{
			g_RecentVIOriginInfo[i].FrameCount = status.gRDPFrame;
			return;
		}
	}

	for( i=0; i<3; i++ )
	{
		if( g_RecentVIOriginInfo[i].addr == 0 )
		{
			// Never used
			g_RecentVIOriginInfo[i].addr = addr;
			g_RecentVIOriginInfo[i].FrameCount = status.gRDPFrame;
			return;
		}
	}

	int index=0;
	u32 minFrameCount = 0xffffffff;

	for( i=0; i<3; i++ )
	{
		if( g_RecentVIOriginInfo[i].FrameCount < minFrameCount )
		{
			index = i;
			minFrameCount = g_RecentVIOriginInfo[i].FrameCount;
		}
	}

	g_RecentVIOriginInfo[index].addr = addr;
	g_RecentVIOriginInfo[index].FrameCount = status.gRDPFrame;
}

bool IsAddrUsedByVIorigin(u32 addr)
{
	addr &= (g_dwRamSize-1);
	for( int i=0; i<3; i++ )
	{
		if( g_uRecentCIInfoPtrs[i]->dwAddr == 0 )
			continue;

		if( g_uRecentCIInfoPtrs[i]->dwAddr == addr )
		{
			if( status.gRDPFrame-g_uRecentCIInfoPtrs[i]->bUsedByVIAtFrame < 20 )
			{
				return true;
			}
			else
			{
				TRACE0("This is a fake frame buffer, the addr is never a displayed buffer");
				return false;
			}
		}
	}

	for( i=0; i<3; i++ )
	{
		if( g_RecentVIOriginInfo[i].addr != 0 )
		{
			if( g_RecentVIOriginInfo[i].addr > addr && g_RecentVIOriginInfo[i].addr < addr + 0x1000 )
			{
				if( status.gRDPFrame-g_RecentVIOriginInfo[i].FrameCount < 20 )
				{
					return true;
				}
				else
				{
					TRACE0("This is a fake frame buffer, the addr is never a displayed buffer");
					return false;
				}
			}
		}
	}
	
	if( status.gRDPFrame > 20 )
		return false;
	else
	{
		TRACE0("This is a fake frame buffer, the addr is never a displayed buffer");
		return true;
	}
}

int FindRecentCIInfoIndex(u32 addr)
{
	for( int i=0; i<3; i++ )
	{
		if( g_uRecentCIInfoPtrs[i]->dwAddr <= addr && addr < g_uRecentCIInfoPtrs[i]->dwAddr+g_uRecentCIInfoPtrs[i]->dwMemSize )
		{
			return i;
		}
	}
	return -1;
}


// Return -1 if the addr is not in recent frame buffer addr
// Return 1 if the addr is in the frame buffer which is currently being displayed
// Return 2 if the addr is in the 3rd frame buffer if we are using triple buffer
// this function will not return 0 which means the addr is within the current rendering frame buffer
//     this should not happen
extern BufferSettingInfo DirectXRenderBufferSettings[];
int IsAddrInRecentFrameBuffers(u32 addr)
{
	if( addr >= g_uRecentCIInfoPtrs[1]->dwAddr && addr < g_uRecentCIInfoPtrs[1]->dwAddr+g_uRecentCIInfoPtrs[1]->dwMemSize )
		return 1;
	else if( DirectXRenderBufferSettings[options.RenderBufferSetting].number > 1 &&
		addr >= g_uRecentCIInfoPtrs[2]->dwAddr && addr < g_uRecentCIInfoPtrs[2]->dwAddr+g_uRecentCIInfoPtrs[1]->dwMemSize )
	{
		return 2;
	}
	else if( addr >= g_uRecentCIInfoPtrs[0]->dwAddr && addr < g_uRecentCIInfoPtrs[0]->dwAddr+g_uRecentCIInfoPtrs[0]->dwMemSize && g_bUsingFakeCI )
	{
		return 1;
	}
	{
		return -1;
	}
}

void CheckAndUpdateFrameBuffer(u32 addr)
{
	DIRECTX_ONLY;

	int r = FindRecentCIInfoIndex(addr);
	if( r != -1 )
	{
		if( r == 0  || r == 1 || r == 2 )	// The current render buffer == 1st back buffer
		{
			if( g_uRecentCIInfoPtrs[r]->bCopied == false )
			{
				if( g_uRecentCIInfoPtrs[r]->dwAddr == g_CI.dwAddr )
				{
					TRACE0("Hit current back buffer");
				}
				else if( r == 2 )
				{
					TRACE0("Hit current 2nd back buffer");
				}
				else
				{
					TRACE0("Hit current front buffer");
					//CGraphicsContext::Get()->SwapBuffer();	//Swap the buffer
				}

				if( g_uRecentCIInfoPtrs[r]->dwWidth == *g_GraphicsInfo.VI_WIDTH_RG && g_uRecentCIInfoPtrs[0]->dwWidth != windowSetting.uViWidth )
				{
					u32 width = windowSetting.uViWidth;
					u32 height = windowSetting.uViHeight;
					CopyBackToFrameBuffer(g_uRecentCIInfoPtrs[r]->dwAddr, g_uRecentCIInfoPtrs[r]->dwFormat,
						g_uRecentCIInfoPtrs[r]->dwSize, width, height, g_uRecentCIInfoPtrs[r]->dwWidth );
				}
				else
				{
					CopyBackToFrameBuffer(g_uRecentCIInfoPtrs[r]->dwAddr, g_uRecentCIInfoPtrs[r]->dwFormat,
						g_uRecentCIInfoPtrs[r]->dwSize, g_uRecentCIInfoPtrs[r]->dwWidth, g_uRecentCIInfoPtrs[r]->dwHeight );
				}
				g_uRecentCIInfoPtrs[r]->bCopied = true;
			}
		}
		else if( r == 1 )		// This is the front buffer, not the current back buffer
		{
			if( g_uRecentCIInfoPtrs[1]->bCopied == false )
			{
				TRACE0("Hit current front buffer");
				IDirect3DSurface8 *pbuffer = NULL;
				u32 width = g_uRecentCIInfoPtrs[r]->dwWidth;
				u32 height = g_uRecentCIInfoPtrs[r]->dwHeight;

				if( g_uRecentCIInfoPtrs[r]->dwWidth == *g_GraphicsInfo.VI_WIDTH_RG && g_uRecentCIInfoPtrs[r]->dwWidth != windowSetting.uViWidth )
				{
					width = windowSetting.uViWidth;
					height = windowSetting.uViHeight;
				}

				g_pD3DDev->CreateImageSurface(windowSetting.uDisplayWidth, windowSetting.uDisplayHeight, D3DFMT_A8R8G8B8, &pbuffer);
				if( pbuffer != NULL )
				{
					g_pD3DDev->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &pbuffer);	//Get a shot of the current front buffer
					CopyBackToFrameBuffer(g_uRecentCIInfoPtrs[r]->dwAddr, g_uRecentCIInfoPtrs[r]->dwFormat,
						g_uRecentCIInfoPtrs[r]->dwSize, width, height, g_uRecentCIInfoPtrs[r]->dwWidth,
						D3DFMT_A8R8G8B8, pbuffer);
					pbuffer->Release();
				}
				else
				{
					TRACE0("Can not access front buffer");
				}

				g_uRecentCIInfoPtrs[1]->bCopied = true;
			}
		}
		else if( r == 2 )
		{
			if( g_uRecentCIInfoPtrs[2]->bCopied == false )
			{
				TRACE0("Frame Buffer Emu: Hit 2nd backbuffer, not implemented yet");
				//LPDIRECT3DSURFACE8 BackBufferSurf;
				
				// Need to copy the 2nd back buffer to RDRAM
				g_uRecentCIInfoPtrs[2]->bCopied = true;
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
			RevTlutTable[g_wRDPPal[i]] = BYTE(i);
		}

		RevTlutTableNeedUpdate = false;
	}
}

BYTE CIFindIndex(WORD val)
{
	for( int i=0; i<=0xFF; i++ )
	{
		if( val == g_wRDPPal[i] )
		{
			return (BYTE)i;
		}
	}
	return 0;
}

void CopyBackToFrameBuffer(DWORD addr, DWORD fmt, DWORD siz, DWORD width, DWORD height, DWORD pitch, D3DFORMAT surf_fmt, IDirect3DSurface8 *surf)
{
	DIRECTX_ONLY;

	if( addr == 0 || addr>=g_dwRamSize )	return;

	if( pitch == 0 ) pitch = width;

	IDirect3DSurface8 *backBuffer = surf;

	if( surf == NULL )
		g_pD3DDev->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO, &backBuffer);

	D3DSURFACE_DESC ddsd;
	backBuffer->GetDesc(&ddsd);

	D3DLOCKED_RECT dlre;
    ZeroMemory( &dlre, sizeof(D3DLOCKED_RECT) );
	//int pixSize = GetPixelSize();
	if (SUCCEEDED(backBuffer->LockRect(&dlre, NULL, D3DLOCK_READONLY)))
	{
		if( siz == G_IM_SIZ_16b )
		{
			WORD *frameBufferBase = (WORD*)(g_pu8RamBase+RDPSegAddr(addr));

			if( surf_fmt==D3DFMT_A8R8G8B8 )
			{
				float sx,sy;
				int sx0, sy0;

				for( uint32 i=0; i<height; i++ )
				{
					sy = i*ddsd.Height/(float)height;
					sy0 = int(sy+0.5);
					//sy0 = int(sy);

					WORD *pD = frameBufferBase + i * pitch;
					BYTE *pS0 = (BYTE *)dlre.pBits + sy0 * dlre.Pitch;

					for( u32 j=0; j<width; j++ )
					{
						sx = j*ddsd.Width/(float)width;
						sx0 = int(sx+0.5);
						//sx0 = int(sx);
						int pos0 = 4*sx0;

						// Point
						BYTE r = pS0[pos0+2];
						BYTE g = pS0[pos0+1];
						BYTE b = pS0[pos0+0];
						BYTE a = pS0[pos0+3];

						// Liner
						*(pD+(j^1)) = ConvertRGBATo555( r, g, b, a);

					}
				}
			}
			else
			{
				TRACE1("Copy %sb FrameBuffer to Rdram, not implemented", pszImgSize[siz]);
			}
		}
		else if( siz == G_IM_SIZ_8b && fmt == G_IM_FMT_CI )
		{
			BYTE *frameBufferBase = (BYTE*)(g_pu8RamBase+RDPSegAddr(addr));

			if( surf_fmt==D3DFMT_A8R8G8B8 )
			{
				WORD tempword;
				InitTlutReverseLookup();

				for( uint32 i=0; i<height; i++ )
				{
					BYTE *pD = frameBufferBase + i * width;
					BYTE *pS = (BYTE *)dlre.pBits + i*ddsd.Height/height * dlre.Pitch;
					for( uint32 j=0; j<width; j++ )
					{
						int pos = 4*(j*ddsd.Width/width);
						tempword = ConvertRGBATo555((pS[pos+2]),		// Red
													(pS[pos+1]),		// G
													(pS[pos+0]),		// B
													(pS[pos+3]));		// Alpha
						//*pD = CIFindIndex(tempword);
						*(pD+(j^3)) = RevTlutTable[tempword];
					}
				}
			}
			else
			{
				//DebuggerAppendMsg("Copy %sb FrameBuffer to Rdram, not implemented", pszImgSize[siz]);
			}
			DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Copy %sb FrameBuffer to Rdram", pszImgSize[siz]);});
		}
		backBuffer->UnlockRect();
	}

	if( surf == NULL )
		backBuffer->Release();
}

void CopyBackToFrameBufferIfReadByCPU(u32 addr)
{
	int i = FindRecentCIInfoIndex(addr);
	if( i != -1 )
	{
		//if( i == 0 ) CGraphicsContext::Get()->SwapBuffer();
		RecentCIInfo *info = g_uRecentCIInfoPtrs[i];
		CopyBackToFrameBuffer( info->dwAddr, info->dwFormat, info->dwSize, info->dwWidth, info->dwHeight);
		TRACE1("Copy back for CI Addr=%08X", info->dwAddr);
	}
}


CTexture *pfakeBuffer = NULL;
extern DWORD dwTvSystem;
LPDIRECT3DSURFACE8 pBackBufferSurf = NULL;
LPDIRECT3DSURFACE8 pBackDepthBufferSurf = NULL;
LPDIRECT3DSURFACE8 pfakeBufferSurf = NULL;
LPDIRECT3DSURFACE8 pfakeDepthBufferSurf = NULL;

void SetFakeFrameBuffer(void)
{
	memcpy(&(g_FakeFrameBufferInfo.CI_Info), &g_CI, sizeof(SetImgInfo));

	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	DWORD dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC);
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+8);
	DWORD dwCmd4 = *(DWORD *)(g_pu8RamBase + dwPC+12);

	g_FakeFrameBufferInfo.width = g_FakeFrameBufferInfo.CI_Info.dwWidth;
	g_bFakeCIUpdated = true;
	if( (dwCmd3>>24) == G_SETSCISSOR )
	{
		g_FakeFrameBufferInfo.height   = ((dwCmd4>>0 )&0xFFF)/4;
	}
	else if( (dwCmd1>>24) == G_SETSCISSOR )
	{
		g_FakeFrameBufferInfo.height   = ((dwCmd2>>0 )&0xFFF)/4;
	}
	else if( gRDP.scissor.left == 0 && gRDP.scissor.top == 0 && gRDP.scissor.right == g_FakeFrameBufferInfo.width )
	{
		g_FakeFrameBufferInfo.height = gRDP.scissor.bottom;
	}
	else
	{
		g_FakeFrameBufferInfo.height = g_FakeFrameBufferInfo.width*3/4;
		if( dwTvSystem == TV_SYSTEM_PAL )
		{
			g_FakeFrameBufferInfo.height = g_FakeFrameBufferInfo.width*9/11;
		}

		if( gRDP.scissor.bottom < (int)g_FakeFrameBufferInfo.height && gRDP.scissor.bottom != 0 )
		{
			g_FakeFrameBufferInfo.height = gRDP.scissor.bottom;
		}
	}

	if( g_FakeFrameBufferInfo.CI_Info.dwAddr + g_FakeFrameBufferInfo.height*g_FakeFrameBufferInfo.width*g_FakeFrameBufferInfo.CI_Info.dwSize >= g_dwRamSize )
	{
		TRACE3("Fake frame buffer out of memory bound: Width=%d, Height=%d, Addr=%08X",
			g_FakeFrameBufferInfo.width, g_FakeFrameBufferInfo.height, g_FakeFrameBufferInfo.CI_Info.dwAddr);
		g_bUsingFakeCI = TRUE;
	}

	//g_FakeFrameBufferInfo.width = GetNextPowerOf2(g_FakeFrameBufferInfo.width);
	//g_FakeFrameBufferInfo.height = GetNextPowerOf2(g_FakeFrameBufferInfo.height);
	
	///if( g_CI.dwSize == G_IM_SIZ_16b )
	{
		// Need to create a D3D surface as fake render target
		// then render all primitive into this fake render target

		//if( g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_8b && currentRomOptions.N64FrameBufferEmuType != FRM_COMPLETE )
		//{
		//	g_bUsingFakeCI = TRUE;
		//	DEBUGGER_IF_DUMP(pauseAtNext,{TRACE0("Skip 8b Fake Framebuffer");});
		//}
		//else 
		if( CDeviceBuilder::GetGeneralDeviceType() != DIRECTX_DEVICE )
		{
			pfakeBufferSurf = NULL;
			//TRACE0("Opengl fake frame buffer is not implemented");
		}
		else 
		{
			if( CDirectXGraphicsContext::IsResultGood(g_pD3DDev->CreateRenderTarget(g_FakeFrameBufferInfo.width, g_FakeFrameBufferInfo.height, D3DFMT_LIN_X8R8G8B8, D3DMULTISAMPLE_NONE, TRUE, &pfakeBufferSurf)) )
			{
				if( options.enableHacks && (IsAddrInRecentFrameBuffers(g_CI.dwAddr) || g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_8b) )
				{
					if( CDirectXGraphicsContext::IsResultGood(g_pD3DDev->CreateDepthStencilSurface(g_FakeFrameBufferInfo.width, g_FakeFrameBufferInfo.height,   D3DFMT_LIN_D24S8 , D3DMULTISAMPLE_NONE, &pfakeDepthBufferSurf)) )
					{
						SetScreenMult(1, 1);
						g_pD3DDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBufferSurf);
						g_pD3DDev->GetDepthStencilSurface(&pBackDepthBufferSurf);
						g_pD3DDev->SetRenderTarget(pfakeBufferSurf, pfakeDepthBufferSurf);

						if( currentRomOptions.N64FrameBufferEmuType == FRM_COMPLETE )
						{
							CDaedalusRender::GetRender()->DrawFakeFrameBufferIntoD3DRenderTarget();
						}
					}
					else
					{
						SAFE_RELEASE(pfakeBufferSurf)
						pfakeDepthBufferSurf = NULL;
						g_bUsingFakeCI = FALSE;
						TRACE0("D3D Error: Can not create fake depth frame buffer");
					}
				}
				else
				{
					SetScreenMult(1, 1);
					g_pD3DDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBufferSurf);
					g_pD3DDev->SetRenderTarget(pfakeBufferSurf, NULL);
					pfakeDepthBufferSurf = NULL;
					pBackDepthBufferSurf = NULL;

					if( currentRomOptions.N64FrameBufferEmuType == FRM_COMPLETE )
					{
						CDaedalusRender::GetRender()->DrawFakeFrameBufferIntoD3DRenderTarget();
					}
				}
			}
			else
			{
				pfakeBufferSurf = NULL;
				g_bUsingFakeCI = FALSE;
				TRACE0("D3D Error: Can not create fake frame buffer");
			}
		}
	}
	
	DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Using Fake Framebuffer at %08X", g_FakeFrameBufferInfo.CI_Info.dwAddr);});
	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FAKE_FRAME_BUFFER, 
		{DebuggerAppendMsg("Paused after setting fake frame buffer:\nAddr: 0x%08x, Fmt: %s Size: %s Width: %d",
			g_CI.dwAddr, pszImgFormat[g_CI.dwFormat], pszImgSize[g_CI.dwSize], g_CI.dwWidth);});
}


void SaveFakeFrameBuffer(void)
{
	g_bUsingFakeCI = FALSE;
	DIRECTX_ONLY;

	// Ok, we are using fake render target right now
	// Need to copy content from the fake render target back to frame buffer
	// then reset the current render target

	// Here we need to copy the content from the fake frame buffer to RDRAM memory

	//if( g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_8b && currentRomOptions.N64FrameBufferEmuType != FRM_COMPLETE  )
	//{
	//	g_bUsingFakeCI = FALSE;
	//}
	//else
	{
		g_pD3DDev->SetRenderTarget(pBackBufferSurf, pBackDepthBufferSurf);
		SAFE_RELEASE(pBackBufferSurf);
		SAFE_RELEASE(pBackDepthBufferSurf);

#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_FAKE_FRAME_BUFFER )
		{
			((D3DRender*)(CDaedalusRender::g_pRender))->SaveSurfaceToFile(pfakeBufferSurf);
		}
#endif

		if( currentRomOptions.N64FrameBufferEmuType >= FRM_FASTER && g_FakeFrameBufferInfo.CI_Info.dwSize != G_IM_SIZ_8b )
		{
			CopyBackToFrameBuffer(g_FakeFrameBufferInfo.CI_Info.dwAddr, g_FakeFrameBufferInfo.CI_Info.dwFormat, 
				g_FakeFrameBufferInfo.CI_Info.dwSize, g_FakeFrameBufferInfo.width, g_FakeFrameBufferInfo.height, g_FakeFrameBufferInfo.width, D3DFMT_A8R8G8B8, pfakeBufferSurf);
		}
		
		SAFE_RELEASE(pfakeBufferSurf);
		SAFE_RELEASE( pfakeDepthBufferSurf );
		SetScreenMult(windowSetting.uDisplayWidth/windowSetting.fViWidth, windowSetting.uDisplayHeight/windowSetting.fViHeight);
	}

	DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Saving Fake Framebuffer at %08X", g_FakeFrameBufferInfo.CI_Info.dwAddr);});
	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FAKE_FRAME_BUFFER, 
	{
		DebuggerAppendMsg("Paused after saving fake frame buffer:\nAddr: 0x%08x, Fmt: %s Size: %s Width: %d",
			g_FakeFrameBufferInfo.CI_Info.dwAddr, pszImgFormat[g_FakeFrameBufferInfo.CI_Info.dwFormat], pszImgSize[g_FakeFrameBufferInfo.CI_Info.dwSize], g_FakeFrameBufferInfo.CI_Info.dwWidth);

		// Dump the fake frame buffer
	});
}


void CloseFakeFrameBufferWithoutSave(void)
{
	DIRECTX_ONLY;

	// Ok, we are using fake render target right now
	// Need to copy content from the fake render target back to frame buffer
	// then reset the current render target

	// Here we need to copy the content from the fake frame buffer to RDRAM memory

	if( g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_8b && currentRomOptions.N64FrameBufferEmuType != FRM_COMPLETE  )
	{
		g_bUsingFakeCI = FALSE;
	}
	else
	{
		g_pD3DDev->SetRenderTarget(pBackBufferSurf, pBackDepthBufferSurf);
		SAFE_RELEASE(pBackBufferSurf);
		SAFE_RELEASE(pBackDepthBufferSurf);

		TRACE0("Closing Fake Framebuffer without save");
	
		g_bUsingFakeCI = FALSE;
		SAFE_RELEASE(pfakeBufferSurf);
		SAFE_RELEASE( pfakeDepthBufferSurf );
		SetScreenMult(windowSetting.uDisplayWidth/windowSetting.fViWidth, windowSetting.uDisplayHeight/windowSetting.fViHeight);
	}
}

DWORD dwOldCIAddr;
void DLParser_SetCImg(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwFmt   = (dwCmd0>>21)&0x7;
	DWORD dwSiz   = (dwCmd0>>19)&0x3;
	DWORD dwWidth = (dwCmd0&0x0FFF) + 1;
	DWORD dwNewAddr = RDPSegAddr(dwCmd1);

	if( dwFmt == G_IM_FMT_YUV || dwFmt == G_IM_FMT_IA )
	{
		DebuggerAppendMsg("Check me:  SetCImg Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
	}

	DL_PF("    Image: 0x%08x", RDPSegAddr(dwCmd1));
	DL_PF("    Fmt: %s Size: %s Width: %d",
		pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);

	if( g_CI.dwAddr == dwNewAddr )	// Check against current CI
	{
		if( g_bUsingFakeCI )
		{
			if( g_CI.dwFormat != dwFmt || g_CI.dwSize != dwSiz || g_CI.dwWidth != dwWidth )
			{
				SaveFakeFrameBuffer();
				goto step2;
			}
		}
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
		CGraphicsContext::Get()->SwapBuffer();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st CI change");});
	}

step2:
	if( g_CI.dwAddr == g_DI.dwAddr )	// Is current CI pointed to DI?
	{
		if( dwOldCIAddr == dwNewAddr )
		{
			// Set CI Back to old value
			g_CI.dwAddr = dwNewAddr;
			g_CI.dwFormat = dwFmt;
			g_CI.dwSize = dwSiz;
			g_CI.dwWidth = dwWidth;
			DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
			{
				DebuggerAppendMsg("Pause after SetCImg, was pointed to ZImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
					g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
			}
			);
			return;
		}
	}

	if( dwNewAddr == g_DI.dwAddr )	// Check against current DI, DI usually never changes
	{
		dwOldCIAddr = g_CI.dwAddr;
		g_CI.dwAddr = dwNewAddr;
		g_CI.dwFormat = dwFmt;
		g_CI.dwSize = dwSiz;
		g_CI.dwWidth = dwWidth;

		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
		{
			DebuggerAppendMsg("Pause after SetCImg == ZImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
		}
		);
		return;
	}

	g_CI.dwAddr = dwNewAddr;
	g_CI.dwFormat = dwFmt;
	g_CI.dwSize = dwSiz;
	g_CI.dwWidth = dwWidth;

	CGraphicsContext::Get()->SetBackBuffer(dwNewAddr);

	bool isfakebuffer = ( (g_CI.dwSize != G_IM_SIZ_16b || g_CI.dwFormat != G_IM_FMT_RGBA || ( g_CI.dwWidth <150 /*!= (*g_GraphicsInfo.VI_WIDTH_RG&0xFFF)&& ((*g_GraphicsInfo.VI_WIDTH_RG&0xFFF) < 0x300)*/  && (g_CI.dwWidth != 512))) );
	if( !isfakebuffer )
	{
		if( !IsAddrUsedByVIorigin(dwNewAddr) ) 
			isfakebuffer = true;
	}
#ifdef _DEBUG
	else if( IsAddrUsedByVIorigin(dwNewAddr) )
	{
		DebuggerAppendMsg("Check me, this is not a fake frame buffer at %08X", dwNewAddr);
	}
#endif

	if( currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_CI_CHANGE && !isfakebuffer )
	{
		if( status.curRenderBuffer == NULL )
		{
			status.curRenderBuffer = g_CI.dwAddr;
		}
		else if( status.curRenderBuffer != g_CI.dwAddr )
		{
			status.curDisplayBuffer = status.curRenderBuffer;
			CGraphicsContext::Get()->SwapBuffer();
			status.curRenderBuffer = g_CI.dwAddr;
			DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Screen Update because CI change to %08X, Display Buf=%08X", status.curRenderBuffer, status.curDisplayBuffer);});
		}
	}

	if( currentRomOptions.N64FrameBufferEmuType == FRM_DISABLE )
	{
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
		{
			DebuggerAppendMsg("Pause after SetCImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
		}
		);
		return;
	}


	switch( currentRomOptions.N64FrameBufferEmuType )
	{
	case FRM_FASTER:
	case FRM_COMPLETE:
	case FRM_FASTER_RECHECK:
	case FRM_COPY_AND_BACK:
	case FRM_WITH_EMULATOR:
	case FRM_IGNORE:
		if( g_bUsingFakeCI )
		{
#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_FAKE_FRAME_BUFFER )
			{
				SaveFakeFrameBuffer();
				pauseAtNext = TRUE;
				eventToPause = NEXT_FAKE_FRAME_BUFFER;
			}
			else
			{
				SaveFakeFrameBuffer();
			}
#else
			SaveFakeFrameBuffer();
#endif
			g_bUsingFakeCI = false;
		}

		if( isfakebuffer )
		{
			g_bUsingFakeCI = true;
			SetFakeFrameBuffer();

			//DEBUGGER_IF_DUMP(pauseAtNext, {DebuggerAppendMsg("Warning SetCImg: new Addr=0x%08X, fmt:%s size=%sb, Width=%d\n", 
			//	g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
			//});
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
		}
		break;
	case FRM_COPY:
		UpdateRecentCIAddr(g_CI);
		if( status.curRenderBuffer != g_CI.dwAddr )
		{
			if( currentRomOptions.N64FrameBufferEmuType == FRM_COPY )
				CopyBackToFrameBuffer(status.curRenderBuffer, dwFmt, dwSiz, windowSetting.uViWidth, windowSetting.uViHeight);
			status.curDisplayBuffer = status.curRenderBuffer;
			status.curRenderBuffer = g_CI.dwAddr;
			//DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Screen Update because CI change to %08X, Display Buf=%08X", status.curRenderBuffer, status.curDisplayBuffer);});
		}
		break;
	}

	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
	{
		DebuggerAppendMsg("Pause after SetCImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
			g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
	}
	);
	DEBUGGER_IF_DUMP((pauseAtNext && (eventToPause==NEXT_FAKE_FRAME_BUFFER)), 
	{
		DebuggerAppendMsg("SetCImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
			g_CI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
	}
	);
}




extern DWORD dwTvSystem;
void TexRectToFrameBuffer_8b(u32 dwXL, u32 dwYL, u32 dwXH, u32 dwYH, float t0u0, float t0v0, float t0u1, float t0v1, u32 dwTile)
{
	// Copy the texture into the fake framebuffer memory
	// Used in Yoshi

	/*
	u32 maxW = g_FakeFrameBufferInfo.CI_Info.dwWidth;
	u32 maxH = maxW*3/4;
	if( dwTvSystem == TV_SYSTEM_PAL )
	{
		maxH = maxW*9/11;
	}
	*/

	u32 maxW = g_FakeFrameBufferInfo.width;
	u32 maxH = g_FakeFrameBufferInfo.height;

	u32 maxOff = maxW*maxH;
	
	TMEMLoadMapInfo &info = g_tmemLoadAddrMap[gRDP.tiles[dwTile].dwTMem];
	u32 dwWidth = dwXH-dwXL;
	u32 dwHeight = dwYH-dwYL;

	float xScale = (t0u1-t0u0)/dwWidth;
	float yScale = (t0v1-t0v0)/dwHeight;

	BYTE* dwSrc = g_pu8RamBase + info.dwLoadAddress;
	BYTE* dwDst = g_pu8RamBase + g_FakeFrameBufferInfo.CI_Info.dwAddr;

	u32 dwSrcPitch = gRDP.tiles[dwTile].dwPitch;
	u32 dwDstPitch = g_FakeFrameBufferInfo.CI_Info.dwWidth;

	u32 dwSrcOffX = gRDP.tiles[dwTile].hilite_sl;
	u32 dwSrcOffY = gRDP.tiles[dwTile].hilite_tl;

	u32 dwLeft = dwXL;
	u32 dwTop = dwYL;

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
	
	DEBUGGER_IF_DUMP((pauseAtNext && eventToPause==NEXT_FAKE_FRAME_BUFFER),
		{
			DebuggerAppendMsg("TexRect To FrameBuffer: X0=%d, Y0=%d, X1=%d, Y1=%d,\n\t\tfS0=%f, fT0=%f, fS1=%f, fT1=%f ",
			dwXL, dwYL, dwXH, dwYH, t0v0, t0v0, t0u1, t0v1);
		}
	);
}

void TexRectToN64FrameBuffer_16b(u32 x0, u32 y0, u32 width, u32 height, u32 dwTile)
{
	// Copy the texture into the N64 RDRAM framebuffer memory

	DrawInfo srcInfo;	
	if( g_textures[dwTile].m_pCTexture->StartUpdate(&srcInfo) == false )
	{
		DebuggerAppendMsg("Fail to lock texture:TexRectToN64FrameBuffer_16b" );
		return;
	}

	u32 n64CIaddr = g_CI.dwAddr;
	u32 n64CIwidth = g_CI.dwWidth;

	for (DWORD y = 0; y < height; y++)
	{
		DWORD* pSrc = (DWORD*)((BYTE*)srcInfo.lpSurface + y * srcInfo.lPitch);
		WORD* pN64Buffer = (WORD*)(g_pu8RamBase+(n64CIaddr&(g_dwRamSize-1)))+(y+y0)*n64CIwidth;
		
		for (DWORD x = 0; x < width; x++)
		{
			pN64Buffer[x+x0] = ConvertRGBATo555(pSrc[x]);
		}
	}

	g_textures[dwTile].m_pCTexture->EndUpdate(&srcInfo);
}

u32 FrameBufferInRDRAMCalculateCRC(u32 addr, u32 width, u32 height, u32 size, u32 pitch)
{
	u32 crc=0;

	u32 len = height*pitch*size;
	if( size == G_IM_SIZ_4b ) len = (height*pitch)>>1;

	len >>= 2;

	DWORD *frameBufferBase = (DWORD*)(g_pu8RamBase+RDPSegAddr(addr));


	for( u32 i=0; i<len; i++ )
	{
		crc += frameBufferBase[i];
	}

	return crc;
}


bool FrameBufferInRDRAMCheckCRC()
{
	RecentCIInfo &p = *(g_uRecentCIInfoPtrs[0]);
	u32 crc = FrameBufferInRDRAMCalculateCRC(p.dwAddr, p.dwWidth, p.dwHeight, p.dwSize, p.dwWidth);
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


void ClearFrameBufferToBlack(DWORD left, DWORD top, DWORD width, DWORD height)
{
	RecentCIInfo &p = *(g_uRecentCIInfoPtrs[0]);
	WORD *frameBufferBase = (WORD*)(g_pu8RamBase+RDPSegAddr(p.dwAddr));
	u32 pitch = p.dwWidth;

	if( width == 0 || height == 0 )
	{
		u32 len = p.dwHeight*p.dwWidth*p.dwSize;
		if( p.dwSize == G_IM_SIZ_4b ) len = (p.dwHeight*p.dwWidth)>>1;
		memset(frameBufferBase, 0, len);
	}
	else
	{
		for( u32 y=0; y<height; y++)
		{
			for( u32 x=0; x<width; x++ )
			{
				*(frameBufferBase+(y+top)*pitch+x+left) = 0;
			}
		}
	}
}

extern std::vector<u32> frameWriteRecord;
void FrameBufferWriteByCPU(u32 addr, u32 size)
{
	if( currentRomOptions.N64FrameBufferEmuType != FRM_WITH_EMULATOR )	return;
	//DebuggerAppendMsg("Frame Buffer Write, addr=%08X, CI Addr=%08X", addr, g_CI.dwAddr);
	status.frameWriteByCPU = TRUE;
	frameWriteRecord.push_back(addr&(g_dwRamSize-1));
}

extern RECT frameWriteByCPURect;

bool ProcessFrameWriteRecord()
{
	int size = frameWriteRecord.size();
	if( size == 0 ) return false;

	int index = FindRecentCIInfoIndex(frameWriteRecord[0]);
	if( index == -1 )
	{
		TRACE1("Frame Buffer Write to non-record addr = %08X", frameWriteRecord[0]);
		frameWriteRecord.clear();
		return false;
	}
	else
	{
		u32 base = g_uRecentCIInfoPtrs[index]->dwAddr;
		u32 uwidth = g_uRecentCIInfoPtrs[index]->dwWidth;
		u32 uheight = g_uRecentCIInfoPtrs[index]->dwHeight;
		u32 upitch = uwidth<<1;

		frameWriteByCPURect.left=uwidth-1;
		frameWriteByCPURect.top = uheight-1;

		frameWriteByCPURect.right=0;
		frameWriteByCPURect.bottom = 0;

		int x, y, off;

		for( int i=0; i<size; i++ )
		{
			off = frameWriteRecord[i]-base;
			if( off < g_uRecentCIInfoPtrs[index]->dwMemSize )
			{
				y = off/upitch;
				x = (off - y*upitch)>>1;

				if( x < frameWriteByCPURect.left )	frameWriteByCPURect.left = x;
				if( x > frameWriteByCPURect.right ) frameWriteByCPURect.right = x;
				if( y < frameWriteByCPURect.top )	frameWriteByCPURect.top = y;
				if( y > frameWriteByCPURect.bottom ) frameWriteByCPURect.bottom = y;
			}
		}

		frameWriteRecord.clear();
		DebuggerAppendMsg("Frame Buffer Write: Left=%d, Top=%d, Right=%d, Bottom=%d", frameWriteByCPURect.left,
			frameWriteByCPURect.top, frameWriteByCPURect.right, frameWriteByCPURect.bottom);
		return true;
	}
}

void FrameBufferReadByCPU( u32 addr )
{
	if( currentRomOptions.N64FrameBufferEmuType != FRM_WITH_EMULATOR )	return;
	
	addr &= (g_dwRamSize-1);
	int index = FindRecentCIInfoIndex(addr);
	if( index == -1 ) return;
	if( status.gRDPFrame - g_uRecentCIInfoPtrs[index]->lastUsedFrame > 3 )
	{
		// Ok, we don't have this frame anymore
		return;
	}

	if( g_uRecentCIInfoPtrs[index]->bCopied )	return;

	if( currentRomOptions.N64FrameBufferEmuType != FRM_DISABLE && status.frameReadByCPU == FALSE )
	{
		TRACE1("Frame Buffer read, reported by emulator, addr=%08X", addr);
		//CopyBackToFrameBufferIfReadByCPU(addr&0x7FFFFF);
		CheckAndUpdateFrameBuffer(addr);
		//status.frameReadByCPU = TRUE;

		DEBUGGER_IF_DUMP(pauseAtNext,{TRACE0("Frame Buffer read");});
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FAKE_FRAME_BUFFER, 
			{DebuggerAppendMsg("Paused after setting Frame Buffer read:\n Cur CI Addr: 0x%08x, Fmt: %s Size: %s Width: %d",
				g_CI.dwAddr, pszImgFormat[g_CI.dwFormat], pszImgSize[g_CI.dwSize], g_CI.dwWidth);});
	}
}