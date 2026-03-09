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

uint32 g_TmemFlag[16];
void SetTmemFlag(uint32 tmemAddr, uint32 size);
bool IsTmemFlagValid(uint32 tmemAddr);
uint32 GetValidTmemInfoIndex(uint32 tmemAddr);

CTexture* EnhanceTexture(TextureEntry *pEntry);
CTexture* MirrorTexture(uint32 tileno, TextureEntry *pEntry);

extern TMEMLoadMapInfo g_tmemInfo0;				// Info for Tmem=0
extern TMEMLoadMapInfo g_tmemInfo1;				// Info for Tmem=0x100

TmemType g_Tmem;

/************************************************************************/
/*                                                                      */
/************************************************************************/
uint32 sizeShift[4] = {2,1,0,0};
uint32 sizeIncr[4] = {3,1,0,0};
uint32 sizeBytes[4] = {0,1,2,4};

inline uint32 Txl2Words(uint32 width, uint32 size)
{
	if( size == TXT_SIZE_4b )
		return max(1, width/16);
	else
		return max(1, width*sizeBytes[size]/8);
}

inline uint32 CalculateImgSize(uint32 width, uint32 height, uint32 size)
{
	//(((width)*(height) + siz##_INCR) >> siz##_SHIFT) -1
	return (((width)*(height) + sizeIncr[size]) >> sizeShift[size]) -1;
}


inline uint32 CalculateDXT(uint32 txl2words)
{
	//#define CALC_DXT(width, b_txl)	((2048 + TXL2WORDS(width, b_txl) - 1) / TXL2WORDS(width, b_txl))
	if( txl2words == 0 ) return 1;
	else return (2048+txl2words-1)/txl2words;
}

inline uint32 ReverseDXT(uint32 val, uint32 lrs, uint32 width, uint32 size)
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

	return	(low+high)/2;	//dxt = 2047 / (dxt-1);
}

// The following inline assemble routines are borrowed from glN64, I am too tired to
// rewrite these routine by myself.
// Rice, 02/24/2004
inline void UnswapCopy( void *src, void *dest, uint32 numBytes )
{
	__asm
	{
		mov		ecx, 0
		mov		esi, dword ptr [src]
		mov		edi, dword ptr [dest]

		mov		ebx, esi
		and		ebx, 3			// ebx = number of leading bytes

		cmp		ebx, 0
		jz		StartDWordLoop
		neg		ebx
		add		ebx, 4

		cmp		ebx, [numBytes]
		jle		NotGreater
		mov		ebx, [numBytes]
NotGreater:
		mov		ecx, ebx
			xor		esi, 3
LeadingLoop:				// Copies leading bytes, in reverse order (un-swaps)
		mov		al, byte ptr [esi]
		mov		byte ptr [edi], al
		sub		esi, 1
		add		edi, 1
		loop	LeadingLoop
		add		esi, 5

StartDWordLoop:
		mov		ecx, dword ptr [numBytes]
		sub		ecx, ebx		// Don't copy what's already been copied

		mov		ebx, ecx
		and		ebx, 3
		//		add		ecx, 3			// Round up to nearest dword
		shr		ecx, 2

		cmp		ecx, 0			// If there's nothing to do, don't do it
		jle		StartTrailingLoop

		// Copies from source to destination, bswap-ing first
DWordLoop:
		mov		eax, dword ptr [esi]
		bswap	eax
		mov		dword ptr [edi], eax
		add		esi, 4
		add		edi, 4
		loop	DWordLoop
StartTrailingLoop:
		cmp		ebx, 0
		jz		Done
		mov		ecx, ebx
		xor		esi, 3

TrailingLoop:
		mov		al, byte ptr [esi]
		mov		byte ptr [edi], al
		sub		esi, 1
		add		edi, 1
		loop	TrailingLoop
Done:
	}
}

inline void DWordInterleave( void *mem, uint32 numDWords )
{
	__asm {
		mov		esi, dword ptr [mem]
		mov		edi, dword ptr [mem]
		add		edi, 4
		mov		ecx, dword ptr [numDWords]
DWordInterleaveLoop:
		mov		eax, dword ptr [esi]
		mov		ebx, dword ptr [edi]
		mov		dword ptr [esi], ebx
		mov		dword ptr [edi], eax
		add		esi, 8
		add		edi, 8
		loop	DWordInterleaveLoop
	}
}

inline void QWordInterleave( void *mem, uint32 numDWords )
{
	__asm
	{
		// Interleave the line on the qword
		mov		esi, dword ptr [mem]
		mov		edi, dword ptr [mem]
		add		edi, 8
		mov		ecx, dword ptr [numDWords]
		shr		ecx, 1
QWordInterleaveLoop:
		mov		eax, dword ptr [esi]
		mov		ebx, dword ptr [edi]
		mov		dword ptr [esi], ebx
		mov		dword ptr [edi], eax
		add		esi, 4
		add		edi, 4
		mov		eax, dword ptr [esi]
		mov		ebx, dword ptr [edi]
		mov		dword ptr [esi], ebx
		mov		dword ptr [edi], eax
		add		esi, 12
		add		edi, 12
		loop	QWordInterleaveLoop
	}
}

inline uint32 swapdword( uint32 value )
{
	__asm
	{
		mov		eax, dword ptr [value]
		bswap	eax
	}
}

inline uint16 swapword( uint16 value )
{
	__asm
	{
		mov		ax, word ptr [value]
		xchg	ah, al
	}
}


