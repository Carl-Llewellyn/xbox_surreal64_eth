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
#include "../../../config.h"

#include <xbapp.h>
#include <xbresource.h>

extern void XboxDrawOSD();
extern int AntiAliasMode;
char emuvidname[128];

LPDIRECT3DDEVICE8 g_pD3DDev = NULL;
D3DCAPS8 g_D3DDeviceCaps;
extern DWORD statusBarHeightToUse;
extern DWORD toolbarHeight;
//D3DPRESENT_PARAMETERS d3dpp;
extern LPDIRECT3DSURFACE8		g_pBackBuffer;
extern LPDIRECT3DSURFACE8		g_pDepthBuffer;
extern void CreateRenderTarget();

/*
 *	Constants
 */
/*
D3DFMT_D16_LOCKABLE         = 70,
D3DFMT_D32                  = 71,
D3DFMT_D15S1                = 73,
D3DFMT_D24S8                = 75,
D3DFMT_D16                  = 80,
D3DFMT_D24X8                = 77,
D3DFMT_D24X4S4              = 79,
*/

BufferSettingInfo DirectXRenderBufferSettings[] =
{
	"Double Buffer Copy",		1,		D3DSWAPEFFECT_COPY,
	"Double Buffer Copy Sync",	1,		D3DSWAPEFFECT_COPY_VSYNC,
	"Double Buffer Flip",		1,		D3DSWAPEFFECT_FLIP,
	//"Double Buffer Discard",	1,		D3DSWAPEFFECT_DISCARD,
	"Triple Buffer Flip",		2,		D3DSWAPEFFECT_FLIP,
	"Quadruple Buffer Flip",	3,		D3DSWAPEFFECT_FLIP,
	//"Triple Buffer Discard",	2,		D3DSWAPEFFECT_DISCARD,
};

BufferSettingInfo DirectXDepthBufferSetting[] =
{
	"Default",					D3DFMT_D16,				D3DFMT_D16,
	"16-bit",					D3DFMT_D16,				D3DFMT_D16,
//	"16-bit signed",			D3DFMT_D15S1,			D3DFMT_D15S1,
	"16-bit lockable",			D3DFMT_D16_LOCKABLE,	D3DFMT_D16_LOCKABLE,
//	"32-bit Depth Buffer",		D3DFMT_D32,				D3DFMT_D32,
	"32-bit signed",			D3DFMT_D24S8,			D3DFMT_D24S8,
//	"32-bit D24X8",				D3DFMT_D24X4S4,			D3DFMT_D24X4S4,
//	"32-bit D24X4S4",			D3DFMT_D24X8,			D3DFMT_D24X8,
};

BufferSettingInfo DirectXCombinerSettings[] =
{
	"To Fit Your Video Card",			DX_BEST_FIT,		DX_BEST_FIT,
	"For Lowest End Video Cards",		DX_LOWEST_END,		DX_LOWEST_END,
	//"For Lower End Video Cards",		DX_LOWER_END,		DX_LOWER_END,
	"For Middle Level Video Cards",		DX_MIDDLE_LEVEL,	DX_MIDDLE_LEVEL,
	"For Higher End Video Card",		DX_HIGHER_END,		DX_HIGHER_END,
	"For NVidia TNT/TNT2/Geforce or better",	DX_NVIDIA_TNT,		DX_NVIDIA_TNT,
	//"For NVidia Geforce2 or better",	DX_NVIDIA_GEFORCE_PLUS,	DX_NVIDIA_GEFORCE_PLUS,
	//"For ATI Radeon or better",		DX_ATI_RADEON_PLUS,	DX_ATI_RADEON_PLUS,
};

int numberOfDirectXRenderBufferSettings = sizeof(DirectXRenderBufferSettings)/sizeof(BufferSettingInfo);
int numberOfDirectXCombinerSettings = sizeof(DirectXCombinerSettings)/sizeof(BufferSettingInfo);
int numberOfDirectXDepthBufferSettings = sizeof(DirectXDepthBufferSetting)/sizeof(BufferSettingInfo);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectXGraphicsContext::CDirectXGraphicsContext() :
	m_pd3dDevice(NULL),
	m_pD3D(NULL),
	m_dwNumAdapters(0),
	m_dwAdapter(0),			// Default Adapter
	m_dwCreateFlags(0),
	m_dwMinDepthBits(16),
	m_dwMinStencilBits(0),
	m_desktopFormat(D3DFMT_A8R8G8B8),
	pCurrentRenderBuffer(NULL)
{
	m_strDeviceStats[0] = '\0';
	m_currentBackBufferIdx = 0;
}

