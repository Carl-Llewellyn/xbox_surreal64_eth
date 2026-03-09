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
//#include "../../../ingamemenu/panel.h" // needed?

#include <xbapp.h>
#include <xbresource.h>

extern void XboxDrawOSD();
extern int AntiAliasMode;
char emuvidname[128];

LPDIRECT3DDEVICE8 g_pD3DDev = NULL;
D3DCAPS8 g_D3DDeviceCaps;
//D3DPRESENT_PARAMETERS d3dpp;
extern void 	CreateRenderTarget();
extern void     SetAsRenderTarget();
extern void     RestoreRenderTarget();
int FormatToSize(D3DFORMAT fmt)
{
	switch(fmt)
	{/*
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		return 32;*/
	default:
	case D3DFMT_LIN_R5G6B5:
	case D3DFMT_D16:
		return 16;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
int				CDXGraphicsContext::m_dwNumAdapters;
D3DAdapterInfo	CDXGraphicsContext::m_1stAdapters;
D3DCAPS8		CDXGraphicsContext::m_d3dCaps;           // Caps for the device
bool			CDXGraphicsContext::m_bSupportAnisotropy;
const	DWORD		dwNumDeviceTypes = 2;
extern const char*	strDXDeviceDescs[];
const	D3DDEVTYPE	DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };


CDXGraphicsContext::CDXGraphicsContext() :
	m_pd3dDevice(NULL),
	m_pD3D(NULL),
	m_hFont(NULL),
	m_dwAdapter(0),			// Default Adapter
	m_dwCreateFlags(0),
	m_dwMinDepthBits(16),
	m_dwMinStencilBits(0),
	m_desktopFormat(D3DFMT_A8R8G8B8),
	//m_desktopFormat(D3DFMT_LIN_R5G6B5),
	m_FSAAIsEnabled(false),
	m_bFontIsCreated(false)
{
	m_strDeviceStats[0] = '\0';
	memset(&m_textureColorBufferInfo, 0, sizeof(TextureBufferShortInfo));
	memset(&m_textureDepthBufferInfo, 0, sizeof(TextureBufferShortInfo));
}

//*****************************************************************************
//
//*****************************************************************************
CDXGraphicsContext::~CDXGraphicsContext()
{
	CGraphicsContext::Get()->CleanUp();
}

//*****************************************************************************
//
//*****************************************************************************
void CDXGraphicsContext::Clear(ClearFlag dwFlags, DWORD color, float depth)
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

LPDIRECT3DVERTEXBUFFER8 m_pVB;
struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
/*LPDIRECT3DSURFACE8 surface2;
D3DSURFACE_DESC    surfDesc2;
LPDIRECT3DTEXTURE8 texture2;
LPDIRECT3DSURFACE8 texSurface2;*/
//bool create=false;

__forceinline void CDXGraphicsContext::UpdateFrame(bool swaponly)
{
	//HRESULT hr; //unreferenced

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

	status.gFrameCount++;
	CGraphicsContext::UpdateFrameBufferBeforeUpdateFrame();

/*	if( options.bDisplayOnscreenFPS && !status.bDisableFPS )
	{
		char str[256];
		RECT rect={windowSetting.uDisplayWidth-100,windowSetting.toolbarHeight+windowSetting.uDisplayHeight-100,windowSetting.uDisplayWidth-10,windowSetting.toolbarHeight+windowSetting.uDisplayHeight};
		if( (options.bDisplayOnscreenFPS == ONSCREEN_DISPLAY_DLIST_PER_SECOND ||
			options.bDisplayOnscreenFPS == ONSCREEN_DISPLAY_DLIST_PER_SECOND_WITH_CORE_MSG ) && windowSetting.dps > 0 )
		{
			sprintf(str,"%.1f dps", windowSetting.dps);
			DrawText(str, rect,2);
		}
		else if( (options.bDisplayOnscreenFPS == ONSCREEN_DISPLAY_FRAME_PER_SECOND ||
			options.bDisplayOnscreenFPS == ONSCREEN_DISPLAY_FRAME_PER_SECOND_WITH_CORE_MSG ) && windowSetting.fps > 0 )
		{
			sprintf(str,"%.1f fps", windowSetting.fps);
			DrawText(str, rect,2);
		}
	
		if( options.bDisplayOnscreenFPS >= ONSCREEN_DISPLAY_TEXT_FROM_CORE_ONLY && status.CPUCoreMsgIsSet && strlen(status.CPUCoreMsgToDisplay) > 0 && windowSetting.dps > 0 )
		{
			RECT rect2={10,windowSetting.toolbarHeight+windowSetting.uDisplayHeight-100,windowSetting.uDisplayWidth-100,windowSetting.toolbarHeight+windowSetting.uDisplayHeight};
			DrawText(status.CPUCoreMsgToDisplay, rect2,1);
		}
	}*/

#ifndef OLDTXTCACHE
	if (!g_bUseSetTextureMem)
		gTextureManager.FreeTextures();
#endif

	XboxDrawOSD();
/*
	Lock();
	if (m_pd3dDevice == NULL)
	{
		hr = E_FAIL;
	}
	else
	{
		if( !g_curRomInfo.bForceScreenClear )	
			Clear(CLEAR_DEPTH_BUFFER);

		if( m_bWindowed )
		{
			RECT dstrect={0,windowSetting.toolbarHeight,windowSetting.uDisplayWidth,windowSetting.toolbarHeight+windowSetting.uDisplayHeight};
			RECT srcrect={0,0,windowSetting.uDisplayWidth,windowSetting.uDisplayHeight};
			hr = m_pd3dDevice->Present( &srcrect, &dstrect, NULL, NULL );
		}
		else
		{
			hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}

		m_backBufferIsSaved = false;

#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_FRAME )
		{
			TRACE0("Update screen");
		}
#endif

	}*/
//exit: // unrefrenced
//	SetAsRenderTarget();

	if( !g_curRomInfo.bForceScreenClear ){
		Clear(CLEAR_DEPTH_BUFFER);}

	m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	m_backBufferIsSaved = false;

//	RestoreRenderTarget();
//	Unlock();

	if( g_curRomInfo.bForceScreenClear )	needCleanScene = true;

// GogoAckman - here are the settings to adjust size (commented out here)
//windowSetting.uDisplayHeight = 240;
//windowSetting.uDisplayWidth = 320;

  //m_pd3dDevice->SetScreenSpaceOffset(50,50);
  
  //m_pd3dDevice->GetBackBuffer(-1,0,&surface2);

  //m_pd3dDevice->SetBackBufferScale(0.5f,0.5f); // remember that for FBA :D

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
BOOL CDXGraphicsContext::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
											  D3DFORMAT TargetFormat,
											  D3DFORMAT* pDepthStencilFormat )
{
	
	// fiddle with this: Lantus
	*pDepthStencilFormat = D3DFMT_D16;
	return TRUE;
}

//*****************************************************************************
//
//*****************************************************************************
extern void WriteConfiguration(void);
extern "C" void _INPUT_LoadButtonMap(int *cfgData); 
extern int ControllerConfig[76];

bool CDXGraphicsContext::Initialize(HWND hWnd, HWND hWndStatus,
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
		DisplayD3DErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
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
        DisplayD3DErrorMsg( hr, MSGERR_APPMUSTEXIT );
		return false;
    }

	CGraphicsContext::Initialize(hWnd, hWndStatus, dwWidth, dwHeight, bWindowed );
	
	hr = Create3D( bWindowed );

	// Clear/Update a few times to ensure that all of the buffers are cleared
	if ( m_pd3dDevice )
	{
		for (int i=0; i<DirectXRenderBufferSettings[options.RenderBufferSetting].number+1; i++ )
		{
			Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);
			m_pd3dDevice->BeginScene();
			m_pd3dDevice->EndScene();
			//UpdateFrame();
			m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}
	}

	D3DVIEWPORT8 vp = {
		0, 
			windowSetting.toolbarHeightToUse, 
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

	
#ifndef _XBOX
	// Force to use Software T&L
	if( options.bForceSoftwareTnL )
		g_pD3DDev->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,TRUE);
	else
		g_pD3DDev->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,FALSE);
