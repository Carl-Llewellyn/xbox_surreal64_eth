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



//////////////////////////////////////////
// Constructors / Deconstructors

// Probably shouldn't need more than 4096 * 4096
BYTE g_ucTempBuffer[1024*1024*2];

CTexture::CTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget) :
	m_pTexture(NULL),
	m_dwTextureFmt(TEXTURE_FMT_A8R8G8B8),
	m_dwWidth(dwWidth),
	m_dwHeight(dwHeight),
	m_dwCreatedTextureWidth(dwWidth),
	m_dwCreatedTextureHeight(dwHeight),
	m_fXScale(1.0f),
	m_fYScale(1.0f),
	m_bScaledS(false),
	m_bScaledT(false),
	m_bClampedS(false),
	m_bClampedT(false),
	m_bAsRenderTarget(asRenderTarget)
{
	// fix me, do something here
}


CTexture::~CTexture(void)
{
}

DaedalusTexFmt CTexture::GetSurfaceFormat(void)
{
	if (m_pTexture == NULL)
		return TEXTURE_FMT_UNKNOWN;
	else
		return m_dwTextureFmt;
}

DWORD CTexture::GetPixelSize()
{
	if( m_dwTextureFmt == TEXTURE_FMT_A8R8G8B8 )
		return 4;
	else
		return 2;
}

// There are reasons to create this function. D3D and OGL will only create surface of width and height
// as 2's pow, for example, N64's 20x14 image, D3D and OGL will create a 32x16 surface.
// When we using such a surface as D3D texture, and the U and V address is for the D3D and OGL surface
// width and height. It is still OK if the U and V addr value is less than the real image within
// the D3D surface. But we will have problems if the U and V addr value is larger than it, or even
// large then 1.0.
// In such a case, we need to scale the image to the D3D surface dimension, to ease the U/V addr
// limition
void CTexture::ScaleImageToSurface(bool scaleS, bool scaleT)
{
	if( scaleS==false && scaleT==false)	return;

	// If the image is not scaled, call this function to scale the real image to
	// the D3D given dimension

	uint32 width = scaleS ? m_dwWidth : m_dwCreatedTextureWidth;
	uint32 height = scaleT ? m_dwHeight : m_dwCreatedTextureHeight;

	DWORD xDst, yDst;
	DWORD xSrc, ySrc;

	DrawInfo di;

	if (!StartUpdate(&di))
	{
		return;
	}

	int pixSize = GetPixelSize();

	// Copy across from the temp buffer to the surface
	switch (pixSize)
	{
	case 4:
		{
			memcpy((BYTE*)g_ucTempBuffer, (BYTE*)(di.lpSurface), m_dwHeight*m_dwCreatedTextureWidth*4);

			DWORD * pDst;
			DWORD * pSrc;
			
			for (yDst = 0; yDst < m_dwCreatedTextureHeight; yDst++)
			{
				// ySrc ranges from 0..m_dwHeight
				// I'd rather do this but sometimes very narrow (i.e. 1 pixel)
				// surfaces are created which results in  /0...
				//ySrc = (yDst * (m_dwHeight-1)) / (d3dTextureHeight-1);
				ySrc = (DWORD)((yDst * height) / m_dwCreatedTextureHeight+0.49f);
				
				pSrc = (DWORD*)((BYTE*)g_ucTempBuffer + (ySrc * m_dwCreatedTextureWidth * 4));
				pDst = (DWORD*)((BYTE*)di.lpSurface + (yDst * di.lPitch));
				
				for (xDst = 0; xDst < m_dwCreatedTextureWidth; xDst++)
				{
					xSrc = (DWORD)((xDst * width) / m_dwCreatedTextureWidth+0.49f);
					pDst[xDst] = pSrc[xSrc];
				}
			}
		}
		
		break;
	case 2:
		{
			memcpy((BYTE*)g_ucTempBuffer, (BYTE*)(di.lpSurface), m_dwHeight*m_dwCreatedTextureWidth*2);

			WORD * pDst;
			WORD * pSrc;
			
			for (yDst = 0; yDst < m_dwCreatedTextureHeight; yDst++)
			{
				// ySrc ranges from 0..m_dwHeight
				ySrc = (yDst * height) / m_dwCreatedTextureHeight;
				
				pSrc = (WORD*)((BYTE*)g_ucTempBuffer + (ySrc * m_dwCreatedTextureWidth * 2));
				pDst = (WORD*)((BYTE*)di.lpSurface + (yDst * di.lPitch));
				
				for (xDst = 0; xDst < m_dwCreatedTextureWidth; xDst++)
				{
					xSrc = (xDst * width) / m_dwCreatedTextureWidth;
					pDst[xDst] = pSrc[xSrc];
				}
			}
		}
		break;
			
	}
			
	EndUpdate(&di);

	if( scaleS ) m_bScaledS = true;
	if( scaleT ) m_bScaledT = true;
}

