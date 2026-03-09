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

// Still to be swapped:
// IA16

ConvertFunction		gConvertFunctions_16_FullTMEM[ 8 ][ 4 ] = 
{
	// 4bpp				8bpp			16bpp				32bpp
	{  Convert4b_16,	Convert8b_16,	Convert16b_16,		ConvertRGBA32_16 },		// RGBA
	{  NULL,			NULL,			ConvertYUV_16,		NULL },					// YUV
	{  Convert4b_16,	Convert8b_16,	NULL,				NULL },					// CI
	{  Convert4b_16,	Convert8b_16,	Convert16b_16,		NULL },					// IA
	{  Convert4b_16,	Convert8b_16,	Convert16b_16,		NULL },					// I
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL }					// ?
};
ConvertFunction		gConvertFunctions_16[ 8 ][ 4 ] = 
{
	// 4bpp				8bpp			16bpp				32bpp
	{  ConvertCI4_16,	ConvertCI8_16,	ConvertRGBA16_16,	ConvertRGBA32_16 },		// RGBA
	{  NULL,			NULL,			ConvertYUV_16,		NULL },					// YUV
	{  ConvertCI4_16,	ConvertCI8_16,	NULL,				NULL },					// CI
	{  ConvertIA4_16,	ConvertIA8_16,	ConvertIA16_16,		NULL },					// IA
	{  ConvertI4_16,	ConvertI8_16,	ConvertRGBA16_16,	NULL },					// I
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL }					// ?
};

ConvertFunction		gConvertTlutFunctions_16[ 8 ][ 4 ] = 
{
	// 4bpp				8bpp			16bpp				32bpp
	{  ConvertCI4_16,	ConvertCI8_16,	ConvertRGBA16_16,	ConvertRGBA32_16 },		// RGBA
	{  NULL,			NULL,			ConvertYUV_16,		NULL },					// YUV
	{  ConvertCI4_16,	ConvertCI8_16,	NULL,				NULL },					// CI
	{  ConvertCI4_16,	ConvertCI8_16,	ConvertIA16_16,		NULL },					// IA
	{  ConvertCI4_16,	ConvertCI8_16,	ConvertRGBA16_16,	NULL },					// I
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL },					// ?
	{  NULL,			NULL,			NULL,				NULL }					// ?
};

extern bool conkerSwapHack;

void ConvertRGBA16_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	DWORD x, y;
	uint32 nFiddle;

	// Copy of the base pointer
	WORD * pSrc = (WORD*)(tinfo.pPhysicalAddress);
	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{

		for (y = 0; y < tinfo.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x2;
			else
				nFiddle = 0x2 | 0x4;

			// dwDst points to start of destination row
			WORD * wDst = (WORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

			for (x = 0; x < tinfo.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset ^ nFiddle];

				wDst[x] = Convert555ToR4G4B4A4(w);
				
				// Increment word offset to point to the next two pixels
				dwWordOffset += 2;
			}
		}
	}
	else
	{
		for (y = 0; y < tinfo.HeightToLoad; y++)
		{
			// dwDst points to start of destination row
			WORD * wDst = (WORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

			for (x = 0; x < tinfo.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset ^ 0x2];

				wDst[x] = Convert555ToR4G4B4A4(w);
				
				// Increment word offset to point to the next two pixels
				dwWordOffset += 2;
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

void ConvertRGBA32_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	DWORD * pSrc = (DWORD*)(tinfo.pPhysicalAddress);
	if (!pTexture->StartUpdate(&dInfo))
		return;

	if( options.bUseFullTMEM )
	{
		Tile &tile = gRDP.tiles[tinfo.tileNo];

		DWORD *pWordSrc;
		if( tinfo.tileNo >= 0 )
		{
			pWordSrc = (DWORD*)&g_Tmem.g_Tmem64bit[tile.dwTMem];


			for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
			{
				WORD * dwDst = (WORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

				uint32 nFiddle = ( y&1 )? 0x2 : 0;
				int idx = tile.dwLine*4*y;

				for (DWORD x = 0; x < tinfo.WidthToLoad; x++, idx++)
				{
					DWORD w = pWordSrc[idx^nFiddle];
					BYTE* psw = (BYTE*)&w;
					dwDst[x] = R4G4B4A4_MAKE( (psw[0]>>4), (psw[1]>>4), (psw[2]>>4), (psw[3]>>4));
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

					WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);
					BYTE *pS = (BYTE *)pSrc + (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);

					for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
					{

						*pDst++ = R4G4B4A4_MAKE((pS[3]>>4),		// Red
							(pS[2]>>4),
							(pS[1]>>4),
							(pS[0]>>4));		// Alpha
						pS+=4;
					}
				}
				else
				{

					WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);
					BYTE *pS = (BYTE *)pSrc + (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);
					LONG n;

					n = 0;
					for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
					{
						*pDst++ = R4G4B4A4_MAKE((pS[(n^0x8) + 3]>>4),		// Red
							(pS[(n^0x8) + 2]>>4),
							(pS[(n^0x8) + 1]>>4),
							(pS[(n^0x8) + 0]>>4));	// Alpha

						n += 4;
					}
				}
			}
		}
		else
		{
			for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
			{
				WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);
				BYTE *pS = (BYTE *)pSrc + (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad*4);

				for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
				{
					*pDst++ = R4G4B4A4_MAKE((pS[3]>>4),		// Red
						(pS[2]>>4),
						(pS[1]>>4),
						(pS[0]>>4));		// Alpha
					pS+=4;
				}
			}

		}
	}



	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}

