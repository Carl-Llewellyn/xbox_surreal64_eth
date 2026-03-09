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
#ifdef _XBOX
#include <xgraphics.h>
#endif

extern BYTE g_ucTempBuffer[1024*1024*2];

CDirectXTexture::CDirectXTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget) :
	CTexture(dwWidth,dwHeight,asRenderTarget)
{
	LPDIRECT3DTEXTURE8 lpSurf;

	if (dwWidth < 1)
		dwWidth = 1;
	else if (dwWidth > 1024)
		dwWidth = 1024;

	if (dwHeight < 1)
		dwHeight = 1;
	else if (dwHeight > 1024)
		dwHeight = 1024;

	if (dwWidth < m_dwWidth)
		TRACE2("New width (%d) < Old Width (%d)", dwWidth, m_dwWidth)
	if (dwHeight < m_dwHeight)
		TRACE2("New height (%d) < Old height (%d)", dwHeight, m_dwHeight)
	
	if (dwWidth*dwHeight > 256*256)
		TRACE2("Large texture: width (%d) , height (%d)", dwWidth, dwHeight)

	lpSurf = (LPDIRECT3DTEXTURE8)CreateTexture(dwWidth, dwHeight);
	if (lpSurf == NULL)
		return;

	// Copy from old surface to new surface
	if (m_pTexture != NULL)
	{
		LPDIRECT3DTEXTURE8(m_pTexture)->Release();
		m_pTexture = NULL;
	}

	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;
	m_pTexture = lpSurf;
}
	
CDirectXTexture::~CDirectXTexture()
{
	if (m_pTexture)
	{
		LPDIRECT3DTEXTURE8(m_pTexture)->Release();
		m_pTexture = NULL;
	}
	m_dwWidth = 0;
	m_dwHeight = 0;
}



//////////////////////////////////////////////////
// Get information about the DIBitmap
// This locks the bitmap (and stops 
// it from being resized). Must be matched by a
// call to EndUpdate();
bool CDirectXTexture::StartUpdate(DrawInfo *di)
{
	if (m_pTexture == NULL)
		return false;


	di->lpSurface = g_ucTempBuffer;
	di->dwWidth = m_dwCreatedTextureWidth;
	di->dwHeight = m_dwCreatedTextureHeight;
	di->lPitch = m_dwCreatedTextureWidth * GetPixelSize();
 
	return true;
}

///////////////////////////////////////////////////
// This releases the DIB information, allowing it
// to be resized again
void CDirectXTexture::EndUpdate(DrawInfo *di)
{
	if (m_pTexture == NULL)
		return;

#ifdef _XBOX
	D3DLOCKED_RECT d3d_lr;
	HRESULT hr;

	hr = LPDIRECT3DTEXTURE8(m_pTexture)->LockRect(0, &d3d_lr, NULL, 0);
	if (SUCCEEDED(hr))
	{
		XGSwizzleRect( g_ucTempBuffer, 0, NULL, d3d_lr.pBits,
					  di->dwWidth, di->dwHeight, 
					  NULL, GetPixelSize() );
 	 
	}
#else
	LPDIRECT3DTEXTURE8(m_pTexture)->UnlockRect( 0 );
#endif
}


LPDAEDALUSTEXTURE CDirectXTexture::CreateTexture(DWORD dwWidth, DWORD dwHeight)
{
	HRESULT hr;
	LPDIRECT3DTEXTURE8 lpSurf;
	LPDIRECT3DDEVICE8 pD3DDev;
	unsigned int dwNumMaps = 1;

	D3DFORMAT pf = D3DFMT_A4R4G4B4;	// Force to use 16 bit texture
	//D3DFORMAT pf = D3DFMT_A8R8G8B8;	// Force to use 32 bit texture
	//D3DFORMAT pf = ((CDirectXGraphicsContext*)(CGraphicsContext::g_pGraphicsContext))->GetFormat();

	switch( pf )
	{
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
		pf = D3DFMT_A4R4G4B4;
		break;
	default:
		pf = D3DFMT_A8R8G8B8;
		break;
	}

	m_dwCreatedTextureWidth = dwWidth;
	m_dwCreatedTextureHeight = dwHeight;

	//if( m_dwCreatedTextureWidth>256) m_dwCreatedTextureWidth = 256;	// testing
	//if( m_dwCreatedTextureHeight>256) m_dwCreatedTextureHeight= 256;

	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;

	pD3DDev = g_pD3DDev;

	if( m_bAsRenderTarget )
	{
		D3DXCheckTextureRequirements(pD3DDev, &m_dwCreatedTextureWidth, &m_dwCreatedTextureHeight, &dwNumMaps, D3DUSAGE_RENDERTARGET, &pf, D3DPOOL_DEFAULT);
		hr = D3DXCreateTexture(pD3DDev, m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 1, D3DUSAGE_RENDERTARGET, pf, D3DPOOL_DEFAULT  , &lpSurf);
	}
	else
	{
		D3DXCheckTextureRequirements(pD3DDev, &m_dwCreatedTextureWidth, &m_dwCreatedTextureHeight, &dwNumMaps, 0, &pf, D3DPOOL_MANAGED);
		hr = D3DXCreateTexture(pD3DDev, m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 1, 0, pf, D3DPOOL_MANAGED  , &lpSurf);
	}

#ifdef DEBUGGING_ENABLED
	if( m_dwCreatedTextureWidth<dwWidth || m_dwCreatedTextureHeight<dwHeight)
	{
		TRACE0("!!Check me, created texture is smaller");
	}
#endif
	
	m_fYScale = (float)m_dwCreatedTextureHeight/(float)m_dwHeight;
	m_fXScale = (float)m_dwCreatedTextureWidth/(float)m_dwWidth;

	if (pf != D3DFMT_A8R8G8B8 && pf != D3DFMT_A4R4G4B4)
	{
		static BOOL bWarned = FALSE;
		if (!bWarned)
		{
			bWarned = TRUE;
		}
	}

	// D3D likes texture w/h to be a power of two, so the condition below
	// will almost always hold. 
	// D3D should usually create textures large enough (on nVidia cards anyway),
	// and so there will usually be some "slack" left in the texture (blank space
	// that isn't used). The D3DRender code takes care of this (mostly)
	// Voodoo cards are limited to 256x256 and so often textures will be
	// created that are too small for the required dimensions. We compensate for
	// this by passing in a dummy area of memory when the surface is locked,
	// and copying across pixels to the real surface when the surface is unlocked.
	// In this case there will be no slack and D3DRender takes this into account.
	// 
	/*if (dwWidth != d3dTextureWidth ||
		dwHeight != d3dTextureHeight)
	{
		DebuggerAppendMsg("Couldn't create texture of size %d x %d (get %d x %d)",
			dwWidth, dwHeight, d3dTextureWidth, d3dTextureHeight);
	}*/

	// HACK - we should only assign this when m_pTexture is assigned!
	
	if( pf == D3DFMT_A8R8G8B8 )
		m_dwTextureFmt = TEXTURE_FMT_A8R8G8B8;
	else
		m_dwTextureFmt = TEXTURE_FMT_A4R4G4B4;

	

	if( !CDirectXGraphicsContext::IsResultGood(hr))
	{
		TRACE2("!!Unable to create surface!! %d x %d", dwWidth, dwHeight);
		return NULL;
	}
	
	return lpSurf;		
}
