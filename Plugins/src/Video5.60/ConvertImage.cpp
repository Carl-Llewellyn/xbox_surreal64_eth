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

ConvertFunction		gConvertFunctions_FullTMEM[ 8 ][ 4 ] = 
{
	// 4bpp				8bpp			16bpp				32bpp
	{  Convert4b,		Convert8b,		Convert16b,			ConvertRGBA32 },		// RGBA
	{  NULL,			NULL,			ConvertYUV,			NULL },					// YUV
	{  Convert4b,		Convert8b,		NULL,				NULL },					// CI
	{  Convert4b,		Convert8b,		Convert16b,			NULL },					// IA
	{  Convert4b,		Convert8b,		Convert16b,			NULL },					// I
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL }					// ?
};
ConvertFunction		gConvertFunctions[ 8 ][ 4 ] = 
{
	// 4bpp				8bpp			16bpp				32bpp
	{  ConvertCI4,		ConvertCI8,		ConvertRGBA16,		ConvertRGBA32 },		// RGBA
	{  NULL,			NULL,			ConvertYUV,			NULL },					// YUV
	{  ConvertCI4,		ConvertCI8,		NULL,				NULL },					// CI
	{  ConvertIA4,		ConvertIA8,		ConvertIA16,		NULL },					// IA
	{  ConvertI4,		ConvertI8,		ConvertIA16,		NULL },					// I
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL }					// ?
};

ConvertFunction		gConvertTlutFunctions[ 8 ][ 4 ] = 
{
	// 4bpp				8bpp			16bpp				32bpp
	{  ConvertCI4,		ConvertCI8,		ConvertRGBA16,		ConvertRGBA32 },		// RGBA
	{  NULL,			NULL,			ConvertYUV,			NULL },					// YUV
	{  ConvertCI4,		ConvertCI8,		NULL,				NULL },					// CI
	{  ConvertCI4,		ConvertCI8,		ConvertIA16,		NULL },					// IA
	{  ConvertCI4,		ConvertCI8,		ConvertIA16,		NULL },					// I
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL }					// ?
};

extern bool conkerSwapHack;

void ConvertRGBA16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;

	// Copy of the base pointer
	WORD * pSrc = (WORD*)(tinfo.pPhysicalAddress);

	BYTE * pByteSrc = (BYTE *)pSrc;
	if (!pTexture->StartUpdate(&dInfo))
		return;

	uint32 nFiddle;

	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			if ((y&1) == 0)
				nFiddle = 0x2;
			else
				nFiddle = 0x2 | 0x4;

			// dwDst points to start of destination row
			DWORD * dwDst = (DWORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset ^ nFiddle];

				dwDst[x] = Convert555ToRGBA(w);
				
				// Increment word offset to point to the next two pixels
				dwWordOffset += 2;
			}
		}
	}
	else
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			// dwDst points to start of destination row
			DWORD * dwDst = (DWORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset ^ 0x2];

				dwDst[x] = Convert555ToRGBA(w);
				
				// Increment word offset to point to the next two pixels
				dwWordOffset += 2;
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