//*****************************************************************************
//
//*****************************************************************************
CDirectXGraphicsContext::~CDirectXGraphicsContext()
{
	CGraphicsContext::Get()->CleanUp();
}

//*****************************************************************************
//
//*****************************************************************************
void CDirectXGraphicsContext::Clear(ClearFlag dwFlags, DWORD color, float depth)
{
	DWORD flag=0;
	if( dwFlags&CLEAR_COLOR_BUFFER )	flag |= D3DCLEAR_TARGET;
	if( dwFlags&CLEAR_DEPTH_BUFFER )	flag |= D3DCLEAR_ZBUFFER;
	Lock();
	if (m_pd3dDevice != NULL)
		m_pd3dDevice->Clear(0, NULL, flag, color, depth, 0);
	Unlock();
}

//*****************************************************************************
//
//*****************************************************************************
bool FrameBufferInRDRAMCheckCRC();
void ClearFrameBufferToBlack(DWORD left=0, DWORD top=0, DWORD width=0, DWORD height=0);
bool ProcessFrameWriteRecord();
extern RECT frameWriteByCPURect;
extern bool bloadstate[MAX_SAVE_STATES];
extern bool bsavestate[MAX_SAVE_STATES];
extern "C" void __EMU_SaveState(int index);
extern "C" void __EMU_LoadState(int index);
extern bool bSatesUpdated;

extern void RenderScreen();
extern void SetAsRenderTarget();
extern LPDIRECT3DSURFACE8 pTargetSurface;
bool draw = true;

__forceinline void CDirectXGraphicsContext::UpdateFrame(bool swaponly)
{

	//HRESULT hr; // unreferenced

	if (bSatesUpdated) {
		bSatesUpdated = false;
		
		for (int i=0; i<MAX_SAVE_STATES; i++) {
			if (bloadstate[i]) {
				__EMU_LoadState(i+1);
				bloadstate[i]=false;
				break;
			}
			else if (bsavestate[i]) {
				__EMU_SaveState(i+1);
				bsavestate[i]=false;
				break;
			}
		}
	}

	CGraphicsContext::UpdateFrameBufferBeforeUpdateFrame();

#ifndef OLDTXTCACHE
	if (!g_bUseSetTextureMem)
		gTextureCache.FreeTextures();
#endif
	
	XboxDrawOSD();
	



/*	Lock();
	if (m_pd3dDevice == NULL)
	{
		hr = E_FAIL;
	}
	else
	{
		if( !currentRomOptions.forceBufferClear )	
				   Clear(CLEAR_DEPTH_BUFFER);
		if( swaponly )
		{
			const RECT rect={0,0,1,1};
			hr = m_pd3dDevice->Present( &rect, &rect, NULL, NULL );
		}
		else {
			hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_FRAME )
		{
			TRACE0("Update screen");
		}
#endif

	}
 
	Unlock();*/

	if( !currentRomOptions.forceBufferClear ){
		Clear(CLEAR_DEPTH_BUFFER);}

	m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	if( currentRomOptions.forceBufferClear )	needCleanScene = true;
}


//*****************************************************************************
//
//*****************************************************************************
void CDirectXGraphicsContext::DumpScreenShot()
{
	 
}