// E.g. Dear Mario text
// Copy, Score etc
void ConvertIA4_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

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
				*pDst++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  OneToFour[(b & 0x10) >> 4]);
	
				// Odd
				*pDst++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  OneToFour[(b & 0x01)]     );

				dwByteOffset++;

			}

		}
	}
	else
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			// This may not work if X is not even?
			DWORD dwByteOffset = (y+tinfo.TopToLoad) * tinfo.Pitch + (tinfo.LeftToLoad/2);

			// Do two pixels at a time
			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				// Even
				*pDst++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  OneToFour[(b & 0x10) >> 4]);
	
				// Odd
				*pDst++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  OneToFour[(b & 0x01)]     );


				dwByteOffset++;

			}
		}	
	}
	
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}

// E.g Mario's head textures
void ConvertIA8_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
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


			WORD *pDst = (WORD *)((BYTE*)dInfo.lpSurface + y * dInfo.lPitch);
			// Points to current byte
			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pDst++ = R4G4B4A4_MAKE( ((b&0xf0)>>4),((b&0xf0)>>4),((b&0xf0)>>4),(b&0x0f));

				dwByteOffset++;
			}

		}		
	}
	else
	{

		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);


			// Points to current byte
			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pDst++ = R4G4B4A4_MAKE(((b&0xf0)>>4),((b&0xf0)>>4),((b&0xf0)>>4),(b&0x0f));

				dwByteOffset++;
			}
		}
	}	
	
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}

// E.g. camera's clouds, shadows
void ConvertIA16_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;

	WORD * pSrc = (WORD*)(tinfo.pPhysicalAddress);
	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pTexture->StartUpdate(&dInfo))
		return;
		
	for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
	{
		WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

		// Points to current word
		DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

		for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
		{
			WORD w = *(WORD *)&pByteSrc[dwWordOffset^0x2];

			BYTE i = (BYTE)(w >> 12);
			BYTE a = (BYTE)(w & 0xFF);

			*pDst++ = R4G4B4A4_MAKE(i, i, i, (a>>4));

			dwWordOffset += 2;
		}
	}		
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}



// Used by MarioKart 
void ConvertI4_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
	if (!pTexture->StartUpdate(&dInfo))
		return;

	if (tinfo.bSwapped)
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

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
				//*pDst++ = R4G4B4A4_MAKE(b>>4, b>>4, b>>4, b>>4);
				*pDst++ = FourToSixteen[(b & 0xF0)>>4];
				// Odd
				//*pDst++ = R4G4B4A4_MAKE(b & 0x0f, b & 0x0f, b & 0x0f, b & 0x0f);
				*pDst++ = FourToSixteen[b & 0x0f];

				dwByteOffset++;
			}

		}	

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
				//*pDst++ = R4G4B4A4_MAKE(b>>4, b>>4, b>>4, b>>4);
				*pDst++ = FourToEight[(b & 0xF0)>>4];

				// Odd
				//*pDst++ = R4G4B4A4_MAKE(b & 0x0f, b & 0x0f, b & 0x0f, b & 0x0f);
				*pDst++ = FourToEight[b & 0x0f];

				dwByteOffset++;
			}
		}
	}
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

