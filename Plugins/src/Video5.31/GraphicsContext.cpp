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
extern bool bEnableHDTV;
CGraphicsContext* CGraphicsContext::g_pGraphicsContext = NULL;
bool CGraphicsContext::needCleanScene = false;
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
	m_ToSaveNextBackBuffer(false)
{
	memset(&m_textureColorBufferInfo,0,sizeof(TextureBufferInfo));
	memset(&m_textureDepthBufferInfo,0,sizeof(TextureBufferInfo));
}


DWORD statusBarHeight = 0;
DWORD statusBarHeightToUse = 0;
DWORD toolbarHeight = 0;
DWORD toolbarHeightToUse = 0;

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

	if(bEnableHDTV){
	windowSetting.uDisplayWidth = 1280;
	windowSetting.uDisplayHeight = 720;
	}else{
	windowSetting.uDisplayWidth = 640;
	windowSetting.uDisplayHeight = 480;
	}
	RECT rcScreen;
	SetRect(&rcScreen, 0,0, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight);
	//rcScreen.bottom += statusBarHeight;
	//rcScreen.bottom += toolbarHeight;

 
	return true;
}

void CGraphicsContext::CleanUp()
{
    m_bActive = false;
    m_bReady  = false;
 
}

bool FrameBufferInRDRAMCheckCRC();
void ClearFrameBufferToBlack(DWORD left=0, DWORD top=0, DWORD width=0, DWORD height=0);
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
							CDaedalusRender::GetRender()->DrawFrameBuffer(false, frameWriteByCPURectArray[i][j].left, frameWriteByCPURectArray[i][j].top,
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
				CDaedalusRender::GetRender()->DrawFrameBuffer(false, frameWriteByCPURect.left, frameWriteByCPURect.top,
					frameWriteByCPURect.right-frameWriteByCPURect.left, frameWriteByCPURect.bottom-frameWriteByCPURect.top);
				ClearFrameBufferToBlack(frameWriteByCPURect.left, frameWriteByCPURect.top,
					frameWriteByCPURect.right-frameWriteByCPURect.left+1, frameWriteByCPURect.bottom-frameWriteByCPURect.top+1);

				/*
				int size = frameWriteByCPURects.size();
				for( int i=0; i<size; i++)
				{
					CDaedalusRender::GetRender()->DrawFrameBuffer(false, frameWriteByCPURects[i].left, frameWriteByCPURects[i].top,
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
			CDaedalusRender::GetRender()->DrawFrameBuffer(false, 0,0,p.dwWidth,p.dwHeight);
			ClearFrameBufferToBlack();
		}
	}
}

void CGraphicsContext::SaveNextBackBuffer()
{
	// We need to save the next backbuffer which is the current front buffer
	m_ToSaveNextBackBuffer = true;
}
