/**
 * Surreal 64 Launcher (C) 2003
 * 
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: buttza@hotmail.com, lantus@lantus-x.com
 */

#include "D3D.h"
#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <xbapp.h>

#include "../config.h"

int fontcenter = 320;
D3D g_d3d;
HRESULT g_hResult;

D3D::D3D(void)
{
}

D3D::~D3D(void)
{
}

bool D3D::Create()
{
	
	// create the direct 3d object
	m_pD3D = Direct3DCreate8(D3D_SDK_VERSION);

	if (m_pD3D == NULL)
		return false;

    // set up the structure used to create the d3d device
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.BackBufferWidth					= 640;
    d3dpp.BackBufferHeight					= 480;
    d3dpp.BackBufferFormat					= D3DFMT_A8R8G8B8;
    d3dpp.BackBufferCount					= 1;
	d3dpp.EnableAutoDepthStencil			= TRUE;
    d3dpp.AutoDepthStencilFormat			= D3DFMT_D16;
    d3dpp.SwapEffect						= D3DSWAPEFFECT_DISCARD;
	d3dpp.Flags								= D3DPRESENTFLAG_INTERLACED; 
    d3dpp.FullScreen_PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.FullScreen_RefreshRateInHz		= 60;

	//TESTME! HDTV Modes (Launcher only!), Skins should be modified accordingly
	//Parts of code by XPORT and nes6502
	DWORD videoFlags = XGetVideoFlags();

	if(XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I)
	{
		if(videoFlags & XC_VIDEO_FLAGS_PAL_60Hz)		// PAL 60 user
			d3dpp.FullScreen_RefreshRateInHz = 60;
		else
			d3dpp.FullScreen_RefreshRateInHz = 50;
	}


    if(XGetAVPack() == XC_AV_PACK_HDTV)
        {/*
                if(videoFlags & XC_VIDEO_FLAGS_HDTV_1080i)
                {
                        d3dpp.Flags            = D3DPRESENTFLAG_WIDESCREEN | D3DPRESENTFLAG_INTERLACED;
                        d3dpp.BackBufferWidth  = 1920;
                        d3dpp.BackBufferHeight = 1080;
                }*/
                if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p)
                {
                        d3dpp.Flags            = D3DPRESENTFLAG_PROGRESSIVE | D3DPRESENTFLAG_WIDESCREEN;
                        d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
                        d3dpp.BackBufferWidth  = 1280;
                        d3dpp.BackBufferHeight = 720;
						fontcenter = 640;
                }
				 else if(videoFlags & XC_VIDEO_FLAGS_HDTV_480p)
                {
                        d3dpp.Flags            = D3DPRESENTFLAG_PROGRESSIVE;
                        d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
                        d3dpp.BackBufferWidth  = 640;
                        d3dpp.BackBufferHeight = 480;
						fontcenter = 320;
                }
		}

	
		//480p 1.6 needs 32 bit backbuffer format
	 /*if(XGetAVPack() == XC_AV_PACK_HDTV){
		if( videoFlags & XC_VIDEO_FLAGS_HDTV_480p){
			d3dpp.Flags = D3DPRESENTFLAG_PROGRESSIVE;
			d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		}
	 }*/



// use an orthogonal matrix for the projection matrix
    D3DXMATRIX mat;
	g_Width = 640;
	g_Height = 480;
	D3DXMatrixOrthoOffCenterLH(&mat, (float)0, (float)g_Width, (float)g_Height, (float)0, 0.0f, 1.0f);
	

	if(XGetAVPack() == XC_AV_PACK_HDTV)
	{/*
          if(videoFlags & XC_VIDEO_FLAGS_HDTV_1080i)
		  {
			  g_Width = 1920;
				g_Height = 1080;
				D3DXMatrixOrthoOffCenterLH(&mat, 0, g_Width, g_Height, 0, 0.0f, 1.0f);
		  }*/
		  if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p)
		  {
				g_Width = 1280;
				g_Height = 720;
				D3DXMatrixOrthoOffCenterLH(&mat, (float)0, (float)g_Width, (float)g_Height, (float)0, 0.0f, 1.0f);
		  }
		  else
		  if(videoFlags & XC_VIDEO_FLAGS_HDTV_480p)
		  {
				g_Width = 640;
				g_Height = 480;
				D3DXMatrixOrthoOffCenterLH(&mat, (float)0, (float)g_Width, (float)g_Height, (float)0, 0.0f, 1.0f);
		  }
	}
	else
	{
    //if HDTV is not supported
				g_Width = 640;
				g_Height = 480;
				D3DXMatrixOrthoOffCenterLH(&mat, (float)0, (float)g_Width, (float)g_Height, (float)0, 0.0f, 1.0f);
	}
	
	m_pd3dDevice = g_pd3dDevice;

	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat);

	// use an identity matrix for the world and view matrices
	D3DXMatrixIdentity(&mat);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &mat);

	// disable lighting
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);


	return true;
}

void D3D::BeginRender()
{

	
	D3DVIEWPORT8 viewport;

		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = g_Width;
		viewport.Height = g_Height;
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;


    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 
                      D3DCOLOR_XRGB(0,0,0), 
						1.0f, 0);

	m_pd3dDevice->SetViewport(&viewport);

    m_pd3dDevice->BeginScene();
	m_pd3dDevice->SetFlickerFilter(1);
	m_pd3dDevice->SetSoftDisplayFilter(false);
}

void D3D::EndRender()
{
	m_pd3dDevice->EndScene();

	m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}