// Used by MarioKart
void ConvertI8_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	DWORD pSrc = (DWORD)(tinfo.pPhysicalAddress);
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

			WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^nFiddle);

				*pDst++ = R4G4B4A4_MAKE(b>>4,
					b>>4,
					b>>4,
					b>>4);

				dwByteOffset++;
			}
		}	
	}
	else
	{
		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			WORD *pDst = (WORD*)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;

			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^0x3);

				*pDst++ = R4G4B4A4_MAKE(b>>4,
									  b>>4,
									  b>>4,
									  b>>4);

				dwByteOffset++;
			}
		}	

	}
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}


// Used by Starfox intro
void ConvertCI4_RGBA16_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
	WORD * pPal = (WORD *)tinfo.PalAddress;
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


			WORD * pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = Convert555ToR4G4B4A4(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = Convert555ToR4G4B4A4(pPal[blo^1]);	// Remember palette is in different endian order!
				pDst+=2;

				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  tinfo.HeightToLoad; y++)
		{
			WORD * pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = Convert555ToR4G4B4A4(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = Convert555ToR4G4B4A4(pPal[blo^1]);	// Remember palette is in different endian order!
				pDst+=2;

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
void	ConvertCI4_16( CTexture * p_texture, const TextureInfo & tinfo )
{
	if ( tinfo.TLutFmt == TLUT_FMT_RGBA16 )
	{
		ConvertCI4_RGBA16_16( p_texture, tinfo );	
	}
	else if ( tinfo.TLutFmt == TLUT_FMT_IA16 )
	{
		ConvertCI4_IA16_16( p_texture, tinfo );					
	}
}

//*****************************************************************************
// Convert CI8 images. We need to switch on the palette type
//*****************************************************************************
void	ConvertCI8_16( CTexture * p_texture, const TextureInfo & tinfo )
{
	if ( tinfo.TLutFmt == TLUT_FMT_RGBA16 )
	{
		ConvertCI8_RGBA16_16( p_texture, tinfo );	
	}
	else if ( tinfo.TLutFmt == TLUT_FMT_IA16 )
	{
		ConvertCI8_IA16_16( p_texture, tinfo );					
	}
}

// Used by Starfox intro
void ConvertCI4_IA16_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
	WORD * pPal = (WORD *)tinfo.PalAddress;
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


			WORD * pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = ConvertIA16ToR4G4B4A4(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = ConvertIA16ToR4G4B4A4(pPal[blo^1]);	// Remember palette is in different endian order!
				pDst += 2;
				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  tinfo.HeightToLoad; y++)
		{
			WORD * pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad / 2);

			for (DWORD x = 0; x < tinfo.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pDst[0] = ConvertIA16ToR4G4B4A4(pPal[bhi^1]);	// Remember palette is in different endian order!
				pDst[1] = ConvertIA16ToR4G4B4A4(pPal[blo^1]);	// Remember palette is in different endian order!
				pDst+=2;

				dwByteOffset++;
			}
		}	

	}
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}




// Used by MarioKart for Cars etc
void ConvertCI8_RGBA16_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
	WORD * pPal = (WORD *)tinfo.PalAddress;
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

			WORD *pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pDst++ = Convert555ToR4G4B4A4(pPal[b^1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}	
	

	}
	else
	{

		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			WORD *pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pDst++ = Convert555ToR4G4B4A4(pPal[b^1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}
	}
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();

}


