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

// Still to be swapped:
// IA16

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

void ConvertRGBA16_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	DWORD x, y;
	LONG nFiddle;

	// Copy of the base pointer
	WORD * pSrc = (WORD*)(ti.pPhysicalAddress);
	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (y = 0; y < ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x2;
			else
				nFiddle = 0x2 | 0x4;

			// dwDst points to start of destination row
			WORD * wDst = (WORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToCreate; x++)
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
		for (y = 0; y < ti.HeightToCreate; y++)
		{
			// dwDst points to start of destination row
			WORD * wDst = (WORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToCreate; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset ^ 0x2];

				wDst[x] = Convert555ToR4G4B4A4(w);
				
				// Increment word offset to point to the next two pixels
				dwWordOffset += 2;
			}
		}
	}

	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

void ConvertRGBA32_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	DWORD * pSrc = (DWORD*)(ti.pPhysicalAddress);
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
			{

				WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);
				BYTE *pS = (BYTE *)pSrc + (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad*4);
				
				for (DWORD x = 0; x < ti.WidthToCreate; x++)
				{

					*pD++ = R4G4B4A4_MAKE((pS[3]>>4),		// Red
										  (pS[2]>>4),
										  (pS[1]>>4),
										  (pS[0]>>4));		// Alpha
					pS+=4;
				}
			}
			else
			{

				WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);
				BYTE *pS = (BYTE *)pSrc + (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad*4);
				LONG n;
				
				n = 0;
				for (DWORD x = 0; x < ti.WidthToCreate; x++)
				{
					*pD++ = R4G4B4A4_MAKE((pS[(n^0x8) + 3]>>4),		// Red
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
		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);
			BYTE *pS = (BYTE *)pSrc + (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad*4);
			
			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				*pD++ = R4G4B4A4_MAKE((pS[3]>>4),		// Red
									  (pS[2]>>4),
									  (pS[1]>>4),
									  (pS[0]>>4));		// Alpha
				pS+=4;
			}
		}

	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}

// E.g. Dear Mario text
// Copy, Score etc
void ConvertIA4_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);

			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			// This may not work if X is not even?
			DWORD dwByteOffset = (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad/2);

			// Do two pixels at a time
			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				// Even
				*pD++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  OneToFour[(b & 0x10) >> 4]);
	
				// Odd
				*pD++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  OneToFour[(b & 0x01)]     );

				dwByteOffset++;

			}

		}
	}
	else
	{
		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);

			// This may not work if X is not even?
			DWORD dwByteOffset = (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad/2);

			// Do two pixels at a time
			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				// Even
				*pD++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  ThreeToFour[(b & 0xE0) >> 5],
									  OneToFour[(b & 0x10) >> 4]);
	
				// Odd
				*pD++ = R4G4B4A4_MAKE(ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  ThreeToFour[(b & 0x0E) >> 1],
									  OneToFour[(b & 0x01)]     );


				dwByteOffset++;

			}
		}	
	}
	
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}

// E.g Mario's head textures
void ConvertIA8_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			WORD *pD = (WORD *)((BYTE*)dst.lpSurface + y * dst.lPitch);
			// Points to current byte
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pD++ = R4G4B4A4_MAKE( ((b&0xf0)>>4),((b&0xf0)>>4),((b&0xf0)>>4),(b&0x0f));

				dwByteOffset++;
			}

		}		
	}
	else
	{

		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);


			// Points to current byte
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pD++ = R4G4B4A4_MAKE(((b&0xf0)>>4),((b&0xf0)>>4),((b&0xf0)>>4),(b&0x0f));

				dwByteOffset++;
			}
		}
	}	
	
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}

// E.g. camera's clouds, shadows
void ConvertIA16_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;

	WORD * pSrc = (WORD*)(ti.pPhysicalAddress);
	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pSurf->StartUpdate(&dst))
		return;
		
	for (DWORD y = 0; y < ti.HeightToCreate; y++)
	{
		WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);

		// Points to current word
		DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

		for (DWORD x = 0; x < ti.WidthToCreate; x++)
		{
			WORD w = *(WORD *)&pByteSrc[dwWordOffset^0x2];

			BYTE i = (BYTE)(w >> 12);
			BYTE a = (BYTE)(w & 0xFF);

			*pD++ = R4G4B4A4_MAKE(i, i, i, (a>>4));

			dwWordOffset += 2;
		}
	}		
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}



// Used by MarioKart 
void ConvertI4_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);

			// Might not work with non-even starting X
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle]>>4;

				// Even
				*pD++ = R4G4B4A4_MAKE(b>>4,
									  b>>4,
									  b>>4,
									  b>>4);
				// Odd
				*pD++ = R4G4B4A4_MAKE(b & 0x0f,
									  b & 0x0f,
									  b & 0x0f,
									  b & 0x0f);

				dwByteOffset++;
			}

		}	

	}
	else
	{

		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);

			// Might not work with non-even starting X
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				// Even
				*pD++ = R4G4B4A4_MAKE(b>>4,
									  b>>4,
									  b>>4,
									  b>>4);

				// Odd
				*pD++ = R4G4B4A4_MAKE(b & 0x0f,
									  b & 0x0f,
									  b & 0x0f,
									  b & 0x0f);

				dwByteOffset++;
			}
		}
	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

