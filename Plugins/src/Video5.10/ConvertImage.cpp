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


void ConvertRGBA16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	DWORD x, y;
	LONG nFiddle;

	// Copy of the base pointer
	WORD * pSrc = (WORD*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (y = 0; y < ti.HeightToLoad; y++)
		{
			if ((y&1) == 0)
				nFiddle = 0x2;
			else
				nFiddle = 0x2 | 0x4;

			// dwDst points to start of destination row
			DWORD * dwDst = (DWORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToLoad; x++)
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
		for (y = 0; y < ti.HeightToLoad; y++)
		{
			// dwDst points to start of destination row
			DWORD * dwDst = (DWORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset ^ 0x2];

				dwDst[x] = Convert555ToRGBA(w);
				
				// Increment word offset to point to the next two pixels
				dwWordOffset += 2;
			}
		}
	}

	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

void ConvertRGBA32(CTexture *pSurf, const TextureInfo &ti)
{
	DWORD * pSrc = (DWORD*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	DrawInfo dst;
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			if ((y%2) == 0)
			{

				BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;
				BYTE *pS = (BYTE *)pSrc + (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad*4);
				
				for (DWORD x = 0; x < ti.WidthToLoad; x++)
				{
					pD[0] = pS[1];	// Blue
					pD[1] = pS[2];	// Green
					pD[2] = pS[3];	// Red
					pD[3] = pS[0];	// Alpha
					pS+=4;
					pD+=4;
				}
			}
			else
			{

				DWORD *pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);
				BYTE *pS = (BYTE *)pSrc;
				LONG n;
				
				n = (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad*4);
				for (DWORD x = 0; x < ti.WidthToLoad; x++)
				{
					*pD++ = DAEDALUS_COLOR_RGBA(pS[(n+3)^0x8],
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
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;
			BYTE *pS = (BYTE *)pSrc + (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad*4);
			
			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				pD[0] = pS[1];	// Blue
				pD[1] = pS[2];	// Green
				pD[2] = pS[3];	// Red
				pD[3] = pS[0];	// Alpha
				pS+=4;
				pD+=4;
			}
		}

	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}

// E.g. Dear Mario text
// Copy, Score etc
void ConvertIA4(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;

			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			// This may not work if X is not even?
			DWORD dwByteOffset = (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad/2);

			// Do two pixels at a time
			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				/*
				// Even
				*pD++ = TwoToEight[(b & 0xc0) >> 6];
				*pD++ = TwoToEight[(b & 0xc0) >> 6];
				*pD++ = TwoToEight[(b & 0xc0) >> 6];
				*pD++ = TwoToEight[(b & 0x30) >> 4];	
				// Odd
				*pD++ = TwoToEight[(b & 0x0c) >> 2];
				*pD++ = TwoToEight[(b & 0x0c) >> 2];
				*pD++ = TwoToEight[(b & 0x0c) >> 2];
				*pD++ = TwoToEight[(b & 0x03)     ];
				*/
				// Even
				*pD++ = ThreeToEight[(b & 0xE0) >> 5];
				*pD++ = ThreeToEight[(b & 0xE0) >> 5];
				*pD++ = ThreeToEight[(b & 0xE0) >> 5];
				*pD++ = OneToEight[(b & 0x10) >> 4];	
				// Odd
				*pD++ = ThreeToEight[(b & 0x0E) >> 1];
				*pD++ = ThreeToEight[(b & 0x0E) >> 1];
				*pD++ = ThreeToEight[(b & 0x0E) >> 1];
				*pD++ = OneToEight[(b & 0x01)     ];

				dwByteOffset++;

			}

		}
	}
	else
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + (y * dst.lPitch);

			// This may not work if X is not even?
			DWORD dwByteOffset = (y+ti.TopToLoad) * ti.Pitch + (ti.LeftToLoad/2);

			// Do two pixels at a time
			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				/*
				// Even
				*pD++ = TwoToEight[(b & 0xc0) >> 6];
				*pD++ = TwoToEight[(b & 0xc0) >> 6];
				*pD++ = TwoToEight[(b & 0xc0) >> 6];
				*pD++ = TwoToEight[(b & 0x30) >> 4];	
				// Odd
				*pD++ = TwoToEight[(b & 0x0c) >> 2];
				*pD++ = TwoToEight[(b & 0x0c) >> 2];
				*pD++ = TwoToEight[(b & 0x0c) >> 2];
				*pD++ = TwoToEight[(b & 0x03)     ];
				*/

				// Even
				*pD++ = ThreeToEight[(b & 0xE0) >> 5];
				*pD++ = ThreeToEight[(b & 0xE0) >> 5];
				*pD++ = ThreeToEight[(b & 0xE0) >> 5];
				*pD++ = OneToEight[(b & 0x10) >> 4];	
				// Odd
				*pD++ = ThreeToEight[(b & 0x0E) >> 1];
				*pD++ = ThreeToEight[(b & 0x0E) >> 1];
				*pD++ = ThreeToEight[(b & 0x0E) >> 1];
				*pD++ = OneToEight[(b & 0x01)     ];

				dwByteOffset++;

			}
		}	
	}
	
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}

