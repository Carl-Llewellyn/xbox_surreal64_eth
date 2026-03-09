/*
Copyright (C) 2002 Rice1964

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

// Texture related ucode

#undef NEW_TEXTURE_LOAD

uint32 g_TmemFlag[16];
void SetTmemFlag(DWORD tmemAddr, DWORD size);
bool IsTmemFlagValid(DWORD tmemAddr);
uint32 GetValidTmemInfoIndex(DWORD tmemAddr);

CTexture* TextureEnhancement(TextureEntry *pEntry);
CTexture* GetMirrorSurfaceHandler(DWORD dwTile, TextureEntry *pEntry);

union {
	BYTE	g_Tmem8bit[0x1000];
	uint16	g_Tmem16bit[0x800];
	uint32	g_Tmem32bit[0x300];
	uint64	g_Tmem64bit[0x200];
} g_Tmem;

extern TMEMLoadMapInfo g_tmemInfo0;				// Info for Tmem=0
extern TMEMLoadMapInfo g_tmemInfo1;				// Info for Tmem=0x100


/************************************************************************/
/*                                                                      */
/************************************************************************/

/*
#define G_IM_SIZ_4b_SHIFT  2
#define G_IM_SIZ_8b_SHIFT  1
#define G_IM_SIZ_16b_SHIFT 0
#define G_IM_SIZ_32b_SHIFT 0

#define G_IM_SIZ_4b_INCR  3
#define G_IM_SIZ_8b_INCR  1
#define G_IM_SIZ_16b_INCR 0
#define G_IM_SIZ_32b_INCR 0

#define G_IM_SIZ_4b		0
#define G_IM_SIZ_8b		1
#define G_IM_SIZ_16b	2
#define G_IM_SIZ_32b	3
*/

DWORD sizeShift[4] = {2,1,0,0};
DWORD sizeIncr[4] = {3,1,0,0};
DWORD sizeBytes[4] = {0,1,2,4};

/*
#define TXL2WORDS(txls, b_txl)	MAX(1, ((txls)*(b_txl)/8))
#define CALC_DXT(width, b_txl)	\
		(((1 << G_TX_DXT_FRAC) + TXL2WORDS(width, b_txl) - 1) / \
					TXL2WORDS(width, b_txl))

#define TXL2WORDS_4b(txls)	MAX(1, ((txls)/16))
#define CALC_DXT_4b(width)	\
		(((1 << G_TX_DXT_FRAC) + TXL2WORDS_4b(width) - 1) / \
					TXL2WORDS_4b(width))
*/

inline DWORD Txl2Words(DWORD width, DWORD size)
{
	if( size == G_IM_SIZ_4b )
		return max(1, width/16);
	else
		return max(1, width*sizeBytes[size]/8);
}

inline DWORD CalculateImgSize(DWORD width, DWORD height, DWORD size)
{
	//(((width)*(height) + siz##_INCR) >> siz##_SHIFT) -1
	return (((width)*(height) + sizeIncr[size]) >> sizeShift[size]) -1;
}


inline DWORD CalculateDXT(DWORD txl2words)
{
	//#define CALC_DXT(width, b_txl)	((2048 + TXL2WORDS(width, b_txl) - 1) / TXL2WORDS(width, b_txl))
	if( txl2words == 0 ) return 1;
	else return (2048+txl2words-1)/txl2words;
}

inline DWORD ReverseDXT(DWORD val, DWORD dwLRS, DWORD width, DWORD size)
{
	//#define TXL2WORDS(txls, b_txl)	MAX(1, ((txls)*(b_txl)/8))
	if( val == 0x800 ) return 1;
	
	int low = 2047/val;
	if( CalculateDXT(low) > val )	low++;
	int high = 2047/(val-1);

	if( low == high )	return low;

	for( int i=low; i<=high; i++ )
	{
		if( Txl2Words(width, size) == i )
			return i;
	}

	return	(low+high)/2;	//dwDXT = 2047 / (dwDXT-1);
}

void ComputeTileDimension(int mask, int clamp, int mirror, int width, DWORD &widthToCreate, DWORD &widthToLoad)
{
	int maskwidth = mask > 0 ? (1<<mask) : 0;
	widthToCreate = widthToLoad = width;

	if( mask > 0 )
	{
		if( width > maskwidth )
		{
			if( clamp == 0 )
			{
				// clamp is not used, so just use the dwTileMaskWidth as the real width
				widthToCreate = widthToLoad = maskwidth;
			}
			else
			{
				widthToLoad = maskwidth;
				//gti.WidthToCreate = dwTileWidth;
				// keep the current WidthToCreate, we will do mirror/wrap
				// during texture loading, not during rendering
			}
		}
		else if( width < maskwidth )
		{
			// dwTileWidth < dwTileMaskWidth

			if( clamp == 0 )
			{
				if( maskwidth%width == 0 )
				{
					if( (maskwidth/width)%2 == 0 || mirror == 0 )
					{
						// Do nothing
						// gti.WidthToLoad = gti.WidthToCreate = gRDP.tiles[dwTile].dwWidth = dwTileWidth
					}
					else
					{
						widthToCreate = maskwidth;
					}
				}
				else
				{
					widthToCreate = maskwidth;
					//gti.WidthToLoad = dwTileWidth;
				}
			}
			else
			{
				widthToCreate = maskwidth;
		}
		}
		else // dwTileWidth == dwTileMaskWidth
		{
		}

		// Some hacks, to limit the image size
		if( mask >= 8 )
		{
			if( maskwidth / width >= 2 )
			{
				widthToCreate = width;
			}
		}
	}
}