void ConvertRGBA32(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	if (!pTexture->StartUpdate(&dInfo))
		return;

	DWORD * pSrc = (DWORD*)(tinfo.pPhysicalAddress);

	if( options.bUseFullTMEM )
	{
		Tile &tile = gRDP.tiles[tinfo.tileNo];

		DWORD *pWordSrc;
		if( tinfo.tileNo >= 0 )
		{
			pWordSrc = (DWORD*)&g_Tmem.g_Tmem64bit[tile.dwTMem];

			for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
			{
				DWORD * dwDst = (DWORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

				uint32 nFiddle = ( y&1 )? 0x2 : 0;
				int idx = tile.dwLine*4*y;

				for (DWORD x = 0; x < tinfo.WidthToLoad; x++, idx++)
				{
					DWORD w = pWordSrc[idx^nFiddle];
					BYTE* psw = (BYTE*)&w;
					BYTE* pdw = (BYTE*)&dwDst[x];
					pdw[0] = psw[2];	// Blue
					pdw[1] = psw[1];	// Green
					pdw[2] = psw[0];	// Red
					pdw[3] = psw[3];	// Alpha
				}
			}
		}
	}
	else
	{

		if (tinfo.bSwapped)
		{
			for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
			{
				if ((y%2) == 0)
				{

					BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;
					BYTE *pS = (BYTE *)pSrc + (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);

					for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
					{
						pDst[0] = pS[1];	// Blue
						pDst[1] = pS[2];	// Green
						pDst[2] = pS[3];	// Red
						pDst[3] = pS[0];	// Alpha
						pS+=4;
						pDst+=4;
					}
				}
				else
				{

					DWORD *pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);
					BYTE *pS = (BYTE *)pSrc;
					LONG n;

					n = (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);
					for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
					{
						*pDst++ = COLOR_RGBA(pS[(n+3)^0x8],
							pS[(n+2)^0x8],
							pS[(n+1)^0x8],
							pS[(n+0)^0x8]);

						n += 4;
					}
				}
			}
		}
		else
		{
			for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
			{
				BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;
				BYTE *pS = (BYTE *)pSrc + (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);

				for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
				{
					pDst[0] = pS[1];	// Blue
					pDst[1] = pS[2];	// Green
					pDst[2] = pS[3];	// Red
					pDst[3] = pS[0];	// Alpha
					pS+=4;
					pDst+=4;
				}
			}

		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

// E.g. Dear Mario text
// Copy, Score etc
void ConvertIA4(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);

#ifdef _DEBUG
	if( ((DWORD)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;

			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			// This may not work if X is not even?
			DWORD dwByteOffset = (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad/2);

			// Do two pixels at a time
			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				// Even
				*pDst++ = ThreeToEight[(b & 0xE0) >> 5];
				*pDst++ = ThreeToEight[(b & 0xE0) >> 5];
				*pDst++ = ThreeToEight[(b & 0xE0) >> 5];
				*pDst++ = OneToEight[(b & 0x10) >> 4];	
				// Odd
				*pDst++ = ThreeToEight[(b & 0x0E) >> 1];
				*pDst++ = ThreeToEight[(b & 0x0E) >> 1];
				*pDst++ = ThreeToEight[(b & 0x0E) >> 1];
				*pDst++ = OneToEight[(b & 0x01)     ];

				dwByteOffset++;

			}

		}
	}
	else
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + (y * dInfo.lPitch);

			// This may not work if X is not even?
			DWORD dwByteOffset = (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad/2);

			// Do two pixels at a time
			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				// Even
				*pDst++ = ThreeToEight[(b & 0xE0) >> 5];
				*pDst++ = ThreeToEight[(b & 0xE0) >> 5];
				*pDst++ = ThreeToEight[(b & 0xE0) >> 5];
				*pDst++ = OneToEight[(b & 0x10) >> 4];	
				// Odd
				*pDst++ = ThreeToEight[(b & 0x0E) >> 1];
				*pDst++ = ThreeToEight[(b & 0x0E) >> 1];
				*pDst++ = ThreeToEight[(b & 0x0E) >> 1];
				*pDst++ = OneToEight[(b & 0x01)     ];

				dwByteOffset++;

			}
		}	
	}
	
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}

// E.g Mario's head textures
void ConvertIA8(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);

#ifdef _DEBUG
	if( ((DWORD)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	if (!pTexture->StartUpdate(&dInfo))
		return;


	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;
			// Points to current byte
			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];
				BYTE I = FourToEight[(b & 0xf0)>>4];

				*pDst++ = I;
				*pDst++ = I;
				*pDst++ = I;
				*pDst++ = FourToEight[(b & 0x0f)   ];

				dwByteOffset++;
			}
		}		
	}
	else
	{
		register const BYTE* FourToEightArray = &FourToEight[0];
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;


			// Points to current byte
			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				register BYTE b = pSrc[(dwByteOffset++) ^ 0x3];
				BYTE I = *(FourToEightArray+(b>>4));

				*pDst++ = I;
				*pDst++ = I;
				*pDst++ = I;
				*pDst++ = *(FourToEightArray+(b&0xF));
			}
		}
	}	
	
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}