// E.g Mario's head textures
void ConvertIA8(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;
			// Points to current byte
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];
				BYTE I = FourToEight[(b & 0xf0)>>4];

				*pD++ = I;
				*pD++ = I;
				*pD++ = I;
				*pD++ = FourToEight[(b & 0x0f)   ];

				dwByteOffset++;
				}

		}		
	}
	else
	{
		register const BYTE* FourToEightArray = &FourToEight[0];
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;


			// Points to current byte
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				register BYTE b = pSrc[(dwByteOffset++) ^ 0x3];
				BYTE I = *(FourToEightArray+(b>>4));

				*pD++ = I;
				*pD++ = I;
				*pD++ = I;
				*pD++ = *(FourToEightArray+(b&0xF));
			}
		}
	}	
	
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}

// E.g. camera's clouds, shadows
void ConvertIA16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	WORD * pSrc = (WORD*)(ti.pPhysicalAddress);
	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;

			if ((y%2) == 0)
				nFiddle = 0x2;
			else
				nFiddle = 0x4 | 0x2;

			// Points to current word
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset^nFiddle];

				*pD++ = (BYTE)(w >> 8);
				*pD++ = (BYTE)(w >> 8);
				*pD++ = (BYTE)(w >> 8);
				*pD++ = (BYTE)(w & 0xFF);

				dwWordOffset += 2;
			}
		}		
	}
	else
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;

			// Points to current word
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset^0x2];

				*pD++ = (BYTE)(w >> 8);
				*pD++ = (BYTE)(w >> 8);
				*pD++ = (BYTE)(w >> 8);
				*pD++ = (BYTE)(w & 0xFF);

				dwWordOffset += 2;
			}
		}		

	}


	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}



// Used by MarioKart
void ConvertI4(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;

			// Might not work with non-even starting X
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			// For odd lines, swap words too
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				// Even
				*pD++ = FourToEight[(b & 0xF0)>>4];	// Other implementations seem to or in (b&0xF0)>>4
				*pD++ = FourToEight[(b & 0xF0)>>4]; // why?
				*pD++ = FourToEight[(b & 0xF0)>>4];
				*pD++ = FourToEight[(b & 0xF0)>>4];	
				// Odd
				*pD++ = FourToEight[(b & 0x0F)];
				*pD++ = FourToEight[(b & 0x0F)];
				*pD++ = FourToEight[(b & 0x0F)];
				*pD++ = FourToEight[(b & 0x0F)];

				dwByteOffset++;
			}

		}	

	}
	else
	{

		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;

			// Might not work with non-even starting X
			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				// Even
				*pD++ = FourToEight[(b & 0xF0)>>4];	// Other implementations seem to or in (b&0xF0)>>4
				*pD++ = FourToEight[(b & 0xF0)>>4]; // why?
				*pD++ = FourToEight[(b & 0xF0)>>4];
				*pD++ = FourToEight[(b & 0xF0)>>4];	
				// Odd
				*pD++ = FourToEight[(b & 0x0F)];
				*pD++ = FourToEight[(b & 0x0F)];
				*pD++ = FourToEight[(b & 0x0F)];
				*pD++ = FourToEight[(b & 0x0F)];

				dwByteOffset++;
			}
		}
	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

// Used by MarioKart
void ConvertI8(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	u32 pSrc = (u32)ti.pPhysicalAddress;
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^nFiddle);

				*pD++ = b;
				*pD++ = b;
				*pD++ = b;
				*pD++ = b;		// Alpha not 255?

				dwByteOffset++;
			}
		}	
	}
	else
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			BYTE *pD = (BYTE *)dst.lpSurface + y * dst.lPitch;

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;

			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				BYTE b = *(BYTE*)((pSrc+dwByteOffset)^0x3);

				*pD++ = b;
				*pD++ = b;
				*pD++ = b;
				*pD++ = b;		// Alpha not 255?

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
void	ConvertCI4( CTexture * p_texture, const TextureInfo & ti )
{
	if ( ti.TLutFmt == G_TT_RGBA16 )
	{
		ConvertCI4_RGBA16( p_texture, ti );	
	}
	else if ( ti.TLutFmt == G_TT_IA16 )
	{
		ConvertCI4_IA16( p_texture, ti );					
	}
}

