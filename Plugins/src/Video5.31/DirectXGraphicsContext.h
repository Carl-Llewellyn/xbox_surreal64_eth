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

#ifndef _DX_GRAPHICS_CONTEXT_H_
#define _DX_GRAPHICS_CONTEXT_H_


//-----------------------------------------------------------------------------
// Error codes
//-----------------------------------------------------------------------------
enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_RESIZEFAILED        0x8200000c




//-----------------------------------------------------------------------------
// Name: struct D3DModeInfo
// Desc: Structure for holding information about a display mode
//-----------------------------------------------------------------------------
struct D3DModeInfo
{
    DWORD      Width;      // Screen width in this mode
    DWORD      Height;     // Screen height in this mode
    D3DFORMAT  Format;     // Pixel format in this mode
    DWORD      dwBehavior; // Hardware / Software / Mixed vertex processing
    D3DFORMAT  DepthStencilFormat; // Which depth/stencil format to use with this mode
};




//-----------------------------------------------------------------------------
// Name: struct D3DDeviceInfo
// Desc: Structure for holding information about a Direct3D device, including
//       a list of modes compatible with this device
//-----------------------------------------------------------------------------
struct D3DDeviceInfo
{
    // Device data
    D3DDEVTYPE   DeviceType;      // Reference, HAL, etc.
    D3DCAPS8     d3dCaps;         // Capabilities of this device
    const TCHAR* strDesc;         // Name of this device
    BOOL         bCanDoWindowed;  // Whether this device can work in windowed mode
	
    // Modes for this device
    DWORD        dwNumModes;
    D3DModeInfo  modes[150];
	
    // Current state
    DWORD        dwCurrentMode;
    BOOL         bWindowed;
    D3DMULTISAMPLE_TYPE MultiSampleType;
};




//-----------------------------------------------------------------------------
// Name: struct D3DAdapterInfo
// Desc: Structure for holding information about an adapter, including a list
//       of devices available on this adapter
//-----------------------------------------------------------------------------
struct D3DAdapterInfo
{
    // Adapter data
    D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
    D3DDISPLAYMODE d3ddmDesktop;      // Desktop display mode for this adapter
	
    // Devices for this adapter
    DWORD          dwNumDevices;
    D3DDeviceInfo  devices[5];
	
    // Current state
    DWORD          dwCurrentDevice;
};

extern BufferSettingInfo DirectXRenderBufferSettings[];
extern BufferSettingInfo DirectXDepthBufferSetting[];
extern BufferSettingInfo DirectXCombinerSettings[];
extern int numberOfDirectXRenderBufferSettings;
extern int numberOfDirectXCombinerSettings;
extern int numberOfDirectXDepthBufferSettings;


// Implementation
class CDirectXGraphicsContext : public CGraphicsContext
{
public:
	virtual ~CDirectXGraphicsContext();
	
	
	bool Initialize(HWND hWnd, HWND hWndStatus, DWORD dwWidth, DWORD dwHeight, BOOL bWindowed );
	void CleanUp();
	
	void Clear(ClearFlag dwFlags, DWORD color=0xFF000000, float depth=1.0f);
	void UpdateFrame(bool swaponly=false);

	void SaveBackBuffer();				// Copy the current back buffer to temp buffer
	void LoadTextureFromSavedBackBuffer(TextureEntry* pEntry, uint32 ciInfoIdx);
	bool ActivateSelfRenderTextureBuffer();		// Use (and create) a self-render-texture buffer 
	void RestoreNormalBackBuffer();				// restore the normal back buffer
	bool IsSelfRenderTextureBufferActive() {return m_bTextureBufferActive;}
	void LoadTextureFromTextureBuffer(TextureEntry* pEntry, uint32 ciInfoIdx);

#ifdef _DEBUG
	void DisplayBackBuffer();
	void DisplayTextureBuffer();
#endif

	int ToggleFullscreen();		// return 0 as the result is windowed
	D3DFORMAT GetFormat() {return m_desktopFormat;}
	static bool IsResultGood(HRESULT hr, bool displayError = false);

	LPDIRECT3DSURFACE8 m_pBackColorBuffer;
	LPDIRECT3DSURFACE8 m_pBackDepthBuffer;
	LPDIRECT3DSURFACE8 m_pTextureColorBuffer;
	LPDIRECT3DSURFACE8 m_pTextureDepthBuffer;

	bool	m_bTextureBufferActive;

protected:
	friend class DirectXDeviceBuilder;
	CDirectXGraphicsContext();

	BOOL FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
		D3DFORMAT TargetFormat,
		D3DFORMAT* pDepthStencilFormat );

	HRESULT BuildDeviceList();
	HRESULT ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT Format );

	HRESULT DisplayErrorMsg( HRESULT hr, DWORD dwType );
	
	HRESULT Create( BOOL bWindowed );
	HRESULT InitializeD3DEnvironment();

	HRESULT ResizeD3DEnvironment();
	HRESULT DoToggleFullscreen();
	HRESULT ForceWindowed();
	HRESULT AdjustWindowForChange();
	
	void Pause( bool bPause );
	
	HRESULT InitDeviceObjects();
	HRESULT RestoreDeviceObjects();
	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();

	LPDIRECT3D8			m_pD3D;
	LPDIRECT3DDEVICE8	m_pd3dDevice;

    D3DCAPS8			m_d3dCaps;           // Caps for the device
    DWORD				m_dwCreateFlags;     // Indicate sw or hw vertex processing

	D3DPRESENT_PARAMETERS m_d3dpp;

    D3DSURFACE_DESC		m_d3dsdBackBuffer;   // Surface desc of the backbuffer
	
    D3DAdapterInfo		m_Adapters[10];
	DWORD				m_dwNumAdapters;
	
	DWORD				m_dwAdapter;
	
	DWORD				m_dwMinDepthBits;
	DWORD				m_dwMinStencilBits;

	D3DFORMAT			m_desktopFormat;

public:
	LPDIRECT3DSURFACE8	pBackBufferSave;
};


#endif