void LoadTexture(DWORD dwTile)
{
	DWORD dwTLutFmt;
	DWORD dwPitch;
	DWORD dwPalOffset;
	TextureEntry *pEntry = NULL;
	TextureInfo gti;

	Tile &tile = gRDP.tiles[dwTile];

	// Retrieve the tile loading info
	DWORD infoTmemAddr = tile.dwTMem;
	TMEMLoadMapInfo *info = &g_tmemLoadAddrMap[infoTmemAddr];
	if( !IsTmemFlagValid(infoTmemAddr) )
	{
		infoTmemAddr =  GetValidTmemInfoIndex(infoTmemAddr);
		info = &g_tmemLoadAddrMap[infoTmemAddr];
	}

	if( info->dwFormat != tile.dwFormat )
	{
		// Check the tile format, hack for Zelda's road
		if( dwTile != gRSP.curTile && tile.dwTMem == gRDP.tiles[gRSP.curTile].dwTMem &&
			tile.dwFormat != gRDP.tiles[gRSP.curTile].dwFormat )
		{
			//TRACE1("Tile %d format is not matching the loaded texture format", dwTile);
			pEntry = gTextureCache.GetBlackTexture();
			CDaedalusRender::g_pRender->SetCurrentTexture( dwTile, pEntry->pTexture, 4, 4);
			return;
		}
	}

	dwTLutFmt = gRDP.otherMode.text_tlut <<G_MDSFT_TEXTLUT;

	// Now Initialize the texture dimension
	int dwTileWidth;
	int dwTileHeight;
	if( info->bSetBy == CMD_LOADTILE )
	{
		if( tile.sl >= tile.sh )
		{
			dwTileWidth = info->dwWidth;	// From SetTImage
			dwTileWidth = dwTileWidth << info->dwSize >> tile.dwSize;
		}
		else
		{
			dwTileWidth= tile.sh - tile.sl + 1;
		}

		if( tile.tl >= tile.th )
		{
			dwTileHeight= info->TH - info->TL + 1;
		}
		else
		{
			dwTileHeight= tile.th - tile.tl + 1;
		}
	}
	else
	{
		if( tile.dwMaskS == 0 || tile.bClampS )
		{
			dwTileWidth = tile.hilite_sh - tile.hilite_sl +1;
			if( dwTileWidth < tile.sh - tile.sl +1 )
				dwTileWidth = tile.sh - tile.sl +1;
			if( dwTileWidth <= 0 )
			{
				DebuggerAppendMsg("Error");
			}
		}
		else
		{
			if( tile.dwMaskS < 8 )
				dwTileWidth = (1 << tile.dwMaskS );
			else
			{
				if( tile.sl <= tile.sh )
				{
					dwTileWidth = tile.sh - tile.sl +1;
				}
				else if( gRDP.tiles[CMD_LOADTILE].sl <= gRDP.tiles[CMD_LOADTILE].sh )
				{
					dwTileWidth = gRDP.tiles[CMD_LOADTILE].sh - gRDP.tiles[CMD_LOADTILE].sl +1;
				}
				else
				{
					dwTileWidth = tile.sh - tile.sl +1;
				}
			}
		}

		if( tile.dwMaskT == 0 || tile.bClampT )
		{
			dwTileHeight= tile.hilite_th - tile.hilite_tl +1;
			if( dwTileHeight < tile.th - tile.tl +1 )
				dwTileHeight = tile.th - tile.tl +1;

			if( dwTileHeight <= 0 )
			{
				DebuggerAppendMsg("Error");
			}
		}
		else
		{
			if( tile.dwMaskT < 8 )
				dwTileHeight = (1 << tile.dwMaskT );
			else if( tile.tl <= tile.th )
			{
				dwTileHeight = tile.th - tile.tl +1;
			}
			else if( gRDP.tiles[CMD_LOADTILE].tl <= gRDP.tiles[CMD_LOADTILE].th )
			{
				dwTileHeight = gRDP.tiles[CMD_LOADTILE].th - gRDP.tiles[CMD_LOADTILE].tl +1;
			}
			else
			{
				dwTileHeight = tile.th - tile.tl +1;
			}
		}
	}

	int dwTileMaskWidth = tile.dwMaskS > 0 ? (1 << tile.dwMaskS ) : 0;
	int dwTileMaskHeight = tile.dwMaskT > 0 ? (1 << tile.dwMaskT ) : 0;

	if( dwTileWidth < 0 || dwTileHeight < 0)
	{
		if( dwTileMaskWidth > 0 )
			dwTileWidth = dwTileMaskWidth;
		else if( dwTileWidth < 0 )
			dwTileWidth = -dwTileWidth;

		if( dwTileMaskHeight > 0 )
			dwTileHeight = dwTileMaskHeight;
		else if( dwTileHeight < 0 )
			dwTileHeight = -dwTileHeight;
	}



	if( dwTileWidth-dwTileMaskWidth == 1 && dwTileHeight-dwTileMaskHeight == 1 )
	{
		// Hack for Mario Kart
		dwTileWidth--;
		dwTileHeight--;
	}

	ComputeTileDimension(tile.dwMaskS, tile.bClampS,
		tile.bMirrorS, dwTileWidth, gti.WidthToCreate, gti.WidthToLoad);
	tile.dwWidth = gti.WidthToCreate;

	ComputeTileDimension(tile.dwMaskT, tile.bClampT,
		tile.bMirrorT, dwTileHeight, gti.HeightToCreate, gti.HeightToLoad);
	tile.dwHeight = gti.HeightToCreate;

#ifdef _DEBUG
	if( gti.WidthToCreate < gti.WidthToLoad )
		DebuggerAppendMsg("Check me, width to create = %d, width to load = %d", gti.WidthToCreate, gti.WidthToLoad);
	if( gti.HeightToCreate < gti.HeightToLoad )
		DebuggerAppendMsg("Check me, height to create = %d, height to load = %d", gti.HeightToCreate, gti.HeightToLoad);
#endif

	// Only needs doing for CI, but never mind
	dwPalOffset = 0;
	switch (tile.dwSize)
	{
	case G_IM_SIZ_4b: 
		dwPalOffset = 16  * 2 * tile.dwPalette; 
		break;
	case G_IM_SIZ_8b: 
		dwPalOffset = 0; 
		break;
	}	
	
	if( info->bSetBy == CMD_LOADTILE )
	{
		// It was a tile - the pitch is set by LoadTile
		dwPitch = info->dwWidth<<(info->dwSize-1);

		if( dwPitch == 0 )
		{
			dwPitch = 1024;		// Hack for Bust-A-Move
		}
	}
	else	//Set by LoadBlock
	{
		// It was a block load - the pitch is determined by the tile size
		if (info->dwDXT == 0)
			dwPitch = tile.dwLine << 3;
		else
		{
			DWORD DXT = info->dwDXT;
			if( info->dwDXT > 1 )
			{
				DXT = ReverseDXT(info->dwDXT, info->SH, dwTileWidth, tile.dwSize);
			}
			dwPitch = DXT << 3;
		}
		
		if (tile.dwSize == G_IM_SIZ_32b)
			dwPitch = tile.dwLine << 4;
	}

	tile.dwPitch = dwPitch;

	if( (gti.WidthToLoad < gti.WidthToCreate || tile.bSizeIsValid == false) && tile.dwMaskS > 0 && gti.WidthToLoad != dwTileMaskWidth &&
		info->bSetBy == CMD_LOADBLOCK )
	//if( (gti.WidthToLoad < gti.WidthToCreate ) && tile.dwMaskS > 0 && gti.WidthToLoad != dwTileMaskWidth &&
	//	info->bSetBy == CMD_LOADBLOCK )
	{
		// We have got the pitch now, recheck the width_to_load
		u32 pitchwidth = dwPitch<<1>>tile.dwSize;
		if( pitchwidth == dwTileMaskWidth )
		{
			gti.WidthToLoad = pitchwidth;
		}
	}
	if( (gti.HeightToLoad < gti.HeightToCreate  || tile.bSizeIsValid == false) && tile.dwMaskT > 0 && gti.HeightToLoad != dwTileMaskHeight &&
		info->bSetBy == CMD_LOADBLOCK )
	//if( (gti.HeightToLoad < gti.HeightToCreate  ) && tile.dwMaskT > 0 && gti.HeightToLoad != dwTileMaskHeight &&
	//	info->bSetBy == CMD_LOADBLOCK )
	{
		u32 pitchwidth = dwPitch<<1>>tile.dwSize;
		u32 pitchHeight = (info->dwTotalWords<<1)/dwPitch;
		if( pitchHeight == dwTileMaskHeight || gti.HeightToLoad == 1 )
		{
			gti.HeightToLoad = pitchHeight;
		}
	}
	if( gti.WidthToCreate < gti.WidthToLoad )	gti.WidthToCreate = gti.WidthToLoad;
	if( gti.HeightToCreate < gti.HeightToLoad )		gti.HeightToCreate = gti.HeightToLoad;


	u32 total64BitWordsToLoad = (gti.HeightToLoad*gti.WidthToLoad)>>(4-tile.dwSize);
	if( total64BitWordsToLoad + tile.dwTMem > 0x200 )
	{
		//TRACE0("Warning: texture loading tmem is over range");
		if( gti.WidthToLoad > gti.HeightToLoad )
		{
			if( info->bSetBy != CMD_LOADTILE )
				tile.dwWidth = gti.WidthToLoad = (dwPitch << 1 )>> tile.dwSize;
			tile.dwHeight = gti.HeightToCreate = gti.HeightToLoad = ((0x200 - tile.dwTMem) << (4-tile.dwSize)) / gti.WidthToLoad;
		}
		else
		{
			tile.dwHeight = gti.HeightToCreate = gti.HeightToLoad = info->dwTotalWords / ((gti.WidthToLoad << tile.dwSize) >> 1);
		}
	}


	// Check the info
	if( (info->dwTotalWords>>2) < total64BitWordsToLoad+tile.dwTMem-info->dwTmem && 
		(info->dwTotalWords>>2) < total64BitWordsToLoad+tile.dwTMem-info->dwTmem - 4 )
	{
		if( total64BitWordsToLoad+tile.dwTMem-info->dwTmem <= 0x200 )
		{
			DEBUGGER_IF_DUMP( logTmem, 
			{
				DebuggerAppendMsg("Fix me, info is not covering this Tmem address,Info start: 0x%x, total=0x%x, Tmem start: 0x%x, total=0x%x", 
					info->dwTmem,info->dwTotalWords>>2, tile.dwTMem, total64BitWordsToLoad);
			});
		}
		if( tile.dwTMem == 0x100 )
		{
			//gti.Address = g_tmemInfo1.dwLoadAddress+(0x100-g_tmemInfo1.dwTmem)*8;
		}
		else
		{
			gti.Address = info->dwLoadAddress+(tile.dwTMem-infoTmemAddr)*8;
			//gti.Address = g_tmemInfo0.dwLoadAddress+(tile.dwTMem-g_tmemInfo0.dwTmem)*8;
		}

		// Hack here
		if( dwTile != gRSP.curTile )
		{
			pEntry = gTextureCache.GetBlackTexture();
			CDaedalusRender::g_pRender->SetCurrentTexture( dwTile, pEntry->pTexture, 4, 4);
			return;
		}
		//gti.Address = g_tmemInfo0.dwLoadAddress+(tile.dwTMem-g_tmemInfo0.dwTmem)*8;
	}
	else
	{
		gti.Address = info->dwLoadAddress+(tile.dwTMem-infoTmemAddr)*8;
	}

	if( info->bSetBy == CMD_LOADTILE )
	{
		gti.LeftToLoad = (info->SL<<info->dwSize)>>tile.dwSize;
		gti.TopToLoad = info->TL;
	}
	else
	{
		gti.LeftToLoad = (info->SL<<info->dwSize)>>tile.dwSize;
		gti.TopToLoad = (info->TL<<info->dwSize)>>tile.dwSize;
	}

	gti = tile;	// Copy tile info to textureInfo entry

	gti.PalAddress = (uint32)(&g_wRDPPal[0]) + dwPalOffset;
	gti.TLutFmt = dwTLutFmt;
	gti.bSwapped = info->bSwapped;

	//Check the memory boundary
	gti.Address &= (g_dwRamSize-1);

	if( gti.Address + gti.HeightToLoad*gti.Pitch >= g_dwRamSize )
	{
		TRACE0("Warning: texture loading tmem is over range 2 ");
		gti.HeightToCreate = tile.dwHeight = dwTileHeight;
	}

	//Check again
	if( gti.Address + gti.HeightToLoad*gti.Pitch >= g_dwRamSize )
	{
		TRACE0("Warning: texture loading tmem is over range 3");
		gti.HeightToCreate = tile.dwHeight = (g_dwRamSize-gti.Address)/gti.Pitch;
	}


	//Check again the memory boundary
	if( gti.Address + gti.HeightToLoad*gti.Pitch >= g_dwRamSize )
	{
		TRACE0("Warning: texture loading tmem is over range 4");
		if( gti.Address >= g_dwRamSize )	
			gti.Address &= (g_dwRamSize-1);
		if( gti.Address + gti.HeightToLoad*gti.Pitch >= g_dwRamSize )
		{
			TRACE0("Check me, gti.Address is incorrect");
			gti.HeightToLoad = (g_dwRamSize-gti.Address)/gti.Pitch;	//Hack
		}
	}

	if (gti.Format == G_IM_FMT_CI && gti.TLutFmt == G_TT_NONE )
		gti.TLutFmt = G_TT_RGBA16;		// Force RGBA

	gti.pPhysicalAddress = ((BYTE*)g_pu32RamBase)+gti.Address;

#ifdef _DEBUG
	if( logTextureDetails || dlistDeassemble )
	{
		TRACE0("Loading texture:\n");
		DebuggerAppendMsg("Left: %d, Top: %d, Width: %d, Height: %d, Size to Load (%d, %d)", 
			gti.LeftToLoad, gti.TopToLoad, gti.WidthToCreate, gti.HeightToCreate, gti.WidthToLoad, gti.HeightToLoad);
		DebuggerAppendMsg("Pitch: %d, Addr: 0x%08x", dwPitch, gti.Address);
	}
#endif

	// Hack for large tile load for LoadTile and TexRect
	if( g_curRomInfo.bFastLoadTile && status.primitiveType == PRIM_TEXTRECT && 
		info->bSetBy == CMD_LOADTILE && ((gti.Pitch<<1)>>gti.Size) > 128 &&
		((gti.Pitch<<1)>>gti.Size) <= 0x400  )
	{
		u32 idx = dwTile-gRSP.curTile;
		status.LargerTileRealLeft[idx] = gti.LeftToLoad;
		gti.LeftToLoad=0;
		gti.WidthToLoad = gti.WidthToCreate = ((gti.Pitch<<1)>>gti.Size);
		status.UseLargerTile[idx]=true;
	}

	pEntry = gTextureCache.GetTexture(&gti, true, true);	// Load the texture by using texture cache

	if (pEntry != NULL && pEntry->pTexture != NULL)
	{
		if( pEntry->pMirroredTexture == NULL )
		{
			GetMirrorSurfaceHandler(dwTile, pEntry);;
		}

		if( pEntry->pMirroredTexture == NULL )
		{
			TextureEnhancement(pEntry);
		}

		CDaedalusRender::g_pRender->SetCurrentTexture( dwTile, 
			(pEntry->pMirroredTexture != NULL)?pEntry->pMirroredTexture:pEntry->pTexture,
			pEntry->ti.WidthToCreate,
			pEntry->ti.HeightToCreate);
	}
	else
	{
		CDaedalusRender::g_pRender->SetCurrentTexture( dwTile, NULL, 64, 64 );
	}
}