void CTexture::ClampImageToSurfaceS()
{
	if( !m_bClampedS && m_dwWidth < m_dwCreatedTextureWidth )
	{		
		DrawInfo di;
		if( StartUpdate(&di) )
		{
			if(  m_dwTextureFmt == TEXTURE_FMT_A8R8G8B8 )
			{
				for( DWORD y = 0; y<m_dwHeight; y++ )
				{
					DWORD* line = (DWORD*)((BYTE*)di.lpSurface+di.lPitch*y);
					DWORD val = line[m_dwWidth-1];
					for( DWORD x=m_dwWidth; x<m_dwCreatedTextureWidth; x++ )
					{
						line[x] = val;
					}
				}
			}
			else
			{
				for( DWORD y = 0; y<m_dwHeight; y++ )
				{
					WORD* line = (WORD*)((BYTE*)di.lpSurface+di.lPitch*y);
					WORD val = line[m_dwWidth-1];
					for( DWORD x=m_dwWidth; x<m_dwCreatedTextureWidth; x++ )
					{
						line[x] = val;
					}
				}
			}
			EndUpdate(&di);
		}
	}
	m_bClampedS = true;
}

void CTexture::ClampImageToSurfaceT()
{
	if( !m_bClampedT && m_dwHeight < m_dwCreatedTextureHeight )
	{
		DrawInfo di;
		if( StartUpdate(&di) )
		{
			if(  m_dwTextureFmt == TEXTURE_FMT_A8R8G8B8 )
			{
				DWORD* linesrc = (DWORD*)((BYTE*)di.lpSurface+di.lPitch*(m_dwHeight-1));
				for( DWORD y = m_dwHeight; y<m_dwCreatedTextureHeight; y++ )
				{
					DWORD* linedst = (DWORD*)((BYTE*)di.lpSurface+di.lPitch*y);
					for( DWORD x=0; x<m_dwCreatedTextureWidth; x++ )
					{
						linedst[x] = linesrc[x];
					}
				}
			}
			else
			{
				WORD* linesrc = (WORD*)((BYTE*)di.lpSurface+di.lPitch*(m_dwHeight-1));
				for( DWORD y = m_dwHeight; y<m_dwCreatedTextureHeight; y++ )
				{
					WORD* linedst = (WORD*)((BYTE*)di.lpSurface+di.lPitch*y);
					for( DWORD x=0; x<m_dwCreatedTextureWidth; x++ )
					{
						linedst[x] = linesrc[x];
					}
				}
			}
			EndUpdate(&di);
		}
	}
	m_bClampedT = true;
}

bool CTexture::DumpImageAsRAW(LPCTSTR szFileName, DWORD dwWidth, DWORD dwHeight)
{
	DrawInfo di;
	FILE * fp = fopen(szFileName, "wb");
	if (fp == NULL)
		return false;
	
	if (!StartUpdate(&di))
	{
		fclose(fp);
		return false;
	}
	
	DWORD *pSrc = (DWORD *)di.lpSurface;
	LONG lPitch = di.lPitch;

	for (DWORD y = 0; y < dwHeight; y++)
	{
		DWORD * dwSrc = (DWORD *)((BYTE *)pSrc + y*lPitch);
		for (DWORD x = 0; x < dwWidth; x++)
		{
			DWORD dw = dwSrc[x];

			BYTE dwRed   = (BYTE)((dw & 0x00FF0000)>>16);
			BYTE dwGreen = (BYTE)((dw & 0x0000FF00)>>8 );
			BYTE dwBlue  = (BYTE)((dw & 0x000000FF)    );
			BYTE dwAlpha = (BYTE)((dw & 0xFF000000)>>24);

			fwrite(&dwRed, 1,1,fp);
			fwrite(&dwGreen, 1,1,fp);
			fwrite(&dwBlue, 1,1,fp);
			fwrite(&dwAlpha, 1,1,fp);
		}
	}
	EndUpdate(&di);

	fclose(fp);

	return true;
}

