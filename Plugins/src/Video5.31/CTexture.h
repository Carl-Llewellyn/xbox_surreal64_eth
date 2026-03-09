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


#ifndef __SURFACEHANDLER_H__
#define __SURFACEHANDLER_H__

/////////////// Define a struct to use as
///////////////  storage for all the surfaces
///////////////  created so far.
class CTexture;

typedef struct {
	unsigned short int	dwWidth;			// Describes the width of the real texture area. Use lPitch to move between successive lines
	unsigned short int	dwHeight;			// Describes the height of the real texture area
	unsigned short int	dwCreatedWidth;		// Describes the width of the created texture area. Use lPitch to move between successive lines
	unsigned short int	dwCreatedHeight;	// Describes the height of the created texture area
	LONG		lPitch;				// Specifies the number of bytes on each row (not necessarily bitdepth*width/8)
	VOID		*lpSurface;			// Pointer to the top left pixel of the image
} DrawInfo;


enum DaedalusTexFmt {
	TEXTURE_FMT_A8R8G8B8,
	TEXTURE_FMT_A4R4G4B4,
	TEXTURE_FMT_UNKNOWN,
};

class CTexture
{
public:
	virtual ~CTexture();

	DWORD		m_dwWidth;			// The requested Texture w/h
	DWORD		m_dwHeight;

	unsigned int		m_dwCreatedTextureWidth;	// What was actually created
	unsigned int		m_dwCreatedTextureHeight;

	float		m_fXScale;		// = m_dwCorrectedWidth/m_dwWidth
	float		m_fYScale;		// = m_dwCorrectedHeight/m_dwWidth

	bool		m_bScaledS;
	bool		m_bScaledT;

	bool		m_bClampedS;
	bool		m_bClampedT;
	
	bool		m_bAsRenderTarget;

	//virtual void ScaleImageToSurface(bool scaleS=true, bool scaleT=true);
	virtual void ClampImageToSurfaceS();
	virtual void ClampImageToSurfaceT();

	virtual LPDAEDALUSTEXTURE GetTexture() { return m_pTexture; }

	DWORD			GetPixelSize();
	DaedalusTexFmt	GetSurfaceFormat(void);	// Surface pixel format...
	inline void		SetOthersVariables(void)
	{
		m_bClampedS = m_bScaledS = (m_dwWidth == m_dwCreatedTextureWidth);
		m_bClampedT = m_bScaledT = (m_dwHeight == m_dwCreatedTextureHeight);
	}

	// Provides access to "surface"
	virtual bool StartUpdate(DrawInfo *di)=0;
	virtual void EndUpdate(DrawInfo *di)=0;

	virtual bool DumpImageAsRAW(LPCTSTR szFileName, DWORD dwWidth, DWORD dwHeight);

protected:
	CTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget = false);
	LPDAEDALUSTEXTURE	m_pTexture;
	DaedalusTexFmt		m_dwTextureFmt;
};

#endif