#endif

	// GogoAckman
	g_pd3dDevice = g_pD3DDev;
	sprintf(emuvidname,"Video 5.60");
    //d3dpp = m_d3dpp;

	//CreateRenderTarget();
	_INPUT_LoadButtonMap(ControllerConfig);
	
	return hr==S_OK;
}


//-----------------------------------------------------------------------------
// Name: SortModesCallback()
// Desc: Callback function for sorting display modes (used by BuildDeviceList).
//-----------------------------------------------------------------------------
static int _cdecl SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
	D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
	D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

	if( p1->Format > p2->Format )   return -1;
	if( p1->Format < p2->Format )   return +1;
	if( p1->Width  < p2->Width )    return -1;
	if( p1->Width  > p2->Width )    return +1;
	if( p1->Height < p2->Height )   return -1;
	if( p1->Height > p2->Height )   return +1;

	return 0;
}

// This is a static function, will be called when the plugin DLL is initialized
void CDXGraphicsContext::InitDeviceParameters()
{

	// Create Direct3D object
	LPDIRECT3D8 pD3D;
	pD3D = Direct3DCreate8( D3D_SDK_VERSION );
	if( pD3D == NULL )
	{
        DisplayD3DErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
		return;
	}

	// Get number of adapters
	int numAvailableAdapters = pD3D->GetAdapterCount();

	// For the 1st adapter, looking through each of its devices
	int iAdapter=0;
	D3DAdapterInfo* pAdapter  = &m_1stAdapters;
	pD3D->GetAdapterIdentifier( iAdapter, 0, &pAdapter->d3dAdapterIdentifier );
	pD3D->GetAdapterDisplayMode( iAdapter, &pAdapter->d3ddmDesktop );	// Get current display mode

	// Get number of devices for each adapters
	pAdapter->dwNumDevices    = dwNumDeviceTypes;
	pAdapter->dwCurrentDevice = 0;

	// Add devices to adapter
	for( UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++ )
	{
		// Fill in device info
		D3DDeviceInfo* pDevice	= &pAdapter->devices[iDevice];
		pDevice->DeviceType     = DeviceTypes[iDevice];
		pDevice->strDesc        = strDXDeviceDescs[iDevice];

		// for each devices in each adapter
		// Get device caps
		pD3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &pDevice->d3dCaps );

		pDevice->dwNumModes     = 0;
		pDevice->MultiSampleType = SetAntiAliasMode(AntiAliasMode);

	}

	// Check FSAA maximum
	for( m_maxFSAA = 16; m_maxFSAA >= 2; m_maxFSAA-- )
	{
		if( SUCCEEDED(pD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL , D3DFMT_X8R8G8B8, FALSE, D3DMULTISAMPLE_TYPE(D3DMULTISAMPLE_NONE+m_maxFSAA) ) ) )
		{
			break;
		}
		else
		{
			continue;
		}
	}

	if( m_maxFSAA < 2 )
	{
		m_maxFSAA = 0;
		TRACE0("Device does not support FSAA");
	}
	else
	{
		TRACE1("Device support FSAA up to %d X", m_maxFSAA);
	}
	if( options.DirectXMaxFSAA != m_maxFSAA )
	{
		options.DirectXMaxFSAA = m_maxFSAA;
		WriteConfiguration();
	}


	// Check Anisotropy Filtering maximum

	m_maxAnisotropy = pAdapter->devices[0].d3dCaps.MaxAnisotropy;
	TRACE1("Max Anisotropy: %d", m_maxAnisotropy);
	if( options.DirectXMaxAnisotropy != m_maxAnisotropy )
	{
		options.DirectXMaxAnisotropy = m_maxAnisotropy;
		WriteConfiguration();
	}

	// Get available refresh rates

	// Get available color quality, 16 bit and 32 bit

	// Get available back buffer format

	// Get available depth buffer format

	// Release the Direct3D object
	pD3D->Release();
}