// E.g. camera's clouds, shadows
void ConvertIA16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	WORD * pSrc = (WORD*)(tinfo.pPhysicalAddress);
	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pTexture->StartUpdate(&dInfo))
		return;


	if (tinfo.bSwapped)
	{
		
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;

			if ((y%2) == 0)
				nFiddle = 0x2;
			else
				nFiddle = 0x4 | 0x2;

			// Points to current word
			DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset^nFiddle];

				*pDst++ = (BYTE)(w >> 8);
				*pDst++ = (BYTE)(w >> 8);
				*pDst++ = (BYTE)(w >> 8);
				*pDst++ = (BYTE)(w & 0xFF);

				dwWordOffset += 2;
			}
		}		
	}
	else
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;

			// Points to current word
			DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset^0x2];

				*pDst++ = (BYTE)(w >> 8);
				*pDst++ = (BYTE)(w >> 8);
				*pDst++ = (BYTE)(w >> 8);
				*pDst++ = (BYTE)(w & 0xFF);

				dwWordOffset += 2;
			}
		}		
	}


	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}



// Used by MarioKart
void ConvertI4(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);

#ifdef _DEBUG
	if( ((DWORD)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;

			// Might not work with non-even starting X
			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			// For odd lines, swap words too
			if( !conkerSwapHack || (y&4) == 0 )
			{
				if ((y%2) == 0)
					nFiddle = 0x3;
				else
					nFiddle = 0x7;
			}
			else
			{
				if ((y%2) == 1)
					nFiddle = 0x3;
				else
					nFiddle = 0x7;
			}

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				// Even
				*pDst++ = FourToEight[(b & 0xF0)>>4];	// Other implementations seem to or in (b&0xF0)>>4
				*pDst++ = FourToEight[(b & 0xF0)>>4]; // why?
				*pDst++ = FourToEight[(b & 0xF0)>>4];
				*pDst++ = FourToEight[(b & 0xF0)>>4];	
				// Odd
				*pDst++ = FourToEight[(b & 0x0F)];
				*pDst++ = FourToEight[(b & 0x0F)];
				*pDst++ = FourToEight[(b & 0x0F)];
				*pDst++ = FourToEight[(b & 0x0F)];

				dwByteOffset++;
			}

		}	

		conkerSwapHack = false;
	}
	else
	{

		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;

			// Might not work with non-even starting X
			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				// Even
				*pDst++ = FourToEight[(b & 0xF0)>>4];	// Other implementations seem to or in (b&0xF0)>>4
				*pDst++ = FourToEight[(b & 0xF0)>>4]; // why?
				*pDst++ = FourToEight[(b & 0xF0)>>4];
				*pDst++ = FourToEight[(b & 0xF0)>>4];	
				// Odd
				*pDst++ = FourToEight[(b & 0x0F)];
				*pDst++ = FourToEight[(b & 0x0F)];
				*pDst++ = FourToEight[(b & 0x0F)];
				*pDst++ = FourToEight[(b & 0x0F)];

				dwByteOffset++;
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

// Used by MarioKart
void ConvertI8(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	DWORD pSrc = (DWORD)tinfo.pPhysicalAddress;
	if (!pTexture->StartUpdate(&dInfo))
		return;


	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^nFiddle);

				*pDst++ = b;
				*pDst++ = b;
				*pDst++ = b;
				*pDst++ = b;		// Alpha not 255?

				dwByteOffset++;
			}
		}	
	}
	else
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			BYTE *pDst = (BYTE *)dInfo.lpSurface + y * dInfo.lPitch;

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^0x3);

				*pDst++ = b;
				*pDst++ = b;
				*pDst++ = b;
				*pDst++ = b;		// Alpha not 255?

				dwByteOffset++;
			}
		}	
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}

//*****************************************************************************
// Convert CI4 images. We need to switch on the palette type
//*****************************************************************************
void	ConvertCI4( CTexture * p_texture, const TextureInfo & tinfo )
{
	if ( tinfo.TLutFmt == TLUT_FMT_RGBA16 )
	{
		ConvertCI4_RGBA16( p_texture, tinfo );	
	}
	else if ( tinfo.TLutFmt == TLUT_FMT_IA16 )
	{
		ConvertCI4_IA16( p_texture, tinfo );					
	}
}

