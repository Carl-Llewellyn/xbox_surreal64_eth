/*
 * XBoxMediaPlayer
 * Copyright (c) 2002 d7o3g4q and RUNTiME
 * Portions Copyright (c) by the authors of ffmpeg and xvid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

//-----------------------------------------------------------------------------
// File: Panel.h
//
// Desc: Support class for rendering a panel image.
//-----------------------------------------------------------------------------
#ifndef Panel_H
#define Panel_H

#include <xtl.h>
 

//-----------------------------------------------------------------------------
// Name: class CPanel
// Desc: Class for rendering an panel image.
//-----------------------------------------------------------------------------
class CPanel 
{
protected:
    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };

    LPDIRECT3DDEVICE8       m_pd3dDevice;
    LPDIRECT3DTEXTURE8      m_pTexture;
    LPDIRECT3DVERTEXBUFFER8 m_pVB;
	FLOAT m_nWidth;
	FLOAT m_nHeight;
	BOOL  m_bManaged;

public:
	BOOL IsValid();
    // Constructor/destructor
    CPanel();
    ~CPanel();

    // Functions to create and destroy the internal objects
    HRESULT Create( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECT3DTEXTURE8 pd3dTexture, BOOL bManaged=FALSE);

    HRESULT Destroy();

    // Renders the panel
    HRESULT Render();
	HRESULT RenderAlpha();
	HRESULT Render(float x, float y);
	HRESULT Render(float x, float y, float w, float h, float x2, float y2, int bRotate=0);
	HRESULT Render(float x, float y, float nw, float nh, int nFilter,int bRotate=0);
	void SetRenderVariables(float x, float y, float nw, float nh);

	FLOAT GetWidth()	{ return m_nWidth; };
	FLOAT GetHeight()	{ return m_nHeight; };
};

#endif
