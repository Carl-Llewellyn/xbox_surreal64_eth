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


#define DWORD_MAKE(r, g, b, a)   ((DWORD) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

void Super2xSaI( DWORD *srcPtr, DWORD *destPtr, DWORD width, DWORD height, DWORD pitch);
void Super2xSaI_16( WORD *srcPtr, WORD *destPtr, DWORD width, DWORD height, DWORD pitch);
void Texture2x( DrawInfo &srcInfo, DrawInfo &destInfo);
void Texture2x_16( DrawInfo &srcInfo, DrawInfo &destInfo);

void SmoothFilter(DWORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter=TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1);
void SmoothFilter_16(WORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter=TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1);
void SharpenFilter(DWORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter=TEXTURE_SHARPEN_ENHANCEMENT);
void SharpenFilter_16(WORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter=TEXTURE_SHARPEN_ENHANCEMENT);



/************************************************************************/
/* 2xSAI filters                                                        */
/************************************************************************/
static __inline int SAI_GetResult1_32( DWORD A, DWORD B, DWORD C, DWORD D, DWORD E )
{
	int x = 0;
	int y = 0;
	int r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r += 1; 
	if (y <= 1) r -= 1;

	return r;
}

static __inline WORD SAI_GetResult1_16( WORD A, WORD B, WORD C, WORD D, WORD E )
{
	WORD x = 0;
	WORD y = 0;
	WORD r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r += 1; 
	if (y <= 1 && r>0) r -= 1;

	return r;
}

static __inline int SAI_GetResult2_32( DWORD A, DWORD B, DWORD C, DWORD D, DWORD E) 
{
	int x = 0; 
	int y = 0;
	int r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r -= 1; 
	if (y <= 1) r += 1;

	return r;
}

static __inline WORD SAI_GetResult2_16( WORD A, WORD B, WORD C, WORD D, WORD E) 
{
	WORD x = 0; 
	WORD y = 0;
	WORD r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1 && r>0 ) r -= 1; 
	if (y <= 1) r += 1;

	return r;
}


static __inline int SAI_GetResult_32( DWORD A, DWORD B, DWORD C, DWORD D )
{
	int x = 0; 
	int y = 0;
	int r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r += 1; 
	if (y <= 1) r -= 1;

	return r;
}

static __inline WORD SAI_GetResult_16( WORD A, WORD B, WORD C, WORD D )
{
	WORD x = 0; 
	WORD y = 0;
	WORD r = 0;

	if (A == C) x += 1; else if (B == C) y += 1;
	if (A == D) x += 1; else if (B == D) y += 1;
	if (x <= 1) r += 1; 
	if (y <= 1 && r>0 ) r -= 1;

	return r;
}


static __inline DWORD SAI_INTERPOLATE_32( DWORD A, DWORD B)
{
	if (A != B)
		return	((A & 0xFEFEFEFE) >> 1) + 
				((B & 0xFEFEFEFE) >> 1) |
				(A & B & 0x01010101);
	else
		return A;
}

static __inline WORD SAI_INTERPOLATE_16( WORD A, WORD B)
{
	if (A != B)
		return	((A & 0xFEFE) >> 1) + 
				((B & 0xFEFE) >> 1) |
				(A & B & 0x0101);
	else
		return A;
}


static __inline DWORD SAI_Q_INTERPOLATE_32( DWORD A, DWORD B, DWORD C, DWORD D)
{
	DWORD x =	((A & 0xFCFCFCFC) >> 2) +
				((B & 0xFCFCFCFC) >> 2) +
				((C & 0xFCFCFCFC) >> 2) +
				((D & 0xFCFCFCFC) >> 2);
	DWORD y =	(((A & 0x03030303) +
				(B & 0x03030303) +
				(C & 0x03030303) +
				(D & 0x03030303)) >> 2) & 0x03030303;
	return x | y;
}

static __inline WORD SAI_Q_INTERPOLATE_16( WORD A, WORD B, WORD C, WORD D)
{
	WORD x =	((A & 0xFCFC) >> 2) +
				((B & 0xFCFC) >> 2) +
				((C & 0xFCFC) >> 2) +
				((D & 0xFCFC) >> 2);
	WORD y =	(((A & 0x0303) +
				(B & 0x0303) +
				(C & 0x0303) +
				(D & 0x0303)) >> 2) & 0x0303;
	return x | y;
}