void ComputeTileDimension(int mask, int clamp, int mirror, int width, uint32 &widthToCreate, uint32 &widthToLoad)
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
						// gti.WidthToLoad = gti.WidthToCreate = gRDP.tiles[tileno].dwWidth = dwTileWidth
					}
					else
					{
						widthToCreate = maskwidth;
					}
				}
				else
				{
					widthToCreate = maskwidth;
					//widthToLoad = maskwidth;
				}
			}
			else
			{
				widthToCreate = maskwidth;
				//widthToLoad = maskwidth;
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

bool conkerSwapHack=false;
void LoadTexture(uint32 tileno)
{
	uint32 dwPitch;
	TextureEntry *pEntry = NULL;
	TextureInfo gti;

	Tile &tile = gRDP.tiles[tileno];
	uint32 dwTLutFmt = gRDP.otherMode.text_tlut <<RSP_SETOTHERMODE_SHIFT_TEXTLUT;

	// Retrieve the tile loading info
	uint32 infoTmemAddr = tile.dwTMem;
	TMEMLoadMapInfo *info = &g_tmemLoadAddrMap[infoTmemAddr];
	if( !IsTmemFlagValid(infoTmemAddr) )
	{
		infoTmemAddr =  GetValidTmemInfoIndex(infoTmemAddr);
		info = &g_tmemLoadAddrMap[infoTmemAddr];
	}

	if( info->dwFormat != tile.dwFormat )
	{
		// Check the tile format, hack for Zelda's road
		if( tileno != gRSP.curTile && tile.dwTMem == gRDP.tiles[gRSP.curTile].dwTMem &&
			tile.dwFormat != gRDP.tiles[gRSP.curTile].dwFormat )
		{
			//TRACE1("Tile %d format is not matching the loaded texture format", tileno);
			pEntry = gTextureManager.GetBlackTexture();
			CRender::g_pRender->SetCurrentTexture( tileno, pEntry->pTexture, 4, 4, pEntry);
			return;
		}
	}


	// Now Initialize the texture dimension
	int dwTileWidth;
	int dwTileHeight;
	if( info->bSetBy == CMD_LOADTILE )
	{
		if( g_curRomInfo.bTxtSizeMethod2 )
		{
			if( tile.dwMaskS == 0 || tile.bClampS )
			{
				dwTileWidth = tile.hilite_sh - tile.hilite_sl +1;
				if( dwTileWidth < tile.sh - tile.sl +1 )
					dwTileWidth = tile.sh - tile.sl +1;
			}
			else
			{
				if( tile.dwMaskS < 8 )
					dwTileWidth = (1 << tile.dwMaskS );
				else
				{
					dwTileWidth = info->dwWidth;	// From SetTImage
					dwTileWidth = dwTileWidth << info->dwSize >> tile.dwSize;
				}
			}

			if( tile.dwMaskT == 0 || tile.bClampT )
			{
				dwTileHeight= tile.hilite_th - tile.hilite_tl +1;
				if( dwTileHeight < tile.th - tile.tl +1 )
					dwTileHeight = tile.th - tile.tl +1;
			}
			else
			{
				if( tile.dwMaskT < 8 )
					dwTileHeight = (1 << tile.dwMaskT );
				else
				{
					if( tile.tl >= tile.th )
					{
						dwTileHeight= info->TH - info->TL + 1;
					}
					else
					{
						dwTileHeight= tile.th - tile.tl + 1;
					}
				}
			}
		}
		else
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
			else if( tile.dwLine )
			{
				dwTileWidth = (tile.dwLine<<5)>>tile.dwSize;
			}
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



	if( dwTileWidth-dwTileMaskWidth == 1 && dwTileMaskWidth && dwTileHeight-dwTileMaskHeight == 1 && dwTileMaskHeight )
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


	gti.bSwapped = info->bSwapped;

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
		if (info->dxt == 0 || info->dwTmem != tile.dwTMem )
		{
			dwPitch = tile.dwLine << 3;
			gti.bSwapped = TRUE;
			if( info->dwTmem != tile.dwTMem && info->dxt != 0 && info->dwSize == TXT_SIZE_16b && tile.dwSize == TXT_SIZE_4b )
				conkerSwapHack = true;
		}
		else
		{
			uint32 DXT = info->dxt;
			if( info->dxt > 1 )
			{
				DXT = ReverseDXT(info->dxt, info->SH, dwTileWidth, tile.dwSize);
			}
			dwPitch = DXT << 3;
		}
		
		if (tile.dwSize == TXT_SIZE_32b)
			dwPitch = tile.dwLine << 4;
	}

	tile.dwPitch = dwPitch;

	if( (gti.WidthToLoad < gti.WidthToCreate || tile.bSizeIsValid == false) && tile.dwMaskS > 0 && gti.WidthToLoad != dwTileMaskWidth &&
		info->bSetBy == CMD_LOADBLOCK )
	//if( (gti.WidthToLoad < gti.WidthToCreate ) && tile.dwMaskS > 0 && gti.WidthToLoad != dwTileMaskWidth &&
	//	info->bSetBy == CMD_LOADBLOCK )
	{
		// We have got the pitch now, recheck the width_to_load
		uint32 pitchwidth = dwPitch<<1>>tile.dwSize;
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
		uint32 pitchwidth = dwPitch<<1>>tile.dwSize;
		uint32 pitchHeight = (info->dwTotalWords<<1)/dwPitch;
		if( pitchHeight == dwTileMaskHeight || gti.HeightToLoad == 1 )
		{
			gti.HeightToLoad = pitchHeight;
		}
	}
	if( gti.WidthToCreate < gti.WidthToLoad )	gti.WidthToCreate = gti.WidthToLoad;
	if( gti.HeightToCreate < gti.HeightToLoad )		gti.HeightToCreate = gti.HeightToLoad;

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

	uint32 total64BitWordsToLoad = (gti.HeightToLoad*gti.WidthToLoad)>>(4-tile.dwSize);
	if( total64BitWordsToLoad + tile.dwTMem > 0x200 )
	{
		//TRACE0("Warning: texture loading tmem is over range");
		if( gti.WidthToLoad > gti.HeightToLoad )
		{
			DWORD newheight = (dwPitch << 1 )>> tile.dwSize;
			tile.dwWidth = gti.WidthToLoad = gti.WidthToCreate = min(newheight, (gti.WidthToLoad&0xFFFFFFFE));
			tile.dwHeight = gti.HeightToCreate = gti.HeightToLoad = ((0x200 - tile.dwTMem) << (4-tile.dwSize)) / gti.WidthToLoad;
		}
		else
		{
			tile.dwHeight = gti.HeightToCreate = gti.HeightToLoad = info->dwTotalWords / ((gti.WidthToLoad << tile.dwSize) >> 1);
		}
	}


	// Check the info
	if( (info->dwTotalWords>>2) < total64BitWordsToLoad+tile.dwTMem-info->dwTmem - 4 )
	{
		if( total64BitWordsToLoad+tile.dwTMem-info->dwTmem <= 0x200 )
		{
			DEBUGGER_IF_DUMP( logTextures, 
			{
				DebuggerAppendMsg("Fix me, info is not covering this Tmem address,Info start: 0x%x, total=0x%x, Tmem start: 0x%x, total=0x%x", 
					info->dwTmem,info->dwTotalWords>>2, tile.dwTMem, total64BitWordsToLoad);
			});
		}

		gti.Address = info->dwLoadAddress+(tile.dwTMem-infoTmemAddr)*8;

		// Hack here
		if( options.enableHackForGames == HACK_FOR_ZELDA )
		{
			if( tileno != gRSP.curTile )
			{
				pEntry = gTextureManager.GetBlackTexture();
				CRender::g_pRender->SetCurrentTexture( tileno, pEntry->pTexture, 4, 4, pEntry);
				return;
			}
		}
		//gti.Address = g_tmemInfo0.dwLoadAddress+(tile.dwTMem-g_tmemInfo0.dwTmem)*8;
	}
	else
	{
		gti.Address = info->dwLoadAddress+(tile.dwTMem-infoTmemAddr)*8;
	}

	gti = tile;	// Copy tile info to textureInfo entry

	gti.PalAddress = (uint32)(&g_wRDPTlut[0]);
	gti.TLutFmt = dwTLutFmt;

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

	if (gti.Format == TXT_FMT_CI && gti.TLutFmt == TLUT_FMT_NONE )
		gti.TLutFmt = TLUT_FMT_RGBA16;		// Force RGBA

	gti.pPhysicalAddress = ((BYTE*)g_pRDRAMu32)+gti.Address;

	//freakdave - found this in 6.11 (RDP_Texture.h -> line 853 -> TxtrCacheEntry* LoadTexture(uint32 tileno)
#ifdef _XBOX
	// Hack for XBOX, don't use too big textures
	if( gti.HeightToCreate/gti.HeightToLoad >= 2 && gti.HeightToCreate > 128 )//&& tile.bClampT )//&& tile.dwMaskT )
	{
		tile.bClampT = 0;
		gti.HeightToCreate = gti.HeightToLoad;
		tile.dwHeight = gti.HeightToLoad;
	}

	if( gti.WidthToCreate/gti.WidthToLoad >= 2 && gti.WidthToCreate > 128 )//&& tile.bClampS )//&& tile.dwMaskS )
	{
		tile.bClampS = 0;
		gti.WidthToCreate = gti.WidthToLoad;
		tile.dwWidth = gti.WidthToLoad;
	}
#endif

#ifdef _DEBUG
	if( logTextures )
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
		uint32 idx = tileno-gRSP.curTile;
		status.LargerTileRealLeft[idx] = gti.LeftToLoad;
		gti.LeftToLoad=0;
		gti.WidthToLoad = gti.WidthToCreate = ((gti.Pitch<<1)>>gti.Size);
		status.UseLargerTile[idx]=true;
	}

	gti.tileNo = tileno;
	pEntry = gTextureManager.GetTexture(&gti, true, true, true);	// Load the texture by using texture cache

	if (pEntry && pEntry->pTexture )
	{
		if( pEntry->txtrBufIdx <= 0 )
		{
			if( pEntry->pEnhancedTexture == NULL )
			{
				MirrorTexture(tileno, pEntry);;
			}

			if( pEntry->pEnhancedTexture == NULL || pEntry->dwEnhancementFlag != options.textureEnhancement )
			{
				EnhanceTexture(pEntry);
			}
		}

		CRender::g_pRender->SetCurrentTexture( tileno, 
			(pEntry->pEnhancedTexture)?pEntry->pEnhancedTexture:pEntry->pTexture,
			pEntry->ti.WidthToLoad, pEntry->ti.HeightToLoad, pEntry);
	}
	else
	{
		CRender::g_pRender->SetCurrentTexture( tileno, NULL, 64, 64, NULL );
	}
}

