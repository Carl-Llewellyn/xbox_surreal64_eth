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

CGraphicsContext* CGraphicsContext::g_pGraphicsContext = NULL;
bool CGraphicsContext::needCleanScene = false;
CGraphicsContext * CGraphicsContext::Get(void)
{	
	return CGraphicsContext::g_pGraphicsContext;
	}
	
CGraphicsContext::CGraphicsContext() :
	m_bReady(false), 
	m_bActive(false),
	m_bWindowed(false),
	m_hWnd(NULL),			// TODO: Assign window handle somewhere
	m_hWndFocus(NULL),		// TODO: Assign window handle somewhere
	m_hMenu(NULL),
	
	m_dwChosenWidth(640),  // This is a default value, its' changed on Initialization. 
	m_dwChosenHeight(480), // Shouldn't need to change these
	
	
    m_bShowCursorWhenFullscreen(FALSE),
	m_supportTextureMirror(false)
{
	m_numOfBackBuffers = 1;
}

DWORD statusBarHeight = 0;
DWORD statusBarHeightToUse = 0;
DWORD toolbarHeight = 0;
DWORD toolbarHeightToUse = 0;

bool CGraphicsContext::Initialize(HWND hWnd, HWND hWndStatus, DWORD dwWidth, DWORD dwHeight, BOOL bWindowed )
{
	m_hWnd = hWnd;
	m_hWndStatus = hWndStatus;
	 
	
	if( windowSetting.bDisplayFullscreen )
	{
		m_dwChosenWidth = windowSetting.uFullScreenDisplayWidth;
		m_dwChosenHeight= windowSetting.uFullScreenDisplayHeight;
	}
	else
	{
		m_dwChosenWidth = windowSetting.uWindowDisplayWidth;
		m_dwChosenHeight= windowSetting.uWindowDisplayHeight;
	}
	
	windowSetting.uCurrentDisplayWidth = (WORD)m_dwChosenWidth;
	windowSetting.uCurrentDisplayHeight = (WORD)m_dwChosenHeight;
 

	RECT rcScreen;

	SetRect(&rcScreen, 0,0, m_dwChosenWidth, m_dwChosenHeight);

 
	// Add extra margin for the status bar
	statusBarHeight = 0;
 
	statusBarHeightToUse = statusBarHeight;

	toolbarHeight = 0;
 
	toolbarHeightToUse = toolbarHeight;
	rcScreen.bottom += toolbarHeight;

 

	m_rcWindowClientSrc.left = 0;
	m_rcWindowClientSrc.top = 0;
	m_rcWindowClientSrc.right = m_rcWindowClient.right;
	m_rcWindowClientSrc.bottom = m_rcWindowClient.bottom - m_rcWindowClient.top;
	
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
void CGraphicsContext::UpdateFrameBufferBeforeUpdateFrame()
{
	if( (currentRomOptions.N64FrameBufferEmuType == FRM_WITH_EMULATOR && status.frameWriteByCPU ) ||
		(currentRomOptions.N64FrameBufferEmuType == FRM_COPY_AND_BACK && !FrameBufferInRDRAMCheckCRC() ) )		
		// Checks if frame buffer has been modified by CPU
		// Only happens to Dr. Mario
	{
		if( currentRomOptions.N64FrameBufferEmuType == FRM_WITH_EMULATOR )
		{
			if( ProcessFrameWriteRecord() )
			{
				CDaedalusRender::GetRender()->DrawFrameBuffer(false, frameWriteByCPURect.left, frameWriteByCPURect.top,
					frameWriteByCPURect.right-frameWriteByCPURect.left, frameWriteByCPURect.bottom-frameWriteByCPURect.top);
				ClearFrameBufferToBlack(frameWriteByCPURect.left, frameWriteByCPURect.top,
					frameWriteByCPURect.right-frameWriteByCPURect.left, frameWriteByCPURect.bottom-frameWriteByCPURect.top);
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


/*
 *	Swap chain management
 */
extern int g_DlistCount;

void CGraphicsContext::SetFrontBuffer(DWORD viorg)	// Change front buffer to the viorg addr
{
	u32 i;
	//UpdateFrame();

	if( !m_bFrontBuffersReady )
	{
		// Doing nothing but initialize the swapchain backbuffers
		for( i=0; i<m_numOfBackBuffers; i++)
		{
			if( m_SwapChainInfos[i].viorg_addr != 0 && m_SwapChainInfos[i].used )
			{
				if( m_SwapChainInfos[i].viorg_addr == viorg )
					return;
				else
					continue;
			}
			else
			{
				break;
			}
		}

		if( i<m_numOfBackBuffers )
		{
			m_SwapChainInfos[i].viorg_addr = viorg;
		}

		if( i>=1 )
		{
			m_bFrontBuffersReady = true;
		}
	}
	else
	{
		// Search a backbuffer by using the viorg, and send this backbuffer
		// as the current render buffer

		/*
		for( i=0; i<m_numOfBackBuffers && m_SwapChainInfos[i].initialized; i++ )
		{
			if( m_SwapChainInfos[i].viorg_addr == viorg )
			{
				// Selecting front buffer #i
				return;
			}
		}
		*/

		for( i=0; i<m_numOfBackBuffers && m_SwapChainInfos[i].initialized; i++ )
		{
			if( (viorg<m_SwapChainInfos[i].ciimg_addr && (m_SwapChainInfos[i].ciimg_addr-viorg)<0x400 ) || 
				(viorg>m_SwapChainInfos[i].ciimg_addr && (viorg-m_SwapChainInfos[i].ciimg_addr)<0x400 ) )
			{
				m_SwapChainInfos[i].viorg_addr = viorg;
				m_currentFrontBufferIdx = i;
				// Selecting front buffer #i
				return;
			}
		}

		if( i<m_numOfBackBuffers )
		{
			m_SwapChainInfos[i].viorg_addr = viorg;
			m_currentFrontBufferIdx = i;
			return;
		}

		// up to here, I can not find a matching record, this is a new CI addr
		u32 j=0;
		u32 oldestDlistCount=0xFFFFFFFF;

		for( i=0; i<m_numOfBackBuffers; i++ )
		{
			if( m_SwapChainInfos[i].lastUsedDListCount < oldestDlistCount )
			{
				oldestDlistCount = m_SwapChainInfos[i].lastUsedDListCount;
				j=i;
			}
		}

		m_SwapChainInfos[j].viorg_addr = viorg;
		m_currentFrontBufferIdx = j;
	}
}

void CGraphicsContext::SetBackBuffer(DWORD ciaddr)	// Change back buffer to the ci addr
{
	u32 i;
	if( !m_bBackBuffersReady )
	{
		// Doing nothing but initialize the swapchain backbuffers
		for( i=0; i<m_numOfBackBuffers; i++)
		{
			if( m_SwapChainInfos[i].viorg_addr != 0 && m_SwapChainInfos[i].used )
			{
				if( m_SwapChainInfos[i].viorg_addr <= ciaddr && (ciaddr-m_SwapChainInfos[i].viorg_addr) < 0x400 )
				{
					if( m_SwapChainInfos[i].ciimg_addr == 0 )
					{
						m_SwapChainInfos[i].ciimg_addr = ciaddr;
						m_SwapChainInfos[i].initialized = true;
						m_SwapChainInfos[i].lastUsedDListCount = g_DlistCount;
					}
					else if( m_SwapChainInfos[i].ciimg_addr != ciaddr )
					{
						TRACE0("Check me, ciaddr is changed a little");
					}
					m_currentBackBufferIdx = i;
					return;
				}
				else
					continue;
			}
			else
			{
				break;
			}
		}

		if( i<m_numOfBackBuffers )
		{
			m_SwapChainInfos[i].ciimg_addr = ciaddr;
			m_SwapChainInfos[i].initialized = true;
			m_SwapChainInfos[i].lastUsedDListCount = g_DlistCount;
			m_currentBackBufferIdx = i;
		}

		if( i>=1 )
		{
			m_bBackBuffersReady = true;
		}
	}
	else
	{
		// Search a backbuffer by using the ciaddr, and send this backbuffer
		// as the current render buffer

		for( i=0; i<m_numOfBackBuffers && m_SwapChainInfos[i].initialized; i++ )
		{
			if( m_SwapChainInfos[i].ciimg_addr == ciaddr )
			{
				// Selecting back buffer #i
				m_SwapChainInfos[i].lastUsedDListCount = g_DlistCount;
				m_currentBackBufferIdx = i;
				return;
			}
		}

		if( i<m_numOfBackBuffers )
		{
			m_SwapChainInfos[i].ciimg_addr = ciaddr;
			m_SwapChainInfos[i].initialized = true;
			m_SwapChainInfos[i].lastUsedDListCount = g_DlistCount;
			m_currentBackBufferIdx = i;
			return;
		}

		// up to here, I can not find a matching record, this is a new CI addr
		u32 j = 0;
		u32 oldestDlistCount=0xFFFFFFFF;

		for( i=0; i<m_numOfBackBuffers; i++ )
		{
			if( !m_SwapChainInfos[i].isFrontBuffer && m_SwapChainInfos[i].lastUsedDListCount < oldestDlistCount )
			{
				oldestDlistCount = m_SwapChainInfos[i].lastUsedDListCount;
				j=i;
			}
		}

		// Find the oldest CI record, and replace it, but not to use the
		// front buffer
		m_SwapChainInfos[j].ciimg_addr = ciaddr;
		m_SwapChainInfos[j].initialized = true;
		m_SwapChainInfos[j].lastUsedDListCount = g_DlistCount;
		m_currentBackBufferIdx = j;
	}
}

void CGraphicsContext::SwapBuffer()
{
#ifdef USE_MY_SWAP_CHAIN
	// Roll the swap chain to the next position
	DWORD lastFrontBuffer = m_currentFrontBufferIdx;

	if( *g_GraphicsInfo.VI_ORIGIN_RG != 0 )
	{
		CGraphicsContext::Get()->SetFrontBuffer(*g_GraphicsInfo.VI_ORIGIN_RG);
	}

	if( m_SwapChainInfos[m_currentFrontBufferIdx].isFrontBuffer )
		return;

	m_SwapChainInfos[lastFrontBuffer].isFrontBuffer = false;
	m_SwapChainInfos[m_currentBackBufferIdx].beingRendered = false;
#ifdef _DEBUG
	if( pauseAtNext && eventToPause == NEXT_FRAME )
	{
		DebuggerAppendMsg("Update screen, last=%d, cur=%d", lastFrontBuffer, m_currentBackBufferIdx);
	}
#endif

	while( lastFrontBuffer !=  m_currentFrontBufferIdx )
	{
		lastFrontBuffer++;
		lastFrontBuffer %= m_numOfBackBuffers;
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_FRAME )
		{
			//UpdateFrame();
			UpdateFrame(lastFrontBuffer !=  m_currentFrontBufferIdx);
			DebuggerAppendMsg("%s screen %d", lastFrontBuffer !=  m_currentFrontBufferIdx?"Skip":"Update",lastFrontBuffer );
			//ErrorMsg("Continue");
		}
		else
		{
			UpdateFrame(lastFrontBuffer !=  m_currentFrontBufferIdx);
		}
#else
		UpdateFrame(lastFrontBuffer !=  m_currentFrontBufferIdx);
#endif
	}

	m_SwapChainInfos[m_currentFrontBufferIdx].isFrontBuffer = true;
	m_currentBackBufferIdx = m_currentFrontBufferIdx+1;
	m_currentBackBufferIdx %= m_numOfBackBuffers;
	m_SwapChainInfos[m_currentBackBufferIdx].beingRendered = true;
	return;

	DWORD nextFrontBuffer = (m_currentFrontBufferIdx+1)%m_numOfBackBuffers;
	DWORD nextBackBuffer = (nextFrontBuffer+1)%m_numOfBackBuffers;
	m_SwapChainInfos[m_currentFrontBufferIdx].isFrontBuffer = false;
	m_SwapChainInfos[nextFrontBuffer].isFrontBuffer = true;
	m_SwapChainInfos[nextFrontBuffer].beingRendered = false;
	m_currentFrontBufferIdx = nextFrontBuffer;

	m_SwapChainInfos[nextBackBuffer].beingRendered = true;
	m_currentBackBufferIdx = nextBackBuffer;
#else
	UpdateFrame();
#endif
}

void CGraphicsContext::InitBackBufferInfoArray(DWORD numOfBuf)
{
	m_numOfBackBuffers = numOfBuf+1;
	for( u32 i=0; i<MAX_BACK_BUFFER; i++)
	{
		m_SwapChainInfos[i].viorg_addr = 0;
		m_SwapChainInfos[i].ciimg_addr = 0;
		m_SwapChainInfos[i].idx = i;
		m_SwapChainInfos[i].used = i<m_numOfBackBuffers?true:false;
		m_SwapChainInfos[i].isFrontBuffer = false;
		m_SwapChainInfos[i].beingRendered = false;
		m_SwapChainInfos[i].initialized = false;
		m_SwapChainInfos[i].lastUsedDListCount = 0;
	}

	m_SwapChainInfos[0].isFrontBuffer = true;
	m_SwapChainInfos[1].beingRendered = true;

	m_currentFrontBufferIdx = 0;
	m_currentBackBufferIdx = 1;
	m_numOfFrontBufferInitialized = 0;
	m_numOfBackBufferInitialized = 0;

	m_bBackBuffersReady = m_bFrontBuffersReady = false;
}