void Super2xSaI( DWORD *srcPtr, DWORD *destPtr, DWORD width, DWORD height, DWORD pitch)
{
	DWORD destWidth = width << 1;
	DWORD destHeight = height << 1;

    DWORD color4, color5, color6;
    DWORD color1, color2, color3;
    DWORD colorA0, colorA1, colorA2, colorA3;
	DWORD colorB0, colorB1, colorB2, colorB3;
	DWORD colorS1, colorS2;
	DWORD product1a, product1b, product2a, product2b;

	int row0, row1, row2, row3;
	int col0, col1, col2, col3;

	for (WORD y = 0; y < height; y++)
	{
		if (y > 0)
		{
			row0 = width;
			row0 = -row0;
		}
		else
			row0 = 0;

		row1 = 0;

		if (y < height - 1)
		{
			row2 = width;

			if (y < height - 2) 
				row3 = width << 1;
			else
				row3 = width;
		}
		else
		{
			row2 = 0;
			row3 = 0;
		}

        for (WORD x = 0; x < width; x++)
        {
//--------------------------------------- B0 B1 B2 B3
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                        A0 A1 A2 A3
			if (x > 0)
				col0 = -1;
			else
				col0 = 0;

			col1 = 0;

			if (x < width - 1)
			{
				col2 = 1;

				if (x < width - 2) 
					col3 = 2;
				else
					col3 = 1;
			}
			else
			{
				col2 = 0;
				col3 = 0;
			}

	        colorB0 = *(srcPtr + col0 + row0);
            colorB1 = *(srcPtr + col1 + row0);
            colorB2 = *(srcPtr + col2 + row0);
            colorB3 = *(srcPtr + col3 + row0);

            color4 = *(srcPtr + col0 + row1);
            color5 = *(srcPtr + col1 + row1);
            color6 = *(srcPtr + col2 + row1);
            colorS2 = *(srcPtr + col3 + row1);

            color1 = *(srcPtr + col0 + row2);
            color2 = *(srcPtr + col1 + row2);
            color3 = *(srcPtr + col2 + row2);
            colorS1 = *(srcPtr + col3 + row2);

            colorA0 = *(srcPtr + col0 + row3);
            colorA1 = *(srcPtr + col1 + row3);
            colorA2 = *(srcPtr + col2 + row3);
            colorA3 = *(srcPtr + col3 + row3);

//--------------------------------------
            if (color2 == color6 && color5 != color3)
                product2b = product1b = color2;
            else if (color5 == color3 && color2 != color6)
                product2b = product1b = color5;
            else if (color5 == color3 && color2 == color6)
            {
                int r = 0;

                r += SAI_GetResult_32 (color6, color5, color1, colorA1);
                r += SAI_GetResult_32 (color6, color5, color4, colorB1);
                r += SAI_GetResult_32 (color6, color5, colorA2, colorS1);
                r += SAI_GetResult_32 (color6, color5, colorB2, colorS2);

                if (r > 0)
                    product2b = product1b = color6;
                else if (r < 0)
                    product2b = product1b = color5;
                else
                    product2b = product1b = SAI_INTERPOLATE_32 (color5, color6);
            }
            else
            {

                if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                    product2b = SAI_Q_INTERPOLATE_32 (color3, color3, color3, color2);
                else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                    product2b = SAI_Q_INTERPOLATE_32 (color2, color2, color2, color3);
                else
                    product2b = SAI_INTERPOLATE_32 (color2, color3);

                if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                    product1b = SAI_Q_INTERPOLATE_32 (color6, color6, color6, color5);
                else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                    product1b = SAI_Q_INTERPOLATE_32 (color6, color5, color5, color5);
                else
                    product1b = SAI_INTERPOLATE_32 (color5, color6);
            }

            if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                product2a = SAI_INTERPOLATE_32 (color2, color5);
            else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                product2a = SAI_INTERPOLATE_32(color2, color5);
            else
                product2a = color2;

            if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                product1a = SAI_INTERPOLATE_32 (color2, color5);
            else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                product1a = SAI_INTERPOLATE_32(color2, color5);
            else
                product1a = color5;


			destPtr[0] = product1a;
			destPtr[1] = product1b;
			destPtr[destWidth] = product2a;
			destPtr[destWidth + 1] = product2b;

			srcPtr++;
			destPtr += 2;
        }
		srcPtr += (pitch-width);
		destPtr += (((pitch-width)<<1)+(pitch<<1));
	}
}