void LoadTextureNew(uint32 tileno)
{
	TextureEntry *pEntry = NULL;
	TextureInfo gti;

	Tile &tile = gRDP.tiles[tileno];
	uint32 dwTLutFmt = gRDP.otherMode.text_tlut <<RSP_SETOTHERMODE_SHIFT_TEXTLUT;

	// Now Initialize the texture dimension
	int dwTileWidth= (tile.sl > tile.sh ? tile.sl - tile.sh : tile.sh - tile.sl) + 1;
	int dwTileHeight= (tile.tl > tile.th ? tile.tl - tile.th : tile.th - tile.tl ) + 1;
	int dwTileMaskWidth = tile.dwMaskS > 0 ? (1 << tile.dwMaskS ) : 0;
	int dwTileMaskHeight = tile.dwMaskT > 0 ? (1 << tile.dwMaskT ) : 0;
	if( dwTileMaskWidth!=0 && !tile.bClampS && tile.dwMaskS<8  ) dwTileWidth = dwTileMaskWidth;
	if( dwTileMaskHeight!=0 && !tile.bClampT && tile.dwMaskT<8  ) dwTileHeight = dwTileMaskHeight;

	if( dwTileWidth-dwTileMaskWidth == 1 && dwTileHeight-dwTileMaskHeight == 1 )
	{
		// Hack for Mario Kart
		dwTileWidth--;
		dwTileHeight--;
	}

	ComputeTileDimension(tile.dwMaskS, tile.bClampS, tile.bMirrorS, dwTileWidth, gti.WidthToCreate, gti.WidthToLoad);
	ComputeTileDimension(tile.dwMaskT, tile.bClampT, tile.bMirrorT, dwTileHeight, gti.HeightToCreate, gti.HeightToLoad);
	tile.dwWidth = gti.WidthToCreate;
	tile.dwHeight = gti.HeightToCreate;

	gti.bSwapped = FALSE;					// Doesn't matter, content in TMEM is already swapped
	tile.dwPitch = tile.dwLine << 3;		// Doesn't matter, Pitch is calculated from tile.dwLine

	if( gti.WidthToCreate < gti.WidthToLoad )	gti.WidthToCreate = gti.WidthToLoad;
	if( gti.HeightToCreate < gti.HeightToLoad )		gti.HeightToCreate = gti.HeightToLoad;


	if( gti.HeightToLoad*tile.dwLine + tile.dwTMem > 0x200 )
	{
		tile.dwHeight = gti.HeightToCreate = gti.HeightToLoad = (0x200 - tile.dwTMem) / tile.dwLine;
	}


	Tile &loadtile = gRDP.tiles[7];

	gti.PalAddress = (uint32)(&g_wRDPTlut[0]);			// Doesn't matter
	//gti.LeftToLoad = loadtile.sl<<loadtile.dwSize>>tile.dwSize;						// Doesn't matter
	//gti.TopToLoad = loadtile.tl;						// Doesn't matter
	//gti.Address = g_TI.dwAddr & (g_dwRamSize-1);					// Doesn't matter
	//gti.pPhysicalAddress = ((BYTE*)g_pRDRAMu32)+gti.Address;		// Doesn't matter

	gti.Address = (g_TI.dwAddr+g_TI.dwWidth*loadtile.tl+loadtile.sl) & (g_dwRamSize-1);
	gti.pPhysicalAddress = (BYTE*)&g_Tmem.g_Tmem64bit[tile.dwTMem];	// Doesn't matter
	gti.LeftToLoad = 0;						// Doesn't matter
	gti.TopToLoad = 0;						// Doesn't matter
	//gti.LeftToLoad = loadtile.sl<<loadtile.dwSize>>tile.dwSize;		// Doesn't matter, but used if loading from back buffer
	//gti.TopToLoad = loadtile.tl;									// Doesn't matter


	gti = tile;	// Copy tile info to textureInfo entry
	gti.TLutFmt = dwTLutFmt;

	if (gti.Format == TXT_FMT_CI && gti.TLutFmt == TLUT_FMT_NONE )
		gti.TLutFmt = TLUT_FMT_RGBA16;		// Force RGBA

		//freakdave - found this in 6.11 (RDP_Texture.h -> line 853 -> TxtrCacheEntry* LoadTexture(uint32 tileno)
#ifdef _XBOX
	// Hack for XBOX, don't use too big textures
	if( gti.HeightToCreate/gti.HeightToLoad >= 2 && gti.HeightToCreate > 128 )//&& tile.bClampT )//&& tile.dwMaskT )
	{
		tile.bClampT = 0;
		gti.HeightToCreate = gti.HeightToLoad;
		tile.dwHeight = gti.HeightToLoad;
	}

	if( gti.WidthToCreate/gti.WidthToLoad >= 2 && gti.WidthToCreate > 128 )//&& tile.bClampS )//&& tile.dwMaskS )
	{
		tile.bClampS = 0;
		gti.WidthToCreate = gti.WidthToLoad;
		tile.dwWidth = gti.WidthToLoad;
	}
#endif

#ifdef _DEBUG
	if( logTextures )
	{
		TRACE0("Loading texture:\n");
		DebuggerAppendMsg("Left: %d, Top: %d, Width: %d, Height: %d, Size to Load (%d, %d)", 
			gti.LeftToLoad, gti.TopToLoad, gti.WidthToCreate, gti.HeightToCreate, gti.WidthToLoad, gti.HeightToLoad);
		DebuggerAppendMsg("Pitch: %d, Addr: 0x%08x", tile.dwPitch, gti.Address);
	}
#endif

	gti.tileNo = tileno;
	pEntry = gTextureManager.GetTexture(&gti, true, true, true);	// Load the texture by using texture cache

	if (pEntry && pEntry->pTexture )
	{
		if( pEntry->txtrBufIdx <= 0 )
		{
			if( pEntry->pEnhancedTexture == NULL )
			{
				MirrorTexture(tileno, pEntry);;
			}

			if( pEntry->pEnhancedTexture == NULL )
			{
				EnhanceTexture(pEntry);
			}
		}

		CRender::g_pRender->SetCurrentTexture( tileno, 
			(pEntry->pEnhancedTexture)?pEntry->pEnhancedTexture:pEntry->pTexture,
			pEntry->ti.WidthToLoad, pEntry->ti.HeightToLoad, pEntry);
	}
	else
	{
		CRender::g_pRender->SetCurrentTexture( tileno, NULL, 64, 64, NULL );
	}
}

