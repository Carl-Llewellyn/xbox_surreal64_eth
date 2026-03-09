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

#ifndef __TEXTUREHANDLER_H__
#define __TEXTUREHANDLER_H__

//#include "RDP.h"

#include "Texture.h"
#include "CritSect.h"
#include "unique.h"

class TextureInfo
{
public:
	DWORD WidthToCreate;
	DWORD HeightToCreate;

	DWORD Address;
	void  *pPhysicalAddress;

	DWORD Format;
	DWORD Size;

	LONG  LeftToLoad;
	LONG  TopToLoad;
	DWORD WidthToLoad;
	DWORD HeightToLoad;
	DWORD Pitch;

	DWORD PalAddress;
	DWORD TLutFmt;
	DWORD Palette;
	
	BOOL  bSwapped;
	
	DWORD maskS;
	DWORD maskT;

	BOOL  clampS;
	BOOL  clampT;
	BOOL  mirrorS;
	BOOL  mirrorT;

	inline TextureInfo& operator = (const TextureInfo& src)
	{
		memcpy(this, &src, sizeof( TextureInfo ));
		return *this;
	}

	inline TextureInfo& operator = (const Tile& tile)
	{
		Format = tile.dwFormat;
		Size = tile.dwSize;
		Pitch = tile.dwPitch;
		Palette = tile.dwPalette;
		
		maskS = tile.dwMaskS;
		maskT = tile.dwMaskT;
		mirrorS = tile.bMirrorS;
		mirrorT = tile.bMirrorT;
		clampS = tile.bClampS;
		clampT = tile.bClampT;

		return *this;
	}

	inline bool operator == ( const TextureInfo& sec)
	{
		return (
			Address == sec.Address &&
			WidthToLoad == sec.WidthToLoad &&
			HeightToLoad == sec.HeightToLoad &&
			WidthToCreate == sec.WidthToCreate &&
			HeightToCreate == sec.HeightToCreate &&
			maskS == sec.maskS &&
			maskT == sec.maskT &&
			TLutFmt == sec.TLutFmt &&
			PalAddress == sec.PalAddress &&
			Palette == sec.Palette &&
			LeftToLoad == sec.LeftToLoad &&
			TopToLoad == sec.TopToLoad &&			
			Format == sec.Format &&			
			Size == sec.Size &&
			Pitch == sec.Pitch &&
			bSwapped == sec.bSwapped &&
			TLutFmt == sec.TLutFmt &&
			mirrorS == sec.mirrorS &&
			mirrorT == sec.mirrorT &&
			clampS == sec.clampS &&
			clampT == sec.clampT
			);
	}

	inline bool isEqual(const TextureInfo& sec)
	{
		return (*this == sec);
	}
	
} ;



typedef struct TextureEntry
{
	TextureEntry():pTexture(NULL),pMirroredTexture(NULL) {}

	~TextureEntry()
	{
		SAFE_DELETE(pTexture);
		SAFE_DELETE(pMirroredTexture);
	}
	
	struct TextureEntry *pNext;		// Must be first element!

	TextureInfo ti;
	DWORD		dwCRC;
	DWORD		dwPalCRC;

	DWORD	dwUses;			// Total times used (for stats)
	DWORD	dwTimeLastUsed;	// timeGetTime of time of last usage
	u32		FrameLastUsed;	// Frame # that this was last used

	CTexture	*pTexture;
	CTexture	*pMirroredTexture;

	DWORD		dwEnhancementFlag;

	TextureEntry *lastEntry;
} TextureEntry;

extern BOOL g_bTHMakeTexturesBlue;
extern BOOL g_bTHDumpTextures;


//*****************************************************************************
// Texture cache implementation
//*****************************************************************************
class CTextureCache
{
protected:
	TextureEntry * CreateEntry(DWORD dwAddress, DWORD dwWidth, DWORD dwHeight);
	void AddTextureEntry(TextureEntry *pEntry);
	void RemoveTextureEntry(TextureEntry * pEntry);
	void AddToRecycleList(TextureEntry *pEntry);
	TextureEntry * ReviveUsedTexture( u32 width, u32 height );
	TextureEntry * GetEntry(TextureInfo * pti);
	
	void DecompressTexture(TextureEntry * pEntry);
	void DecompressTexture_16(TextureEntry * pEntry);

	void Clamp(void *array, u32 width, u32 towidth, u32 arrayWidth, u32 rows, int flag, int size );
	void ClampS32(DWORD *array, u32 width, u32 towidth, u32 arrayWidth, u32 rows);
	void ClampS16(WORD *array, u32 width, u32 towidth, u32 arrayWidth, u32 rows);
	void ClampT32(DWORD *array, u32 height, u32 toheight, u32 arrayWidth, u32 cols);
	void ClampT16(WORD *array, u32 height, u32 toheight, u32 arrayWidth, u32 cols);

	void Mirror(void *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows, int flag, int size );
	void MirrorS32(DWORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows);
	void MirrorS16(WORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows);
	void MirrorT32(DWORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols);
	void MirrorT16(WORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols);

	void Wrap(void *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows, int flag, int size );
	void WrapS32(DWORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows);
	void WrapS16(WORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows);
	void WrapT32(DWORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols);
	void WrapT16(WORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols);

	void ExpandTextureS(TextureEntry * pEntry);
	void ExpandTextureT(TextureEntry * pEntry);
	void ExpandTexture(TextureEntry * pEntry, u32 sizeOfLoad, u32 sizeToCreate, u32 sizeCreated,
		int arrayWidth, int flag, int mask, int mirror, int clamp, u32 otherSize);

	u32 CalculateCRC(TextureInfo * pti) const;
	u32 Hash(u32 dwValue);

	void updateColorTexture(CTexture *ptexture, DWORD color);
	
protected:
	TextureEntry * m_pFirstUsedSurface;
	TextureEntry ** m_pTextureHash;
	u32 m_dwTextureHashSize;

	TextureEntry m_blackTextureEntry;
	TextureEntry m_PrimColorTextureEntry;
	TextureEntry m_EnvColorTextureEntry;
	TextureEntry m_LODFracTextureEntry;
	TextureEntry m_PrimLODFracTextureEntry;
	TextureEntry * GetPrimColorTexture(DWORD color);
	TextureEntry * GetEnvColorTexture(DWORD color);
	TextureEntry * GetLODFracTexture(BYTE fac);
	TextureEntry * GetPrimLODFracTexture(BYTE fac);

public:
	CTextureCache();
	~CTextureCache();

	TextureEntry * GetBlackTexture(void);
	TextureEntry * GetConstantColorTexture(DWORD constant);
	TextureEntry * GetTexture(TextureInfo * pgti, bool doCRCCheck=true, bool AutoExtendTexture = false);
	void PurgeOldTextures();
	void DropTextures();

	HRESULT InvalidateDeviceObjects();
	
#ifdef _DEBUG
	TextureEntry * GetCachedTexture(u32 tex);
	u32 GetNumOfCachedTexture();
#endif
};

extern CTextureCache gTextureCache;		// The global instance of CTextureCache class

#endif