void Super2xSaI_16( WORD *srcPtr, WORD *destPtr, DWORD width, DWORD height, DWORD pitch)
{
	DWORD destWidth = width << 1;
	DWORD destHeight = height << 1;

    WORD color4, color5, color6;
    WORD color1, color2, color3;
    WORD colorA0, colorA1, colorA2, colorA3;
	WORD colorB0, colorB1, colorB2, colorB3;
	WORD colorS1, colorS2;
	WORD product1a, product1b, product2a, product2b;

	int row0, row1, row2, row3;
	int col0, col1, col2, col3;

	for (WORD y = 0; y < height; y++)
	{
		if (y > 0)
		{
			row0 = width;
			row0 = -row0;
		}
		else
			row0 = 0;

		row1 = 0;

		if (y < height - 1)
		{
			row2 = width;

			if (y < height - 2) 
				row3 = width << 1;
			else
				row3 = width;
		}
		else
		{
			row2 = 0;
			row3 = 0;
		}

        for (WORD x = 0; x < width; x++)
        {
//--------------------------------------- B0 B1 B2 B3
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                        A0 A1 A2 A3
			if (x > 0)
				col0 = -1;
			else
				col0 = 0;

			col1 = 0;

			if (x < width - 1)
			{
				col2 = 1;

				if (x < width - 2) 
					col3 = 2;
				else
					col3 = 1;
			}
			else
			{
				col2 = 0;
				col3 = 0;
			}

	        colorB0 = *(srcPtr + col0 + row0);
            colorB1 = *(srcPtr + col1 + row0);
            colorB2 = *(srcPtr + col2 + row0);
            colorB3 = *(srcPtr + col3 + row0);

            color4 = *(srcPtr + col0 + row1);
            color5 = *(srcPtr + col1 + row1);
            color6 = *(srcPtr + col2 + row1);
            colorS2 = *(srcPtr + col3 + row1);

            color1 = *(srcPtr + col0 + row2);
            color2 = *(srcPtr + col1 + row2);
            color3 = *(srcPtr + col2 + row2);
            colorS1 = *(srcPtr + col3 + row2);

            colorA0 = *(srcPtr + col0 + row3);
            colorA1 = *(srcPtr + col1 + row3);
            colorA2 = *(srcPtr + col2 + row3);
            colorA3 = *(srcPtr + col3 + row3);

//--------------------------------------
            if (color2 == color6 && color5 != color3)
                product2b = product1b = color2;
            else if (color5 == color3 && color2 != color6)
                product2b = product1b = color5;
            else if (color5 == color3 && color2 == color6)
            {
                int r = 0;

                r += SAI_GetResult_16 (color6, color5, color1, colorA1);
                r += SAI_GetResult_16 (color6, color5, color4, colorB1);
                r += SAI_GetResult_16 (color6, color5, colorA2, colorS1);
                r += SAI_GetResult_16 (color6, color5, colorB2, colorS2);

                if (r > 0)
                    product2b = product1b = color6;
                else if (r < 0)
                    product2b = product1b = color5;
                else
                    product2b = product1b = SAI_INTERPOLATE_16 (color5, color6);
            }
            else
            {

                if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                    product2b = SAI_Q_INTERPOLATE_16 (color3, color3, color3, color2);
                else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                    product2b = SAI_Q_INTERPOLATE_16 (color2, color2, color2, color3);
                else
                    product2b = SAI_INTERPOLATE_16 (color2, color3);

                if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                    product1b = SAI_Q_INTERPOLATE_16 (color6, color6, color6, color5);
                else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                    product1b = SAI_Q_INTERPOLATE_16 (color6, color5, color5, color5);
                else
                    product1b = SAI_INTERPOLATE_16 (color5, color6);
            }

            if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                product2a = SAI_INTERPOLATE_16 (color2, color5);
            else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                product2a = SAI_INTERPOLATE_16(color2, color5);
            else
                product2a = color2;

            if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                product1a = SAI_INTERPOLATE_16 (color2, color5);
            else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                product1a = SAI_INTERPOLATE_16(color2, color5);
            else
                product1a = color5;


			destPtr[0] = product1a;
			destPtr[1] = product1b;
			destPtr[destWidth] = product2a;
			destPtr[destWidth + 1] = product2b;

			srcPtr++;
			destPtr += 2;
        }
		srcPtr += (pitch-width);
		destPtr += (((pitch-width)<<1)+(pitch<<1));
	}
}