void PrepareTextures()
{
	StartProfiler(PROFILE_TEXTURE);
	if( gRDP.textureIsChanged || !currentRomOptions.bFastTexCRC ||
		CRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[0] ||
		CRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[1] )
	{
		status.UseLargerTile[0]=false;
		status.UseLargerTile[1]=false;

		if( CRender::g_pRender->IsTexel0Enable() || gRDP.otherMode.cycle_type  == CYCLE_TYPE_COPY )
		{
			if( CRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[0] )
			{
				TextureEntry *pEntry = gTextureManager.GetConstantColorTexture(CRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[0]);
				CRender::g_pRender->SetCurrentTexture( gRSP.curTile, pEntry->pTexture, 4, 4, pEntry);
			}
			else
			{
				LoadTexture(gRSP.curTile);
				//LoadTextureNew(gRSP.curTile);
			}
		}

		if( gRSP.curTile<7 && CRender::g_pRender->IsTexel1Enable() )
		{
			if( CRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[1] )
			{
				TextureEntry *pEntry = gTextureManager.GetConstantColorTexture(CRender::g_pRender->m_pColorCombiner->m_pDecodedMux->m_ColorTextureFlag[1]);
				CRender::g_pRender->SetCurrentTexture( gRSP.curTile+1, pEntry->pTexture, 4, 4, pEntry);
			}
			else
			{
				LoadTexture(gRSP.curTile+1);
				//LoadTextureNew(gRSP.curTile+1);
			}
		}
		gRDP.textureIsChanged = false;
	}
	StopProfiler(PROFILE_TEXTURE);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
extern uint32 g_TxtLoadBy;;

void DLParser_LoadTLut(uint32 word0, uint32 word1)
{
	gRDP.textureIsChanged = true;

	uint32 tileno  = ((word1 >> 24) & 0x07);
	uint32 dwTLutFmt = (gRDP.otherModeH >> RSP_SETOTHERMODE_SHIFT_TEXTLUT)&0x3;

	uint32 dwCount;

	uint32 dwTMEMOffset = gRDP.tiles[tileno].dwTMem - 256;				// starting location in the palettes
	dwCount = ((WORD)(word1 >> 14) & 0x03FF) + 1;		// number to copy
	uint32 dwRDRAMOffset = 0;

	uint32 uls   = ((word0 >> 12) & 0xfff)/4;
	uint32 ult   = ((word0      ) & 0xfff)/4;
	uint32 lrs   = ((word1 >> 12) & 0xfff)/4;
	uint32 lrt   = ((word1      ) & 0xfff)/4;

	Tile &tile = gRDP.tiles[tileno];

	tile.hilite_sl = tile.sl = uls;
	tile.hilite_tl = tile.tl = ult;
	tile.sh = lrs;
	tile.th = lrt;
	tile.bSizeIsValid = true;

	tile.lastTileCmd = CMD_LOADTLUT;

#ifdef _DEBUG
	if( ((word0>>12)&0x3) != 0 || ((word0)&0x3) != 0 || ((word1>>12)&0x3) != 0 || ((word1)&0x3) != 0 )
	{
		//TRACE0("Load tlut, sl,tl,sh,th are not integers");
	}
#endif

	dwCount = (lrs - uls)+1;
	dwRDRAMOffset = (uls + ult*g_TI.dwWidth )*2;
	uint32 dwPalAddress = g_TI.dwAddr + dwRDRAMOffset;

	//Copy PAL to the PAL memory
	WORD *srcPal = (WORD*)(g_pRDRAMu8 + (dwPalAddress& (g_dwRamSize-1)) );
	for (uint32 i=0; i<dwCount && i<0x100; i++)
	{
		g_wRDPTlut[(i+dwTMEMOffset)^1] = srcPal[i^1];
	}

	if( options.bUseFullTMEM )
	{
		for (uint32 i=0; i<dwCount && i+tile.dwTMem<0x200; i++)
		{
			*(WORD*)(&g_Tmem.g_Tmem64bit[tile.dwTMem+i]) = srcPal[i^1];
		}
	}

#ifdef _DEBUG
	if( (pauseAtNext && eventToPause == NEXT_LOADTLUT)  || logTextures )
	{	
		DebuggerAppendMsg("LoadTLut Tile: %d Start: 0x%X+0x%X, Count: 0x%X\nFmt is %s, TMEM=0x%X\n", 
		tileno, g_TI.dwAddr, dwRDRAMOffset, dwCount,textluttype[dwTLutFmt], dwTMEMOffset);
		DebuggerAppendMsg("    :ULS: 0x%X, ULT:0x%X, LRS: 0x%X, LRT:0x%X\n", 
		uls, ult, lrs,lrt);

		if( pauseAtNext && eventToPause == NEXT_LOADTLUT && dwCount == 16 ) 
		{
			char buf[2000];
			strcpy(buf, "Data:\n");
			for( i=0; i<16; i++ )
			{
				sprintf(buf+strlen(buf), "%04X ", g_wRDPTlut[dwTMEMOffset+i]);
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

	extern bool RevTlutTableNeedUpdate;
	RevTlutTableNeedUpdate = true;
	g_TxtLoadBy = CMD_LOADTLUT;
}


void DLParser_LoadBlock(uint32 word0, uint32 word1)
{
	gRDP.textureIsChanged = true;

	uint32 uls		= ((word0>>12)&0x0FFF);
	uint32 ult		= ((word0    )&0x0FFF);
	uint32 lrs		= ((word1>>12)&0x0FFF);
	uint32 tileno	=  (word1>>24)&0x07;
	uint32 dxt		=  (word1    )&0x0FFF;		// 1.11 fixed point
	Tile &tile = gRDP.tiles[tileno];

	uint32 size		= lrs+1;
	if( tile.dwSize == TXT_SIZE_32b )	size<<=1;

	SetTmemFlag(tile.dwTMem, size>>2);

	TMEMLoadMapInfo &info = g_tmemLoadAddrMap[tile.dwTMem];

	info.bSwapped = (dxt == 0? TRUE : FALSE);

	info.SL = tile.hilite_sl = tile.sl = uls;
	info.SH = tile.hilite_tl = tile.tl = lrs;
	info.TL = tile.sh = ult;
	info.TH = tile.th = dxt;
	tile.bSizeIsValid = true;

	for( int i=0; i<8; i++ )
	{
		if( tile.dwTMem == tile.dwTMem )
			tile.lastTileCmd = CMD_LOADBLOCK;
	}

	info.dwLoadAddress = g_TI.dwAddr;
	info.bSetBy = CMD_LOADBLOCK;
	info.dxt = dxt;
	info.dwLine = tile.dwLine;

	info.dwFormat = g_TI.dwFormat;
	info.dwSize = g_TI.dwSize;
	info.dwWidth = g_TI.dwWidth;
	info.dwTotalWords = size;
	info.dwTmem = tile.dwTMem;

	if( gRDP.tiles[tileno].dwTMem == 0 )
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
	else if( tile.dwTMem == 0x100 )
	{
		if( size == 1024 )
		{
			memcpy(&g_tmemInfo1, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo1.dwTotalWords = size>>2;
		}
	}

	g_TxtLoadBy = CMD_LOADBLOCK;


	if( options.bUseFullTMEM )
	{
		uint32 bytes = (lrs + 1) << tile.dwSize >> 1;
		uint32 address = g_TI.dwAddr + ult * g_TI.bpl + (uls << g_TI.dwSize >> 1);
		if ((bytes == 0) || ((address + bytes) > g_dwRamSize) || (((tile.dwTMem << 3) + bytes) > 4096))
		{
			return;
		}
		uint64* src = (uint64*)(g_pRDRAMu8+address);
		uint64* dest = &g_Tmem.g_Tmem64bit[tile.dwTMem];

		if( dxt > 0)
		{
			void (*Interleave)( void *mem, uint32 numDWords );

			uint32 line = (2047 + dxt) / dxt;
			uint32 bpl = line << 3;
			uint32 height = bytes / bpl;

			if (tile.dwSize == TXT_SIZE_32b)
				Interleave = QWordInterleave;
			else
				Interleave = DWordInterleave;

			for (uint32 y = 0; y < height; y++)
			{
				UnswapCopy( src, dest, bpl );
				if (y & 1) Interleave( dest, line );

				src += line;
				dest += line;
			}
		}
		else
			UnswapCopy( src, dest, bytes );
	}


#ifdef _DEBUG
	LOG_DL("    Tile:%d (%d,%d) DXT:0x%04x\n",
		tileno, uls, ult, dxt);

	if( logTextures   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
		DebuggerAppendMsg("LoadBlock:%d (%d,%d,%d) DXT:0x%04x(%X)\n",
			tileno, uls, ult, ((word1>>12)&0x0FFF), dxt, (word1&0x0FFF));
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);
#endif
}

void swap(int &a, int &b)
{
	int temp = a;
	a = b;
	b = temp;
}
void DLParser_LoadTile(uint32 word0, uint32 word1)
{
	gRDP.textureIsChanged = true;

	uint32 tileno	=  (word1>>24)&0x07;
	uint32 uls		= ((word0>>12)&0x0FFF)/4;
	uint32 ult		= ((word0    )&0x0FFF)/4;
	uint32 lrs		= ((word1>>12)&0x0FFF)/4;
	uint32 lrt		= ((word1    )&0x0FFF)/4;

	Tile &tile = gRDP.tiles[tileno];

	if( lrt < ult )
	{
		uint32 temp = ult;
		ult = lrt;
		lrt = temp;
	}

	tile.hilite_sl = tile.sl = uls;
	tile.hilite_tl = tile.tl = ult;
	tile.hilite_sh = tile.sh = lrs;
	tile.hilite_th = tile.th = lrt;
	tile.bSizeIsValid = true;

	if( options.bUseFullTMEM )
	{
		void (*Interleave)( void *mem, uint32 numDWords );
		uint32 address, height, bpl, line, y;
		uint64 *dest;
		uint8 *src;

		if( g_TI.bpl == 0 )
		{
			if( options.enableHackForGames == HACK_FOR_BUST_A_MOVE )
			{
				g_TI.bpl = 1024;		// Hack for Bust-A-Move
			}
			else
			{
				TRACE0("Warning: g_TI.bpl = 0" );
			}
		}

		address = g_TI.dwAddr + tile.tl * g_TI.bpl + (tile.sl << g_TI.dwSize >> 1);
		dest = &g_Tmem.g_Tmem64bit[tile.dwTMem];
		if( tile.sh < tile.sl )	swap(tile.sh, tile.sl);
		bpl = (tile.sh - tile.sl + 1) << tile.dwSize >> 1;
		height = tile.th - tile.tl + 1;
		src = &g_pRDRAMu8[address];

		if (((address + height * bpl) > g_dwRamSize) || (((tile.dwTMem << 3) + bpl * height) > 4096)) // Stay within TMEM
		{
			return;
		}

		// Line given for 32-bit is half what it seems it should since they split the
		// high and low words. I'm cheating by putting them together.
		if (tile.dwSize == TXT_SIZE_32b)
		{
			line = tile.dwLine << 1;
			Interleave = QWordInterleave;
		}
		else
		{
			line = tile.dwLine;
			Interleave = DWordInterleave;
		}

		if( tile.dwLine == 0 )
		{
			//tile.dwLine = 1;
			return;
		}

		for (y = 0; y < height; y++)
		{
			UnswapCopy( src, dest, bpl );
			if (y & 1) Interleave( dest, line );

			src += g_TI.bpl;
			dest += line;
		}
	}


	for( int i=0; i<8; i++ )
	{
		if( gRDP.tiles[i].dwTMem == tile.dwTMem )
			gRDP.tiles[i].lastTileCmd = CMD_LOADTILE;
	}

	uint32 size = tile.dwLine*(lrt-ult+1);
	if( tile.dwSize == TXT_SIZE_32b )	size<<=1;
	SetTmemFlag(tile.dwTMem,size );

	uint32 dwPitch = g_TI.dwWidth;
	if( tile.dwSize != TXT_SIZE_4b )
	{
		dwPitch = g_TI.dwWidth<<(tile.dwSize-1);
	}

#ifdef _DEBUG
	if( logTextures   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
		DebuggerAppendMsg("LoadTile:%d (%d,%d) -> (%d,%d) [%d x %d]\n",
			tileno, uls, ult, lrs, lrt,
			(lrs - uls)+1, (lrt - ult)+1);
	}

	
	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);
#endif

	LOG_DL("    Tile:%d (%d,%d) -> (%d,%d) [%d x %d]",
		tileno, uls, ult, lrs, lrt,
		(lrs - uls)+1, (lrt - ult)+1);

	TMEMLoadMapInfo &info = g_tmemLoadAddrMap[tile.dwTMem];

	info.dwLoadAddress = g_TI.dwAddr;
	info.dwFormat = g_TI.dwFormat;
	info.dwSize = g_TI.dwSize;
	info.dwWidth = g_TI.dwWidth;

	info.SL = uls;
	info.SH = lrs;
	info.TL = ult;
	info.TH = lrt;
	
	info.dxt = 0;
	info.dwLine = tile.dwLine;
	info.dwTmem = tile.dwTMem;
	info.dwTotalWords = size<<2;

	info.bSetBy = CMD_LOADTILE;
	info.bSwapped =FALSE;

	g_TxtLoadBy = CMD_LOADTILE;

	if( tile.dwTMem == 0 )
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
	else if( tile.dwTMem == 0x100 )
	{
		if( size == 256 )
		{
			memcpy(&g_tmemInfo1, &info, sizeof(TMEMLoadMapInfo) );
			g_tmemInfo1.dwTotalWords = size;
		}
	}
}


static char *pszOnOff[2]     = {"Off", "On"};
uint32 lastSetTile;
void DLParser_SetTile(uint32 word0, uint32 word1)
{
	gRDP.textureIsChanged = true;

	uint32 tileno		= (word1>>24)&0x7;
	Tile &tile = gRDP.tiles[tileno];
	lastSetTile = tileno;

	tile.dwFormat	= (word0>>21)&0x7;
	tile.dwSize		= (word0>>19)&0x3;
	tile.dwLine		= (word0>>9 )&0x1FF;
	tile.dwTMem		= (word0    )&0x1FF;

	tile.dwPalette	= (word1>>20)&0x0F;
	tile.bClampT		= (word1>>19)&0x01;
	tile.bMirrorT		= (word1>>18)&0x01;
	tile.dwMaskT		= (word1>>14)&0x0F;
	tile.dwShiftT		= (word1>>10)&0x0F;		// LOD stuff
	tile.bClampS		= (word1>>9 )&0x01;
	tile.bMirrorS		= (word1>>8 )&0x01;
	tile.dwMaskS		= (word1>>4 )&0x0F;
	tile.dwShiftS		= (word1    )&0x0F;		// LOD stuff

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
	if( logTextures   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
	DebuggerAppendMsg("SetTile:%d  Fmt: %s/%s Line:%d TMem:0x%04x Palette:%d\n",
		tileno, pszImgFormat[tile.dwFormat], pszImgSize[tile.dwSize],
		tile.dwLine,  tile.dwTMem, tile.dwPalette);
	DebuggerAppendMsg("         S: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x\n",
		pszOnOff[tile.bClampS],pszOnOff[tile.bMirrorS],
		tile.dwMaskS, tile.dwShiftS);
	DebuggerAppendMsg("         T: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x\n",
		pszOnOff[tile.bClampT],pszOnOff[tile.bMirrorT],
		tile.dwMaskT, tile.dwShiftT);
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);

	LOG_DL("    Tile:%d  Fmt: %s/%s Line:%d TMem:0x%04x Palette:%d",
		tileno, pszImgFormat[tile.dwFormat], pszImgSize[tile.dwSize],
		tile.dwLine, tile.dwTMem, tile.dwPalette);
	LOG_DL("         S: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x",
		pszOnOff[tile.bClampS],pszOnOff[tile.bMirrorS],
		tile.dwMaskS, tile.dwShiftS);
	LOG_DL("         T: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x",
		pszOnOff[tile.bClampT],pszOnOff[tile.bMirrorT],
		tile.dwMaskT, tile.dwShiftT);
#endif
}

void DLParser_SetTileSize(uint32 word0, uint32 word1)
{
	gRDP.textureIsChanged = true;

	uint32 tileno	= (word1>>24)&0x07;
	int sl		= (uint32)((word0>>12)&0x0FFF);
	int tl		= (uint32)((word0    )&0x0FFF);
	int sh		= (uint32)((word1>>12)&0x0FFF);
	int th		= (uint32)((word1    )&0x0FFF);

#ifdef _DEBUG
	if( ((word0>>12)&0x3) != 0 || ((word0)&0x3) != 0 || ((word1>>12)&0x3) != 0 || ((word1)&0x3) != 0 )
	{
		//DebuggerAppendMsg("Set tile size, sl,tl,sh,th are not integers");
	}
#endif

	Tile &tile = gRDP.tiles[tileno];

	if( options.bUseFullTMEM )
	{
		tile.bSizeIsValid = true;
		tile.hilite_sl = tile.sl = sl / 4;
		tile.hilite_tl = tile.tl = tl / 4;
		tile.hilite_sh = tile.sh = sh / 4;
		tile.hilite_th = tile.th = th / 4;

		//if( (sh - sl)&3 )	tile.hilite_sh = ++tile.sh;
		//if( (th - tl)&3 )	tile.hilite_th = ++tile.th;
		//if( sh == sl && sl > 0 && sl < 4 )	tile.hilite_sh = ++tile.sh;
		//if( th == tl && tl > 0 && tl < 4 )	tile.hilite_th = ++tile.th;

		tile.fhilite_sl = tile.fsl = sl / 4.0f;
		tile.fhilite_tl = tile.ftl = tl / 4.0f;
		tile.fhilite_sh = tile.fsh = sh / 4.0f;
		tile.fhilite_th = tile.fth = th / 4.0f;

		tile.lastTileCmd = CMD_SETTILE_SIZE;
	}
	else
	{
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
				//if( sl>sh ) {uint32 temp=sl;sl=sh;sh=temp;}
				//if( tl>th ) {uint32 temp=tl;tl=th;th=temp;}
				tile.bSizeIsValid = false;
			}
			tile.hilite_sl = tile.sl = sl / 4;
			tile.hilite_tl = tile.tl = tl / 4;
			tile.hilite_sh = tile.sh = sh / 4;
			tile.hilite_th = tile.th = th / 4;

			//if( (sh - sl)&3 )	tile.hilite_sh = ++tile.sh;
			//if( (th - tl)&3 )	tile.hilite_th = ++tile.th;
			//if( sh == sl && sl > 0 && sl < 4 )	tile.hilite_sh = ++tile.sh;
			//if( th == tl && tl > 0 && tl < 4 )	tile.hilite_th = ++tile.th;

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

			//if( (sh - sl)&3 )	tile.hilite_sh++;
			//if( (th - tl)&3 )	tile.hilite_th++;
			//if( sh == sl && sl > 0 && sl < 4 )	tile.hilite_sh++;
			//if( th == tl && tl > 0 && tl < 4 )	tile.hilite_th++;

			tile.lastTileCmd = CMD_SETTILE_SIZE;
		}
	}

#ifdef _DEBUG
	if( logTextures   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
	DebuggerAppendMsg("SetTileSize:%d (%d/4,%d/4) -> (%d/4,%d/4) [%d x %d]\n",
		tileno, sl, tl, sh, th, 
		((sh/4) - (sl/4)) + 1, ((th/4) - (tl/4)) + 1);
	}
	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);

	LOG_DL("    Tile:%d (%d,%d) -> (%d,%d) [%d x %d]",
		tileno, sl/4, tl/4, sh/4, th/4,
		((sh/4) - (sl/4)) + 1, ((th/4) - (tl/4)) + 1);
#endif
}

extern char *pszImgFormat[8];// = {"RGBA", "YUV", "CI", "IA", "I", "?1", "?2", "?3"};
extern char *pszImgSize[4];// = {"4", "8", "16", "32"};
void DLParser_SetTImg(uint32 word0, uint32 word1)
{
	gRDP.textureIsChanged = true;

	g_TI.dwFormat 	= (word0>>21)&0x7;
	g_TI.dwSize   	= (word0>>19)&0x3;
	g_TI.dwWidth  	= (word0&0x0FFF) + 1;
	g_TI.dwAddr   	= RSPSegmentAddr(word1);
	g_TI.bpl		= g_TI.dwWidth << g_TI.dwSize >> 1;

#ifdef _DEBUG
	if( g_TI.dwAddr == 0x00ffffff)
	{
		TRACE0("Check me here in setTimg");
	}

	if( logTextures  || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
		DebuggerAppendMsg("SetTImage: 0x%08x Fmt: %s/%s Width in Pixel: %d\n", g_TI.dwAddr,
			pszImgFormat[g_TI.dwFormat], pszImgSize[g_TI.dwSize], g_TI.dwWidth);
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);

	LOG_DL("Image: 0x%08x Fmt: %s/%s Width in Pixel: %d", g_TI.dwAddr,
		pszImgFormat[g_TI.dwFormat], pszImgSize[g_TI.dwSize], g_TI.dwWidth);
#endif
}

void DLParser_TexRect(uint32 word0, uint32 word1)
{
	status.bCIBufferIsRendered = true;
	status.primitiveType = PRIM_TEXTRECT;

	// This command used 128bits, and not 64 bits. This means that we have to look one 
	// Command ahead in the buffer, and update the PC.
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	uint32 dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwPC+4+8);
	uint32 dwHalf1 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	uint32 dwHalf2 = *(uint32 *)(g_pRDRAMu8 + dwPC+8);

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
	if( !status.bHandleN64TextureBuffer && g_CI.dwAddr == g_ZI.dwAddr )
	{
		return;
	}


	LOG_DL("0x%08x: %08x %08x", dwPC, *(uint32 *)(g_pRDRAMu8 + dwPC+0), *(uint32 *)(g_pRDRAMu8 + dwPC+4));
	LOG_DL("0x%08x: %08x %08x", dwPC+8, *(uint32 *)(g_pRDRAMu8 + dwPC+8), *(uint32 *)(g_pRDRAMu8 + dwPC+8+4));

	uint32 dwXH		= ((word0>>12)&0x0FFF)/4;
	uint32 dwYH		= ((word0    )&0x0FFF)/4;
	uint32 tileno	= (word1>>24)&0x07;
	uint32 dwXL		= ((word1>>12)&0x0FFF)/4;
	uint32 dwYL		= ((word1    )&0x0FFF)/4;
	

	if( (int)dwXL >= gRDP.scissor.right || (int)dwYL >= gRDP.scissor.bottom || (int)dwXH < gRDP.scissor.left || (int)dwYH < gRDP.scissor.top )
	{
		// Clipping
		return;
	}

	uint16 uS		= (uint16)(  dwCmd2>>16)&0xFFFF;
	uint16 uT		= (uint16)(  dwCmd2    )&0xFFFF;
	short s16S = *(short*)(&uS);
	short s16T = *(short*)(&uT);

	uint16  uDSDX 	= (uint16)((  dwCmd3>>16)&0xFFFF);
	uint16  uDTDY	    = (uint16)((  dwCmd3    )&0xFFFF);
	short	 s16DSDX  = *(short*)(&uDSDX);
	short  s16DTDY	= *(short*)(&uDTDY);

	uint32 curTile = gRSP.curTile;
	ForceMainTextureIndex(tileno);

	float fS0 = s16S / 32.0f;
	float fT0 = s16T / 32.0f;

	float fDSDX = s16DSDX / 1024.0f;
	float fDTDY = s16DTDY / 1024.0f;

	uint32 cycletype = gRDP.otherMode.cycle_type;

	if (cycletype == CYCLE_TYPE_COPY)
	{
		fDSDX /= 4.0f;	// In copy mode 4 pixels are copied at once.
		dwXH++;
		dwYH++;
	}
	else if (cycletype == CYCLE_TYPE_FILL)
	{
		dwXH++;
		dwYH++;
	}

	if( fDSDX == 0 )	fDSDX = 1;
	if( fDTDY == 0 )	fDTDY = 1;

	float fS1 = fS0 + (fDSDX * (dwXH - dwXL));
	float fT1 = fT0 + (fDTDY * (dwYH - dwYL));

	LOG_DL("    Tile:%d Screen(%d,%d) -> (%d,%d)", tileno, dwXL, dwYL, dwXH, dwYH);
	LOG_DL("           Tex:(%#5f,%#5f) -> (%#5f,%#5f) (DSDX:%#5f DTDY:%#5f)",
                                            fS0, fT0, fS1, fT1, fDSDX, fDTDY);
	LOG_DL("");

	float t0u0 = (fS0-gRDP.tiles[tileno].hilite_sl) * gRDP.tiles[tileno].fShiftScaleS;
	float t0v0 = (fT0-gRDP.tiles[tileno].hilite_tl) * gRDP.tiles[tileno].fShiftScaleT;
	float t0u1 = t0u0 + (fDSDX * (dwXH - dwXL))*gRDP.tiles[tileno].fShiftScaleS;
	float t0v1 = t0v0 + (fDTDY * (dwYH - dwYL))*gRDP.tiles[tileno].fShiftScaleT;

	if( dwXL==0 && dwYL==0 && dwXH==windowSetting.fViWidth-1 && dwYH==windowSetting.fViHeight-1 &&
		t0u0 == 0 && t0v0==0 && t0u1==0 && t0v1==0 )
	{
		//Using TextRect to clear the screen
	}
	else
	{
		if( status.bHandleN64TextureBuffer && //status.bDirectWriteIntoRDRAM && 
			g_pTextureBufferInfo->CI_Info.dwFormat == gRDP.tiles[tileno].dwFormat && 
			g_pTextureBufferInfo->CI_Info.dwSize == gRDP.tiles[tileno].dwSize && 
			gRDP.tiles[tileno].dwFormat == TXT_FMT_CI && gRDP.tiles[tileno].dwSize == TXT_SIZE_8b )
		{
			if( options.enableHackForGames == HACK_FOR_YOSHI )
			{
				// Hack for Yoshi background image
				PrepareTextures();
				TexRectToFrameBuffer_8b(dwXL, dwYL, dwXH, dwYH, t0u0, t0v0, t0u1, t0v1, tileno);
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
					PrepareTextures();
					TexRectToFrameBuffer_8b(dwXL, dwYL, dwXH, dwYH, t0u0, t0v0, t0u1, t0v1, tileno);
				}

				if( !status.bDirectWriteIntoRDRAM )
				{
					CRender::g_pRender->TexRect(dwXL, dwYL, dwXH, dwYH, fS0, fT0, fDSDX, fDTDY);

					status.dwNumTrisRendered += 2;
				}
			}
		}
		else
		{
			CRender::g_pRender->TexRect(dwXL, dwYL, dwXH, dwYH, fS0, fT0, fDSDX, fDTDY);
			status.bFrameBufferDrawnByTriangles = true;

			status.dwNumTrisRendered += 2;
		}
	}

	if( status.bHandleN64TextureBuffer )	g_pTextureBufferInfo->maxUsedHeight = max(g_pTextureBufferInfo->maxUsedHeight,(int)dwYH);

	ForceMainTextureIndex(curTile);
}


