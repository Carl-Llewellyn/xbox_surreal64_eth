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
	CTexture(dwWidth, dwHeight, asRenderTarget)
{
	if (dwWidth < 1)
		dwWidth = 16;
	else if (dwWidth > 1024)
		dwWidth = 1024;

	if (dwHeight < 1)
		dwHeight = 16;
	else if (dwHeight > 1024)
		dwHeight = 1024;

	if (m_pTexture)
	{
		LPDIRECT3DTEXTURE8(m_pTexture)->Release();
		m_pTexture = NULL;
	}

	m_pTexture = CreateTexture(dwWidth, dwHeight);
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

void CDirectXTexture::EndUpdate(DrawInfo *di)
{
	if (m_pTexture == NULL)
		return;

	LPDIRECT3DTEXTURE8 pTexture = (LPDIRECT3DTEXTURE8)m_pTexture;
	LPDIRECT3DSURFACE8 pSurface;
	
	pTexture->GetSurfaceLevel(0, &pSurface);

	RECT srcRect;
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = di->dwWidth;
	srcRect.bottom = di->dwHeight;

	D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL,
		di->lpSurface, (GetPixelSize() == 4)?D3DFMT_A8R8G8B8:D3DFMT_A4R4G4B4, 
		di->lPitch, NULL, &srcRect, D3DX_FILTER_BOX, 0);

	pSurface->Release();
}

LPDAEDALUSTEXTURE CDirectXTexture::CreateTexture(DWORD dwWidth, DWORD dwHeight)
{
	HRESULT hr;
	LPDIRECT3DTEXTURE8 lpSurf;

	// textures are always 16bit, we let the plugin create 32bit textures
	// then we reduce them to 16bit in EndUpdate
	D3DFORMAT pf = D3DFMT_A4R4G4B4;
	UINT dwNumMaps = 1;

	m_dwCreatedTextureWidth		= dwWidth;
	m_dwCreatedTextureHeight	= dwHeight;
	m_dwWidth					= dwWidth;
	m_dwHeight					= dwHeight;

	// if texture > 512x512 (eg. 1024x1024) use 16bit
	if (dwWidth * dwHeight > 0x40000)
		m_dwTextureFmt = TEXTURE_FMT_A4R4G4B4;
	else
		m_dwTextureFmt = TEXTURE_FMT_A8R8G8B8;

	if(m_bAsRenderTarget)
	{
		D3DXCheckTextureRequirements(g_pD3DDev, &m_dwCreatedTextureWidth, &m_dwCreatedTextureHeight, &dwNumMaps, D3DUSAGE_RENDERTARGET, &pf, D3DPOOL_DEFAULT);
		hr = D3DXCreateTexture(g_pD3DDev, m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 1, D3DUSAGE_RENDERTARGET, pf, D3DPOOL_DEFAULT, &lpSurf);
	}
	else
	{
		D3DXCheckTextureRequirements(g_pD3DDev, &m_dwCreatedTextureWidth, &m_dwCreatedTextureHeight, &dwNumMaps, 0, &pf, D3DPOOL_MANAGED);
		hr = D3DXCreateTexture(g_pD3DDev, m_dwCreatedTextureWidth, m_dwCreatedTextureHeight, 1, 0, pf, D3DPOOL_MANAGED, &lpSurf);
	}
	
	m_fYScale = (float)m_dwCreatedTextureHeight/(float)m_dwHeight;
	m_fXScale = (float)m_dwCreatedTextureWidth/(float)m_dwWidth;
	
	if(!CDirectXGraphicsContext::IsResultGood(hr))
		return NULL;
	
	return lpSurf;		
}