/************************************************************************/
/* 2X filters                                                           */
/************************************************************************/
void Texture2x( DrawInfo &srcInfo, DrawInfo &destInfo)
{
	DWORD *pDst1, *pDst2;
	DWORD *pSrc, *pSrc2;
	DWORD nWidth = srcInfo.dwWidth;
	DWORD nHeight = srcInfo.dwHeight;

	for (DWORD ySrc = 0; ySrc < nHeight; ySrc++)
	{
		pSrc = (DWORD*)(((BYTE*)srcInfo.lpSurface)+ySrc*srcInfo.lPitch);
		pSrc2 = (DWORD*)(((BYTE*)srcInfo.lpSurface)+(ySrc+1)*srcInfo.lPitch);
		pDst1 = (DWORD*)(((BYTE*)destInfo.lpSurface)+(ySrc*2)*destInfo.lPitch);
		pDst2 = (DWORD*)(((BYTE*)destInfo.lpSurface)+(ySrc*2+1)*destInfo.lPitch);

		for (DWORD xSrc = 0; xSrc < nWidth; xSrc++)
		{
			/*
			pDst1[xSrc*2] = pSrc[xSrc];
			pDst1[xSrc*2+1] = pSrc[xSrc];
			pDst2[xSrc*2] = pSrc[xSrc];
			pDst2[xSrc*2+1] = pSrc[xSrc];
			continue;
			*/

			// Super 2x filter
			pDst1[xSrc*2] = pSrc[xSrc];

			if( xSrc<nWidth-1 )
			{
				DWORD b1 = (pSrc[xSrc]>>0)&0xFF;
				DWORD g1 = (pSrc[xSrc]>>8)&0xFF;
				DWORD r1 = (pSrc[xSrc]>>16)&0xFF;
				DWORD a1 = (pSrc[xSrc]>>24)&0xFF;
				DWORD b2 = (pSrc[xSrc+1]>>0)&0xFF;
				DWORD g2 = (pSrc[xSrc+1]>>8)&0xFF;
				DWORD r2 = (pSrc[xSrc+1]>>16)&0xFF;
				DWORD a2 = (pSrc[xSrc+1]>>24)&0xFF;
				pDst1[xSrc*2+1] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
			}
			else
				pDst1[xSrc*2+1] = pSrc[xSrc];

			if( ySrc<nHeight-1 )
			{
				DWORD b1 = (pSrc[xSrc]>>0)&0xFF;
				DWORD g1 = (pSrc[xSrc]>>8)&0xFF;
				DWORD r1 = (pSrc[xSrc]>>16)&0xFF;
				DWORD a1 = (pSrc[xSrc]>>24)&0xFF;
				DWORD b2 = (pSrc2[xSrc]>>0)&0xFF;
				DWORD g2 = (pSrc2[xSrc]>>8)&0xFF;
				DWORD r2 = (pSrc2[xSrc]>>16)&0xFF;
				DWORD a2 = (pSrc2[xSrc]>>24)&0xFF;
				pDst2[xSrc*2] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
			}
			else
				pDst2[xSrc*2] = pSrc[xSrc];

			if( xSrc<nWidth-1 )
			{
				if( ySrc<nHeight-1 )
				{
					DWORD b1 = (pSrc[xSrc]>>0)&0xFF;
					DWORD g1 = (pSrc[xSrc]>>8)&0xFF;
					DWORD r1 = (pSrc[xSrc]>>16)&0xFF;
					DWORD a1 = (pSrc[xSrc]>>24)&0xFF;
					DWORD b2 = (pSrc[xSrc+1]>>0)&0xFF;
					DWORD g2 = (pSrc[xSrc+1]>>8)&0xFF;
					DWORD r2 = (pSrc[xSrc+1]>>16)&0xFF;
					DWORD a2 = (pSrc[xSrc+1]>>24)&0xFF;
					DWORD b3 = (pSrc2[xSrc]>>0)&0xFF;
					DWORD g3 = (pSrc2[xSrc]>>8)&0xFF;
					DWORD r3 = (pSrc2[xSrc]>>16)&0xFF;
					DWORD a3 = (pSrc2[xSrc]>>24)&0xFF;
					DWORD b4 = (pSrc2[xSrc+1]>>0)&0xFF;
					DWORD g4 = (pSrc2[xSrc+1]>>8)&0xFF;
					DWORD r4 = (pSrc2[xSrc+1]>>16)&0xFF;
					DWORD a4 = (pSrc2[xSrc+1]>>24)&0xFF;
					pDst2[xSrc*2+1] = DWORD_MAKE((r1+r2+r3+r4)/4, (g1+g2+g3+g4)/4, (b1+b2+b3+b4)/4, (a1+a2+a3+a4)/4);
				}
				else
				{
					DWORD b1 = (pSrc[xSrc]>>0)&0xFF;
					DWORD g1 = (pSrc[xSrc]>>8)&0xFF;
					DWORD r1 = (pSrc[xSrc]>>16)&0xFF;
					DWORD a1 = (pSrc[xSrc]>>24)&0xFF;
					DWORD b2 = (pSrc[xSrc+1]>>0)&0xFF;
					DWORD g2 = (pSrc[xSrc+1]>>8)&0xFF;
					DWORD r2 = (pSrc[xSrc+1]>>16)&0xFF;
					DWORD a2 = (pSrc[xSrc+1]>>24)&0xFF;
					pDst2[xSrc*2+1] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
				}
			}
			else
			{
				if( ySrc<nHeight-1 )
				{
					DWORD b1 = (pSrc[xSrc]>>0)&0xFF;
					DWORD g1 = (pSrc[xSrc]>>8)&0xFF;
					DWORD r1 = (pSrc[xSrc]>>16)&0xFF;
					DWORD a1 = (pSrc[xSrc]>>24)&0xFF;
					DWORD b2 = (pSrc2[xSrc]>>0)&0xFF;
					DWORD g2 = (pSrc2[xSrc]>>8)&0xFF;
					DWORD r2 = (pSrc2[xSrc]>>16)&0xFF;
					DWORD a2 = (pSrc2[xSrc]>>24)&0xFF;
					pDst2[xSrc*2+1] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
				}
				else
					pDst2[xSrc*2+1] = pSrc[xSrc];
			}
		}
	}
}
void Texture2x_16( DrawInfo &srcInfo, DrawInfo &destInfo )
{
	WORD *pDst1, *pDst2;
	WORD *pSrc;
	DWORD nWidth = srcInfo.dwWidth;
	DWORD nHeight = srcInfo.dwHeight;

	for (WORD ySrc = 0; ySrc < nHeight; ySrc++)
	{
		pSrc = (WORD*)(((BYTE*)srcInfo.lpSurface)+ySrc*srcInfo.lPitch);
		pDst1 = (WORD*)(((BYTE*)destInfo.lpSurface)+(ySrc*2)*destInfo.lPitch);
		pDst2 = (WORD*)(((BYTE*)destInfo.lpSurface)+(ySrc*2+1)*destInfo.lPitch);

		for (WORD xSrc = 0; xSrc < nWidth; xSrc++)
		{
			pDst1[xSrc*2] = pSrc[xSrc];
			pDst1[xSrc*2+1] = pSrc[xSrc];
			pDst2[xSrc*2] = pSrc[xSrc];
			pDst2[xSrc*2+1] = pSrc[xSrc];
		}
	}

}