//*****************************************************************************
// Convert CI8 images. We need to switch on the palette type
//*****************************************************************************
void	ConvertCI8( CTexture * p_texture, const TextureInfo & ti )
{
	if ( ti.TLutFmt == G_TT_RGBA16 )
	{
		ConvertCI8_RGBA16( p_texture, ti );	
	}
	else if ( ti.TLutFmt == G_TT_IA16 )
	{
		ConvertCI8_IA16( p_texture, ti );					
	}
}

// Used by Starfox intro
void ConvertCI4_RGBA16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);
	WORD * pPal = (WORD *)ti.PalAddress;
	bool bIgnoreAlpha = (ti.TLutFmt==G_TT_NONE);
	
	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (DWORD y = 0; y <  ti.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			DWORD * pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch);

			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = Convert555ToRGBA(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = Convert555ToRGBA(pPal[blo^0x1]);	// Remember palette is in different endian order!

				if( bIgnoreAlpha )
				{
					pD[0] |= 0xFF000000;
					pD[1] |= 0xFF000000;
				}

				pD+=2;

				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  ti.HeightToLoad; y++)
		{
			DWORD * pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = Convert555ToRGBA(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = Convert555ToRGBA(pPal[blo^0x1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					pD[0] |= 0xFF000000;
					pD[1] |= 0xFF000000;
				}

				pD+=2;

				dwByteOffset++;
			}
		}	

	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

// Used by Starfox intro
void ConvertCI4_IA16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif


	WORD * pPal = (WORD *)ti.PalAddress;
	bool bIgnoreAlpha = (ti.TLutFmt==G_TT_UNKNOWN);

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{

		for (DWORD y = 0; y <  ti.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;


			DWORD * pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = ConvertIA16ToRGBA(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = ConvertIA16ToRGBA(pPal[blo^0x1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					pD[0] |= 0xFF000000;
					pD[1] |= 0xFF000000;
				}

				pD+=2;

				dwByteOffset++;
			}
		}	
		
	}
	else
	{

		for (DWORD y = 0; y <  ti.HeightToLoad; y++)
		{
			DWORD * pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad / 2);

			for (DWORD x = 0; x < ti.WidthToLoad; x+=2)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				BYTE bhi = (b&0xf0)>>4;
				BYTE blo = (b&0x0f);

				pD[0] = ConvertIA16ToRGBA(pPal[bhi^0x1]);	// Remember palette is in different endian order!
				pD[1] = ConvertIA16ToRGBA(pPal[blo^0x1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					pD[0] |= 0xFF000000;
					pD[1] |= 0xFF000000;
				}

				pD+=2;

				dwByteOffset++;
			}
		}	

	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}




// Used by MarioKart for Cars etc
void ConvertCI8_RGBA16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	WORD * pPal = (WORD *)ti.PalAddress;
	bool bIgnoreAlpha = (ti.TLutFmt==G_TT_NONE);

	if (!pSurf->StartUpdate(&dst))
		return;
	
	if (ti.bSwapped)
	{


		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			DWORD *pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pD++ = Convert555ToRGBA(pPal[b^0x1]);	// Remember palette is in different endian order!
				
				if( bIgnoreAlpha )
				{
					*(pD-1) |= 0xFF000000;
				}


				dwByteOffset++;
			}
		}	
	

	}
	else
	{

		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			DWORD *pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			LONG dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pD++ = Convert555ToRGBA(pPal[b^0x1]);	// Remember palette is in different endian order!
				if( bIgnoreAlpha )
				{
					*(pD-1) |= 0xFF000000;
				}

				dwByteOffset++;
			}
		}
	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();

}