void DLParser_TexRectFlip(uint32 word0, uint32 word1)
{ 
	status.bCIBufferIsRendered = true;
	status.primitiveType = PRIM_TEXTRECTFLIP;

	// This command used 128bits, and not 64 bits. This means that we have to look one 
	// Command ahead in the buffer, and update the PC.
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	uint32 dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwPC+4+8);

	// Increment PC so that it points to the right place
	g_dwPCStack[g_dwPCindex].addr += 16;

	uint32 dwXH		= ((word0>>12)&0x0FFF)/4;
	uint32 dwYH		= ((word0    )&0x0FFF)/4;
	uint32 tileno	= (word1>>24)&0x07;
	uint32 dwXL		= ((word1>>12)&0x0FFF)/4;
	uint32 dwYL		= ((word1    )&0x0FFF)/4;
	uint32 dwS		= (  dwCmd2>>16)&0xFFFF;
	uint32 dwT		= (  dwCmd2    )&0xFFFF;
	LONG  nDSDX 	= (LONG)(short)((  dwCmd3>>16)&0xFFFF);
	LONG  nDTDY	    = (LONG)(short)((  dwCmd3    )&0xFFFF);


	uint32 curTile = gRSP.curTile;
	ForceMainTextureIndex(tileno);
	
	float fS0 = (float)dwS / 32.0f;
	float fT0 = (float)dwT / 32.0f;

	float fDSDX = (float)nDSDX / 1024.0f;
	float fDTDY = (float)nDTDY / 1024.0f;

	uint32 cycletype = gRDP.otherMode.cycle_type;

	if (cycletype == CYCLE_TYPE_COPY)
	{
		fDSDX /= 4.0f;	// In copy mode 4 pixels are copied at once.
		dwXH++;
		dwYH++;
	}
	else if (cycletype == CYCLE_TYPE_FILL)
	{
		dwXH++;
		dwYH++;
	}

	float fS1 = fS0 + (fDSDX * (dwYH - dwYL));
	float fT1 = fT0 + (fDTDY * (dwXH - dwXL));
	
	LOG_DL("    Tile:%d (%d,%d) -> (%d,%d)",
		tileno, dwXL, dwYL, dwXH, dwYH);
	LOG_DL("    Tex:(%#5f,%#5f) -> (%#5f,%#5f) (DSDX:%#5f DTDY:%#5f)",
		fS0, fT0, fS1, fT1, fDSDX, fDTDY);
	LOG_DL("");

	float t0u0 = (fS0) * gRDP.tiles[tileno].fShiftScaleS-gRDP.tiles[tileno].sl;
	float t0v0 = (fT0) * gRDP.tiles[tileno].fShiftScaleT-gRDP.tiles[tileno].tl;
	float t0u1 = t0u0 + (fDSDX * (dwYH - dwYL))*gRDP.tiles[tileno].fShiftScaleS;
	float t0v1 = t0v0 + (fDTDY * (dwXH - dwXL))*gRDP.tiles[tileno].fShiftScaleT;

	CRender::g_pRender->TexRectFlip(dwXL, dwYL, dwXH, dwYH, t0u0, t0v0, t0u1, t0v1);
	status.dwNumTrisRendered += 2;

	if( status.bHandleN64TextureBuffer )	g_pTextureBufferInfo->maxUsedHeight = max(g_pTextureBufferInfo->maxUsedHeight,int(dwYL+(dwXH-dwXL)));

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
typedef struct TmemInfoEntry{
	uint32 start;
	uint32 length;
	uint32 rdramAddr;
	TmemInfoEntry* next;
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

void TMEM_SetBlock(uint32 tmemstart, uint32 length, uint32 rdramaddr)
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

uint32 TMEM_GetRdramAddr(uint32 tmemstart, uint32 length)
{
	return 0;
}


/*
 *  New implementation of texture loading
 */

bool IsTmemFlagValid(uint32 tmemAddr)
{
	uint32 index = tmemAddr>>5;
	uint32 bitIndex = (tmemAddr&0x1F);
	return ((g_TmemFlag[index] & (1<<bitIndex))!=0);
}

uint32 GetValidTmemInfoIndex(uint32 tmemAddr)
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


void SetTmemFlag(uint32 tmemAddr, uint32 size)
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