/************************************************************************/
/* Sharpen filters                                                      */
/************************************************************************/
void SharpenFilter(DWORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter)
{
	DWORD len=height*pitch;
	DWORD *pcopy = new DWORD[len];
	memcpy(pcopy, pdata, len<<2);

	DWORD mul1, mul2, mul3, shift4;
	switch( filter )
	{
	case TEXTURE_SHARPEN_MORE_ENHANCEMENT:
		mul1=1;
		mul2=8;
		mul3=12;
		shift4=2;
		break;
	case TEXTURE_SHARPEN_ENHANCEMENT:
	default:
		mul1=1;
		mul2=8;
		mul3=16;
		shift4=3;
		break;
	}

	DWORD x,y,z;
	DWORD *src1, *src2, *src3, *dest;
	DWORD val[4];
	DWORD t1,t2,t3,t4,t5,t6,t7,t8,t9;

	for( y=1; y<height-1; y++)
	{
		dest = pdata+y*pitch;
		src1 = pcopy+(y-1)*pitch;
		src2 = src1 + pitch;
		src3 = src2 + pitch;
		for( x=1; x<width-1; x++)
		{
			for( z=0; z<4; z++ )
			{
				t1 = *((BYTE*)(src1+x-1)+z);
				t2 = *((BYTE*)(src1+x  )+z);
				t3 = *((BYTE*)(src1+x+1)+z);
				t4 = *((BYTE*)(src2+x-1)+z);
				t5 = *((BYTE*)(src2+x  )+z);
				t6 = *((BYTE*)(src2+x+1)+z);
				t7 = *((BYTE*)(src3+x-1)+z);
				t8 = *((BYTE*)(src3+x  )+z);
				t9 = *((BYTE*)(src3+x+1)+z);
				val[z]=t5;
				if( (t5*mul2) > (t1+t3+t7+t9+t2+t4+t6+t8)*mul1 )
				{
					val[z]= min((((t5*mul3) - (t1+t3+t7+t9+t2+t4+t6+t8)*mul1)>>shift4),0xFF);
				}
			}
			dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
		}
	}
	delete [] pcopy;
}

void SharpenFilter_16(WORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter)
{
	//return;	// Sharpen does not make sense for 16 bits

	DWORD len=height*pitch;
	WORD *pcopy = new WORD[len];
	memcpy(pcopy, pdata, len<<1);

	WORD mul1, mul2, mul3, shift4;
	switch( filter )
	{
	case TEXTURE_SHARPEN_MORE_ENHANCEMENT:
		mul1=1;
		mul2=8;
		mul3=12;
		shift4=2;
		break;
	case TEXTURE_SHARPEN_ENHANCEMENT:
	default:
		mul1=1;
		mul2=8;
		mul3=16;
		shift4=3;
		break;
	}

	DWORD x,y,z;
	WORD *src1, *src2, *src3, *dest;
	WORD val[4];
	WORD t1,t2,t3,t4,t5,t6,t7,t8,t9;

	for( y=1; y<height-1; y++)
	{
		dest = pdata+y*pitch;
		src1 = pcopy+(y-1)*pitch;
		src2 = src1 + pitch;
		src3 = src2 + pitch;
		for( x=1; x<width-1; x++)
		{
			for( z=0; z<4; z++ )
			{
				DWORD shift = (z%1)?4:0;
				t1 = (*((BYTE*)(src1+x-1)+(z>>1)))>>shift;
				t2 = (*((BYTE*)(src1+x  )+(z>>1)))>>shift;
				t3 = (*((BYTE*)(src1+x+1)+(z>>1)))>>shift;
				t4 = (*((BYTE*)(src2+x-1)+(z>>1)))>>shift;
				t5 = (*((BYTE*)(src2+x  )+(z>>1)))>>shift;
				t6 = (*((BYTE*)(src2+x+1)+(z>>1)))>>shift;
				t7 = (*((BYTE*)(src3+x-1)+(z>>1)))>>shift;
				t8 = (*((BYTE*)(src3+x  )+(z>>1)))>>shift;
				t9 = (*((BYTE*)(src3+x+1)+(z>>1)))>>shift;
				val[z]=t5;
				if( (t5*mul2) > (t1+t3+t7+t9+t2+t4+t6+t8)*mul1 )
				{
					val[z] = (((t5*mul3) - (t1+t3+t7+t9+t2+t4+t6+t8)*mul1)>>shift4);
					val[z]= min(val[z],0xF);
				}
			}
			dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
		}
	}
	delete [] pcopy;
}

