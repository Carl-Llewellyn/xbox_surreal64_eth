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

#ifndef __TEXTUREHANDLER_H__
#define __TEXTUREHANDLER_H__

#include "Texture.h"
#include "CritSect.h"
#include "unique.h"

class TextureInfo
{
public:
	uint32 WidthToCreate;
	uint32 HeightToCreate;

	uint32 Address;
	void  *pPhysicalAddress;

	uint32 Format;
	uint32 Size;

	LONG  LeftToLoad;
	LONG  TopToLoad;
	uint32 WidthToLoad;
	uint32 HeightToLoad;
	uint32 Pitch;

	uint32 PalAddress;
	uint32 TLutFmt;
	uint32 Palette;
	
	BOOL  bSwapped;
	
	uint32 maskS;
	uint32 maskT;

	BOOL  clampS;
	BOOL  clampT;
	BOOL  mirrorS;
	BOOL  mirrorT;

	int	  tileNo;

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
	TextureEntry():
		pTexture(NULL),pEnhancedTexture(NULL),txtrBufIdx(0) {}

	~TextureEntry()
	{
		SAFE_DELETE(pTexture);
		SAFE_DELETE(pEnhancedTexture);
	}
	
	struct TextureEntry *pNext;		// Must be first element!

	struct TextureEntry *pNextYoungest;
	struct TextureEntry *pLastYoungest;

	TextureInfo ti;
	DWORD		dwCRC;
	DWORD		dwPalCRC;

	DWORD	dwUses;			// Total times used (for stats)
	DWORD	dwTimeLastUsed;	// timeGetTime of time of last usage
	DWORD	FrameLastUsed;	// Frame # that this was last used
	DWORD	FrameLastUpdated;

	CTexture	*pTexture;
	CTexture	*pEnhancedTexture;

	DWORD		dwEnhancementFlag;
	int			txtrBufIdx;

	TextureEntry *lastEntry;
} TextureEntry;


//*****************************************************************************
// Texture cache implementation
//*****************************************************************************
class CTextureManager
{
protected:
	TextureEntry * CreateEntry(DWORD dwAddr, DWORD dwWidth, DWORD dwHeight);
	void MakeTextureYoungest(TextureEntry *pEntry);
	void AddTextureEntry(TextureEntry *pEntry);
	void RemoveTextureEntry(TextureEntry * pEntry);
	void AddToRecycleList(TextureEntry *pEntry);
	TextureEntry * ReviveUsedTexture( DWORD width, DWORD height );
	TextureEntry * GetEntry(TextureInfo * pti);
	
	void DecompressTexture(TextureEntry * pEntry, bool fromTMEM);
	void DecompressTexture_16(TextureEntry * pEntry, bool fromTMEM);

	void Clamp(void *array, DWORD width, DWORD towidth, DWORD arrayWidth, DWORD rows, int flag, int size );
	void ClampS32(DWORD *array, DWORD width, DWORD towidth, DWORD arrayWidth, DWORD rows);
	void ClampS16(WORD *array, DWORD width, DWORD towidth, DWORD arrayWidth, DWORD rows);
	void ClampT32(DWORD *array, DWORD height, DWORD toheight, DWORD arrayWidth, DWORD cols);
	void ClampT16(WORD *array, DWORD height, DWORD toheight, DWORD arrayWidth, DWORD cols);

	void Mirror(void *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows, int flag, int size );
	void MirrorS32(DWORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows);
	void MirrorS16(WORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows);
	void MirrorT32(DWORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols);
	void MirrorT16(WORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols);

	void Wrap(void *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows, int flag, int size );
	void WrapS32(DWORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows);
	void WrapS16(WORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows);
	void WrapT32(DWORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols);
	void WrapT16(WORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols);

	void ExpandTextureS(TextureEntry * pEntry);
	void ExpandTextureT(TextureEntry * pEntry);
	void ExpandTexture(TextureEntry * pEntry, DWORD sizeOfLoad, DWORD sizeToCreate, DWORD sizeCreated,
		int arrayWidth, int flag, int mask, int mirror, int clamp, DWORD otherSize);

	DWORD Hash(DWORD dwValue);

	void updateColorTexture(CTexture *ptexture, DWORD color);
	
protected:
	TextureEntry * m_pFirstUsedSurface;
	TextureEntry ** m_pTextureHash;
	DWORD m_dwTextureHashSize;

	TextureEntry m_blackTextureEntry;
	TextureEntry m_PrimColorTextureEntry;
	TextureEntry m_EnvColorTextureEntry;
	TextureEntry m_LODFracTextureEntry;
	TextureEntry m_PrimLODFracTextureEntry;
	TextureEntry * GetPrimColorTexture(DWORD color);
	TextureEntry * GetEnvColorTexture(DWORD color);
	TextureEntry * GetLODFracTexture(BYTE fac);
	TextureEntry * GetPrimLODFracTexture(BYTE fac);

	// Ez0n3 - old way
	DWORD m_currentTextureMemUsage;
	
	TextureEntry *m_pYoungestTexture;
	TextureEntry *m_pOldestTexture;

public:
	CTextureManager();
	~CTextureManager();

	TextureEntry * GetBlackTexture(void);
	TextureEntry * GetConstantColorTexture(DWORD constant);
	TextureEntry * GetTexture(TextureInfo * pgti, bool fromTMEM, bool doCRCCheck=true, bool AutoExtendTexture = false);
	
	void PurgeOldTextures();
	void DropTextures();
	bool CleanUp();
	
#ifndef OLDTXTCACHE
	void FreeTextures();
#endif
	
#ifdef _DEBUG
	TextureEntry * GetCachedTexture(DWORD tex);
	DWORD GetNumOfCachedTexture();
#endif
};

extern CTextureManager gTextureManager;		// The global instance of CTextureManager class


// Ez0n3 - reinstate max video mem until freakdave finishes this
extern bool g_bUseSetTextureMem;
extern DWORD g_maxTextureMemUsage;


#endif