//*****************************************************************************
//
//*****************************************************************************
HRESULT CDXGraphicsContext::Create3D( BOOL bWindowed )
{
    HRESULT hr;
	
    // Get access to current adapter, device, and mode
    D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
    //D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
	D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[options.DirectXDevice];
	
 	m_bWindowed = (bWindowed==TRUE);		// Make user Toggle manually for now!
    pDeviceInfo->bWindowed = m_bWindowed;

    // Initialize the 3D environment for the app
    if( FAILED( hr = InitializeD3D() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayD3DErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }
	
    // The app is ready to go
    m_bReady = true;
	
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitializeD3D()
// Desc:
//-----------------------------------------------------------------------------

HRESULT CDXGraphicsContext::InitializeD3D()
{
     HRESULT hr;
 
    // Set up the presentation parameters
    ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
    m_d3dpp.Windowed               = FALSE;
    m_d3dpp.BackBufferCount        = 1;
	m_d3dpp.MultiSampleType        = SetAntiAliasMode(AntiAliasMode);
	//m_d3dpp.SwapEffect             = bufferSettings[curBufferSetting].swapEffect;
    m_d3dpp.SwapEffect             =  D3DSWAPEFFECT_COPY;
	m_d3dpp.EnableAutoDepthStencil = TRUE; /*m_bUseDepthBuffer;*/
    //m_d3dpp.AutoDepthStencilFormat = pModeInfo->DepthStencilFormat;
	m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    m_d3dpp.hDeviceWindow          = m_hWnd;
	//m_d3dpp.Flags = D3DPRESENTFLAG_EMULATE_REFRESH_RATE;
	//m_d3dpp.BackBufferWidth = 720;
	//m_d3dpp.BackBufferHeight = 576;
	m_d3dpp.BackBufferWidth = 640;
	m_d3dpp.BackBufferHeight = 480;
    m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;

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
	m_d3dpp.FullScreen_RefreshRateInHz = 60;


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
    
	windowSetting.uDisplayWidth = m_d3dpp.BackBufferWidth;
	windowSetting.uDisplayHeight = m_d3dpp.BackBufferHeight;

	m_desktopFormat = D3DFMT_A8R8G8B8;
	//m_desktopFormat = D3DFMT_LIN_R5G6B5;
	
//freakdave
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
	 
	 
	if(  m_pd3dDevice )
	{
		g_pD3DDev = m_pd3dDevice;
		
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
		//freakdave
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
 
		
		// Store render target surface desc
		LPDIRECT3DSURFACE8 pBackBuffer;
		m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
		pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
		pBackBuffer->Release();
	
		 // Initialize the app's device-dependent objects
        hr = InitDeviceObjects();
        if( IsResultGood(hr,true) )
        {
			if ( CRender::IsAvailable() )
			{
				CRender::GetRender()->InitDeviceObjects();
			}
            m_bActive = true;
            return S_OK;
        }
		
 
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
HRESULT CDXGraphicsContext::ResizeD3DEnvironment()
{
    HRESULT hr;
	
    // Release all vidmem objects
    if( FAILED( hr = CleanDeviceObjects() ) )
        return hr;
	
    // Reset the device
    if( IsResultGood( hr = m_pd3dDevice->Reset( &m_d3dpp ), true ) )
	{
		while ( hr == D3DERR_DEVICELOST )
		{
			Pause(true);
			hr = m_pd3dDevice->Reset( &m_d3dpp );
		}
	}
	else
	{
        return hr;
	}
	
    // Store render target surface desc
    LPDIRECT3DSURFACE8 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
    pBackBuffer->Release();
	
    // Initialize the app's device-dependent objects
	if ( CRender::IsAvailable() )
	{
		CRender::GetRender()->InitDeviceObjects();
	}
	
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: ToggleFullScreen()
// Desc: Called when user toggles between fullscreen mode and windowed mode
//-----------------------------------------------------------------------------
HRESULT CDXGraphicsContext::DoToggleFullscreen()
{
    // Get access to current adapter, device, and mode
    m_bReady = true;
	
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ForceWindowed()
// Desc: Switch to a windowed mode, even if that means picking a new device
//       and/or adapter
//-----------------------------------------------------------------------------
HRESULT CDXGraphicsContext::ForceWindowed()
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
HRESULT CDXGraphicsContext::AdjustWindowForChange()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: BuildDeviceList()
// Desc: From DX8 SDK Copyright (c) 1998-2000 Microsoft
//-----------------------------------------------------------------------------
HRESULT CDXGraphicsContext::BuildDeviceList()
{
 return S_OK;
}

int	CDXGraphicsContext::FindCurrentDisplayModeIndex()
{
	D3DDISPLAYMODE dMode;
	D3DAdapterInfo &adapter = m_Adapters[m_dwAdapter];
	//D3DDeviceInfo &device = adapter.devices[adapter.dwCurrentDevice];
	D3DDeviceInfo &device = adapter.devices[options.DirectXDevice];
	int m;

	m_pD3D->GetAdapterDisplayMode(m_dwAdapter,&dMode);

	for( m=0; m<device.dwNumModes; m++ )
	{
		if( device.modes[m].Width==windowSetting.uFullScreenDisplayWidth && device.modes[m].Height==windowSetting.uFullScreenDisplayHeight 
			//&& device.modes[m].Format == dMode.Format 
			&& FormatToSize(device.modes[m].Format) == FormatToSize((D3DFORMAT)options.colorQuality) 
			)
		{
			return m;
		}
	}

	for( m=0; m<device.dwNumModes; m++ )
	{
		if( device.modes[m].Width==windowSetting.uFullScreenDisplayWidth && device.modes[m].Height==windowSetting.uFullScreenDisplayHeight )
		{
			return m;
		}
	}

	// Cannot find a matching mode
	TRACE0("Cannot find a matching mode");
	for( m=0; m<device.dwNumModes; m++ )
	{
		if( device.modes[m].Width==1280 && device.modes[m].Height==720 )
			return m;
		else if( device.modes[m].Width==640 && device.modes[m].Height==480 )
		{
			return m;
		}
	}

	return 0;
}


//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CDXGraphicsContext::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
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
// Name: DisplayD3DErrorMsg()
// Desc: Displays error messages in a message box
//-----------------------------------------------------------------------------
HRESULT CDXGraphicsContext::DisplayD3DErrorMsg( HRESULT hr, DWORD dwType )
{
 
    return hr;
}


//-----------------------------------------------------------------------------
// Name: CleanUp()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
VOID CDXGraphicsContext::CleanUp()
{
	CGraphicsContext::CleanUp();
	
    if( m_pd3dDevice )
    {
        CleanDeviceObjects();
		
		 
		m_bFontIsCreated = false;
		SAFE_RELEASE(m_pd3dDevice);
		SAFE_RELEASE(m_pD3D);
    }
}


int CDXGraphicsContext::ToggleFullscreen()
{
    return 1;
}


//-----------------------------------------------------------------------------
// Name: Pause()
// Desc: Called in to toggle the pause state of the app.
//-----------------------------------------------------------------------------
void CDXGraphicsContext::Pause( bool bPause )
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
HRESULT CDXGraphicsContext::InitDeviceObjects()
{
	// Set the device's states for rendering
	//return RDPInit();

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CleanDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CDXGraphicsContext::CleanDeviceObjects()
{
	if ( CRender::IsAvailable() )
	{
		CRender::GetRender()->ClearDeviceObjects();
	}

	// Kill all textures?
	gTextureManager.DropTextures();
	gTextureManager.CleanUp();

	return S_OK;
}

bool CDXGraphicsContext::IsResultGood(HRESULT hr, bool displayError)
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

extern RecentCIInfo* g_uRecentCIInfoPtrs[3];
extern TextureBufferInfo gTextureBufferInfos[];
void CDXGraphicsContext::SaveBackBuffer(int ciInfoIdx, RECT* pSrcRect)
{
	HRESULT res;
	SetImgInfo tempinfo;
	RecentCIInfo &ciInfo = *g_uRecentCIInfoPtrs[ciInfoIdx];
	tempinfo.dwAddr = ciInfo.dwAddr;
	tempinfo.dwFormat = ciInfo.dwFormat;
	tempinfo.dwSize = ciInfo.dwSize;
	tempinfo.dwWidth = ciInfo.dwWidth;

	int idx = SetTextureBuffer(tempinfo, ciInfoIdx,true );

	LPDIRECT3DSURFACE8 pSavedBuffer;
	LPDIRECT3DTEXTURE8(gTextureBufferInfos[idx].pTxtBuffer->m_pTexture->GetTexture())->GetSurfaceLevel(0,&pSavedBuffer);

	if( pSavedBuffer != NULL )
	{
		LPDIRECT3DSURFACE8 pBackBufferToSave = NULL;
		g_pD3DDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBufferToSave);

		if( pBackBufferToSave )
		{
			if( defaultRomOptions.bInN64Resolution )
			{
				// Need to scale down from PC resolution to N64 resolution
				if( pSrcRect == NULL )
				{
					RECT srcrect = {0,0,m_d3dpp.BackBufferWidth-1,m_d3dpp.BackBufferHeight-1};
					RECT dstrect = {0,0,ciInfo.dwWidth-1,ciInfo.dwHeight-1};
					res = D3DXLoadSurfaceFromSurface(pSavedBuffer,NULL,&dstrect,pBackBufferToSave,NULL,&srcrect,D3DX_FILTER_LINEAR,0xFF000000);
				}
				else
				{
					float scaleX = windowSetting.uDisplayWidth/(float)ciInfo.dwWidth;
					float scaleY = windowSetting.uDisplayHeight/(float)ciInfo.dwHeight;
					RECT srcr2 = { DWORD(pSrcRect->left*scaleX), DWORD(pSrcRect->top*scaleY),
						DWORD(pSrcRect->right*scaleX), DWORD(pSrcRect->bottom*scaleY) };
					res = D3DXLoadSurfaceFromSurface(pSavedBuffer,NULL,pSrcRect,pBackBufferToSave,NULL,&srcr2,D3DX_FILTER_LINEAR,0xFF000000);
				}
			}
			else
			{
				if( pSrcRect == NULL )
				{
					RECT dstrect = {0,0,gTextureBufferInfos[idx].pTxtBuffer->m_pTexture->m_dwWidth,gTextureBufferInfos[idx].pTxtBuffer->m_pTexture->m_dwHeight};
					res = D3DXLoadSurfaceFromSurface(pSavedBuffer,NULL,&dstrect,pBackBufferToSave,NULL,NULL,D3DX_FILTER_LINEAR,0xFF000000);
					//res = g_pD3DDev->CopyRects(pBackBufferToSave,NULL,0,pSavedBuffer,NULL);
				}
				else
				{
					float scaleX = windowSetting.uDisplayWidth/(float)ciInfo.dwWidth;
					float scaleY = windowSetting.uDisplayHeight/(float)ciInfo.dwHeight;
					RECT srcr = { DWORD(pSrcRect->left*scaleX), DWORD(pSrcRect->top*scaleY),
						DWORD(pSrcRect->right*scaleX), DWORD(pSrcRect->bottom*scaleY) };
					//POINT srcp = {DWORD(pSrcRect->left*scaleX), DWORD(pSrcRect->top*scaleY)};
					//res = g_pD3DDev->CopyRects(pBackBufferToSave,&srcr,0,pSavedBuffer,&srcp);
					res = D3DXLoadSurfaceFromSurface(pSavedBuffer,NULL,&srcr,pBackBufferToSave,NULL,&srcr,D3DX_FILTER_LINEAR,0xFF000000);
				}
			}

			if( res != S_OK )
			{
				TRACE0("Cannot save back buffer");
			}

			pBackBufferToSave->Release();
		}
	}


	if( frameBufferOptions.bWriteBackBufToRDRAM )
	{
		DWORD width = ciInfo.dwWidth;
		DWORD height = ciInfo.dwHeight;

		if( ciInfo.dwWidth == *g_GraphicsInfo.VI_WIDTH_REG && ciInfo.dwWidth != windowSetting.uViWidth )
		{
			width = windowSetting.uViWidth;
			height = windowSetting.uViHeight;
		}

		CopyBackToRDRAM(ciInfo.dwAddr, ciInfo.dwFormat, ciInfo.dwSize, width, height, 
			windowSetting.uDisplayWidth, windowSetting.uDisplayHeight,
			ciInfo.dwAddr, ciInfo.dwMemSize , ciInfo.dwWidth,
			D3DFMT_X8R8G8B8, pSavedBuffer);

	}

	gTextureBufferInfos[idx].crcCheckedAtFrame = status.gDlistCount;
	gTextureBufferInfos[idx].crcInRDRAM = ComputeTextureBufferCRCInRDRAM(m_curTextureBufferIndex);

	pSavedBuffer->Release();
	DEBUGGER_IF_DUMP((logTextureBuffer&&pSrcRect==NULL),TRACE1("SaveBackBuffer at 0x%08X", ciInfo.dwAddr));
	DEBUGGER_IF_DUMP((logTextureBuffer&&pSrcRect),TRACE5("SaveBackBuffer at 0x%08X, {%d,%d -%d,%d)", ciInfo.dwAddr,
		pSrcRect->left,pSrcRect->top,pSrcRect->right,pSrcRect->bottom));
	DEBUGGER_IF_DUMP(( pauseAtNext && eventToPause == NEXT_TEXTURE_BUFFER),{CGraphicsContext::g_pGraphicsContext->DisplayTextureBuffer(idx);});

	g_uRecentCIInfoPtrs[ciInfoIdx]->bCopied = true;
}


#ifdef _DEBUG
void CDXGraphicsContext::DisplayTextureBuffer(int infoIdx)
{
	if( infoIdx < 0 )
		infoIdx = m_lastTextureBufferIndex;

	if( gTextureBufferInfos[infoIdx].pTxtBuffer )
	{
		if( gTextureBufferInfos[infoIdx].pTxtBuffer->IsBeingRendered() )
		{
			TRACE1("Texture buffer %d is being rendered", infoIdx);
		}
		else
		{
			CDXTextureBuffer* pTxtBuffer = (CDXTextureBuffer*)(gTextureBufferInfos[infoIdx].pTxtBuffer);
			LPDIRECT3DSURFACE8 pSourceSurface = NULL;
			(LPDIRECT3DTEXTURE8(pTxtBuffer->m_pTexture->GetTexture()))->GetSurfaceLevel(0,&pSourceSurface);
			SaveSurfaceToFile(pSourceSurface, infoIdx);
			SAFE_RELEASE(pSourceSurface);
		}
		TRACE1("Texture buffer %d:", infoIdx);
		TRACE1("Addr=%08X", gTextureBufferInfos[infoIdx].CI_Info.dwAddr);
		TRACE2("Width=%d, Created Height=%d", gTextureBufferInfos[infoIdx].N64Width,gTextureBufferInfos[infoIdx].N64Height);
		TRACE2("Fmt=%d, Size=%d", gTextureBufferInfos[infoIdx].CI_Info.dwFormat,gTextureBufferInfos[infoIdx].CI_Info.dwSize);
	}
	else
	{
		TRACE1("Texture buffer %d is not used", infoIdx);
	}
}
#endif

CDXTextureBuffer::CDXTextureBuffer(int width, int height, TextureBufferInfo* pInfo, TextureUsage usage)
	: CTextureBuffer(width, height, pInfo, usage)
{
	m_pTexture = new CDirectXTexture(width, height, usage);
	if( m_pTexture )
	{
		m_width = width;
		m_height = height;
	}
	else
	{
		TRACE0("Error to create DX texture buffer");
		SAFE_DELETE(m_pTexture);
	}

	m_pColorBufferSave = NULL;
	m_pDepthBufferSave = NULL;
	m_beingRendered = false;
}

CDXTextureBuffer::~CDXTextureBuffer()
{
	if( m_beingRendered )
	{
		CGraphicsContext::g_pGraphicsContext->CloseTextureBuffer(false);
		SetAsRenderTarget(false);
	}

	SAFE_DELETE(m_pTexture);
	
	m_beingRendered = false;
}

bool CDXTextureBuffer::SetAsRenderTarget(bool enable)
{
	if( m_usage != AS_RENDER_TARGET )	return false;

	if( enable )
	{
		if( !m_beingRendered )
		{
			if(m_pTexture )
			{
				LPDIRECT3DSURFACE8 pColorBuffer;

				// save the current back buffer
				g_pD3DDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_pColorBufferSave);
				g_pD3DDev->GetDepthStencilSurface(&m_pDepthBufferSave);

				// Activate the texture buffer
				(LPDIRECT3DTEXTURE8(m_pTexture->GetTexture()))->GetSurfaceLevel(0,&pColorBuffer);
				HRESULT res = g_pD3DDev->SetRenderTarget(pColorBuffer, NULL);
				SAFE_RELEASE(pColorBuffer);
				if( res != S_OK )
				{
					return false;
				}

				m_beingRendered = true;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		if( m_beingRendered )
		{
			if( m_pColorBufferSave && m_pDepthBufferSave )
			{
				g_pD3DDev->SetRenderTarget(m_pColorBufferSave, m_pDepthBufferSave);
				m_beingRendered = false;
				SAFE_RELEASE(m_pColorBufferSave);
				SAFE_RELEASE(m_pDepthBufferSave);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}
}

void CDXTextureBuffer::LoadTexture(TextureEntry* pEntry)
{
	bool IsBeingRendered = m_beingRendered;
	if( IsBeingRendered )
	{
		DEBUGGER_IF_DUMP((logTextureBuffer), 
		{TRACE0("Warning, loading from texture buffer while it is being rendered");});

		SetAsRenderTarget(false);
		//return;
	}

	CTexture *pSurf = pEntry->pTexture;
	TextureInfo &ti = pEntry->ti;

	// Need to load texture from the saved back buffer

	LPDIRECT3DTEXTURE8 pNewTexture = LPDIRECT3DTEXTURE8(pSurf->GetTexture());
	LPDIRECT3DSURFACE8 pNewSurface = NULL;
	HRESULT res = pNewTexture->GetSurfaceLevel(0,&pNewSurface);
	LPDIRECT3DSURFACE8 pSourceSurface = NULL;
	(LPDIRECT3DTEXTURE8(m_pTexture->GetTexture()))->GetSurfaceLevel(0,&pSourceSurface);

	int left = (pEntry->ti.Address- m_pInfo->CI_Info.dwAddr )%m_pInfo->CI_Info.bpl + pEntry->ti.LeftToLoad;
	int top = (pEntry->ti.Address- m_pInfo->CI_Info.dwAddr)/m_pInfo->CI_Info.bpl + pEntry->ti.TopToLoad;
	RECT srcrect = {DWORD(left*m_pInfo->scaleX) ,DWORD(top*m_pInfo->scaleY), 
		DWORD(min(m_width, left+(int)ti.WidthToLoad)*m_pInfo->scaleX), 
		DWORD(min(m_height,top+(int)ti.HeightToLoad)*m_pInfo->scaleY) };

	if( pNewSurface != NULL && pSourceSurface != NULL )
	{
		if( left < m_width && top<m_height )
		{
			RECT dstrect = {0,0,ti.WidthToLoad,ti.HeightToLoad};
			HRESULT res = D3DXLoadSurfaceFromSurface(pNewSurface,NULL,&dstrect,pSourceSurface,NULL,&srcrect,D3DX_FILTER_POINT ,0xFF000000);
			DEBUGGER_IF_DUMP(( res != S_OK), {DebuggerAppendMsg("Error to reload texture from texture buffer, res=%x", res);} );
		}
	}

	if( IsBeingRendered )
	{
		SetAsRenderTarget(true);
	}

	pSurf->SetOthersVariables();
	SAFE_RELEASE(pSourceSurface);
	DEBUGGER_IF_DUMP((logTextureBuffer), 
		{DebuggerAppendMsg("Load texture from texture buffer");});
}

#ifdef _DEBUG
void CDXGraphicsContext::SaveSurfaceToFile(LPDIRECT3DSURFACE8 surf, int seq)
{ 
}
#endif


void CDXGraphicsContext::StoreTextureBufferToRDRAM(int infoIdx)
{
	if( infoIdx < 0 )
		infoIdx = m_lastTextureBufferIndex;

	if( gTextureBufferInfos[infoIdx].pTxtBuffer && gTextureBufferInfos[infoIdx].pTxtBuffer->IsBeingRendered() )
	{
		DEBUGGER_IF_DUMP((logTextureBuffer), TRACE1("Cannot SaveTextureBuffer %d, it is being rendered", infoIdx));
		return;
	}

	if( gTextureBufferInfos[infoIdx].pTxtBuffer )
	{
		TextureBufferInfo &info = gTextureBufferInfos[infoIdx];

		DWORD fmt = info.CI_Info.dwFormat;
		if( frameBufferOptions.bTxtBufWriteBack )
		{
			CDXTextureBuffer* pTxtBuffer = (CDXTextureBuffer*)(info.pTxtBuffer);
			LPDIRECT3DSURFACE8 pSourceSurface = NULL;
			(LPDIRECT3DTEXTURE8(pTxtBuffer->m_pTexture->GetTexture()))->GetSurfaceLevel(0,&pSourceSurface);

			// Ok, we are using texture render target right now
			// Need to copy content from the texture render target back to frame buffer
			// then reset the current render target

			// Here we need to copy the content from the texture frame buffer to RDRAM memory

			DEBUGGER_IF_DUMP((logTextureBuffer), TRACE2("SaveTextureBuffer %d to N64 RDRAM addr=%08X", infoIdx, info.CI_Info.dwAddr));

			if( pSourceSurface )
			{
				DWORD width, height, bufWidth, bufHeight, memsize; 
				width = info.N64Width;
				height = info.N64Height;
				bufWidth = info.bufferWidth;
				bufHeight = info.bufferHeight;
				if( info.CI_Info.dwSize == TXT_SIZE_8b && fmt == TXT_FMT_CI )
				{
					info.CI_Info.dwFormat = TXT_FMT_I;
					height = info.knownHeight ? info.N64Height : info.maxUsedHeight;
					memsize = info.N64Width*height;
					CopyBackToRDRAM(info.CI_Info.dwAddr, fmt, info.CI_Info.dwSize, width, height,
						bufWidth, bufHeight, info.CI_Info.dwAddr, memsize, info.N64Width, D3DFMT_A8R8G8B8, pSourceSurface);
					info.CI_Info.dwFormat = TXT_FMT_CI;
				}
				else
				{
					if( info.CI_Info.dwSize == TXT_SIZE_8b )
					{
						height = info.knownHeight ? info.N64Height : info.maxUsedHeight;
						memsize = info.N64Width*height;
						CopyBackToRDRAM(info.CI_Info.dwAddr, fmt, info.CI_Info.dwSize, width, height,
							bufWidth, bufHeight, info.CI_Info.dwAddr, memsize, info.N64Width, D3DFMT_A8R8G8B8, pSourceSurface);
					}
					else
					{
						height = info.knownHeight ? info.N64Height : info.maxUsedHeight;
						memsize = g_pTextureBufferInfo->N64Width*height*2;
						CopyBackToRDRAM(info.CI_Info.dwAddr, fmt, info.CI_Info.dwSize, width, height,
							bufWidth, bufHeight, info.CI_Info.dwAddr, memsize, info.N64Width, D3DFMT_X8R8G8B8, pSourceSurface);
					}
				}
				DEBUGGER_IF_DUMP(logTextureBuffer,TRACE2("Write back: width=%d, height=%d", width, height));	

				SAFE_RELEASE(pSourceSurface);
			}
			else
			{
				TRACE0("Error, cannot lock the texture buffer");
			}
		}

	}

	DEBUGGER_IF_DUMP(logTextureBuffer,{DebuggerAppendMsg("Saving texture buffer at %08X", gTextureBufferInfos[infoIdx].CI_Info.dwAddr);});
}


WORD ConvertRGBATo555(BYTE r, BYTE g, BYTE b, BYTE a);
WORD ConvertRGBATo555(DWORD color32);
void InitTlutReverseLookup(void);
extern BYTE RevTlutTable[0x10000];

void CDXGraphicsContext::CopyBackToRDRAM(uint32 addr, uint32 fmt, uint32 siz, uint32 width, uint32 height, uint32 bufWidth, uint32 bufHeight, uint32 startaddr, uint32 memsize, uint32 pitch, D3DFORMAT surf_fmt, IDirect3DSurface8 *surf)
{
	if( addr == 0 || addr>=g_dwRamSize )	return;

	if( pitch == 0 ) pitch = width;

	IDirect3DSurface8 *backBuffer = surf;
	IDirect3DSurface8 *backBuffer2 = NULL;

	if( surf == NULL )	
	{
		DEBUGGER_IF_DUMP((logTextureBuffer),{DebuggerAppendMsg("Save Backbuffer: surf=NULL");});
		g_pD3DDev->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO, &backBuffer);
	}

	DEBUGGER_IF_DUMP((logTextureBuffer),{DebuggerAppendMsg("Copy Back to N64 RDRAM");});

	//int pixSize = GetPixelSize();
	if( !backBuffer )
	{
		TRACE0("Error, cannot lock backbuffer to save to RDRAM");
		return;
	}

	D3DLOCKED_RECT dlre;
	ZeroMemory( &dlre, sizeof(D3DLOCKED_RECT) );
	if( !SUCCEEDED(backBuffer->LockRect(&dlre, NULL, D3DLOCK_READONLY)) )
	{
		D3DSURFACE_DESC desc;
		//TRACE0("Error, cannot lock the surface");
		backBuffer->GetDesc(&desc);
		g_pD3DDev->CreateImageSurface(desc.Width,desc.Height,desc.Format,&backBuffer2);
		g_pD3DDev->CopyRects(backBuffer,NULL,0,backBuffer2,NULL);
		ZeroMemory( &dlre, sizeof(D3DLOCKED_RECT) );
		if( !SUCCEEDED(backBuffer2->LockRect(&dlre, NULL, D3DLOCK_READONLY)) )
		{
			TRACE0("Error, cannot lock the copied surface");
			return;
		}
	}

	DWORD startline=0;
	if( startaddr == 0xFFFFFFFF )	startaddr = addr;

	startline = (startaddr-addr)/siz/pitch;
	if( startline >= height )
	{
		//TRACE0("Warning: check me");
		startline = height;
	}

	DWORD endline = height;
	if( memsize != 0xFFFFFFFF )
	{
		endline = (startaddr+memsize-addr)/siz;
		if( endline % pitch == 0 )
			endline /= pitch;
		else
			endline = endline/pitch+1;
	}
	if( endline > height )
	{
		endline = height;
	}

	if( memsize != 0xFFFFFFFF )
	{
		DEBUGGER_IF_DUMP((logTextureBuffer) ,{DebuggerAppendMsg("Start at: 0x%X, from line %d to %d", startaddr-addr, startline, endline);});
	}

	int indexes[600];
	{
		float sx;
		int sx0;
		float ratio = bufWidth/(float)width;
		for( DWORD j=0; j<width; j++ )
		{
			sx = j*ratio;
			sx0 = int(sx+0.5);
			indexes[j] = 4*sx0;
		}
	}

	if( siz == TXT_SIZE_16b )
	{
		WORD *frameBufferBase = (WORD*)(g_pRDRAMu8+addr);

		if( surf_fmt==D3DFMT_A8R8G8B8 || surf_fmt==D3DFMT_X8R8G8B8 )
		{
			int  sy0;
			float ratio = bufHeight/(float)height;

			for( DWORD i=startline; i<endline; i++ )
			{
				sy0 = int(i*ratio+0.5);

				WORD *pD = frameBufferBase + i * pitch;
				BYTE *pS0 = (BYTE *)dlre.pBits + sy0 * dlre.Pitch;

				for( DWORD j=0; j<width; j++ )
				{
					// Point
					BYTE r = pS0[indexes[j]+2];
					BYTE g = pS0[indexes[j]+1];
					BYTE b = pS0[indexes[j]+0];
					BYTE a;
					if( surf_fmt==D3DFMT_X8R8G8B8 )
					{
						a = (r+g+b)>1 ? 0xFF : 0;
					}
					else
					{
						a = pS0[indexes[j]+3];
					}

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
	else if( siz == TXT_SIZE_8b && fmt == TXT_FMT_CI )
	{
		BYTE *frameBufferBase = (BYTE*)(g_pRDRAMu8+addr);

		if( surf_fmt==D3DFMT_A8R8G8B8 )
		{
			WORD tempword;
			InitTlutReverseLookup();

			for( DWORD i=startline; i<endline; i++ )
			{
				BYTE *pD = frameBufferBase + i * width;
				BYTE *pS = (BYTE *)dlre.pBits + i*bufHeight/height * dlre.Pitch;
				for( DWORD j=0; j<width; j++ )
				{
					int pos = 4*(j*bufWidth/width);
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
	else if( siz == TXT_SIZE_8b && fmt == TXT_FMT_I )
	{
		BYTE *frameBufferBase = (BYTE*)(g_pRDRAMu8+addr);

		if( surf_fmt==D3DFMT_A8R8G8B8 )
		{
			int sy0;
			float ratio = bufHeight/(float)height;

			for( DWORD i=startline; i<endline; i++ )
			{
				sy0 = int(i*ratio+0.5);

				BYTE *pD = frameBufferBase + i * width;
				BYTE *pS0 = (BYTE *)dlre.pBits + sy0 * dlre.Pitch;

				for( DWORD j=0; j<width; j++ )
				{
					// Point
					DWORD r = pS0[indexes[j]+2];
					DWORD g = pS0[indexes[j]+1];
					DWORD b = pS0[indexes[j]+0];

					// Liner
					*(pD+(j^3)) = (BYTE)((r+b+g)/3);

				}
			}
		}
		else
		{
			//DebuggerAppendMsg("Copy %sb FrameBuffer to Rdram, not implemented", pszImgSize[siz]);
		}
		DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Copy %sb FrameBuffer to Rdram", pszImgSize[siz]);});
	}

	if( backBuffer2 )	
	{
		backBuffer2->UnlockRect();
		backBuffer2->Release();
	}
	else
		backBuffer->UnlockRect();

	if( surf == NULL )
		backBuffer->Release();
}

bool CDXGraphicsContext::CreateFontObjects()
{
 
	m_bFontIsCreated = true;
	return true;
}
bool CDXGraphicsContext::DrawText(char *str, RECT &rect, int alignment)
{
	 
	return true;
}
