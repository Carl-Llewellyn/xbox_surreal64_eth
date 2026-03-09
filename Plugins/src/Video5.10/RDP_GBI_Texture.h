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

union {
	BYTE	g_Tmem8bit[0x1000];
	uint16	g_Tmem16bit[0x800];
	uint32	g_Tmem32bit[0x300];
	uint64	g_Tmem64bit[0x200];
}g_TMEM;

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

/************************************************************************/
/*                                                                      */
/************************************************************************/
//void Super2xSaI( DWORD *srcPtr, DWORD *destPtr, DWORD width, DWORD height, DWORD pitch);
//void Super2xSaI_16( WORD *srcPtr, WORD *destPtr, DWORD width, DWORD height, DWORD pitch);

void SmoothFilter(DWORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter=TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1);
void SharpenFilter(DWORD *pdata, DWORD width, DWORD height, DWORD pitch, DWORD filter=TEXTURE_SHARPEN_ENHANCEMENT);
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

#define DWORD_MAKE(r, g, b, a)   ((DWORD) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

CTexture* TextureEnhancement(DWORD dwTile, TextureEntry *pEntry)
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
		nWidth = srcInfo.dwWidth;
		nHeight = srcInfo.dwHeight;
		if( pEntry->pTexture->GetPixelSize() == 4 )
		{
			DWORD *pDst1, *pDst2;
			DWORD *pSrc, *pSrc2;

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
		else
		{
			WORD *pDst1, *pDst2;
			WORD *pSrc;

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
	}
	else
	{
		/*if( pEntry->pTexture->GetPixelSize() == 4 )
			Super2xSaI((DWORD*)(srcInfo.lpSurface),(DWORD*)(destInfo.lpSurface), nWidth, realheight, nWidth);
		else
			Super2xSaI_16((WORD*)(srcInfo.lpSurface),(WORD*)(destInfo.lpSurface), nWidth, realheight, nWidth);*/
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

	// Retrieve the tile loading info
	DWORD infoTmemAddr = gRDP.tiles[dwTile].dwTMem;
	TMEMLoadMapInfo *info = &g_tmemLoadAddrMap[infoTmemAddr];
	if( !IsTmemFlagValid(infoTmemAddr) )
	{
		infoTmemAddr =  GetValidTmemInfoIndex(infoTmemAddr);
		info = &g_tmemLoadAddrMap[infoTmemAddr];
	}

	if( info->dwFormat != gRDP.tiles[dwTile].dwFormat && gRDP.tiles[dwTile].dwSize != G_IM_SIZ_4b )
	{
		//TRACE1("Tile %d format is not matching the loaded texture format", dwTile);
		pEntry = gTextureCache.GetBlackTexture();
		CDaedalusRender::g_pRender->SetCurrentTexture( dwTile, pEntry->pTexture, 4, 4);
		return;
	}

	dwTLutFmt = gRDP.otherMode.text_tlut <<G_MDSFT_TEXTLUT;

	bool useSizeFromSetTile = false;
	if( gRDP.tiles[dwTile].sh == 0 && gRDP.tiles[dwTile].sl == 0 && gRDP.tiles[dwTile].th == 0 && gRDP.tiles[dwTile].tl == 0 )
	{
		// The tile size has never been set
		if( info->bSetBy == CMD_LOADTILE )
		{
			useSizeFromSetTile = true;
			//memcpy(&gRDP.tiles[dwTile], &gRDP.tiles[G_TX_LOADTILE], sizeof(Tile));
		}
		else if( info->dwTotalWords > 4 )
		{
			TRACE1("Tile %d is not set yet, check me", dwTile );
			return;	// Loading blocks, we don't have tile info, this is wrong
		}
	}

	// Now Initialize the texture dimension
	int dwTileWidth;
	int dwTileHeight;
	if( useSizeFromSetTile )
	{
		//dwTileWidth = info->SH - info->SL + 1;
		dwTileWidth = info->dwWidth;	// From SetTImage
		dwTileWidth = dwTileWidth << info->dwSize >> gRDP.tiles[dwTile].dwSize;
		dwTileHeight= info->TH - info->TL + 1;
	}
	else
	{
		dwTileWidth = gRDP.tiles[dwTile].hilite_sh - gRDP.tiles[dwTile].hilite_sl +1;
		if( dwTileWidth < gRDP.tiles[dwTile].sh - gRDP.tiles[dwTile].sl +1 )
			dwTileWidth = gRDP.tiles[dwTile].sh - gRDP.tiles[dwTile].sl +1;
		dwTileHeight= gRDP.tiles[dwTile].hilite_th - gRDP.tiles[dwTile].hilite_tl +1;
		if( dwTileHeight < gRDP.tiles[dwTile].th - gRDP.tiles[dwTile].tl +1 )
			dwTileHeight = gRDP.tiles[dwTile].th - gRDP.tiles[dwTile].tl +1;
	}

	int dwTileMaskWidth = gRDP.tiles[dwTile].dwMaskS > 0 ? (1 << gRDP.tiles[dwTile].dwMaskS ) : 0;
	int dwTileMaskHeight = gRDP.tiles[dwTile].dwMaskT > 0 ? (1 << gRDP.tiles[dwTile].dwMaskT ) : 0;

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

	ComputeTileDimension(gRDP.tiles[dwTile].dwMaskS, gRDP.tiles[dwTile].bClampS,
		gRDP.tiles[dwTile].bMirrorS, dwTileWidth, gti.WidthToCreate, gti.WidthToLoad);
	gRDP.tiles[dwTile].dwWidth = gti.WidthToCreate;
	ComputeTileDimension(gRDP.tiles[dwTile].dwMaskT, gRDP.tiles[dwTile].bClampT,
		gRDP.tiles[dwTile].bMirrorT, dwTileHeight, gti.HeightToCreate, gti.HeightToLoad);
	gRDP.tiles[dwTile].dwHeight = gti.HeightToCreate;

#ifdef _DEBUG
	if( gti.WidthToCreate < gti.WidthToLoad )
		DebuggerAppendMsg("Check me, width to create = %d, width to load = %d", gti.WidthToCreate, gti.WidthToLoad);
	if( gti.HeightToCreate < gti.HeightToLoad )
		DebuggerAppendMsg("Check me, height to create = %d, height to load = %d", gti.HeightToCreate, gti.HeightToLoad);
#endif

	// Only needs doing for CI, but never mind
	dwPalOffset = 0;
	switch (gRDP.tiles[dwTile].dwSize)
	{
	case G_IM_SIZ_4b: 
		dwPalOffset = 16  * 2 * gRDP.tiles[dwTile].dwPalette; 
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
			dwPitch = gRDP.tiles[dwTile].dwLine << 3;
		else
		{
			DWORD DXT = info->dwDXT;
			if( info->dwDXT > 1 )
			{
				DXT = ReverseDXT(info->dwDXT, info->SH, dwTileWidth, gRDP.tiles[dwTile].dwSize);
			}
			dwPitch = DXT << 3;
		}
		
		if (gRDP.tiles[dwTile].dwSize == G_IM_SIZ_32b)
			dwPitch = gRDP.tiles[dwTile].dwLine << 4;
	}

	gRDP.tiles[dwTile].dwPitch = dwPitch;

	if( gti.WidthToLoad < gti.WidthToCreate && gRDP.tiles[dwTile].dwMaskS > 0 && gti.WidthToLoad != dwTileMaskWidth &&
		info->bSetBy == CMD_LOADBLOCK )
	{
		// We have got the pitch now, recheck the width_to_load
		u32 pitchwidth = dwPitch<<1>>gRDP.tiles[dwTile].dwSize;
		if( pitchwidth == dwTileMaskWidth )
		{
			gti.WidthToLoad = pitchwidth;
		}
	}
	if( gti.HeightToLoad < gti.HeightToCreate && gRDP.tiles[dwTile].dwMaskT > 0 && gti.HeightToLoad != dwTileMaskHeight &&
		info->bSetBy == CMD_LOADBLOCK )
	{
		u32 pitchwidth = dwPitch<<1>>gRDP.tiles[dwTile].dwSize;
		u32 pitchHeight = (info->dwTotalWords<<1)/dwPitch;
		if( pitchHeight == dwTileMaskHeight )
		{
			gti.HeightToLoad = pitchHeight;
		}
	}



	if( ((gti.HeightToLoad*gti.WidthToLoad)>>(4-gRDP.tiles[dwTile].dwSize)) + gRDP.tiles[dwTile].dwTMem > 0x200 )
	{
		//TRACE0("Warning: texture loading tmem is over range");
		if( gti.WidthToLoad > gti.HeightToLoad )
		{
			if( info->bSetBy != CMD_LOADTILE )
				gRDP.tiles[dwTile].dwWidth = gti.WidthToLoad = (dwPitch << 1 )>> gRDP.tiles[dwTile].dwSize;
			gRDP.tiles[dwTile].dwHeight = gti.HeightToCreate = gti.HeightToLoad = ((0x200 - gRDP.tiles[dwTile].dwTMem) << (4-gRDP.tiles[dwTile].dwSize)) / gti.WidthToLoad;
		}
		else
		{
			gRDP.tiles[dwTile].dwHeight = gti.HeightToCreate = gti.HeightToLoad = info->dwTotalWords / ((gti.WidthToLoad << gRDP.tiles[dwTile].dwSize) >> 1);
		}
	}

	gti.Address = info->dwLoadAddress+(gRDP.tiles[dwTile].dwTMem-infoTmemAddr)*8;

	if( info->bSetBy == CMD_LOADTILE )
	{
		gti.LeftToLoad = (info->SL<<info->dwSize)>>gRDP.tiles[dwTile].dwSize;
		gti.TopToLoad = info->TL;
	}
	else
	{
		/*
		gti.LeftToLoad = (info->SL<<info->dwSize)>>gRDP.tiles[dwTile].dwSize;
		gti.TopToLoad = 0;
		gti.Address += ((info->SH+1)*info->TL*info->dwSize);
		*/
		gti.LeftToLoad = (info->SL<<info->dwSize)>>gRDP.tiles[dwTile].dwSize;
		gti.TopToLoad = (info->TL<<info->dwSize)>>gRDP.tiles[dwTile].dwSize;
		//gti.TopToLoad = info->TL;
	}

	gti = gRDP.tiles[dwTile];	// Copy tile info to textureInfo entry

	gti.PalAddress = (uint32)(&g_wRDPPal[0]) + dwPalOffset;
	gti.TLutFmt = dwTLutFmt;
	gti.bSwapped = info->bSwapped;

	//Check the memory boundary
	gti.Address &= (g_dwRamSize-1);

	if( gti.Address + gti.HeightToLoad*gti.Pitch >= g_dwRamSize )
	{
		TRACE0("Warning: texture loading tmem is over range 2 ");
		gti.HeightToCreate = gRDP.tiles[dwTile].dwHeight = dwTileHeight;
	}

	//Check again
	if( gti.Address + gti.HeightToLoad*gti.Pitch >= g_dwRamSize )
	{
		TRACE0("Warning: texture loading tmem is over range 3");
		gti.HeightToCreate = gRDP.tiles[dwTile].dwHeight = (g_dwRamSize-gti.Address)/gti.Pitch;
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

	if( currentRomOptions.N64FrameBufferEmuType == FRM_COMPLETE || currentRomOptions.N64FrameBufferEmuType == FRM_FASTER_RECHECK )
		CheckAndUpdateFrameBuffer(gti.Address);

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
		GetMirrorSurfaceHandler(dwTile, pEntry);;
		if( pEntry->pMirroredTexture == NULL )
		{
			TextureEnhancement(dwTile, pEntry);
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
	if( gRDP.textureIsChanged || !currentRomOptions.bFastTexCRC )
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

	gRDP.tiles[dwTile].lastTileCmd = CMD_LOADTLUT;

#ifdef _DEBUG
	if( ((dwCmd0>>12)&0x3) != 0 || ((dwCmd0)&0x3) != 0 || ((dwCmd1>>12)&0x3) != 0 || ((dwCmd1)&0x3) != 0 )
	{
		//TRACE0("Load tlut, sl,tl,sh,th are not integers");
	}
#endif

	dwCount = (dwLRS - dwULS)+1;
	
	dwRDRAMOffset = (dwULS + dwULT)*2;

	TMEMLoadMapInfo &info = g_tmemLoadAddrMap[0x100];
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

#ifdef _DEBUG
	//if( ((dwCmd0>>12)&0x3) != 0 || ((dwCmd0)&0x3) != 0 )
	//{
	//	TRACE0("Load block sl, tl are not integers");
	//	DebuggerAppendMsg("sl = %d, tl = %d", (dwCmd0>>12)&0x0FFF, (dwCmd0)&0x0FFF);
	//}
#endif

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

	info.bSetBy = CMD_LOADTILE;
	info.bSwapped =FALSE;

	g_TxtLoadBy = CMD_LOADTILE;
}


static char *pszOnOff[2]     = {"Off", "On"};
void DLParser_SetTile(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;

	DWORD dwTile		= (dwCmd1>>24)&0x7;

	gRDP.tiles[dwTile].dwFormat	= (dwCmd0>>21)&0x7;
	gRDP.tiles[dwTile].dwSize		= (dwCmd0>>19)&0x3;
	gRDP.tiles[dwTile].dwLine		= (dwCmd0>>9 )&0x1FF;
	gRDP.tiles[dwTile].dwTMem		= (dwCmd0    )&0x1FF;

	gRDP.tiles[dwTile].dwPalette	= (dwCmd1>>20)&0x0F;
	gRDP.tiles[dwTile].bClampT		= (dwCmd1>>19)&0x01;
	gRDP.tiles[dwTile].bMirrorT		= (dwCmd1>>18)&0x01;
	gRDP.tiles[dwTile].dwMaskT		= (dwCmd1>>14)&0x0F;
	gRDP.tiles[dwTile].dwShiftT		= (dwCmd1>>10)&0x0F;		// LOD stuff
	gRDP.tiles[dwTile].bClampS		= (dwCmd1>>9 )&0x01;
	gRDP.tiles[dwTile].bMirrorS		= (dwCmd1>>8 )&0x01;
	gRDP.tiles[dwTile].dwMaskS		= (dwCmd1>>4 )&0x0F;
	gRDP.tiles[dwTile].dwShiftS		= (dwCmd1    )&0x0F;		// LOD stuff

	gRDP.tiles[dwTile].fShiftScaleS = 1.0f;
	if( gRDP.tiles[dwTile].dwShiftS )
	{
		if( gRDP.tiles[dwTile].dwShiftS > 10 )
		{
			gRDP.tiles[dwTile].fShiftScaleS = (float)(1 << (16 - gRDP.tiles[dwTile].dwShiftS));
		}
		else
		{
			gRDP.tiles[dwTile].fShiftScaleS = (float)1.0f/(1 << gRDP.tiles[dwTile].dwShiftS);
		}
	}

	gRDP.tiles[dwTile].fShiftScaleT = 1.0f;
	if( gRDP.tiles[dwTile].dwShiftT )
	{
		if( gRDP.tiles[dwTile].dwShiftT > 10 )
		{
			gRDP.tiles[dwTile].fShiftScaleT = (float)(1 << (16 - gRDP.tiles[dwTile].dwShiftT));
		}
		else
		{
			gRDP.tiles[dwTile].fShiftScaleT = (float)1.0f/(1 << gRDP.tiles[dwTile].dwShiftT);
		}
	}

	gRDP.tiles[dwTile].lastTileCmd = CMD_SETTILE;

#ifdef _DEBUG
	if( logTextureDetails   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
	DebuggerAppendMsg("SetTile:%d  Fmt: %s/%s Line:%d TMem:0x%04x Palette:%d\n",
		dwTile, pszImgFormat[gRDP.tiles[dwTile].dwFormat], pszImgSize[gRDP.tiles[dwTile].dwSize],
		gRDP.tiles[dwTile].dwLine,  gRDP.tiles[dwTile].dwTMem, gRDP.tiles[dwTile].dwPalette);
	DebuggerAppendMsg("         S: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x\n",
		pszOnOff[gRDP.tiles[dwTile].bClampS],pszOnOff[gRDP.tiles[dwTile].bMirrorS],
		gRDP.tiles[dwTile].dwMaskS, gRDP.tiles[dwTile].dwShiftS);
	DebuggerAppendMsg("         T: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x\n",
		pszOnOff[gRDP.tiles[dwTile].bClampT],pszOnOff[gRDP.tiles[dwTile].bMirrorT],
		gRDP.tiles[dwTile].dwMaskT, gRDP.tiles[dwTile].dwShiftT);
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_TEXTURE_CMD);

	DL_PF("    Tile:%d  Fmt: %s/%s Line:%d TMem:0x%04x Palette:%d",
		dwTile, pszImgFormat[gRDP.tiles[dwTile].dwFormat], pszImgSize[gRDP.tiles[dwTile].dwSize],
		gRDP.tiles[dwTile].dwLine, gRDP.tiles[dwTile].dwTMem, gRDP.tiles[dwTile].dwPalette);
	DL_PF("         S: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x",
		pszOnOff[gRDP.tiles[dwTile].bClampS],pszOnOff[gRDP.tiles[dwTile].bMirrorS],
		gRDP.tiles[dwTile].dwMaskS, gRDP.tiles[dwTile].dwShiftS);
	DL_PF("         T: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x",
		pszOnOff[gRDP.tiles[dwTile].bClampT],pszOnOff[gRDP.tiles[dwTile].bMirrorT],
		gRDP.tiles[dwTile].dwMaskT, gRDP.tiles[dwTile].dwShiftT);
#endif
}

void DLParser_SetTileSize(DWORD dwCmd0, DWORD dwCmd1)
{
	gRDP.textureIsChanged = true;

	DWORD dwTile	= (dwCmd1>>24)&0x07;
	DWORD sl		= (DWORD)((dwCmd0>>12)&0x0FFF);
	DWORD tl		= (DWORD)((dwCmd0    )&0x0FFF);
	DWORD sh		= (DWORD)((dwCmd1>>12)&0x0FFF);
	DWORD th		= (DWORD)((dwCmd1    )&0x0FFF);

#ifdef _DEBUG
	if( ((dwCmd0>>12)&0x3) != 0 || ((dwCmd0)&0x3) != 0 || ((dwCmd1>>12)&0x3) != 0 || ((dwCmd1)&0x3) != 0 )
	{
		//DebuggerAppendMsg("Set tile size, sl,tl,sh,th are not integers");
	}
#endif

	if( gRDP.tiles[dwTile].lastTileCmd != CMD_SETTILE_SIZE )
	{
		if( sl > sh || tl > th || (sh == 0 && th == 0) )
		{
#ifdef _DEBUG
			if( sl != 0 || tl != 0 || sh != 0 || th != 0 )
				TRACE0("Check me, setTileSize is not correct");
#endif
			if( sl>sh ) {DWORD temp=sl;sl=sh;sh=temp;}
			if( tl>th ) {DWORD temp=tl;tl=th;th=temp;}
		}
		gRDP.tiles[dwTile].hilite_sl = gRDP.tiles[dwTile].sl = sl / 4;
		gRDP.tiles[dwTile].hilite_tl = gRDP.tiles[dwTile].tl = tl / 4;
		gRDP.tiles[dwTile].hilite_sh = gRDP.tiles[dwTile].sh = sh / 4;
		gRDP.tiles[dwTile].hilite_th = gRDP.tiles[dwTile].th = th / 4;
		gRDP.tiles[dwTile].lastTileCmd = CMD_SETTILE_SIZE;
	}
	else
	{
		gRDP.tiles[dwTile].hilite_sl = sl>0x7ff ? (sl-0xfff) : sl;
		gRDP.tiles[dwTile].hilite_tl = tl>0x7ff ? (tl-0xfff) : tl;
		gRDP.tiles[dwTile].hilite_sl /= 4;
		gRDP.tiles[dwTile].hilite_tl /= 4;
		gRDP.tiles[dwTile].hilite_sh = sh/4;
		gRDP.tiles[dwTile].hilite_th = th/4;
		gRDP.tiles[dwTile].lastTileCmd = CMD_SETTILE_SIZE;
	}

#ifdef _DEBUG
	if( logTextureDetails   || (pauseAtNext && eventToPause==NEXT_TEXTURE_CMD) )
	{
	DebuggerAppendMsg("SetTileSize:%d (%d,%d) -> (%d,%d) [%d x %d]\n",
		dwTile, sl/4, tl/4, sh/4, th/4, 
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
	status.primitiveType = PRIM_TEXTRECT;

	// This command used 128bits, and not 64 bits. This means that we have to look one 
	// Command ahead in the buffer, and update the PC.
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+4+8);
	DWORD dwHalf1 = *(DWORD *)(g_pu8RamBase + dwPC);
	DWORD dwHalf2 = *(DWORD *)(g_pu8RamBase + dwPC+8);
	if( (dwHalf1>>24) == 0xb4 || (dwHalf1>>24) == 0xb3 || (dwHalf1>>24) == 0xe1 )
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
		if( ((currentRomOptions.N64FrameBufferEmuType == FRM_FASTER||currentRomOptions.N64FrameBufferEmuType ==FRM_FASTER_RECHECK||currentRomOptions.N64FrameBufferEmuType ==FRM_COPY_AND_BACK) || currentRomOptions.N64FrameBufferEmuType == FRM_IGNORE ) && g_bUsingFakeCI && g_FakeFrameBufferInfo.CI_Info.dwFormat == gRDP.tiles[dwTile].dwFormat &&
			g_FakeFrameBufferInfo.CI_Info.dwSize == gRDP.tiles[dwTile].dwSize && 
			gRDP.tiles[dwTile].dwFormat == G_IM_FMT_CI && gRDP.tiles[dwTile].dwSize == G_IM_SIZ_8b )
		{
			if( (currentRomOptions.N64FrameBufferEmuType == FRM_FASTER||currentRomOptions.N64FrameBufferEmuType ==FRM_FASTER_RECHECK||currentRomOptions.N64FrameBufferEmuType ==FRM_COPY_AND_BACK) )
			{
				SetupTextures();
				TexRectToFrameBuffer_8b(dwXL, dwYL, dwXH, dwYH, t0u0, t0v0, t0u1, t0v1, dwTile);
			}
		}
		else
		{
			CDaedalusRender::g_pRender->TexRect(dwXL, dwYL, dwXH, dwYH, fS0, fT0, fDSDX, fDTDY);

			status.dwNumTrisRendered += 2;
		}
	}

	ForceMainTextureIndex(curTile);
}


void DLParser_TexRectFlip(DWORD dwCmd0, DWORD dwCmd1)
{ 
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