// Used by MarioKart
void ConvertI8_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	u32 pSrc = (u32)(ti.pPhysicalAddress);
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^nFiddle);

				*pD++ = R4G4B4A4_MAKE(b>>4,
									  b>>4,
									  b>>4,
									  b>>4);

				dwByteOffset++;
			}
		}	
	}
	else
	{
		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD*)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^0x3);

				*pD++ = R4G4B4A4_MAKE(b>>4,
									  b>>4,
									  b>>4,
									  b>>4);

				dwByteOffset++;
			}
		}	

	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}


// Used by Starfox intro
void ConvertCI4_RGBA16_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	WORD * pPal = (WORD *)ti.PalAddress;
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (DWORD y = 0; y <  ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			WORD * pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = Convert555ToR4G4B4A4(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = Convert555ToR4G4B4A4(pPal[blo^0x1]);	// Remember palette is in different endian order!
				pD+=2;

				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  ti.HeightToCreate; y++)
		{
			WORD * pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = Convert555ToR4G4B4A4(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = Convert555ToR4G4B4A4(pPal[blo^0x1]);	// Remember palette is in different endian order!
				pD+=2;

				dwByteOffset++;
			}
		}	

	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

//*****************************************************************************
// Convert CI4 images. We need to switch on the palette type
//*****************************************************************************
void	ConvertCI4_16( CTexture * p_texture, const TextureInfo & ti )
{
	if ( ti.TLutFmt == G_TT_RGBA16 )
	{
		ConvertCI4_RGBA16_16( p_texture, ti );	
	}
	else if ( ti.TLutFmt == G_TT_IA16 )
	{
		ConvertCI4_IA16_16( p_texture, ti );					
	}
}

//*****************************************************************************
// Convert CI8 images. We need to switch on the palette type
//*****************************************************************************
void	ConvertCI8_16( CTexture * p_texture, const TextureInfo & ti )
{
	if ( ti.TLutFmt == G_TT_RGBA16 )
	{
		ConvertCI8_RGBA16_16( p_texture, ti );	
	}
	else if ( ti.TLutFmt == G_TT_IA16 )
	{
		ConvertCI8_IA16_16( p_texture, ti );					
	}
}

// Used by Starfox intro
void ConvertCI4_IA16_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	WORD * pPal = (WORD *)ti.PalAddress;
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (DWORD y = 0; y <  ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			WORD * pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = ConvertIA16ToR4G4B4A4(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = ConvertIA16ToR4G4B4A4(pPal[blo^0x1]);	// Remember palette is in different endian order!
				pD += 2;
				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  ti.HeightToCreate; y++)
		{
			WORD * pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToCreate; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = ConvertIA16ToR4G4B4A4(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = ConvertIA16ToR4G4B4A4(pPal[blo^0x1]);	// Remember palette is in different endian order!
				pD+=2;

				dwByteOffset++;
			}
		}	

	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}




// Used by MarioKart for Cars etc
void ConvertCI8_RGBA16_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	WORD * pPal = (WORD *)ti.PalAddress;
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{


		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			WORD *pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pD++ = Convert555ToR4G4B4A4(pPal[b^0x1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}	
	

	}
	else
	{

		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pD++ = Convert555ToR4G4B4A4(pPal[b^0x1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}
	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}


// Used by MarioKart for Cars etc
void ConvertCI8_IA16_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	WORD * pPal = (WORD *)ti.PalAddress;
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{


		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			WORD *pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pD++ = ConvertIA16ToR4G4B4A4(pPal[b^0x1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}	
	

	}
	else
	{

		for (DWORD y = 0; y < ti.HeightToCreate; y++)
		{
			WORD *pD = (WORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToCreate; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pD++ = ConvertIA16ToR4G4B4A4(pPal[b^0x1]);	// Remember palette is in different endian order!

				dwByteOffset++;
			}
		}
	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}


void ConvertYUV_16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	DWORD x, y;
	LONG nFiddle;

	// Copy of the base pointer
	WORD * pSrc = (WORD*)(ti.pPhysicalAddress);
	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (y = 0; y < ti.HeightToCreate; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x2;
			else
				nFiddle = 0x2 | 0x4;

			// dwDst points to start of destination row
			WORD * wDst = (WORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToLoad/2; x++)
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
		for (y = 0; y < ti.HeightToCreate; y++)
		{
			// dwDst points to start of destination row
			WORD * wDst = (WORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToLoad/2; x++)
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

	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
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