//*****************************************************************************
// Convert CI8 images. We need to switch on the palette type
//*****************************************************************************
void	ConvertCI8( CTexture * p_texture, const TextureInfo & tinfo )
{
	if ( tinfo.TLutFmt == TLUT_FMT_RGBA16 )
	{
		ConvertCI8_RGBA16( p_texture, tinfo );	
	}
	else if ( tinfo.TLutFmt == TLUT_FMT_IA16 )
	{
		ConvertCI8_IA16( p_texture, tinfo );					
	}
}

// Used by Starfox intro
void ConvertCI4_RGBA16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
	WORD * pPal = (WORD *)tinfo.PalAddress;
	bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);
	
	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{

		for (DWORD y = 0; y <  tinfo.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			DWORD * pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = Convert555ToRGBA(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = Convert555ToRGBA(pPal[blo^1]);	// Remember palette is in different endian order!

				if( bIgnoreAlpha )
				{
					pDst[0] |= 0xFF000000;
					pDst[1] |= 0xFF000000;
				}

				pDst+=2;

				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  tinfo.HeightToLoad; y++)
		{
			DWORD * pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = Convert555ToRGBA(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = Convert555ToRGBA(pPal[blo^1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					pDst[0] |= 0xFF000000;
					pDst[1] |= 0xFF000000;
				}

				pDst+=2;

				dwByteOffset++;
			}
		}	
	}
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

// Used by Starfox intro
void ConvertCI4_IA16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);

#ifdef _DEBUG
	if( ((DWORD)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif


	WORD * pPal = (WORD *)tinfo.PalAddress;
	bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);

	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{

		for (DWORD y = 0; y <  tinfo.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			DWORD * pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = ConvertIA16ToRGBA(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = ConvertIA16ToRGBA(pPal[blo^1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					pDst[0] |= 0xFF000000;
					pDst[1] |= 0xFF000000;
				}

				pDst+=2;

				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  tinfo.HeightToLoad; y++)
		{
			DWORD * pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = ConvertIA16ToRGBA(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = ConvertIA16ToRGBA(pPal[blo^1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					pDst[0] |= 0xFF000000;
					pDst[1] |= 0xFF000000;
				}

				pDst+=2;

				dwByteOffset++;
			}
		}	
	}
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}




// Used by MarioKart for Cars etc
void ConvertCI8_RGBA16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);

#ifdef _DEBUG
	if( ((DWORD)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	WORD * pPal = (WORD *)tinfo.PalAddress;
	bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);

	if (!pTexture->StartUpdate(&dInfo))
		return;
	
	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			DWORD *pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pDst++ = Convert555ToRGBA(pPal[b^1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					*(pDst-1) |= 0xFF000000;
				}


				dwByteOffset++;
			}
		}	
	

	}
	else
	{

		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			DWORD *pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			LONG dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pDst++ = Convert555ToRGBA(pPal[b^1]);	// Remember palette is in different endian order!
				if( bIgnoreAlpha )
				{
					*(pDst-1) |= 0xFF000000;
				}

				dwByteOffset++;
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}



// Used by MarioKart for Cars etc
void ConvertCI8_IA16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);

#ifdef _DEBUG
	if( ((DWORD)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	WORD * pPal = (WORD *)tinfo.PalAddress;
	bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);

	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			DWORD *pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pDst++ = ConvertIA16ToRGBA(pPal[b^1]);	// Remember palette is in different endian order!
				if( bIgnoreAlpha )
				{
					*(pDst-1) |= 0xFF000000;
				}

				dwByteOffset++;
			}
		}	
	}
	else
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			DWORD *pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pDst++ = ConvertIA16ToRGBA(pPal[b^1]);	// Remember palette is in different endian order!
				if( bIgnoreAlpha )
				{
					*(pDst-1) |= 0xFF000000;
				}

				dwByteOffset++;
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