/************************************************************************/
/* Smooth filters                                                       */
/************************************************************************/
void SmoothFilter(DWORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter)
{
	DWORD len=height*pitch;
	DWORD *pcopy = new DWORD[len];
	memcpy(pcopy, pdata, len<<2);

	DWORD mul1, mul2, mul3, shift4;
	switch( filter )
	{
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1:
		mul1=1;
		mul2=2;
		mul3=4;
		shift4=4;
		break;
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2:
		mul1=1;
		mul2=1;
		mul3=8;
		shift4=4;
		break;
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3:
		mul1=1;
		mul2=1;
		mul3=2;
		shift4=2;
		break;
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4:
	default:
		mul1=1;
		mul2=1;
		mul3=6;
		shift4=3;
		break;
	}

	DWORD x,y,z;
	DWORD *src1, *src2, *src3, *dest;
	DWORD val[4];
	DWORD t1,t2,t3,t4,t5,t6,t7,t8,t9;

	if( filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3 || filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4 )
	{
		for( y=1; y<height-1; y+=2)
		{
			dest = pdata+y*pitch;
			src1 = pcopy+(y-1)*pitch;
			src2 = src1 + pitch;
			src3 = src2 + pitch;
			for( x=0; x<width; x++)
			{
				for( z=0; z<4; z++ )
				{
					t2 = *((BYTE*)(src1+x  )+z);
					t5 = *((BYTE*)(src2+x  )+z);
					t8 = *((BYTE*)(src3+x  )+z);
					val[z] = ((t2+t8)*mul2+(t5*mul3))>>shift4;
				}
				dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
			}
		}
	}
	else
	{
		for( y=0; y<height; y++)
		{
			dest = pdata+y*pitch;
			if( y>0 )
			{
				src1 = pcopy+(y-1)*pitch;
				src2 = src1 + pitch;
			}
			else
			{
				src1 = src2 = pcopy;
			}

			src3 = src2;
			if( y<height-1)	src3 += pitch;

			for( x=1; x<width-1; x++)
			{
				for( z=0; z<4; z++ )
				{
					t1 = *((BYTE*)(src1+x-1)+z);
					t2 = *((BYTE*)(src1+x  )+z);
					t3 = *((BYTE*)(src1+x+1)+z);
					t4 = *((BYTE*)(src2+x-1)+z);
					t5 = *((BYTE*)(src2+x  )+z);
					t6 = *((BYTE*)(src2+x+1)+z);
					t7 = *((BYTE*)(src3+x-1)+z);
					t8 = *((BYTE*)(src3+x  )+z);
					t9 = *((BYTE*)(src3+x+1)+z);
					val[z] = ((t1+t3+t7+t9)*mul1+((t2+t4+t6+t8)*mul2)+(t5*mul3))>>shift4;
				}
				dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
			}
		}
	}
	delete [] pcopy;
}

void SmoothFilter_16(WORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter)
{
	DWORD len=height*pitch;
	WORD *pcopy = new WORD[len];
	memcpy(pcopy, pdata, len<<1);

	WORD mul1, mul2, mul3, shift4;
	switch( filter )
	{
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1:
		mul1=1;
		mul2=2;
		mul3=4;
		shift4=4;
		break;
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2:
		mul1=1;
		mul2=1;
		mul3=8;
		shift4=4;
		break;
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3:
		mul1=1;
		mul2=1;
		mul3=2;
		shift4=2;
		break;
	case TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4:
	default:
		mul1=1;
		mul2=1;
		mul3=6;
		shift4=3;
		break;
	}

	DWORD x,y,z;
	WORD *src1, *src2, *src3, *dest;
	WORD val[4];
	WORD t1,t2,t3,t4,t5,t6,t7,t8,t9;

	if( filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3 || filter == TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4 )
	{
		for( y=1; y<height-1; y+=2)
		{
			dest = pdata+y*pitch;
			src1 = pcopy+(y-1)*pitch;
			src2 = src1 + pitch;
			src3 = src2 + pitch;
			for( x=0; x<width; x++)
			{
				for( z=0; z<4; z++ )
				{
					DWORD shift = (z&1)?4:0;
					t2 = (*((BYTE*)(src1+x  )+(z>>1)))>>shift;
					t5 = (*((BYTE*)(src2+x  )+(z>>1)))>>shift;
					t8 = (*((BYTE*)(src3+x  )+(z>>1)))>>shift;
					val[z] = ((t2+t8)*mul2+(t5*mul3))>>shift4;
				}
				dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
			}
		}
	}
	else
	{
		for( y=0; y<height; y++)
		{
			dest = pdata+y*pitch;
			if( y>0 )
			{
				src1 = pcopy+(y-1)*pitch;
				src2 = src1 + pitch;
			}
			else
			{
				src1 = src2 = pcopy;
			}

			src3 = src2;
			if( y<height-1)	src3 += pitch;

			for( x=1; x<width-1; x++)
			{
				for( z=0; z<4; z++ )
				{
					DWORD shift = (z&1)?4:0;
					t1 = (*((BYTE*)(src1+x-1)+(z>>1)))>>shift;
					t2 = (*((BYTE*)(src1+x  )+(z>>1)))>>shift;
					t3 = (*((BYTE*)(src1+x+1)+(z>>1)))>>shift;
					t4 = (*((BYTE*)(src2+x-1)+(z>>1)))>>shift;
					t5 = (*((BYTE*)(src2+x  )+(z>>1)))>>shift;
					t6 = (*((BYTE*)(src2+x+1)+(z>>1)))>>shift;
					t7 = (*((BYTE*)(src3+x-1)+(z>>1)))>>shift;
					t8 = (*((BYTE*)(src3+x  )+(z>>1)))>>shift;
					t9 = (*((BYTE*)(src3+x+1)+(z>>1)))>>shift;
					val[z] = ((t1+t3+t7+t9)*mul1+((t2+t4+t6+t8)*mul2)+(t5*mul3))>>shift4;
				}
				dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
			}
		}
	}
	delete [] pcopy;
}