void SetupTextures()
{
	StartProfiler(PROFILE_TEXTURE);
	if( gRDP.textureIsChanged || !currentRomOptions.bFastTexCRC ||
		CDaedalusRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[0] ||
		CDaedalusRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[1] )
	{
		status.UseLargerTile[0]=false;
		status.UseLargerTile[1]=false;

		if( CDaedalusRender::g_pRender->IsTexel0Enable() || gRDP.otherMode.cycle_type  == CYCTYPE_COPY )
		{
			if( CDaedalusRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[0] )
			{
				TextureEntry *pEntry = gTextureCache.GetConstantColorTexture(CDaedalusRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[0]);
				CDaedalusRender::g_pRender->SetCurrentTexture( gRSP.curTile, pEntry->pTexture, 4, 4);
			}
			else
			{
				LoadTexture(gRSP.curTile);
			}
		}

		if( gRSP.curTile<7 && CDaedalusRender::g_pRender->IsTexel1Enable() )
		{
			if( CDaedalusRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[1] )
			{
				TextureEntry *pEntry = gTextureCache.GetConstantColorTexture(CDaedalusRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[1]);
				CDaedalusRender::g_pRender->SetCurrentTexture( gRSP.curTile+1, pEntry->pTexture, 4, 4);
			}
			else
			{
				LoadTexture(gRSP.curTile+1);
			}
		}
		gRDP.textureIsChanged = false;
	}
	StopProfiler(PROFILE_TEXTURE);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
extern u32 g_TxtLoadBy;;

void DLParser_LoadTLut(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;
	status.gTexUcodeCount++;

	DWORD dwTile  = ((dwCmd1 >> 24) & 0x07);
	DWORD dwTLutFmt = (gRDP.otherModeH >> G_MDSFT_TEXTLUT)&0x3;

	DWORD dwCount;

	DWORD dwTMEMOffset = gRDP.tiles[dwTile].dwTMem - 256;				// starting location in the palettes
	dwCount = ((WORD)(dwCmd1 >> 14) & 0x03FF) + 1;		// number to copy
	DWORD dwRDRAMOffset = 0;

	DWORD dwULS   = ((dwCmd0 >> 12) & 0xfff)/4;
	DWORD dwULT   = ((dwCmd0      ) & 0xfff)/4;
	DWORD dwLRS   = ((dwCmd1 >> 12) & 0xfff)/4;
	DWORD dwLRT   = ((dwCmd1      ) & 0xfff)/4;

	gRDP.tiles[dwTile].hilite_sl = gRDP.tiles[dwTile].sl = dwULS;
	gRDP.tiles[dwTile].hilite_tl = gRDP.tiles[dwTile].tl = dwULT;
	gRDP.tiles[dwTile].sh = dwLRS;
	gRDP.tiles[dwTile].th = dwLRT;
	gRDP.tiles[dwTile].bSizeIsValid = true;

	gRDP.tiles[dwTile].lastTileCmd = CMD_LOADTLUT;

#ifdef _DEBUG
	if( ((dwCmd0>>12)&0x3) != 0 || ((dwCmd0)&0x3) != 0 || ((dwCmd1>>12)&0x3) != 0 || ((dwCmd1)&0x3) != 0 )
	{
		//TRACE0("Load tlut, sl,tl,sh,th are not integers");
	}
#endif

	dwCount = (dwLRS - dwULS)+1;
	dwRDRAMOffset = (dwULS + dwULT)*2;
	DWORD dwPalAddress = g_TI.dwAddr + dwRDRAMOffset;

	//Copy PAL to the PAL memory
	WORD *srcPal = (WORD*)(g_pu8RamBase + (dwPalAddress& (g_dwRamSize-1)) );
	for (uint32 i=dwTMEMOffset; i<dwTMEMOffset+dwCount && i<0x100; i++)
	{
		g_wRDPPal[i] = *(srcPal++);
	}

#ifdef _DEBUG
	if( (pauseAtNext && eventToPause == NEXT_LOADTLUT)  || logTextureDetails )
	{	
		DebuggerAppendMsg("LoadTLut Tile: %d Start: 0x%X+0x%X, Count: 0x%X\nFmt is %s, TMEM=0x%X\n", 
		dwTile, g_TI.dwAddr, dwRDRAMOffset, dwCount,textluttype[dwTLutFmt], dwTMEMOffset);
		DebuggerAppendMsg("    :ULS: 0x%X, ULT:0x%X, LRS: 0x%X, LRT:0x%X\n", 
		dwULS, dwULT, dwLRS,dwLRT);

		if( pauseAtNext && eventToPause == NEXT_LOADTLUT && dwCount == 16 ) 
		{
			char buf[2000];
			strcpy(buf, "Data:\n");
			for( i=0; i<16; i++ )
			{
				sprintf(buf+strlen(buf), "%04X ", g_wRDPPal[dwTMEMOffset+i]);
				if( i%4 == 3 )
				{
					sprintf(buf+strlen(buf), "\n");
				}
			}
			sprintf(buf+strlen(buf), "\n");
			TRACE0(buf);
		}
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_LOADTLUT);
#endif

	RevTlutTableNeedUpdate = true;
	g_TxtLoadBy = CMD_LOADTLUT;
}


void DLParser_LoadBlock(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;
	status.gTexUcodeCount++;

	DWORD dwULS		= ((dwCmd0>>12)&0x0FFF);
	DWORD dwULT		= ((dwCmd0    )&0x0FFF);
	DWORD dwLRS		= ((dwCmd1>>12)&0x0FFF);
	DWORD dwTile	=  (dwCmd1>>24)&0x07;
	DWORD dwDXT		=  (dwCmd1    )&0x0FFF;		// 1.11 fixed point

	uint32 size = dwLRS+1;
	if( gRDP.tiles[dwTile].dwSize == G_IM_SIZ_32b )	size<<=1;

	SetTmemFlag(gRDP.tiles[dwTile].dwTMem, size>>2);

	TMEMLoadMapInfo &info = g_tmemLoadAddrMap[gRDP.tiles[dwTile].dwTMem];

	if( dwDXT == 0 )
	{
		info.bSwapped = TRUE;
	}
	else
	{
		info.bSwapped = FALSE;
	}

	info.SL = gRDP.tiles[dwTile].hilite_sl = gRDP.tiles[dwTile].sl = dwULS;
	info.SH = gRDP.tiles[dwTile].hilite_tl = gRDP.tiles[dwTile].tl = dwLRS;
	info.TL = gRDP.tiles[dwTile].sh = dwULT;
	info.TH = gRDP.tiles[dwTile].th = dwDXT;
	gRDP.tiles[dwTile].bSizeIsValid = true;

	for( int i=0; i<8; i++ )
	{
		if( gRDP.tiles[i].dwTMem == gRDP.tiles[dwTile].dwTMem )
			gRDP.tiles[i].lastTileCmd = CMD_LOADBLOCK;
	}

	info.dwLoadAddress = g_TI.dwAddr;
	info.bSetBy = CMD_LOADBLOCK;
	info.dwDXT = dwDXT;
	info.dwLine = gRDP.tiles[dwTile].dwLine;

	info.dwFormat = g_TI.dwFormat;
	info.dwSize = g_TI.dwSize;
	info.dwWidth = g_TI.dwWidth;
	info.dwTotalWords = size;
	info.dwTmem = gRDP.tiles[dwTile].dwTMem;
	info.ucodeCount = status.gTexUcodeCount;

	if( gRDP.tiles[dwTile].dwTMem == 0 )
	{
		if( size >= 1024 )
		{
			memcpy(&g_tmemInfo0, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo0.dwTotalWords = size>>2;
		}
		
		if( size == 2048 )
		{
			memcpy(&g_tmemInfo1, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo1.dwTotalWords = size>>2;
		}
	}
	else if( gRDP.tiles[dwTile].dwTMem == 0x100 )
	{
		if( size == 1024 )
		{
			memcpy(&g_tmemInfo1, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo1.dwTotalWords = size>>2;
		}
	}

	g_TxtLoadBy = CMD_LOADBLOCK;

#ifdef _DEBUG
	DL_PF("    Tile:%d (%d,%d) DXT:0x%04x\n",
		dwTile, dwULS, dwULT, dwDXT);

	if( logTextureDetails   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
		DebuggerAppendMsg("LoadBlock:%d (%d,%d,%d) DXT:0x%04x(%X)\n",
			dwTile, dwULS, dwULT, ((dwCmd1>>12)&0x0FFF), dwDXT, (dwCmd1&0x0FFF));
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);
#endif
}

void DLParser_LoadTile(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;
	status.gTexUcodeCount++;

#ifdef _DEBUG
	if( ((dwCmd0>>12)&0x3) != 0 || ((dwCmd0)&0x3) != 0 || ((dwCmd1>>12)&0x3) != 0 || ((dwCmd1)&0x3) != 0 )
	{
		//TRACE0("Load tile, sl,tl,sh,th are not integers");
	}
#endif

	DWORD dwTile	=  (dwCmd1>>24)&0x07;
	DWORD dwULS		= ((dwCmd0>>12)&0x0FFF)/4;
	DWORD dwULT		= ((dwCmd0    )&0x0FFF)/4;
	DWORD dwLRS		= ((dwCmd1>>12)&0x0FFF)/4;
	DWORD dwLRT		= ((dwCmd1    )&0x0FFF)/4;

	gRDP.tiles[dwTile].hilite_sl = gRDP.tiles[dwTile].sl = dwULS;
	gRDP.tiles[dwTile].hilite_tl = gRDP.tiles[dwTile].tl = dwULT;
	gRDP.tiles[dwTile].hilite_sh = gRDP.tiles[dwTile].sh = dwLRS;
	gRDP.tiles[dwTile].hilite_th = gRDP.tiles[dwTile].th = dwLRT;
	gRDP.tiles[dwTile].bSizeIsValid = true;

	for( int i=0; i<8; i++ )
	{
		if( gRDP.tiles[i].dwTMem == gRDP.tiles[dwTile].dwTMem )
			gRDP.tiles[i].lastTileCmd = CMD_LOADTILE;
	}

	DWORD size = gRDP.tiles[dwTile].dwLine*(dwLRT-dwULT+1);
	if( gRDP.tiles[dwTile].dwSize == G_IM_SIZ_32b )	size<<=1;
	SetTmemFlag(gRDP.tiles[dwTile].dwTMem,size );

	DWORD dwPitch = g_TI.dwWidth;
	if( gRDP.tiles[dwTile].dwSize != G_IM_SIZ_4b )
	{
		dwPitch = g_TI.dwWidth<<(gRDP.tiles[dwTile].dwSize-1);
	}

#ifdef _DEBUG
	if( logTextureDetails   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
		DebuggerAppendMsg("LoadTile:%d (%d,%d) -> (%d,%d) [%d x %d]\n",
			dwTile, dwULS, dwULT, dwLRS, dwLRT,
			(dwLRS - dwULS)+1, (dwLRT - dwULT)+1);
	}

	
	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);
#endif

	DL_PF("    Tile:%d (%d,%d) -> (%d,%d) [%d x %d]",
		dwTile, dwULS, dwULT, dwLRS, dwLRT,
		(dwLRS - dwULS)+1, (dwLRT - dwULT)+1);

	TMEMLoadMapInfo &info = g_tmemLoadAddrMap[gRDP.tiles[dwTile].dwTMem];

	info.dwLoadAddress = g_TI.dwAddr;
	info.dwFormat = g_TI.dwFormat;
	info.dwSize = g_TI.dwSize;
	info.dwWidth = g_TI.dwWidth;

	info.SL = dwULS;
	info.SH = dwLRS;
	info.TL = dwULT;
	info.TH = dwLRT;
	
	info.dwDXT = 0;
	info.dwLine = gRDP.tiles[dwTile].dwLine;
	info.ucodeCount = status.gTexUcodeCount;
	info.dwTmem = gRDP.tiles[dwTile].dwTMem;
	info.dwTotalWords = size<<2;

	info.bSetBy = CMD_LOADTILE;
	info.bSwapped =FALSE;

	g_TxtLoadBy = CMD_LOADTILE;

	if( gRDP.tiles[dwTile].dwTMem == 0 )
	{
		if( size >= 256 )
		{
			memcpy(&g_tmemInfo0, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo0.dwTotalWords = size;
		}

		if( size == 512 )
		{
			memcpy(&g_tmemInfo1, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo1.dwTotalWords = size;
		}
	}
	else if( gRDP.tiles[dwTile].dwTMem == 0x100 )
	{
		if( size == 256 )
		{
			memcpy(&g_tmemInfo1, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo1.dwTotalWords = size;
		}
	}
}


static char *pszOnOff[2]     = {"Off", "On"};
void DLParser_SetTile(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;
	status.gTexUcodeCount++;

	DWORD dwTile		= (dwCmd1>>24)&0x7;
	Tile &tile = gRDP.tiles[dwTile];

	tile.dwFormat	= (dwCmd0>>21)&0x7;
	tile.dwSize		= (dwCmd0>>19)&0x3;
	tile.dwLine		= (dwCmd0>>9 )&0x1FF;
	tile.dwTMem		= (dwCmd0    )&0x1FF;

	tile.dwPalette	= (dwCmd1>>20)&0x0F;
	tile.bClampT		= (dwCmd1>>19)&0x01;
	tile.bMirrorT		= (dwCmd1>>18)&0x01;
	tile.dwMaskT		= (dwCmd1>>14)&0x0F;
	tile.dwShiftT		= (dwCmd1>>10)&0x0F;		// LOD stuff
	tile.bClampS		= (dwCmd1>>9 )&0x01;
	tile.bMirrorS		= (dwCmd1>>8 )&0x01;
	tile.dwMaskS		= (dwCmd1>>4 )&0x0F;
	tile.dwShiftS		= (dwCmd1    )&0x0F;		// LOD stuff

	tile.fShiftScaleS = 1.0f;
	if( tile.dwShiftS )
	{
		if( tile.dwShiftS > 10 )
		{
			tile.fShiftScaleS = (float)(1 << (16 - tile.dwShiftS));
		}
		else
		{
			tile.fShiftScaleS = (float)1.0f/(1 << tile.dwShiftS);
		}
	}

	tile.fShiftScaleT = 1.0f;
	if( tile.dwShiftT )
	{
		if( tile.dwShiftT > 10 )
		{
			tile.fShiftScaleT = (float)(1 << (16 - tile.dwShiftT));
		}
		else
		{
			tile.fShiftScaleT = (float)1.0f/(1 << tile.dwShiftT);
		}
	}

	// Hack for DK64
	/*
	if( tile.dwMaskS > 0 && tile.dwMaskT > 0 && tile.dwMaskS < 8 && tile.dwMaskT < 8 )
	{
		tile.sh = tile.sl + (1<<tile.dwMaskS);
		tile.th = tile.tl + (1<<tile.dwMaskT);
		tile.hilite_sl = tile.sl;
		tile.hilite_tl = tile.tl;
	}
	*/

	tile.lastTileCmd = CMD_SETTILE;

#ifdef _DEBUG
	if( logTextureDetails   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
	DebuggerAppendMsg("SetTile:%d  Fmt: %s/%s Line:%d TMem:0x%04x Palette:%d\n",
		dwTile, pszImgFormat[tile.dwFormat], pszImgSize[tile.dwSize],
		tile.dwLine,  tile.dwTMem, tile.dwPalette);
	DebuggerAppendMsg("         S: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x\n",
		pszOnOff[tile.bClampS],pszOnOff[tile.bMirrorS],
		tile.dwMaskS, tile.dwShiftS);
	DebuggerAppendMsg("         T: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x\n",
		pszOnOff[tile.bClampT],pszOnOff[tile.bMirrorT],
		tile.dwMaskT, tile.dwShiftT);
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);

	DL_PF("    Tile:%d  Fmt: %s/%s Line:%d TMem:0x%04x Palette:%d",
		dwTile, pszImgFormat[tile.dwFormat], pszImgSize[tile.dwSize],
		tile.dwLine, tile.dwTMem, tile.dwPalette);
	DL_PF("         S: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x",
		pszOnOff[tile.bClampS],pszOnOff[tile.bMirrorS],
		tile.dwMaskS, tile.dwShiftS);
	DL_PF("         T: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x",
		pszOnOff[tile.bClampT],pszOnOff[tile.bMirrorT],
		tile.dwMaskT, tile.dwShiftT);
#endif
}

void DLParser_SetTileSize(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;
	status.gTexUcodeCount++;

	DWORD dwTile	= (dwCmd1>>24)&0x07;
	int sl		= (DWORD)((dwCmd0>>12)&0x0FFF);
	int tl		= (DWORD)((dwCmd0    )&0x0FFF);
	int sh		= (DWORD)((dwCmd1>>12)&0x0FFF);
	int th		= (DWORD)((dwCmd1    )&0x0FFF);

#ifdef _DEBUG
	if( ((dwCmd0>>12)&0x3) != 0 || ((dwCmd0)&0x3) != 0 || ((dwCmd1>>12)&0x3) != 0 || ((dwCmd1)&0x3) != 0 )
	{
		//DebuggerAppendMsg("Set tile size, sl,tl,sh,th are not integers");
	}
#endif

	Tile &tile = gRDP.tiles[dwTile];

	if( tile.lastTileCmd != CMD_SETTILE_SIZE )
	{
		tile.bSizeIsValid = true;
		if( sl/4 > sh/4 || tl/4 > th/4 || (sh == 0 && tile.dwShiftS==0 && th == 0 && tile.dwShiftT ==0 ) )
		{
#ifdef _DEBUG
			if( sl != 0 || tl != 0 || sh != 0 || th != 0 )
			{
				if( tile.dwMaskS==0 || tile.dwMaskT==0 )
					TRACE0("Check me, setTileSize is not correct");
			}
#endif
			//if( sl>sh ) {DWORD temp=sl;sl=sh;sh=temp;}
			//if( tl>th ) {DWORD temp=tl;tl=th;th=temp;}
			tile.bSizeIsValid = false;
		}
		tile.hilite_sl = tile.sl = sl / 4;
		tile.hilite_tl = tile.tl = tl / 4;
		tile.hilite_sh = tile.sh = sh / 4;
		tile.hilite_th = tile.th = th / 4;

		tile.fhilite_sl = tile.fsl = sl / 4.0f;
		tile.fhilite_tl = tile.ftl = tl / 4.0f;
		tile.fhilite_sh = tile.fsh = sh / 4.0f;
		tile.fhilite_th = tile.fth = th / 4.0f;

		tile.lastTileCmd = CMD_SETTILE_SIZE;
	}
	else
	{
		tile.fhilite_sh = tile.fsh;
		tile.fhilite_th = tile.fth;
		tile.fhilite_sl = tile.fsl = (sl>0x7ff ? (sl-0xfff) : sl)/4.0f;
		tile.fhilite_tl = tile.ftl = (tl>0x7ff ? (tl-0xfff) : tl)/4.0f;

		tile.hilite_sl = sl>0x7ff ? (sl-0xfff) : sl;
		tile.hilite_tl = tl>0x7ff ? (tl-0xfff) : tl;
		tile.hilite_sl /= 4;
		tile.hilite_tl /= 4;
		tile.hilite_sh = sh/4;
		tile.hilite_th = th/4;
		tile.lastTileCmd = CMD_SETTILE_SIZE;
	}

#ifdef _DEBUG
	if( logTextureDetails   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
	DebuggerAppendMsg("SetTileSize:%d (%d/4,%d/4) -> (%d/4,%d/4) [%d x %d]\n",
		dwTile, sl, tl, sh, th, 
		((sh/4) - (sl/4)) + 1, ((th/4) - (tl/4)) + 1);
	}
	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);

	DL_PF("    Tile:%d (%d,%d) -> (%d,%d) [%d x %d]",
		dwTile, sl/4, tl/4, sh/4, th/4,
		((sh/4) - (sl/4)) + 1, ((th/4) - (tl/4)) + 1);
#endif
}

extern char *pszImgFormat[8];// = {"RGBA", "YUV", "CI", "IA", "I", "?1", "?2", "?3"};
extern char *pszImgSize[4];// = {"4", "8", "16", "32"};
void DLParser_SetTImg(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;
	status.gTexUcodeCount++;

	g_TI.dwFormat = (dwCmd0>>21)&0x7;
	g_TI.dwSize   = (dwCmd0>>19)&0x3;
	g_TI.dwWidth  = (dwCmd0&0x0FFF) + 1;
	g_TI.dwAddr   = RDPSegAddr(dwCmd1);

#ifdef _DEBUG
	if( g_TI.dwAddr == 0x00ffffff)
	{
		TRACE0("Check me here in setTimg");
	}

	if( logTextureDetails  || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
		DebuggerAppendMsg("SetTImage: 0x%08x Fmt: %s/%s Width in Pixel: %d\n", g_TI.dwAddr,
			pszImgFormat[g_TI.dwFormat], pszImgSize[g_TI.dwSize], g_TI.dwWidth);
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);

	DL_PF("Image: 0x%08x Fmt: %s/%s Width in Pixel: %d", g_TI.dwAddr,
		pszImgFormat[g_TI.dwFormat], pszImgSize[g_TI.dwSize], g_TI.dwWidth);
#endif
}

void DLParser_TexRect(DWORD dwCmd0, DWORD dwCmd1)
{
	status.bCIBufferIsRendered = true;
	status.primitiveType = PRIM_TEXTRECT;

	// This command used 128bits, and not 64 bits. This means that we have to look one 
	// Command ahead in the buffer, and update the PC.
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+4+8);
	DWORD dwHalf1 = *(DWORD *)(g_pu8RamBase + dwPC);
	DWORD dwHalf2 = *(DWORD *)(g_pu8RamBase + dwPC+8);

	if( options.enableHackForGames == HACK_FOR_ALL_STAR_BASEBALL )
	{
		if( ((dwHalf1>>24) == 0xb4 || (dwHalf1>>24) == 0xb3 || (dwHalf1>>24) == 0xb2 || (dwHalf1>>24) == 0xe1) && 
			((dwHalf2>>24) == 0xb4 || (dwHalf2>>24) == 0xb3 || (dwHalf2>>24) == 0xb2 || (dwHalf2>>24) == 0xf1) )
		{
			// Increment PC so that it points to the right place
			g_dwPCStack[g_dwPCindex].addr += 16;
		}
		else
		{
			// Hack for some games, All_Star_Baseball_2000
			g_dwPCStack[g_dwPCindex].addr += 8;
			dwCmd3 = dwCmd2;
			//dwCmd2 = dwHalf1;
			dwCmd2 = 0;
		}	
	}
	else
	{
		g_dwPCStack[g_dwPCindex].addr += 16;
	}


	// Hack for Mario Tennis
	if( !status.bHandleN64TextureBuffer && g_CI.dwAddr == g_DI.dwAddr )
	{
		return;
	}


	DL_PF("0x%08x: %08x %08x", dwPC, *(DWORD *)(g_pu8RamBase + dwPC+0), *(DWORD *)(g_pu8RamBase + dwPC+4));
	DL_PF("0x%08x: %08x %08x", dwPC+8, *(DWORD *)(g_pu8RamBase + dwPC+8), *(DWORD *)(g_pu8RamBase + dwPC+8+4));

	DWORD dwXH		= ((dwCmd0>>12)&0x0FFF)/4;
	DWORD dwYH		= ((dwCmd0    )&0x0FFF)/4;
	DWORD dwTile	= (dwCmd1>>24)&0x07;
	DWORD dwXL		= ((dwCmd1>>12)&0x0FFF)/4;
	DWORD dwYL		= ((dwCmd1    )&0x0FFF)/4;
	

	if( (int)dwXL >= gRDP.scissor.right || (int)dwYL >= gRDP.scissor.bottom || (int)dwXH < gRDP.scissor.left || (int)dwYH < gRDP.scissor.top )
	{
		// Clipping
		return;
	}

	u16 uS		= (u16)(  dwCmd2>>16)&0xFFFF;
	u16 uT		= (u16)(  dwCmd2    )&0xFFFF;
	s16 s16S = *(s16*)(&uS);
	s16 s16T = *(s16*)(&uT);

	u16  uDSDX 	= (u16)((  dwCmd3>>16)&0xFFFF);
	u16  uDTDY	    = (u16)((  dwCmd3    )&0xFFFF);
	s16	 s16DSDX  = *(s16*)(&uDSDX);
	s16  s16DTDY	= *(s16*)(&uDTDY);

	DWORD curTile = gRSP.curTile;
	ForceMainTextureIndex(dwTile);

	float fS0 = s16S / 32.0f;
	float fT0 = s16T / 32.0f;

	float fDSDX = s16DSDX / 1024.0f;
	float fDTDY = s16DTDY / 1024.0f;

	DWORD cycletype = gRDP.otherMode.cycle_type;

	if (cycletype == CYCTYPE_COPY)
	{
		fDSDX /= 4.0f;	// In copy mode 4 pixels are copied at once.
		dwXH++;
		dwYH++;
	}
	else if (cycletype == CYCTYPE_FILL)
	{
		dwXH++;
		dwYH++;
	}

	if( fDSDX == 0 )	fDSDX = 1;
	if( fDTDY == 0 )	fDTDY = 1;

	float fS1 = fS0 + (fDSDX * (dwXH - dwXL));
	float fT1 = fT0 + (fDTDY * (dwYH - dwYL));

	DL_PF("    Tile:%d Screen(%d,%d) -> (%d,%d)", dwTile, dwXL, dwYL, dwXH, dwYH);
	DL_PF("           Tex:(%#5f,%#5f) -> (%#5f,%#5f) (DSDX:%#5f DTDY:%#5f)",
                                            fS0, fT0, fS1, fT1, fDSDX, fDTDY);
	DL_PF("");

	float t0u0 = (fS0-gRDP.tiles[dwTile].hilite_sl) * gRDP.tiles[dwTile].fShiftScaleS;
	float t0v0 = (fT0-gRDP.tiles[dwTile].hilite_tl) * gRDP.tiles[dwTile].fShiftScaleT;
	float t0u1 = t0u0 + (fDSDX * (dwXH - dwXL))*gRDP.tiles[dwTile].fShiftScaleS;
	float t0v1 = t0v0 + (fDTDY * (dwYH - dwYL))*gRDP.tiles[dwTile].fShiftScaleT;

	if( dwXL==0 && dwYL==0 && dwXH==windowSetting.fViWidth-1 && dwYH==windowSetting.fViHeight-1 &&
		t0u0 == 0 && t0v0==0 && t0u1==0 && t0v1==0 )
	{
		//Using TextRect to clear the screen
	}
	else
	{
		if( status.bHandleN64TextureBuffer && //status.bDirectWriteIntoRDRAM && 
			g_FakeFrameBufferInfo.CI_Info.dwFormat == gRDP.tiles[dwTile].dwFormat && 
			g_FakeFrameBufferInfo.CI_Info.dwSize == gRDP.tiles[dwTile].dwSize && 
			gRDP.tiles[dwTile].dwFormat == G_IM_FMT_CI && gRDP.tiles[dwTile].dwSize == G_IM_SIZ_8b )
		{
			if( options.enableHackForGames == HACK_FOR_YOSHI )
			{
				// Hack for Yoshi background image
				SetupTextures();
				TexRectToFrameBuffer_8b(dwXL, dwYL, dwXH, dwYH, t0u0, t0v0, t0u1, t0v1, dwTile);
				DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_FLUSH_TRI || eventToPause == NEXT_TEXTRECT), {
					DebuggerAppendMsg("TexRect: tile=%d, X0=%d, Y0=%d, X1=%d, Y1=%d,\nfS0=%f, fT0=%f, ScaleS=%f, ScaleT=%f\n",
						gRSP.curTile, dwXL, dwYL, dwXH, dwYH, fS0, fT0, fDSDX, fDTDY);
					DebuggerAppendMsg("Pause after TexRect for Yoshi\n");
				});

			}
			else
			{
				if( frameBufferOptions.bUpdateCIInfo )
				{
					SetupTextures();
					TexRectToFrameBuffer_8b(dwXL, dwYL, dwXH, dwYH, t0u0, t0v0, t0u1, t0v1, dwTile);
				}

				if( !status.bDirectWriteIntoRDRAM )
				{
					CDaedalusRender::g_pRender->TexRect(dwXL, dwYL, dwXH, dwYH, fS0, fT0, fDSDX, fDTDY);

					status.dwNumTrisRendered += 2;
				}
			}
		}
		else
		{
			CDaedalusRender::g_pRender->TexRect(dwXL, dwYL, dwXH, dwYH, fS0, fT0, fDSDX, fDTDY);
			status.bTextureBufferDrawnByTriangles = true;

			status.dwNumTrisRendered += 2;
		}
	}

	if( status.bHandleN64TextureBuffer )	g_FakeFrameBufferInfo.maxUsedHeight = max(g_FakeFrameBufferInfo.maxUsedHeight,dwYH);

	ForceMainTextureIndex(curTile);
}


void DLParser_TexRectFlip(DWORD dwCmd0, DWORD dwCmd1)
{ 
	status.bCIBufferIsRendered = true;
	status.primitiveType = PRIM_TEXTRECTFLIP;

	// This command used 128bits, and not 64 bits. This means that we have to look one 
	// Command ahead in the buffer, and update the PC.
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+4+8);

	// Increment PC so that it points to the right place
	g_dwPCStack[g_dwPCindex].addr += 16;

	DWORD dwXH		= ((dwCmd0>>12)&0x0FFF)/4;
	DWORD dwYH		= ((dwCmd0    )&0x0FFF)/4;
	DWORD dwTile	= (dwCmd1>>24)&0x07;
	DWORD dwXL		= ((dwCmd1>>12)&0x0FFF)/4;
	DWORD dwYL		= ((dwCmd1    )&0x0FFF)/4;
	DWORD dwS		= (  dwCmd2>>16)&0xFFFF;
	DWORD dwT		= (  dwCmd2    )&0xFFFF;
	LONG  nDSDX 	= (LONG)(SHORT)((  dwCmd3>>16)&0xFFFF);
	LONG  nDTDY	    = (LONG)(SHORT)((  dwCmd3    )&0xFFFF);


	DWORD curTile = gRSP.curTile;
	ForceMainTextureIndex(dwTile);
	
	float fS0 = (float)dwS / 32.0f;
	float fT0 = (float)dwT / 32.0f;

	float fDSDX = (float)nDSDX / 1024.0f;
	float fDTDY = (float)nDTDY / 1024.0f;

	DWORD cycletype = gRDP.otherMode.cycle_type;

	if (cycletype == CYCTYPE_COPY)
	{
		fDSDX /= 4.0f;	// In copy mode 4 pixels are copied at once.
		dwXH++;
		dwYH++;
	}
	else if (cycletype == CYCTYPE_FILL)
	{
		dwXH++;
		dwYH++;
	}

	float fS1 = fS0 + (fDSDX * (dwYH - dwYL));
	float fT1 = fT0 + (fDTDY * (dwXH - dwXL));
	
	DL_PF("    Tile:%d (%d,%d) -> (%d,%d)",
		dwTile, dwXL, dwYL, dwXH, dwYH);
	DL_PF("    Tex:(%#5f,%#5f) -> (%#5f,%#5f) (DSDX:%#5f DTDY:%#5f)",
		fS0, fT0, fS1, fT1, fDSDX, fDTDY);
	DL_PF("");

	float t0u0 = (fS0) * gRDP.tiles[dwTile].fShiftScaleS-gRDP.tiles[dwTile].sl;
	float t0v0 = (fT0) * gRDP.tiles[dwTile].fShiftScaleT-gRDP.tiles[dwTile].tl;
	float t0u1 = t0u0 + (fDSDX * (dwYH - dwYL))*gRDP.tiles[dwTile].fShiftScaleS;
	float t0v1 = t0v0 + (fDTDY * (dwXH - dwXL))*gRDP.tiles[dwTile].fShiftScaleT;

	CDaedalusRender::g_pRender->TexRectFlip(dwXL, dwYL, dwXH, dwYH, t0u0, t0v0, t0u1, t0v1);
	status.dwNumTrisRendered += 2;

	if( status.bHandleN64TextureBuffer )	g_FakeFrameBufferInfo.maxUsedHeight = max(g_FakeFrameBufferInfo.maxUsedHeight,dwYL+(dwXH-dwXL));

	ForceMainTextureIndex(curTile);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
/*
 *	TMEM emulation
 *  There are 0x200's 64bits entry in TMEM
 *  Usually, textures are loaded into TMEM at 0x0, and TLUT is loaded at 0x100
 *  of course, the whole TMEM can be used by textures if TLUT is not used, and TLUT
 *  can be at other address of TMEM.
 *
 *  We don't want to emulate TMEM by creating a block of memory for TMEM and load
 *  everything into the block of memory, this will be slow.
 */
typedef struct TmemInfoEntryStruct{
	DWORD start;
	DWORD length;
	DWORD rdramAddr;
	TmemInfoEntryStruct* next;
} TmemInfoEntry;

const int tmenMaxEntry=20;
TmemInfoEntry tmenEntryBuffer[20]={0};
TmemInfoEntry *g_pTMEMInfo=NULL;
TmemInfoEntry *g_pTMEMFreeList=tmenEntryBuffer;

void TMEM_Init()
{
	g_pTMEMInfo=NULL;
	g_pTMEMFreeList=tmenEntryBuffer;
	for( int i=0; i<tmenMaxEntry; i++ )
	{
		tmenEntryBuffer[i].start=0;
		tmenEntryBuffer[i].length=0;
		tmenEntryBuffer[i].rdramAddr=0;
		tmenEntryBuffer[i].next = &(tmenEntryBuffer[i+1]);
	}
	tmenEntryBuffer[i].next = NULL;
}

void TMEM_SetBlock(DWORD tmemstart, DWORD length, DWORD rdramaddr)
{
	TmemInfoEntry *p=g_pTMEMInfo;

	if( p == NULL )
	{
		// Move an entry from freelist and link it to the header
		p = g_pTMEMFreeList;
		g_pTMEMFreeList = g_pTMEMFreeList->next;

		p->start = tmemstart;
		p->length = length;
		p->rdramAddr = rdramaddr;
		p->next = NULL;
	}
	else
	{
		while ( tmemstart > (p->start+p->length) )
		{
			if( p->next != NULL ) {
				p = p->next;
				continue;
			}
			else {
				break;
			}
		}

		if ( p->start == tmemstart ) 
		{
			// need to replace the block of 'p'
			// or append a new block depend the block lengths
			if( length == p->length )
			{
				p->rdramAddr = rdramaddr;
				return;
			}
			else if( length < p->length )
			{
				TmemInfoEntry *newentry = g_pTMEMFreeList;
				g_pTMEMFreeList = g_pTMEMFreeList->next;

				newentry->length = p->length - length;
				newentry->next = p->next;
				newentry->rdramAddr = p->rdramAddr + p->length;
				newentry->start = p->start + p->length;

				p->length = length;
				p->next = newentry;
				p->rdramAddr = rdramaddr;
			}
		}
		else if( p->start > tmemstart )
		{
			// p->start > tmemstart, need to insert the new block before 'p'
			TmemInfoEntry *newentry = g_pTMEMFreeList;
			g_pTMEMFreeList = g_pTMEMFreeList->next;

			if( length+tmemstart < p->start+p->length )
			{
				newentry->length = p->length - length;
				newentry->next = p->next;
				newentry->rdramAddr = p->rdramAddr + p->length;
				newentry->start = p->start + p->length;

				p->length = length;
				p->next = newentry;
				p->rdramAddr = rdramaddr;
				p->start = tmemstart;
			}
			else if( length+tmemstart == p->start+p->length )
			{

			}
		}
		else
		{
		}
	}
}

DWORD TMEM_GetRdramAddr(DWORD tmemstart, DWORD length)
{
	return 0;
}


/*
 *  New implementation of texture loading
 */

bool IsTmemFlagValid(DWORD tmemAddr)
{
	uint32 index = tmemAddr>>5;
	uint32 bitIndex = (tmemAddr&0x1F);
	return ((g_TmemFlag[index] & (1<<bitIndex))!=0);
}

uint32 GetValidTmemInfoIndex(DWORD tmemAddr)
{
	return 0;
	uint32 index = tmemAddr>>5;
	uint32 bitIndex = (tmemAddr&0x1F);

	if ((g_TmemFlag[index] & (1<<bitIndex))!=0 )	//This address is valid
		return tmemAddr;
	else
	{
		for( uint32 i=index; i>=0; i-- )
		{
			if( g_TmemFlag[i] != 0 )
			{
				for( uint32 j=0x1F; j>=0; j-- )
				{
					if( (g_TmemFlag[i] & (1<<j)) != 0 )
					{
						return ((i<<5)+j);
					}
				}
			}
		}
		TRACE0("Error, check me");
		return 0;
	}
}


void SetTmemFlag(DWORD tmemAddr, DWORD size)
{
	uint32 index = tmemAddr>>5;
	uint32 bitIndex = (tmemAddr&0x1F);

#ifdef _DEBUG
	if( size > 0x200 )
	{
		DebuggerAppendMsg("Check me: tmemaddr=%X, size=%x", tmemAddr, size);
		size = 0x200-tmemAddr;
	}
#endif

	if( bitIndex == 0 )
	{
		uint32 i;
		for( i=0; i< (size>>5); i++ )
		{
			g_TmemFlag[index+i] = 0;
		}

		if( (size&0x1F) != 0 )
		{
			//ErrorMsg("Check me: tmemaddr=%X, size=%x", tmemAddr, size);
			g_TmemFlag[index+i] &= ~((1<<(size&0x1F))-1);
		}

		g_TmemFlag[index] |= 1;
	}
	else
	{
		if( bitIndex + size <= 0x1F )
		{
			uint32 val = g_TmemFlag[index];
			uint32 mask = (1<<(bitIndex))-1;
			mask |= ~((1<<(bitIndex + size))-1);
			val &= mask;
			val |= (1<<bitIndex);
			g_TmemFlag[index] = val;
		}
		else
		{
			//ErrorMsg("Check me: tmemaddr=%X, size=%x", tmemAddr, size);
			uint32 val = g_TmemFlag[index];
			uint32 mask = (1<<bitIndex)-1;
			val &= mask;
			val |= (1<<bitIndex);
			g_TmemFlag[index] = val;
			index++;
			size -= (0x20-bitIndex);

			uint32 i;
			for( i=0; i< (size>>5); i++ )
			{
				g_TmemFlag[index+i] = 0;
			}

			if( (size&0x1F) != 0 )
			{
				//ErrorMsg("Check me: tmemaddr=%X, size=%x", tmemAddr, size);
				g_TmemFlag[index+i] &= ~((1<<(size&0x1F))-1);
			}
		}
	}
}

