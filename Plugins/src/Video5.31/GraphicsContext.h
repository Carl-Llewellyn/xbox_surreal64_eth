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

enum ClearFlag
{
	CLEAR_COLOR_BUFFER=0x01,
	CLEAR_DEPTH_BUFFER=0x02,
	CLEAR_COLOR_AND_DEPTH_BUFFER=0x03,
};


typedef struct
{
	uint32	addr;	//N64 RDRAM address
	uint32	size;	//N64 buffer size
	uint32  format;	//N64 format
	uint32	width;
	uint32	height;
} TextureBufferInfo;


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

	virtual void SaveBackBuffer() {}			// Copy the current back buffer to temp buffer
	virtual void SaveNextBackBuffer();			// Copy the current back buffer to temp buffer
	virtual void LoadTextureFromSavedBackBuffer(TextureEntry* pEntry, uint32 ciInfoIdx) {}
	virtual bool ActivateSelfRenderTextureBuffer() {return false;}		// Use (and create) a self-render-texture buffer 
															// according to the current fake buffer setting
	virtual void RestoreNormalBackBuffer() {}				// restore the normal back buffer
	virtual void LoadTextureFromTextureBuffer(TextureEntry* pEntry, uint32 ciInfoIdx) {}
	virtual bool IsSelfRenderTextureBufferActive() {return false;}


#ifdef _DEBUG
	virtual void DisplayBackBuffer() {}
	virtual void DisplayTextureBuffer() {}
#endif

	static void InitWindowInfo();

	bool m_supportTextureMirror;
	TextureBufferInfo m_textureColorBufferInfo;
	TextureBufferInfo m_textureDepthBufferInfo;

public:
	static	HWND		m_hWnd;
	static	HWND		m_hWndStatus;
	static	HWND		m_hWndToolbar;
	static	HMENU		m_hMenu;

protected:
    static	DWORD      m_dwWindowStyle;     // Saved window style for mode switches
    static	DWORD      m_dwWindowExStyle;   // Saved window style for mode switches
	static	DWORD      m_dwStatusWindowStyle;     // Saved window style for mode switches

	bool				m_backBufferIsSaved;
	bool				m_ToSaveNextBackBuffer;

	bool				m_bReady;
	bool				m_bActive;
	
	bool				m_bWindowed;
	RECT				m_rcWindowBounds;

	//DEVMODEA m_DMsaved;
	
    char				m_strDeviceStats[90];

	virtual ~CGraphicsContext() {};
	CGraphicsContext();
	
	void UpdateFrameBufferBeforeUpdateFrame();

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
extern FakeFrameBufferInfo g_FakeFrameBufferInfo;

#endif