// Used by MarioKart for Cars etc
void ConvertCI8_IA16(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	LONG nFiddle;

	BYTE * pSrc = (BYTE*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	WORD * pPal = (WORD *)ti.PalAddress;
	bool bIgnoreAlpha = (ti.TLutFmt==G_TT_UNKNOWN);

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			if ((y%2) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			DWORD *pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ nFiddle];

				*pD++ = ConvertIA16ToRGBA(pPal[b^0x1]);	// Remember palette is in different endian order!
				if( bIgnoreAlpha )
				{
					*(pD-1) |= 0xFF000000;
				}

				dwByteOffset++;
			}
		}	
	}
	else
	{
		for (DWORD y = 0; y < ti.HeightToLoad; y++)
		{
			DWORD *pD = (DWORD *)((BYTE *)dst.lpSurface + y * dst.lPitch);

			DWORD dwByteOffset = ((y+ti.TopToLoad) * ti.Pitch) + ti.LeftToLoad;
			
			for (DWORD x = 0; x < ti.WidthToLoad; x++)
			{
				BYTE b = pSrc[dwByteOffset ^ 0x3];

				*pD++ = ConvertIA16ToRGBA(pPal[b^0x1]);	// Remember palette is in different endian order!
				if( bIgnoreAlpha )
				{
					*(pD-1) |= 0xFF000000;
				}

				dwByteOffset++;
			}
		}
	}
	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

void ConvertYUV(CTexture *pSurf, const TextureInfo &ti)
{
	DrawInfo dst;
	DWORD x, y;
	LONG nFiddle;

	// Copy of the base pointer
	WORD * pSrc = (WORD*)(ti.pPhysicalAddress);

#ifdef _DEBUG
	if( ((u32)pSrc)%4 )	TRACE0("Texture src addr is not aligned to 4 bytes, check me");
#endif

	BYTE * pByteSrc = (BYTE *)pSrc;

	if (!pSurf->StartUpdate(&dst))
		return;

	if (ti.bSwapped)
	{
		for (y = 0; y < ti.HeightToLoad; y++)
		{
			if ((y&1) == 0)
				nFiddle = 0x3;
			else
				nFiddle = 0x7;

			// dwDst points to start of destination row
			DWORD * dwDst = (DWORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToLoad/2; x++)
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
		/*
		for (y = 0; y < ti.HeightToLoad; y++)
		{
			// dwDst points to start of destination row
			DWORD * dwDst = (DWORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);

			// DWordOffset points to the current dword we're looking at
			// (process 2 pixels at a time). May be a problem if we don't start on even pixel
			DWORD dwWordOffset = ((y+ti.TopToLoad) * ti.Pitch) + (ti.LeftToLoad * 2);

			for (x = 0; x < ti.WidthToLoad; x++)
			{
				WORD w = *(WORD *)&pByteSrc[dwWordOffset ^ 0x2];

				dwDst[x] = ConvertYUV16ToR8G8B8(w);

				// Increment word offset to point to the next two pixels
				dwWordOffset += 2;
			}
		}
		*/

		for (y = 0; y < ti.HeightToLoad; y++)
		{
			// dwDst points to start of destination row
			DWORD * dwDst = (DWORD *)((BYTE *)dst.lpSurface + y*dst.lPitch);
			DWORD dwByteOffset = y * 32;

			for (x = 0; x < ti.WidthToLoad/2; x++)
			{
				DWORD y0 = *(BYTE*)&pByteSrc[(dwByteOffset+1)^3];
				DWORD y1 = *(BYTE*)&pByteSrc[(dwByteOffset+3)^3];
				DWORD u0 = *(BYTE*)&pByteSrc[(dwByteOffset  )^3];
				DWORD v0 = *(BYTE*)&pByteSrc[(dwByteOffset+2)^3];

				//dwDst[x<<1+0] = ConvertYUV16ToR8G8B8(y0,u0,v0);
				dwDst[(x<<1)+1] = DAEDALUS_COLOR_RGBA(y0,y0,y0,0xFF);
				//dwDst[x<<1+1] = ConvertYUV16ToR8G8B8(y1,u0,v0);
				dwDst[(x<<1)+0] = DAEDALUS_COLOR_RGBA(y1,y1,y1,0xFF);

				// Increment word offset to point to the next two pixels
				dwByteOffset += 4;
			}
		}
	}

	pSurf->EndUpdate(&dst);
	pSurf->SetOthersVariables();
}

DWORD ConvertYUV16ToR8G8B8(int Y, int U, int V)
{
	DWORD A= 1;

	int R = int(g_convc0 *(Y-16) + g_convc1 * V);
	int G = int(g_convc0 *(Y-16) + g_convc2 * U - g_convc3 * V);
	int B = int(g_convc0 *(Y-16) + g_convc4 * U);

	R = R<0 ? 0 : R;
	G = G<0 ? 0 : G;
	B = B<0 ? 0 : B;

	DWORD R2 = R>255 ? 255 : R;
	DWORD G2 = G>255 ? 255 : G;
	DWORD B2 = B>255 ? 255 : B;

	//return DAEDALUS_COLOR_RGBA(R, G, V, 0xFF);
	return DAEDALUS_COLOR_RGBA(R2, G2, B2, 0xFF*A);
}