void ConvertYUV(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	if (!pTexture->StartUpdate(&dInfo))
		return;


	DWORD x, y;
	uint32 nFiddle;

	if( options.bUseFullTMEM )
	{
		Tile &tile = gRDP.tiles[tinfo.tileNo];

		WORD * pSrc;
		if( tinfo.tileNo >= 0 )
			pSrc = (WORD*)&g_Tmem.g_Tmem64bit[tile.dwTMem];
		else
			pSrc = (WORD*)(tinfo.pPhysicalAddress);

		BYTE * pByteSrc = (BYTE *)pSrc;

		for (y = 0; y < tinfo.HeightToLoad; y++)
		{
			nFiddle = ( y&1 )? 0x4 : 0;
			int dwWordOffset = tinfo.tileNo>=0? tile.dwLine*8*y : ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);
			DWORD * dwDst = (DWORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

			for (x = 0; x < tinfo.WidthToLoad/2; x++)
			{
				int y0 = *(BYTE*)&pByteSrc[(dwWordOffset+1)^nFiddle];
				int y1 = *(BYTE*)&pByteSrc[(dwWordOffset+3)^nFiddle];
				int u0 = *(BYTE*)&pByteSrc[(dwWordOffset  )^nFiddle];
				int v0 = *(BYTE*)&pByteSrc[(dwWordOffset+2)^nFiddle];

				dwDst[x*2+0] = ConvertYUV16ToR8G8B8(y0,u0,v0);
				dwDst[x*2+1] = ConvertYUV16ToR8G8B8(y1,u0,v0);

				dwWordOffset += 4;
			}
		}
	}
	else
	{
		WORD * pSrc = (WORD*)(tinfo.pPhysicalAddress);
		BYTE * pByteSrc = (BYTE *)pSrc;

		if (tinfo.bSwapped)
		{
			for (y = 0; y < tinfo.HeightToLoad; y++)
			{
				if ((y&1) == 0)
					nFiddle = 0x3;
				else
					nFiddle = 0x7;

				// dwDst points to start of destination row
				DWORD * dwDst = (DWORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

				// DWordOffset points to the current dword we're looking at
				// (process 2 pixels at a time). May be a problem if we don't start on even pixel
				DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

				for (x = 0; x < tinfo.WidthToLoad/2; x++)
				{
					int y0 = *(BYTE*)&pByteSrc[(dwWordOffset  )^nFiddle];
					int v0 = *(BYTE*)&pByteSrc[(dwWordOffset+1)^nFiddle];
					int y1 = *(BYTE*)&pByteSrc[(dwWordOffset+2)^nFiddle];
					int u0 = *(BYTE*)&pByteSrc[(dwWordOffset+3)^nFiddle];

					dwDst[x*2+0] = ConvertYUV16ToR8G8B8(y0,u0,v0);
					dwDst[x*2+1] = ConvertYUV16ToR8G8B8(y1,u0,v0);

					dwWordOffset += 4;
				}
			}
		}
		else
		{
			for (y = 0; y < tinfo.HeightToLoad; y++)
			{
				// dwDst points to start of destination row
				DWORD * dwDst = (DWORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);
				DWORD dwByteOffset = y * 32;

				for (x = 0; x < tinfo.WidthToLoad/2; x++)
				{
					int y0 = *(BYTE*)&pByteSrc[dwByteOffset];
					int v0 = *(BYTE*)&pByteSrc[dwByteOffset+1];
					int y1 = *(BYTE*)&pByteSrc[dwByteOffset+2];
					int u0 = *(BYTE*)&pByteSrc[dwByteOffset+3];

					dwDst[x*2+0] = ConvertYUV16ToR8G8B8(y0,u0,v0);
					dwDst[x*2+1] = ConvertYUV16ToR8G8B8(y1,u0,v0);

					// Increment word offset to point to the next two pixels
					dwByteOffset += 4;
				}
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

DWORD ConvertYUV16ToR8G8B8(int Y, int U, int V)
{
	DWORD A= 1;

	/*
	int R = int(g_convc0 *(Y-16) + g_convc1 * V);
	int G = int(g_convc0 *(Y-16) + g_convc2 * U - g_convc3 * V);
	int B = int(g_convc0 *(Y-16) + g_convc4 * U);
	*/

	int R = (int)(Y + (1.370705f * (V-128)));
	int G = (int)(Y - (0.698001f * (V-128)) - (0.337633f * (U-128)));
	int B = (int)(Y + (1.732446f * (U-128)));

	R = R<0 ? 0 : R;
	G = G<0 ? 0 : G;
	B = B<0 ? 0 : B;

	DWORD R2 = R>255 ? 255 : R;
	DWORD G2 = G>255 ? 255 : G;
	DWORD B2 = B>255 ? 255 : B;

	return COLOR_RGBA(R2, G2, B2, 0xFF*A);
}




// Used by Starfox intro
void Convert4b(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;

	if (!pTexture->StartUpdate(&dInfo))	
		return;

	WORD * pPal = (WORD *)tinfo.PalAddress;
	bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);
	if( tinfo.Format <= TXT_FMT_CI ) bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);

	Tile &tile = gRDP.tiles[tinfo.tileNo];

	BYTE *pByteSrc = tinfo.tileNo >= 0 ? (BYTE*)&g_Tmem.g_Tmem64bit[tile.dwTMem] : (BYTE*)(tinfo.pPhysicalAddress);

	for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
	{
		DWORD * pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

		uint32 nFiddle;
		if( tinfo.tileNo < 0 )	
		{
			if (tinfo.bSwapped)
			{
				if ((y%2) == 0)
					nFiddle = 0x3;
				else
					nFiddle = 0x7;
			}
			else
			{
				nFiddle = 3;
			}
		}
		else
		{
			nFiddle = ( y&1 )? 0x4 : 0;
		}

		int idx = tinfo.tileNo>=0 ? tile.dwLine*8*y : ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

		for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2, idx++)
		{
			BYTE b = pByteSrc[idx^nFiddle];
			BYTE bhi = (b&0xf0)>>4;
			BYTE blo = (b&0x0f);

			if( gRDP.otherMode.text_tlut>=2 || ( tinfo.Format != TXT_FMT_IA && tinfo.Format != TXT_FMT_I) )
			{
				if( tinfo.TLutFmt == TLUT_FMT_IA16 )
				{
					if( tinfo.tileNo>=0 )
					{
						pDst[0] = ConvertIA16ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
						pDst[1] = ConvertIA16ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(blo<<2)]);
					}
					else
					{
						pDst[0] = ConvertIA16ToRGBA(pPal[bhi^1]);
						pDst[1] = ConvertIA16ToRGBA(pPal[blo^1]);
					}
				}
				else
				{
					if( tinfo.tileNo>=0 )
					{
						pDst[0] = Convert555ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
						pDst[1] = Convert555ToRGBA(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(blo<<2)]);
					}
					else
					{
						pDst[0] = Convert555ToRGBA(pPal[bhi^1]);
						pDst[1] = Convert555ToRGBA(pPal[blo^1]);
					}
				}
			}
			else if( tinfo.Format == TXT_FMT_IA )
			{
				BYTE * pByteDst = (BYTE*)pDst;
				pDst[0] = ConvertIA4ToRGBA(b>>4);
				pDst[1] = ConvertIA4ToRGBA(b&0xF);
			}
			else	// if( tinfo.Format == TXT_FMT_I )
			{
				BYTE * pByteDst = (BYTE*)pDst;
				pDst[0] = ConvertI4ToRGBA(b>>4);
				pDst[1] = ConvertI4ToRGBA(b&0xF);
			}

			if( bIgnoreAlpha )
			{
				pDst[0] |= 0xFF000000;
				pDst[1] |= 0xFF000000;
			}
			pDst+=2;
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

void Convert8b(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	if (!pTexture->StartUpdate(&dInfo))	
		return;

	WORD * pPal = (WORD *)tinfo.PalAddress;
	bool bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_UNKNOWN);
	if( tinfo.Format <= TXT_FMT_CI ) bIgnoreAlpha = (tinfo.TLutFmt==TLUT_FMT_NONE);

	Tile &tile = gRDP.tiles[tinfo.tileNo];

	BYTE *pByteSrc;
	if( tinfo.tileNo >= 0 )
	{
		pByteSrc = (BYTE*)&g_Tmem.g_Tmem64bit[tile.dwTMem];
	}
	else
	{
		pByteSrc = (BYTE*)(tinfo.pPhysicalAddress);
	}


	for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
	{
		DWORD * pDst = (DWORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

		uint32 nFiddle;
		if( tinfo.tileNo < 0 )	
		{
			if (tinfo.bSwapped)
			{
				if ((y%2) == 0)
					nFiddle = 0x3;
				else
					nFiddle = 0x7;
			}
			else
			{
				nFiddle = 3;
			}
		}
		else
		{
			nFiddle = ( y&1 )? 0x4 : 0;
		}


		int idx = tinfo.tileNo>=0? tile.dwLine*8*y : ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

		for (DWORD x = 0; x < tinfo.WidthToLoad; x++, idx++)
		{
			BYTE b = pByteSrc[idx^nFiddle];

			if( gRDP.otherMode.text_tlut>=2 || ( tinfo.Format != TXT_FMT_IA && tinfo.Format != TXT_FMT_I) )
			{
				if( tinfo.TLutFmt == TLUT_FMT_IA16 )
				{
					if( tinfo.tileNo>=0 )
						*pDst = ConvertIA16ToRGBA(g_Tmem.g_Tmem16bit[0x400+(b<<2)]);
					else
						*pDst = ConvertIA16ToRGBA(pPal[b^1]);
				}
				else
				{
					if( tinfo.tileNo>=0 )
						*pDst = Convert555ToRGBA(g_Tmem.g_Tmem16bit[0x400+(b<<2)]);
					else
						*pDst = Convert555ToRGBA(pPal[b^1]);
				}
			}
			else if( tinfo.Format == TXT_FMT_IA )
			{
				BYTE I = FourToEight[(b & 0xf0)>>4];
				BYTE * pByteDst = (BYTE*)pDst;
				pByteDst[0] = I;
				pByteDst[1] = I;
				pByteDst[2] = I;
				pByteDst[3] = FourToEight[(b & 0x0f)   ];
			}
			else	// if( tinfo.Format == TXT_FMT_I )
			{
				BYTE * pByteDst = (BYTE*)pDst;
				pByteDst[0] = b;
				pByteDst[1] = b;
				pByteDst[2] = b;
				pByteDst[3] = b;
			}

			if( bIgnoreAlpha )
			{
				*pDst |= 0xFF000000;
			}
			pDst++;
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}


void Convert16b(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	if (!pTexture->StartUpdate(&dInfo))	
		return;

	Tile &tile = gRDP.tiles[tinfo.tileNo];

	WORD *pWordSrc;
	if( tinfo.tileNo >= 0 )
		pWordSrc = (WORD*)&g_Tmem.g_Tmem64bit[tile.dwTMem];
	else
		pWordSrc = (WORD*)(tinfo.pPhysicalAddress);


	for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
	{
		DWORD * dwDst = (DWORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

		uint32 nFiddle;
		if( tinfo.tileNo < 0 )	
		{
			if (tinfo.bSwapped)
			{
				if ((y&1) == 0)
					nFiddle = 0x1;
				else
					nFiddle = 0x3;
			}
			else
			{
				nFiddle = 0x1;
			}
		}
		else
		{
			nFiddle = ( y&1 )? 0x2 : 0;
		}


		int idx = tinfo.tileNo>=0? tile.dwLine*4*y : (((y+tinfo.TopToLoad) * tinfo.Pitch)>>1) + tinfo.LeftToLoad;

		for (DWORD x = 0; x < tinfo.WidthToLoad; x++, idx++)
		{
			WORD w = pWordSrc[idx^nFiddle];
			WORD w2 = tinfo.tileNo>=0? ((w>>8)|(w<<8)) : w;

			if( tinfo.Format == TXT_FMT_RGBA )
			{
				dwDst[x] = Convert555ToRGBA(w2);
			}
			else if( tinfo.Format == TXT_FMT_YUV )
			{
			}
			else if( tinfo.Format >= TXT_FMT_IA )
			{
				BYTE * pByteDst = (BYTE*)&dwDst[x];
				*pByteDst++ = (BYTE)(w2 >> 8);
				*pByteDst++ = (BYTE)(w2 >> 8);
				*pByteDst++ = (BYTE)(w2 >> 8);
				*pByteDst++ = (BYTE)(w2 & 0xFF);
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}