/************************************************************************/
/* hq2x filters                                                         */
/************************************************************************/

/************************************************************************/
/* hq3x filters                                                         */
/************************************************************************/

/************************************************************************/
/* scale2x filters                                                      */
/************************************************************************/

/************************************************************************/
/* scale3x filters                                                      */
/************************************************************************/

CTexture* TextureEnhancement(TextureEntry *pEntry)
{
	if( pEntry->dwEnhancementFlag == options.textureEnhancement )
	{
		return NULL;
	}
	else if( options.textureEnhancement == TEXTURE_NO_ENHANCEMENT )
	{
		SAFE_DELETE(pEntry->pMirroredTexture);
		if( pEntry->dwEnhancementFlag < TEXTURE_SHARPEN_ENHANCEMENT )
			pEntry->dwEnhancementFlag = TEXTURE_NO_ENHANCEMENT;
		return NULL;
	}

	if( status.primitiveType != PRIM_TEXTRECT && 
		(options.textureEnhancement == TEXTURE_2X_TEXRECT_ENHANCEMENT || 
		options.textureEnhancement == TEXTURE_2XSAI_TEXRECT_ENHANCEMENT ) )
	{
		return NULL;
	}

	DrawInfo srcInfo;	
	if( pEntry->pTexture->StartUpdate(&srcInfo) == false )
	{
		pEntry->pMirroredTexture = NULL;
		return NULL;
	}

	DWORD realwidth = srcInfo.dwWidth;
	DWORD realheight = srcInfo.dwHeight;
	DWORD nWidth = srcInfo.dwCreatedWidth;
	DWORD nHeight = srcInfo.dwCreatedHeight;

	if( options.textureEnhancement == TEXTURE_SHARPEN_ENHANCEMENT || options.textureEnhancement == TEXTURE_SHARPEN_MORE_ENHANCEMENT )
	{
		if( pEntry->pTexture->GetPixelSize() == 4 )
			SharpenFilter((DWORD*)srcInfo.lpSurface, nWidth, nHeight, nWidth, options.textureEnhancement);
		else
			SharpenFilter_16((WORD*)srcInfo.lpSurface, nWidth, nHeight, nWidth, options.textureEnhancement);
		pEntry->dwEnhancementFlag = options.textureEnhancement;
		pEntry->pTexture->EndUpdate(&srcInfo);
		pEntry->pMirroredTexture = NULL;
		return NULL;
	}

	pEntry->dwEnhancementFlag = options.textureEnhancement;
	if( options.textureEnhancementControl == TEXTURE_ENHANCEMENT_SMALL_TEXTURE_ONLY )
	{
		if( realwidth > 128 || realheight > 128 || realwidth+realheight> 128 )
		{
			pEntry->pTexture->EndUpdate(&srcInfo);
			pEntry->pMirroredTexture = NULL;
			return NULL;
		}
	}

	CTexture* pSurfaceHandler = NULL;
	pSurfaceHandler = CDeviceBuilder::GetBuilder()->CreateTexture(nWidth*2, nHeight*2);
	DrawInfo destInfo;
	pSurfaceHandler->StartUpdate(&destInfo);

	if( options.textureEnhancement == TEXTURE_2X_ENHANCEMENT || options.textureEnhancement == TEXTURE_2X_TEXRECT_ENHANCEMENT  )
	{
		if( pEntry->pTexture->GetPixelSize() == 4 )
			Texture2x( srcInfo, destInfo);
		else
			Texture2x_16( srcInfo, destInfo);
	}
	else
	{
		if( pEntry->pTexture->GetPixelSize() == 4 )
			Super2xSaI((DWORD*)(srcInfo.lpSurface),(DWORD*)(destInfo.lpSurface), nWidth, realheight, nWidth);
		else
			Super2xSaI_16((WORD*)(srcInfo.lpSurface),(WORD*)(destInfo.lpSurface), nWidth, realheight, nWidth);
	}

	if( options.textureEnhancementControl >= TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1 )
	{
		if( pEntry->pTexture->GetPixelSize() == 4 )
			SmoothFilter((DWORD*)destInfo.lpSurface, realwidth<<1, realheight<<1, nWidth<<1, options.textureEnhancementControl);
		else
			SmoothFilter_16((WORD*)destInfo.lpSurface, realwidth<<1, realheight<<1, nWidth<<1, options.textureEnhancementControl);

	}

	pSurfaceHandler->EndUpdate(&destInfo);
	
	pSurfaceHandler->SetOthersVariables();
	pSurfaceHandler->m_dwCreatedTextureWidth = pEntry->pTexture->m_dwCreatedTextureWidth;
	pSurfaceHandler->m_dwCreatedTextureHeight = pEntry->pTexture->m_dwCreatedTextureHeight;

	pEntry->pTexture->EndUpdate(&srcInfo);

	pEntry->pMirroredTexture = pSurfaceHandler;
	return pSurfaceHandler;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void MirrorEmulator_DrawLine(DrawInfo& destInfo, DrawInfo& srcInfo, LPDWORD pSource, LPDWORD pDest, DWORD nWidth, BOOL bFlipLeftRight)
{
	if(!bFlipLeftRight)
	{
		memcpy(pDest, pSource, nWidth * 4);
	}
	else
	{
		LPDWORD pMaxDest = pDest + nWidth;
		pSource += nWidth - 1;
		for(; pDest < pMaxDest; pDest++, pSource--)
		{
			*pDest = *pSource;
		}
	}
}


void MirrorEmulator_Draw(DrawInfo& destInfo, DrawInfo& srcInfo, DWORD nDestX, DWORD nDestY, BOOL bFlipLeftRight, BOOL bFlipUpDown)
{
	LPBYTE pDest = (LPBYTE)((DWORD)destInfo.lpSurface + (destInfo.lPitch * nDestY) + (4 * nDestX));
	LPBYTE pMaxDest = pDest + (destInfo.lPitch * srcInfo.dwHeight);
	LPBYTE pSource = (LPBYTE)(srcInfo.lpSurface);
	if(!bFlipUpDown)
	{
		for(; pDest < pMaxDest; pDest += destInfo.lPitch, pSource += srcInfo.lPitch)
		{
			MirrorEmulator_DrawLine(destInfo, srcInfo, (LPDWORD)pSource, (LPDWORD)pDest, srcInfo.dwWidth, bFlipLeftRight);
		}
	}
	else
	{
		pSource += (srcInfo.lPitch * (srcInfo.dwHeight - 1));
		for(; pDest < pMaxDest; pDest += destInfo.lPitch, pSource -= srcInfo.lPitch)
		{
			MirrorEmulator_DrawLine(destInfo, srcInfo, (LPDWORD)pSource, (LPDWORD)pDest, srcInfo.dwWidth, bFlipLeftRight);
		}
	}
}

CTexture* GetMirrorSurfaceHandler(DWORD dwTile, TextureEntry *pEntry)
{
	if( ((gRDP.tiles[dwTile].bMirrorS) || (gRDP.tiles[dwTile].bMirrorT)) && CGraphicsContext::Get()->m_supportTextureMirror == false )
	{
		if(pEntry->pMirroredTexture)
		{
			return pEntry->pMirroredTexture;
		}
		else
		{
			CTexture* pSurfaceHandler = NULL;

			// FIXME: Compute the correct values. 2/2 seems to always work correctly in Mario64
			DWORD nXTimes = gRDP.tiles[dwTile].bMirrorS ? 2 : 1;
			DWORD nYTimes = gRDP.tiles[dwTile].bMirrorT ? 2 : 1;
			
			// For any texture need to use mirror, we should not need to rescale it
			// because texture need to be mirrored must with MaskS and MaskT

			// But again, check me

			//if( pEntry->pTexture->m_bScaledS == false || pEntry->pTexture->m_bScaledT == false)
			//{
			//	pEntry->pTexture->ScaleImageToSurface();
			//}

			DrawInfo srcInfo;	
			pEntry->pTexture->StartUpdate(&srcInfo);
			DWORD nWidth = srcInfo.dwWidth;
			DWORD nHeight = srcInfo.dwHeight;

			pSurfaceHandler = CDeviceBuilder::GetBuilder()->CreateTexture(nWidth * nXTimes, nHeight * nYTimes);
			DrawInfo destInfo;
			pSurfaceHandler->StartUpdate(&destInfo);
			
			
			for(DWORD nY = 0; nY < nYTimes; nY++)
			{
				for(DWORD nX = 0; nX < nXTimes; nX++)
				{
					MirrorEmulator_Draw(destInfo, srcInfo, nWidth * nX, nHeight * nY, nX & 0x1, nY & 0x1);
				}
			}

			pSurfaceHandler->EndUpdate(&destInfo);
			
			// fix me, there should be a flag to tell that it is a mirrored texture handler
			// not the original texture handlers, so all texture coordinate should be divided by 2
			pSurfaceHandler->SetOthersVariables();

			pEntry->pTexture->EndUpdate(&srcInfo);

			pEntry->pMirroredTexture = pSurfaceHandler;

			return pSurfaceHandler;
		}
	}
	return NULL;
}

void ConvertTextureRGBAtoI(TextureEntry* pEntry, bool alpha)
{
	DrawInfo srcInfo;	
	if( pEntry->pTexture->StartUpdate(&srcInfo) )
	{
		DWORD *buf;
		DWORD val;
		DWORD r,g,b,a,i;

		for(int nY = 0; nY < srcInfo.dwCreatedHeight; nY++)
		{
			buf = (DWORD*)((BYTE*)srcInfo.lpSurface+nY*srcInfo.lPitch);
			for(int nX = 0; nX < srcInfo.dwCreatedWidth; nX++)
			{
				val = buf[nX];
				b = (val>>0)&0xFF;
				g = (val>>8)&0xFF;
				r = (val>>16)&0xFF;
				i = (r+g+b)/3;
				a = alpha?(val&0xFF000000):(i<<24);
				buf[nX] = (a|(i<<16)|(i<<8)|i);
			}
		}
		pEntry->pTexture->EndUpdate(&srcInfo);	}
}