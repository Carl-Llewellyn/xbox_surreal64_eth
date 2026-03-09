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

#ifndef _AFX_GFXCONTEXT_H_
#define _AFX_GFXCONTEXT_H_

#include "CritSect.h"
#include "Unique.h"

typedef struct {
	DWORD	viorg_addr;
	DWORD	ciimg_addr;
	bool	used;
	int		idx;
	bool	isFrontBuffer;
	bool	beingRendered;
	bool	initialized;
	DWORD	lastUsedDListCount;
} BackBuferInfo;

enum ClearFlag
{
	CLEAR_COLOR_BUFFER=0x01,
	CLEAR_DEPTH_BUFFER=0x02,
	CLEAR_COLOR_AND_DEPTH_BUFFER=0x03,
};

#define MAX_BACK_BUFFER	5
///#define USE_MY_SWAP_CHAIN

// This class basically provides an extra level of security for our
// multithreaded code. Threads can Grab the CGraphicsContext to prevent
// other threads from changing/releasing any of the pointers while it is
// running.

// It is based on CCritSect for Lock() and Unlock()

class CGraphicsContext : public CCritSect
{
	friend class CDeviceBuilder;
	
public:
	bool Ready() { return m_bReady; }

	virtual bool Initialize(HWND hWnd, HWND hWndStatus, DWORD dwWidth, DWORD dwHeight, BOOL bWindowed );
	virtual void CleanUp();

	virtual void Clear(ClearFlag flags, DWORD color=0xFF000000, float depth=1.0f) = 0;

	virtual void UpdateFrame(bool swaponly=false) = 0;
	virtual int ToggleFullscreen()=0;		// return 0 as the result is windowed

	virtual void DumpScreenShot() = 0;
	
	virtual void SetFrontBuffer(DWORD viorg);	// Change front buffer to the viorg addr
	virtual void SetBackBuffer(DWORD ciaddr);	// Change back buffer to the ci addr
	virtual void SwapBuffer();					// Present the latest back buffer

	bool m_supportTextureMirror;

protected:
	HWND				m_hWnd;
	HWND				m_hWndStatus;
	HWND				m_hWndFocus;
	HMENU				m_hMenu;
	bool				m_bReady;
	bool				m_bActive;
	
    BOOL m_bShowCursorWhenFullscreen;
	
	DWORD				m_dwChosenWidth;
	DWORD				m_dwChosenHeight;
	
	bool				m_bWindowed;
	
	RECT				m_rcWindowClient;
	RECT				m_rcWindowClientSrc;
	RECT				m_rcWindowBounds;
	//WINDOWPLACEMENT		m_WindowPlacement;

	//DEVMODEA m_DMsaved;
	
    DWORD               m_dwWindowStyle;     // Saved window style for mode switches
    DWORD               m_dwWindowExStyle;   // Saved window style for mode switches
	DWORD               m_dwStatusWindowStyle;     // Saved window style for mode switches

    char				m_strDeviceStats[90];

	// Back buffer swap chain management
	DWORD				m_numOfBackBuffers;
	DWORD				m_currentFrontBufferIdx;
	DWORD				m_currentBackBufferIdx;
	DWORD				m_numOfFrontBufferInitialized;
	DWORD				m_numOfBackBufferInitialized;
	bool				m_bFrontBuffersReady;
	bool				m_bBackBuffersReady;
	BackBuferInfo		m_SwapChainInfos[MAX_BACK_BUFFER];
	
	virtual ~CGraphicsContext() {};
	CGraphicsContext();
	
	void UpdateFrameBufferBeforeUpdateFrame();
	void InitBackBufferInfoArray(DWORD numOfBuf);

public:
	static CGraphicsContext *g_pGraphicsContext;
	static CGraphicsContext * Get(void);
	inline const char* GetDeviceStr() {return m_strDeviceStats;}
	static bool needCleanScene;
};

extern DWORD statusBarHeight;
extern DWORD statusBarHeightToUse;
extern DWORD toolbarHeight;
extern DWORD toolbarHeightToUse;

#endif