//-----------------------------------------------------------------------------
// Name: SetAntiAliasMode()
// Desc: Surreal64 function to set the antialiasing mode determined by the
//       Launcher. Edge AntiAliasing may work better on the xbox compared to
//	     the FSAA modes. 4x Gaussian is the reccomended then to 2x Quincunx if 
//	     the framerate plunges. Linear modes are also available if those
//	     methods are prefered.
//-----------------------------------------------------------------------------
DWORD SetAntiAliasMode(int AAMode){
	DWORD useAAMode;
	switch (AAMode)
	{
		case 0:
			useAAMode = D3DMULTISAMPLE_NONE;
		break;

		case 1:
			useAAMode = D3DMULTISAMPLE_NONE;
		break;

		case 2:
			useAAMode = D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR;
		break;

		case 3:
			useAAMode = D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX;
		break;

		case 4:
			useAAMode = D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR;
		break;

		case 5:
			useAAMode = D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN;
		break;
	}
	return useAAMode;
}
//-----------------------------------------------------------------------------
// Name: FindDepthStencilFormat()
// Desc: Finds a depth/stencil format for the given device that is compatible
//       with the render target format and meets the needs of the app.
//-----------------------------------------------------------------------------
BOOL CDirectXGraphicsContext::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
											  D3DFORMAT TargetFormat,
											  D3DFORMAT* pDepthStencilFormat )
{

	*pDepthStencilFormat = D3DFMT_D16;
	return TRUE;
}

//*****************************************************************************
//
//*****************************************************************************
extern void WriteConfiguration(void);
extern "C" void _INPUT_LoadButtonMap(int *cfgData); 
extern int ControllerConfig[76];

bool CDirectXGraphicsContext::Initialize(HWND hWnd, HWND hWndStatus,
									 DWORD dwWidth, DWORD dwHeight,
									 BOOL bWindowed )
{
	HRESULT hr;
  
	Lock();

	  // Create the Direct3D object
    m_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
    if( m_pD3D == NULL )
	{
		Unlock();
        DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
		return false;
	}
	
    // Build a list of Direct3D adapters, modes and devices. The
    // ConfirmDevice() callback is used to confirm that only devices that
    // meet the app's requirements are considered.
    if( FAILED( hr = BuildDeviceList() ) )
    {
        if ( m_pD3D )
		{
			m_pD3D->Release();
			m_pD3D = NULL;
		}
		Unlock();
        DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		return false;
    }

	CGraphicsContext::Initialize(hWnd, hWndStatus, dwWidth, dwHeight, bWindowed );
	
	hr = Create( bWindowed );

	// Clear/Update a few times to ensure that all of the buffers are cleared
	if ( m_pd3dDevice )
	{
		for (int i=0; i<DirectXRenderBufferSettings[options.RenderBufferSetting].number+1; i++ )
		{
			Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);
			m_pd3dDevice->BeginScene();
			m_pd3dDevice->EndScene();
			//SwapBuffer();
			m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}
	}

	D3DVIEWPORT8 vp = {
		0, 
			toolbarHeightToUse, 
			windowSetting.uDisplayWidth, 
			windowSetting.uDisplayHeight, 0, 1
	};
	g_pD3DDev->SetViewport(&vp);
	
	Unlock();

	g_pD3DDev->GetDeviceCaps(&g_D3DDeviceCaps);
	CGraphicsContext::Get()->m_supportTextureMirror = false;
	if( g_D3DDeviceCaps.TextureAddressCaps & D3DPTADDRESSCAPS_MIRROR )
	{
		CGraphicsContext::Get()->m_supportTextureMirror = true;
	}
	
	if( g_D3DDeviceCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT || g_D3DDeviceCaps.DevCaps&D3DDEVCAPS_PUREDEVICE )
	{
		// Force to use Software T&L
		//g_pD3DDev->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,TRUE);
	}
  

	// GogoAckman
	g_pd3dDevice = g_pD3DDev;
	sprintf(emuvidname,"Video 5.10");

	_INPUT_LoadButtonMap(ControllerConfig);

	//D3DDevice::GetDepthStencilSurface(&g_pDepthBuffer);

	//CreateRenderTarget();
	return hr==S_OK;
}