// Used by MarioKart for Cars etc
void ConvertCI8_IA16_16(CTexture *pTexture, const TextureInfo &tinfo)
{
	DrawInfo dInfo;
	uint32 nFiddle;

	BYTE * pSrc = (BYTE*)(tinfo.pPhysicalAddress);
	WORD * pPal = (WORD *)tinfo.PalAddress;
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

			WORD *pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pDst++ = ConvertIA16ToR4G4B4A4(pPal[b^1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}	
	

	}
	else
	{

		for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
		{
			WORD *pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

			DWORD dwByteOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + tinfo.LeftToLoad;
			
			for (DWORD x = 0; x < tinfo.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pDst++ = ConvertIA16ToR4G4B4A4(pPal[b^1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}
	}
	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}


void ConvertYUV_16(CTexture *pTexture, const TextureInfo &tinfo)
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
			WORD * wDst = (WORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

			for (x = 0; x < tinfo.WidthToLoad/2; x++)
			{
				int y0 = *(BYTE*)&pByteSrc[(dwWordOffset+1)^nFiddle];
				int y1 = *(BYTE*)&pByteSrc[(dwWordOffset+3)^nFiddle];
				int u0 = *(BYTE*)&pByteSrc[(dwWordOffset  )^nFiddle];
				int v0 = *(BYTE*)&pByteSrc[(dwWordOffset+2)^nFiddle];

				wDst[x*2+0] = ConvertYUV16ToR4G4B4(y0,u0,v0);
				wDst[x*2+1] = ConvertYUV16ToR4G4B4(y1,u0,v0);

				dwWordOffset += 4;
			}
		}
	}
	else
	{
		// Copy of the base pointer
		WORD * pSrc = (WORD*)(tinfo.pPhysicalAddress);
		BYTE * pByteSrc = (BYTE *)pSrc;


		if (tinfo.bSwapped)
		{

			for (y = 0; y < tinfo.HeightToLoad; y++)
			{
				if ((y%2) == 0)
					nFiddle = 0x2;
				else
					nFiddle = 0x2 | 0x4;

				// dwDst points to start of destination row
				WORD * wDst = (WORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

				// DWordOffset points to the current dword we're looking at
				// (process 2 pixels at a time). May be a problem if we don't start on even pixel
				DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

				for (x = 0; x < tinfo.WidthToLoad/2; x++)
				{
					DWORD y0 = *(BYTE*)&pByteSrc[(dwWordOffset+1)^nFiddle];
					DWORD y1 = *(BYTE*)&pByteSrc[(dwWordOffset+3)^nFiddle];
					DWORD u0 = *(BYTE*)&pByteSrc[(dwWordOffset  )^nFiddle];
					DWORD v0 = *(BYTE*)&pByteSrc[(dwWordOffset+2)^nFiddle];

					wDst[x*2+0] = ConvertYUV16ToR4G4B4(y0,u0,v0);
					wDst[x*2+1] = ConvertYUV16ToR4G4B4(y1,u0,v0);

					dwWordOffset += 4;
				}
			}
		}
		else
		{
			for (y = 0; y < tinfo.HeightToLoad; y++)
			{
				// dwDst points to start of destination row
				WORD * wDst = (WORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

				// DWordOffset points to the current dword we're looking at
				// (process 2 pixels at a time). May be a problem if we don't start on even pixel
				DWORD dwWordOffset = ((y+tinfo.TopToLoad) * tinfo.Pitch) + (tinfo.LeftToLoad * 2);

				for (x = 0; x < tinfo.WidthToLoad/2; x++)
				{
					DWORD y0 = *(BYTE*)&pByteSrc[(dwWordOffset+1)^3];
					DWORD y1 = *(BYTE*)&pByteSrc[(dwWordOffset+3)^3];
					DWORD u0 = *(BYTE*)&pByteSrc[(dwWordOffset  )^3];
					DWORD v0 = *(BYTE*)&pByteSrc[(dwWordOffset+2)^3];

					wDst[x*2+0] = ConvertYUV16ToR4G4B4(y0,u0,v0);
					wDst[x*2+1] = ConvertYUV16ToR4G4B4(y1,u0,v0);

					dwWordOffset += 4;
				}
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

WORD ConvertYUV16ToR4G4B4(int Y, int U, int V)
{
	DWORD A=1;
	DWORD R1 = Y + g_convk0 * V;
	DWORD G1 = Y + g_convk1 * U + g_convk2 * V;
	DWORD B1 = Y + g_convk3 * U;
	DWORD R = (R1 - g_convk4) * g_convk5 + R1;
	DWORD G = (G1 - g_convk4) * g_convk5 + G1;
	DWORD B = (B1 - g_convk4) * g_convk5 + B1;
	return (WORD)R4G4B4A4_MAKE((R>>4), (G>>4), (B>>4), 0xF*A);
}




// Used by Starfox intro
void Convert4b_16(CTexture *pTexture, const TextureInfo &tinfo)
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
		WORD * pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

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
						pDst[0] = ConvertIA16ToR4G4B4A4(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
						pDst[1] = ConvertIA16ToR4G4B4A4(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(blo<<2)]);
					}
					else
					{
						pDst[0] = ConvertIA16ToR4G4B4A4(pPal[bhi^1]);
						pDst[1] = ConvertIA16ToR4G4B4A4(pPal[blo^1]);
					}
				}
				else
				{
					if( tinfo.tileNo>=0 )
					{
						pDst[0] = Convert555ToR4G4B4A4(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(bhi<<2)]);
						pDst[1] = Convert555ToR4G4B4A4(g_Tmem.g_Tmem16bit[0x400+tinfo.Palette*0x40+(blo<<2)]);
					}
					else
					{
						pDst[0] = Convert555ToR4G4B4A4(pPal[bhi^1]);
						pDst[1] = Convert555ToR4G4B4A4(pPal[blo^1]);
					}
				}
			}
			else if( tinfo.Format == TXT_FMT_IA )
			{
				pDst[0] = ConvertIA4ToR4G4B4A4(b>>4);
				pDst[1] = ConvertIA4ToR4G4B4A4(b&0xF);
			}
			else //if( tinfo.Format == TXT_FMT_I )
			{
				pDst[0] = ConvertI4ToR4G4B4A4(b>>4);
				pDst[1] = ConvertI4ToR4G4B4A4(b&0xF);
			}

			if( bIgnoreAlpha )
			{
				pDst[0] |= 0xF000;
				pDst[1] |= 0xF000;
			}
			pDst+=2;
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}

void Convert8b_16(CTexture *pTexture, const TextureInfo &tinfo)
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
		pByteSrc = (BYTE*)&g_Tmem.g_Tmem64bit[tile.dwTMem];
	else
		pByteSrc = (BYTE*)(tinfo.pPhysicalAddress);


	for (DWORD y = 0; y < tinfo.HeightToLoad; y++)
	{
		WORD * pDst = (WORD *)((BYTE *)dInfo.lpSurface + y * dInfo.lPitch);

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
						*pDst = ConvertIA16ToR4G4B4A4(g_Tmem.g_Tmem16bit[0x400+(b<<2)]);
					else
						*pDst = ConvertIA16ToR4G4B4A4(pPal[b^1]);
				}
				else
				{
					if( tinfo.tileNo>=0 )
						*pDst = Convert555ToR4G4B4A4(g_Tmem.g_Tmem16bit[0x400+(b<<2)]);
					else
						*pDst = Convert555ToR4G4B4A4(pPal[b^1]);
				}
			}
			else if( tinfo.Format == TXT_FMT_IA )
			{
				*pDst = R4G4B4A4_MAKE( ((b&0xf0)>>4),((b&0xf0)>>4),((b&0xf0)>>4),(b&0x0f));
			}
			else //if( tinfo.Format == TXT_FMT_I )
			{
				*pDst = R4G4B4A4_MAKE(b>>4, b>>4, b>>4, b>>4);
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


void Convert16b_16(CTexture *pTexture, const TextureInfo &tinfo)
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
		WORD * dwDst = (WORD *)((BYTE *)dInfo.lpSurface + y*dInfo.lPitch);

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
				dwDst[x] = Convert555ToR4G4B4A4(w2);
			}
			else if( tinfo.Format == TXT_FMT_YUV )
			{
			}
			else if( tinfo.Format >= TXT_FMT_IA )
			{
				BYTE i = (BYTE)(w2 >> 12);
				BYTE a = (BYTE)(w2 & 0xFF);
				dwDst[x] = R4G4B4A4_MAKE(i, i, i, (a>>4));
			}
		}
	}

	pTexture->EndUpdate(&dInfo);
	pTexture->SetOthersVariables();
}