//*****************************************************************************
//
//*****************************************************************************
HRESULT CDirectXGraphicsContext::Create( BOOL bWindowed )
{
    HRESULT hr;
	
    // Get access to current adapter, device, and mode
//    D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
//    D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
	
 	m_bWindowed = (bWindowed==TRUE);		// Make user Toggle manually for now!
    //pDeviceInfo->bWindowed = m_bWindowed;

    // Initialize the 3D environment for the app
    if( FAILED( hr = InitializeD3DEnvironment() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }
	
    // The app is ready to go
    m_bReady = true;
	
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitializeD3DEnvironment()
// Desc: I can has cheezburger?
//-----------------------------------------------------------------------------

HRESULT CDirectXGraphicsContext::InitializeD3DEnvironment()
{
     HRESULT hr;
	
//    D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
//    D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
//    D3DModeInfo*    pModeInfo    = &pDeviceInfo->modes[pDeviceInfo->dwCurrentMode];
 
    // Set up the presentation parameters
    ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
    m_d3dpp.Windowed               = FALSE;
    m_d3dpp.BackBufferCount        = 1;
	//m_d3dpp.BackBufferWidth  = pModeInfo->Width;
    //m_d3dpp.BackBufferHeight = pModeInfo->Height;
	m_d3dpp.BackBufferWidth = 640;
	m_d3dpp.BackBufferHeight = 480;
    //m_d3dpp.BackBufferFormat = D3DFMT_LIN_R5G6B5;
	m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	m_d3dpp.MultiSampleType        = SetAntiAliasMode(AntiAliasMode);
    //m_d3dpp.SwapEffect             = bufferSettings[curBufferSetting].swapEffect;
    m_d3dpp.SwapEffect             =  D3DSWAPEFFECT_COPY;
	m_d3dpp.EnableAutoDepthStencil = TRUE; /*m_bUseDepthBuffer;*/
    //m_d3dpp.AutoDepthStencilFormat = pModeInfo->DepthStencilFormat;
	m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    m_d3dpp.hDeviceWindow          = m_hWnd;
	switch (VSync){
		case 0 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			break;
		case 1 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			break;
		case 2 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
			break;
/*		case 2 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_TWO;
			break;
		case 3 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_THREE;
			break;
		case 4 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
			break;
		case 5 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
			break;
		case 6 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_TWO_OR_IMMEDIATE;
			break;
		case 7 : 	m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE;
			break;*/
	}
	
	m_d3dpp.FullScreen_RefreshRateInHz = 60;//D3DPRESENT_RATE_DEFAULT;
	//m_d3dpp.Flags = D3DPRESENTFLAG_EMULATE_REFRESH_RATE;

	m_desktopFormat = D3DFMT_A8R8G8B8;//D3DFMT_X1R5G5B5;
	//m_desktopFormat = D3DFMT_LIN_R5G6B5;

	DWORD videoFlags = XGetVideoFlags();
	if(XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I)
	{
		if(videoFlags & XC_VIDEO_FLAGS_PAL_60Hz)		// PAL 60 user
			m_d3dpp.FullScreen_RefreshRateInHz = 60;
		else
			m_d3dpp.FullScreen_RefreshRateInHz = 50;
	}

	//Widescreen
	if((videoFlags & XC_VIDEO_FLAGS_WIDESCREEN) !=0)
	 {
		m_d3dpp.Flags = D3DPRESENTFLAG_WIDESCREEN;
	 }

	 if(XGetAVPack() == XC_AV_PACK_HDTV){
		//720p
		if( videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV){
			m_d3dpp.BackBufferWidth = 1280;
			m_d3dpp.BackBufferHeight = 720;
			m_d3dpp.Flags = D3DPRESENTFLAG_PROGRESSIVE | D3DPRESENTFLAG_WIDESCREEN;
			m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		}
		//480p
		else if( videoFlags & XC_VIDEO_FLAGS_HDTV_480p){
			m_d3dpp.Flags = D3DPRESENTFLAG_PROGRESSIVE;
			m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		}
	 }
    
	//apply final width and height. Leave this here!
	windowSetting.uDisplayWidth = m_d3dpp.BackBufferWidth;
	windowSetting.uDisplayHeight = m_d3dpp.BackBufferHeight;

	if(VertexMode == 0){
    // Create the device
    hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
								NULL, D3DCREATE_PUREDEVICE, &m_d3dpp,
								&m_pd3dDevice );
	}

	else if(VertexMode == 1){
    // Create the device
    hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
								NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp,
								&m_pd3dDevice );
	}

	else if(VertexMode == 2){
    // Create the device
    hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
								NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_d3dpp,
								&m_pd3dDevice );
	}

	else if(VertexMode == 3){
    // Create the device
    hr = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
								NULL, D3DCREATE_MIXED_VERTEXPROCESSING, &m_d3dpp,
								&m_pd3dDevice );
	}

    if( SUCCEEDED(hr) && m_pd3dDevice )
    {
		g_pD3DDev = m_pd3dDevice;
		//d3dpp = m_d3dpp;
		
        // When moving from fullscreen to windowed mode, it is important to
        // adjust the window size after recreating the device rather than
        // beforehand to ensure that you get the window size you want.  For
        // example, when switching from 640x480 fullscreen to windowed with
        // a 1000x600 window on a 1024x768 desktop, it is impossible to set
        // the window size to 1000x600 until after the display mode has
        // changed to 1024x768, because windows cannot be larger than the
        // desktop.
      
		
        // Store device Caps
        m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );

		if(VertexMode == 0){
        m_dwCreateFlags = D3DCREATE_PUREDEVICE;
		}

		else if(VertexMode == 1){
        m_dwCreateFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		else if(VertexMode == 2){
        m_dwCreateFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}

		else if(VertexMode == 3){
        m_dwCreateFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
		}

	/*
        // Store device description
        if( pDeviceInfo->DeviceType == D3DDEVTYPE_REF )
            lstrcpy( m_strDeviceStats, "REF" );
        else if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
            lstrcpy( m_strDeviceStats, "HAL" );
        else if( pDeviceInfo->DeviceType == D3DDEVTYPE_SW )
            lstrcpy( m_strDeviceStats, "SW" );
		
        if( pModeInfo->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
            pModeInfo->dwBehavior & D3DCREATE_PUREDEVICE )
        {
            if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, " (pure hw vp)" );
            else
                lstrcat( m_strDeviceStats, " (simulated pure hw vp)" );
        }
        else if( pModeInfo->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, " (hw vp)" );
            else
                lstrcat( m_strDeviceStats, " (simulated hw vp)" );
        }
        else if( pModeInfo->dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, " (mixed vp)" );
            else
                lstrcat( m_strDeviceStats, " (simulated mixed vp)" );
        }
        else if( pModeInfo->dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
        {
            lstrcat( m_strDeviceStats, " (sw vp)" );
        }
		
        if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
        {
            lstrcat( m_strDeviceStats, ": " );
            lstrcat( m_strDeviceStats, pAdapterInfo->d3dAdapterIdentifier.Description );
        } */

        // Store render target surface desc
      LPDIRECT3DSURFACE8 pBackBuffer;
        m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
        pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
        pBackBuffer->Release();
 	    //g_pBackBuffer 
		/*m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &g_pBackBuffer );
        g_pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
        g_pBackBuffer->Release();*/
        // Initialize the app's device-dependent objects
        hr = InitDeviceObjects();
        if( IsResultGood(hr,true) )
        {
            hr = RestoreDeviceObjects();
            if( SUCCEEDED(hr) )
            {
                m_bActive = true;
                return S_OK;
            }
        }
		
        // Cleanup before we try again (- shouldn't get here)
        InvalidateDeviceObjects();
        DeleteDeviceObjects();
        SAFE_RELEASE( m_pd3dDevice );
    }
	else
	{	 
	}

    // If that failed, fall back to the reference rasterizer (removed)
	
    return hr;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::ResizeD3DEnvironment()
{	
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: ToggleFullScreen()
// Desc: Called when user toggles between fullscreen mode and windowed mode
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::DoToggleFullscreen()
{
    m_bReady = true;
	
    return S_OK; 
}




//-----------------------------------------------------------------------------
// Name: ForceWindowed()
// Desc: Switch to a windowed mode, even if that means picking a new device
//       and/or adapter
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::ForceWindowed()
{	
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: AdjustWindowForChange()
// Desc: Prepare the window for a possible change between windowed mode and
//       fullscreen mode.  This function is virtual and thus can be overridden
//       to provide different behavior, such as switching to an entirely
//       different window for fullscreen mode (as in the MFC sample apps).
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::AdjustWindowForChange()
{
 
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SortModesCallback()
// Desc: Callback function for sorting display modes (used by BuildDeviceList).
//-----------------------------------------------------------------------------
static int _cdecl SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
    
    return 0;
}




//-----------------------------------------------------------------------------
// Name: BuildDeviceList()
// Desc: From DX8 SDK Copyright (c) 1998-2000 Microsoft
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::BuildDeviceList()
{	
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
										D3DFORMAT Format )
{
    /*if( ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        ( dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( pCaps->VertexShaderVersion < D3DVS_VERSION(1,0) )
            return E_FAIL;
    }*/
	
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: DisplayErrorMsg()
// Desc: Displays error messages in a message box
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::DisplayErrorMsg( HRESULT hr, DWORD dwType )
{
     
 
    return hr;
}






//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
VOID CDirectXGraphicsContext::CleanUp()
{
	CGraphicsContext::CleanUp();
	
    if( m_pd3dDevice )
    {
        InvalidateDeviceObjects();
        DeleteDeviceObjects();
		
        m_pd3dDevice->Release();
        m_pD3D->Release();
		
        m_pd3dDevice = NULL;
        m_pD3D       = NULL;
    }
	
	// FinalCleanup();
}


int CDirectXGraphicsContext::ToggleFullscreen()
{
// Toggle the fullscreen/window mode
	return 1;
}


//-----------------------------------------------------------------------------
// Name: Pause()
// Desc: Called in to toggle the pause state of the app.
//-----------------------------------------------------------------------------
void CDirectXGraphicsContext::Pause( bool bPause )
{
    static DWORD dwAppPausedCount = 0L;
	
    dwAppPausedCount += ( bPause ? +1 : -1 );
    m_bReady          = ( dwAppPausedCount ? false : true );
}





//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize device-dependent objects. This is the place to create mesh
//       and texture objects.
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::InitDeviceObjects()
{
	// Set the device's states for rendering
	//return RDPInit();

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Restore device-memory objects and state after a device is created or
//       resized.
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::RestoreDeviceObjects()
{
	if ( CDaedalusRender::IsAvailable() )
	{
		CDaedalusRender::GetRender()->RestoreDeviceObjects();
	}
	
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::InvalidateDeviceObjects()
{
	// Kill all textures?
	gTextureCache.InvalidateDeviceObjects();

	if ( CDaedalusRender::IsAvailable() )
	{
		CDaedalusRender::GetRender()->InvalidateDeviceObjects();
	}

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CDirectXGraphicsContext::DeleteDeviceObjects()
{
	//RDPCleanup();
    return S_OK;
}


bool CDirectXGraphicsContext::IsResultGood(HRESULT hr, bool displayError)
{
	if(FAILED(hr))
	{
		if( displayError )
		{
			char szError[200+1];
			D3DXGetErrorString(hr, szError, 200);
			TRACE1("D3D Error: %s", szError);
			ErrorMsg(szError);
		}
		return false;
	}
	else
		return true;
}

void CDirectXGraphicsContext::SetFrontBuffer(DWORD viorg)	// Change front buffer to the viorg addr
{
	//UpdateFrame();
	CGraphicsContext::SetFrontBuffer(viorg);
}
void CDirectXGraphicsContext::SetBackBuffer(DWORD ciaddr)	// Change back buffer to the ci addr
{
	u32 lastBackBufferIdx = m_currentBackBufferIdx;
	CGraphicsContext::SetBackBuffer(ciaddr);

	if( lastBackBufferIdx!=m_currentBackBufferIdx )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_FRAME )
		{
			DebuggerAppendMsg("Draw screen %d", m_currentBackBufferIdx);
		}
#endif

		m_SwapChainInfos[lastBackBufferIdx].beingRendered = false;
		m_SwapChainInfos[m_currentBackBufferIdx].beingRendered = true;
		int idx = (m_currentBackBufferIdx+m_numOfBackBuffers)-m_currentFrontBufferIdx-1;
		idx %= m_numOfBackBuffers;
		m_pd3dDevice->GetBackBuffer(idx, D3DBACKBUFFER_TYPE_MONO, &pCurrentRenderBuffer);
		LPDIRECT3DSURFACE8 pDepthBufferSurf;
		g_pD3DDev->GetDepthStencilSurface(&pDepthBufferSurf);
		m_pd3dDevice->SetRenderTarget(pCurrentRenderBuffer, pDepthBufferSurf);
		SAFE_RELEASE(pDepthBufferSurf);
		SAFE_RELEASE(pCurrentRenderBuffer);
	}
}


