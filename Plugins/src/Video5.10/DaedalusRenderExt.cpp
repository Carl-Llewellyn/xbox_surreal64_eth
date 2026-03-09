/*
Copyright (C) 2002 Rice & StrmnNrmn

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

extern uObjMtxReal gObjMtxReal;
extern DaedalusMatrix gD3DMtxReal;
extern DaedalusMatrix gD3DObjOffset;

extern SetImgInfo g_CI;
extern int g_DlistCount;
extern FakeFrameBufferInfo g_FakeFrameBufferInfo;

extern SetImgInfo g_DI;
extern bool g_bFakeCIUpdated;

//========================================================================

void CDaedalusRender::LoadFrameBuffer(bool useVIreg, DWORD left, DWORD top, DWORD width, DWORD height)
{
	u32 VIwidth = *g_GraphicsInfo.VI_WIDTH_RG;

	TextureInfo gti;

	gti.clampS = gti.clampT = 0;
	gti.maskS = gti.maskT = gti.mirrorS = gti.mirrorT = 0;
	gti.TLutFmt	= G_TT_RGBA16;	//RGBA16
	gti.bSwapped	= FALSE;
	gti.Palette	= 0;

	if( useVIreg && *g_GraphicsInfo.VI_ORIGIN_RG > VIwidth*2 )
	{
		gti.Format	= 0;
		gti.Size	= 2;

		gti.Address	= (*g_GraphicsInfo.VI_ORIGIN_RG & (g_dwRamSize-1) ) - VIwidth*2;
		gti.LeftToLoad	= 0;
		gti.TopToLoad	= 0;

		gti.PalAddress = (uint32)(&g_wRDPPal[0]);

		gti.WidthToCreate	= windowSetting.uViWidth;;
		gti.HeightToCreate	= windowSetting.uViHeight;

		if( gti.WidthToCreate == 0 || gti.HeightToCreate == 0 )
		{
			TRACE0("Loading frame buffer: size = 0 x 0");
			return;
		}

		gti.Pitch	= VIwidth << gti.Size >> 1;
	}
	else
	{
		gti.Format	= g_CI.dwFormat;
		gti.Size	= g_CI.dwSize;
		gti.PalAddress = (uint32)(&g_wRDPPal[0]);

		gti.Address	= RDPSegAddr(g_CI.dwAddr);

		if( width == 0 || height == 0 )
		{
			gti.LeftToLoad		= 0;
			gti.TopToLoad		= 0;

			gti.WidthToCreate		= g_CI.dwWidth;
			gti.HeightToCreate		= g_CI.dwWidth*3/4;
		}
		else
		{
			gti.LeftToLoad		= left;
			gti.TopToLoad		= top;

			gti.WidthToCreate		= width;
			gti.HeightToCreate		= height;
		}

		if( gti.Size == G_IM_SIZ_4b )
		{
			gti.Pitch	= g_CI.dwWidth >> 1;
		}
		else
		{
			gti.Pitch	= g_CI.dwWidth << (gti.Size-1);

		}
	}


	if( gti.Address + gti.Pitch*gti.HeightToCreate > g_dwRamSize )
	{
		TRACE0("Skip frame buffer loading, memory out of bound");
		return;
	}

#ifdef _DEBUG
	if( pauseAtNext )
	{
		DebuggerAppendMsg("Load Frame Buffer Imag at: %08X, (%d, %d) - (%d, %d)", gti.Address,
			gti.LeftToLoad, gti.TopToLoad, gti.WidthToCreate, gti.HeightToCreate );
	}
#endif


	gti.HeightToLoad = gti.HeightToCreate;
	gti.WidthToLoad = gti.WidthToCreate;

	gti.pPhysicalAddress = ((BYTE*)g_pu32RamBase)+gti.Address;
	TextureEntry *pEntry = gTextureCache.GetTexture(&gti, true, false);
	if( pEntry ) SetCurrentTexture( 0, pEntry->pTexture,	pEntry->ti.WidthToCreate, pEntry->ti.HeightToCreate);
}

void CDaedalusRender::LoadTextureFromMemory(void *buf, DWORD left, DWORD top, DWORD width, DWORD height, DWORD pitch, DWORD format)
{
	TextureInfo gti;
	gti.Format	= g_CI.dwFormat;
	gti.Size	= g_CI.dwSize;
	gti.Palette	= 0;
	gti.TLutFmt	= G_TT_RGBA16;	//RGBA16
	gti.PalAddress = 0;
	gti.bSwapped	= FALSE;
	gti.Address	= 0;
	gti.LeftToLoad		= 0;
	gti.TopToLoad		= 0;
	gti.WidthToCreate		= width;
	gti.HeightToCreate		= height;

	gti.Pitch	= pitch;

	gti.HeightToLoad = height;
	gti.WidthToLoad = width;
	gti.pPhysicalAddress = (BYTE*)buf;

	TextureEntry *pEntry = gTextureCache.GetTexture(&gti);
	//Upto here, the texture is loaded wrong because the format is wrong

	DrawInfo info;	
	if( pEntry->pTexture->StartUpdate(&info) )
	{
		for( DWORD i=0; i<height; i++)
		{
			DWORD *psrc = (DWORD*)((BYTE*)buf+pitch*(i+top))+left;
			DWORD *pdst = (DWORD*)((BYTE*)info.lpSurface+i*info.lPitch);
			for( DWORD j=0; j<width; j++)
			{
				pdst[j]=psrc[j];
			}
		}
		pEntry->pTexture->EndUpdate(&info);
	}
	SetCurrentTexture( 0, pEntry->pTexture,	width, height);
}

void CDaedalusRender::LoadObjBGCopy(uObjBg &info)
{
	TextureInfo gti;
	gti.Format		= info.imageFmt;
	gti.Size		= info.imageSiz;
	gti.Address		= RDPSegAddr(info.imagePtr);
	gti.LeftToLoad	= 0;
	gti.TopToLoad	= 0;
	gti.Palette		= info.imagePal;

	gti.PalAddress	= (uint32)(&g_wRDPPal[0]);
	gti.bSwapped	= FALSE;
	gti.TLutFmt		= G_TT_RGBA16;	//RGBA16

	gti.WidthToCreate	= info.imageW/4;
	gti.HeightToCreate	= info.imageH/4;

	if( options.enableHacks )
	{
		if( g_CI.dwWidth == 0x200 && gti.Format == g_CI.dwFormat && gti.Size == g_CI.dwSize &&
			gti.WidthToCreate == 0x200 )
		{
			// Hack for RE2
			u32 w = *g_GraphicsInfo.VI_WIDTH_RG & 0xFFF;
			gti.HeightToCreate = (gti.WidthToCreate*gti.HeightToCreate)/w;
			gti.WidthToCreate = w;
		}
	}

	gti.Pitch	= gti.WidthToCreate << gti.Size >> 1;
	gti.Pitch	= (gti.Pitch>>3)<<3;	// Align to 8 bytes

	if( gti.Address + gti.Pitch*gti.HeightToCreate > g_dwRamSize )
	{
		TRACE0("Skip BG copy loading, memory out of bound");
		return;
	}

	gti.HeightToLoad = gti.HeightToCreate;
	gti.WidthToLoad = gti.WidthToCreate;
	gti.pPhysicalAddress = ((BYTE*)g_pu32RamBase)+gti.Address;
	TextureEntry *pEntry = gTextureCache.GetTexture(&gti);
	SetCurrentTexture(0,pEntry);

	DEBUGGER_IF_DUMP((pauseAtNext && (eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI||eventToPause == NEXT_OBJ_BG)),
	{
		TRACE0("Load Obj BG Copy:\n");
		DebuggerAppendMsg("Addr=0x%08X, W=%d, H=%d, Left=%d, Top=%d\n", 
			gti.Address, gti.WidthToCreate, gti.HeightToCreate, gti.LeftToLoad, gti.TopToLoad);
		DebuggerAppendMsg("Fmt=%s-%db, Pal=%d\n",
			pszImgFormat[gti.Format], pnImgSize[gti.Size], gti.Palette);
	}
	);
}

void CDaedalusRender::LoadFakeFrameBuffer(void)
{
	TextureInfo gti;

	gti.Format	= g_FakeFrameBufferInfo.CI_Info.dwFormat;
	gti.Size		= g_FakeFrameBufferInfo.CI_Info.dwSize;

	g_FakeFrameBufferInfo.width = g_FakeFrameBufferInfo.CI_Info.dwWidth;

	gti.Address	= RDPSegAddr(g_FakeFrameBufferInfo.CI_Info.dwAddr);
	gti.LeftToLoad		= 0;
	gti.TopToLoad		= 0;
	gti.Palette	= 0;

	gti.PalAddress = (uint32)(&g_wRDPPal[0]);
	gti.bSwapped	= FALSE;

	gti.WidthToCreate		= g_FakeFrameBufferInfo.width;
	gti.HeightToCreate	= g_FakeFrameBufferInfo.height;
	gti.TLutFmt	= G_TT_RGBA16;	//RGBA16

	gti.Pitch	= gti.WidthToCreate << (gti.Size-1);

	gti.HeightToLoad = gti.HeightToCreate;
	gti.WidthToLoad = gti.WidthToCreate;
	gti.pPhysicalAddress = ((BYTE*)g_pu32RamBase)+gti.Address;
	TextureEntry *pEntry = gTextureCache.GetTexture(&gti);
	SetCurrentTexture(0,pEntry);
}


void CDaedalusRender::WriteTriToFrameBuffer(DWORD dwV0, DWORD dwV1, DWORD dwV2)
{
	bool openGL = CDeviceBuilder::m_deviceGeneralType == OGL_DEVICE;
	InitVertex(g_ucVertexBuffer[0], dwV0, 0, false,openGL);
	InitVertex(g_ucVertexBuffer[1], dwV1, 1, false,openGL);
	InitVertex(g_ucVertexBuffer[2], dwV2, 2, false,openGL);

	float miny = min(g_ucVertexBuffer[0].y, min(g_ucVertexBuffer[1].y, g_ucVertexBuffer[2].y));
	int topmost;
	int leftmost;
	int other;

	if( g_ucVertexBuffer[0].y == miny )
	{
		topmost = 0;
		if( g_ucVertexBuffer[1].x <= g_ucVertexBuffer[2].x )
		{
			leftmost = 1;
			other = 2;
		}
		else
		{
			leftmost = 2;
			other = 1;
		}

	}
	else if( g_ucVertexBuffer[1].y == miny )
	{
		topmost = 1;
		if( g_ucVertexBuffer[0].x <= g_ucVertexBuffer[2].x )
		{
			leftmost = 0;
			other = 2;
		}
		else
		{
			leftmost = 2;
			other = 0;
		}
	}
	else
	{
		topmost = 2;
		if( g_ucVertexBuffer[0].x <= g_ucVertexBuffer[1].x )
		{
			leftmost = 0;
			other = 1;
		}
		else
		{
			leftmost = 1;
			other = 0;
		}
	}
}


void CDaedalusRender::DrawSprite2D(Sprite2DInfo &info, DWORD ucode)
{
	LoadSprite2D(info, ucode);

	info.scaleX = 1/info.scaleX;
	info.scaleY = 1/info.scaleY;

	int x0, y0, x1, y1;
	float t0, s0, t1, s1;

	if( info.flipX )
	{
		//x0 = info.px*info.scaleX + info.spritePtr->SubImageWidth*info.scaleX;
		//x1 = info.px*info.scaleX;
		x0 = info.px + int(info.spritePtr->SubImageWidth*info.scaleX);
		x1 = info.px;
	}
	else
	{
		//x0 = info.px*info.scaleX;
		//x1 = info.px*info.scaleX + info.spritePtr->SubImageWidth*info.scaleX;
		x0 = info.px;
		x1 = info.px + int(info.spritePtr->SubImageWidth*info.scaleX);
	}

	if( info.flipY )
	{
		//y0 = info.py*info.scaleY + info.spritePtr->SubImageHeight*info.scaleY;
		//y1 = info.py*info.scaleY;
		y0 = info.py + int(info.spritePtr->SubImageHeight*info.scaleY);
		y1 = info.py;
	}
	else
	{
		//y0 = info.py*info.scaleY;
		//y1 = info.py*info.scaleY + info.spritePtr->SubImageHeight*info.scaleY;
		y0 = info.py;
		y1 = info.py + int(info.spritePtr->SubImageHeight*info.scaleY);
	}

	t0 = s0 = 0;
	t1 = info.spritePtr->SubImageWidth/g_textures[0].m_fTexWidth;
	s1 = info.spritePtr->SubImageHeight/g_textures[0].m_fTexHeight;

	//InitCombinerBlenderForSimpleTextureDraw();
	InitCombinerAndBlenderMode();
	SetAddressUAllStages( 0, TEXTURE_UV_FLAG_CLAMP );
	SetAddressVAllStages( 0, TEXTURE_UV_FLAG_CLAMP );

	DaedalusColor speColor = PostProcessSpecularColor();
	DaedalusColor difColor = PostProcessDiffuseColor(0xffffffff);

	float depth = ( gRDP.otherMode.depth_source == 1 ) ? gRDP.fPrimitiveDepth : 0;
	DrawSimple2DTexture((float)x0, (float)y0, (float)x1, (float)y1, t0, s0, t1, s1, speColor, difColor, depth, 1.0f);
}


void CDaedalusRender::DrawSpriteR(uObjTxSprite &sprite, bool initCombiner, DWORD tile, DWORD left, DWORD top, DWORD width, DWORD height)	// With Rotation
{
	InitCombinerAndBlenderMode();

	float scaleX = sprite.sprite.scaleW/1024.0f;
	float scaleY = sprite.sprite.scaleH/1024.0f;

	if( width == 0 || height == 0 )
	{
		width = g_textures[tile].m_dwTileWidth;
		height = g_textures[tile].m_dwTileHeight;
	}

	RECT src = {left,top,width, height};
	float depth = ( gRDP.otherMode.depth_source == 1 ) ? depth = gRDP.fPrimitiveDepth : 0;

	float x0 = sprite.sprite.objX/4.0f;
	float y0 = sprite.sprite.objY/4.0f;
	float x1 = sprite.sprite.imageW / 32.0f / scaleX + x0;
	float y1 = sprite.sprite.imageH / 32.0f / scaleY + y0;

	if( sprite.sprite.imageFlags&1 ) { float temp = x0; x0 = x1; x1 = temp;	} // flip X 
	if( sprite.sprite.imageFlags&0x10 ) { float temp = y0; y0 = y1; y1 = temp; } // flip Y

	m_texRectTVtx[0].x = (gObjMtxReal.A*x0 + gObjMtxReal.B*y0 + gObjMtxReal.X)*windowSetting.fMultX;
	m_texRectTVtx[0].y = (gObjMtxReal.C*x0 + gObjMtxReal.D*y0 + gObjMtxReal.Y)*windowSetting.fMultY;
	m_texRectTVtx[0].z = depth;
	m_texRectTVtx[0].rhw = 1;
	m_texRectTVtx[1].x = (gObjMtxReal.A*x1 + gObjMtxReal.B*y0 + gObjMtxReal.X)*windowSetting.fMultX;
	m_texRectTVtx[1].y = (gObjMtxReal.C*x1 + gObjMtxReal.D*y0 + gObjMtxReal.Y)*windowSetting.fMultY;
	m_texRectTVtx[1].z = depth;
	m_texRectTVtx[1].rhw = 1;
	m_texRectTVtx[2].x = (gObjMtxReal.A*x1 + gObjMtxReal.B*y1 + gObjMtxReal.X)*windowSetting.fMultX;
	m_texRectTVtx[2].y = (gObjMtxReal.C*x1 + gObjMtxReal.D*y1 + gObjMtxReal.Y)*windowSetting.fMultY;
	m_texRectTVtx[2].z = depth;
	m_texRectTVtx[2].rhw = 1;
	m_texRectTVtx[3].x = (gObjMtxReal.A*x0 + gObjMtxReal.B*y1 + gObjMtxReal.X)*windowSetting.fMultX;
	m_texRectTVtx[3].y = (gObjMtxReal.C*x0 + gObjMtxReal.D*y1 + gObjMtxReal.Y)*windowSetting.fMultY;
	m_texRectTVtx[3].z = depth;
	m_texRectTVtx[3].rhw = 1;

	m_texRectTVtx[0].tcord[0].u = left/g_textures[tile].m_fTexWidth;
	m_texRectTVtx[0].tcord[0].v = top/g_textures[tile].m_fTexHeight;
	m_texRectTVtx[1].tcord[0].u = (left+width)/g_textures[tile].m_fTexWidth;
	m_texRectTVtx[1].tcord[0].v = top/g_textures[tile].m_fTexHeight;
	m_texRectTVtx[2].tcord[0].u = (left+width)/g_textures[tile].m_fTexWidth;
	m_texRectTVtx[2].tcord[0].v = (top+height)/g_textures[tile].m_fTexHeight;
	m_texRectTVtx[3].tcord[0].u = left/g_textures[tile].m_fTexWidth;
	m_texRectTVtx[3].tcord[0].v = (top+height)/g_textures[tile].m_fTexHeight;

	DaedalusColor speColor = PostProcessSpecularColor();
	DaedalusColor difColor = PostProcessDiffuseColor(0xffffffff);

	m_texRectTVtx[0].dcDiffuse = m_texRectTVtx[1].dcDiffuse 
		= m_texRectTVtx[2].dcDiffuse = m_texRectTVtx[3].dcDiffuse = difColor;
	m_texRectTVtx[0].dcSpecular = m_texRectTVtx[1].dcSpecular 
		= m_texRectTVtx[2].dcSpecular = m_texRectTVtx[3].dcSpecular = difColor;

	DrawSpriteR_Render();
}

void CDaedalusRender::DrawFrameBuffer(bool useVIreg, DWORD left, DWORD top, DWORD width, DWORD height)
{
	LoadFrameBuffer(useVIreg, left, top, width, height);

	m_pColorCombiner->InitCombinerBlenderForSimpleTextureDraw(0);

	ZBufferEnable(FALSE);
	SetZUpdate(FALSE);
	SetAlphaTestEnable(FALSE);
	m_pAlphaBlender->Disable();

	CTexture *pTexture = g_textures[0].m_pCTexture;
	if( pTexture )
	{
		if( useVIreg )
		{
			// Draw the whole frame buffer
			DrawSimple2DTexture(0, 0, windowSetting.uViWidth, windowSetting.uViHeight, 
				0, 0, 1/pTexture->m_fXScale, 1/pTexture->m_fYScale, 0xFFFFFFFF, 0xFFFFFFFF, 0, 1);
		}
		else
		{
			// Draw a small texture in frame buffer
			DrawSimple2DTexture(left, top, left+width, top+height, 
				0, 0, 1/pTexture->m_fXScale, 1/pTexture->m_fYScale, 0xFFFFFFFF, 0xFFFFFFFF, 0, 1);
		}
	}
#ifdef _DEBUG
	if( pauseAtNext && ( eventToPause == NEXT_FRAME || eventToPause == NEXT_FLUSH_TRI ) )
	{
		TRACE0("Draw Frame Buffer Img");
		debuggerPause = true;
		DebuggerPause();
	}
#endif
}

void CDaedalusRender::DrawObjBGCopy(uObjBg &info)
{
	InitCombinerAndBlenderMode();

	DWORD frameH = info.frameH;
	DWORD frameW = info.frameW;
	DWORD imageH = info.imageH;
	DWORD imageW = info.imageW;

	if( options.enableHacks )
	{
		if( g_CI.dwWidth == 0x200 && info.imageFmt == g_CI.dwFormat && info.imageSiz == g_CI.dwSize &&
			frameW == 0x800 )
		{
			// Hack for RE2
			u32 width = *g_GraphicsInfo.VI_WIDTH_RG & 0xFFF;
			imageH = frameH = (frameW/4*frameH/4)/width*4;
			imageW = frameW = width*4;
		}
	}

	float x0 = info.frameX / 4.0f;
	float y0 = info.frameY / 4.0f;
	float x1 = frameW / 4.0f + x0;
	float y1 = frameH / 4.0f + y0;

	float s0 = info.imageX / 32.0f;
	float t0 = info.imageY / 32.0f;

	float texwidth = imageW/4.0f;
	float texheight = imageH/4.0f;

	float u0 = s0/g_textures[0].m_fTexWidth;
	float v0 = t0/g_textures[0].m_fTexHeight;
	float maxu = texwidth/g_textures[0].m_fTexWidth;
	float maxv = texheight/g_textures[0].m_fTexHeight;

	float x2 = x0 + (texwidth-s0);
	float y2 = y0 + (texheight-t0);
	float u1 = (x1-x2)/g_textures[0].m_fTexWidth;
	float v1 = (y1-y2)/g_textures[0].m_fTexHeight;

	float depth = (gRDP.otherMode.depth_source == 1 ? gRDP.fPrimitiveDepth : 0.0f);

	DaedalusColor speColor = PostProcessSpecularColor();
	DaedalusColor difColor = PostProcessDiffuseColor(0xffffffff);


	if( x2 > x1 )
	{
		float s1 = (x1-x0) + s0;
		if( y2 > y1 )
		{
			float t1 = (y1-y0) + t0;
			DrawSimple2DTexture(x0, y0, x1, y1, u0, v0, 
				s1/g_textures[0].m_fTexWidth, t1/g_textures[0].m_fTexHeight, difColor, speColor, depth, 1);
		}
		else
		{
			DrawSimple2DTexture(x0, y0, x1, y2, u0, v0, 
				s1/g_textures[0].m_fTexWidth, maxv, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x0, y2, x1, y1, u0, 0, 
				s1/g_textures[0].m_fTexWidth, v1, difColor, speColor, depth, 1);
		}
	}
	else
	{
		if( y2 > y1 )
		{
			float t1 = (y1-y0) + t0;
			DrawSimple2DTexture(x0, y0, x2, y1, u0, v0, 
				maxu, t1/g_textures[0].m_fTexHeight, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x2, y0, x1, y1, 0, v0, 
				u1, t1/g_textures[0].m_fTexHeight, difColor, speColor, depth, 1);
		}
		else
		{
			DrawSimple2DTexture(x0, y0, x2, y2, u0, v0, maxu, maxv, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x2, y0, x1, y2, 0, v0, u1, maxv, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x0, y2, x2, y1, u0, 0, maxu, v1, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x2, y2, x1, y1, 0, 0, u1, v1, difColor, speColor, depth, 1);
		}
	}

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N(
		(pauseAtNext&&g_bFakeCIUpdated&&(eventToPause==NEXT_OBJ_BG||eventToPause==NEXT_FLUSH_TRI||eventToPause==NEXT_OBJ_TXT_CMD)),
		{
			TRACE0("Pause ObjBG Copy");
		}
	);
}

void CDaedalusRender::DrawObjBG1CYC(uObjScaleBg &bg, bool scaled)	//Without Ratation
{
	if( !currentRomOptions.dwEnableObjBG )
		return;

	InitCombinerAndBlenderMode();

	float depth = (gRDP.otherMode.depth_source == 1 ? gRDP.fPrimitiveDepth : 0.0f);

	float x0 = bg.frameX / 4.0f;
	float y0 = bg.frameY / 4.0f;
	float x1 = bg.frameW / 4.0f + x0;
	float y1 = bg.frameH / 4.0f + y0;

	float s0 = bg.imageX / 32.0f;
	float t0 = bg.imageY / 32.0f;

	float scaleX = bg.scaleW/1024.0f;
	float scaleY = bg.scaleH/1024.0f;

	float texwidth = bg.imageW/4.0f;
	float texheight = bg.imageH/4.0f;

	float u0 = s0/g_textures[0].m_fTexWidth;
	float v0 = t0/g_textures[0].m_fTexHeight;
	float maxu = texwidth/g_textures[0].m_fTexWidth;
	float maxv = texheight/g_textures[0].m_fTexHeight;

	float x2 = x0 + (texwidth-s0)/scaleX;
	float y2 = y0 + (texheight-t0)/scaleY;
	float u1 = (x1-x2)*scaleX/g_textures[0].m_fTexWidth;
	float v1 = (y1-y2)*scaleY/g_textures[0].m_fTexHeight;

	DaedalusColor speColor = PostProcessSpecularColor();
	DaedalusColor difColor = PostProcessDiffuseColor(0xffffffff);

	SetAlphaTestEnable(FALSE);

	if( x2 > x1 )
	{
		float s1 = (x1-x0)*scaleX + s0;
		if( y2 > y1 )
		{
			float t1 = (y1-y0)*scaleY + t0;
			DrawSimple2DTexture(x0, y0, x1, y1, u0, v0, 
				s1/g_textures[0].m_fTexWidth, t1/g_textures[0].m_fTexHeight, difColor, speColor, depth, 1);
		}
		else
		{
			DrawSimple2DTexture(x0, y0, x1, y2, u0, v0, 
				s1/g_textures[0].m_fTexWidth, maxv, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x0, y2, x1, y1, u0, 0, 
				s1/g_textures[0].m_fTexWidth, v1, difColor, speColor, depth, 1);
		}
	}
	else
	{
		if( y2 > y1 )
		{
			float t1 = (y1-y0)*scaleY + t0;
			DrawSimple2DTexture(x0, y0, x2, y1, u0, v0, 
				maxu, t1/g_textures[0].m_fTexHeight, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x2, y0, x1, y1, 0, v0, 
				u1, t1/g_textures[0].m_fTexHeight, difColor, speColor, depth, 1);
		}
		else
		{
			DrawSimple2DTexture(x0, y0, x2, y2, u0, v0, maxu, maxv, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x2, y0, x1, y2, 0, v0, u1, maxv, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x0, y2, x2, y1, u0, 0, maxu, v1, difColor, speColor, depth, 1);
			DrawSimple2DTexture(x2, y2, x1, y1, 0, 0, u1, v1, difColor, speColor, depth, 1);
		}
	}

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N(
		(pauseAtNext&&g_bFakeCIUpdated&&(eventToPause==NEXT_OBJ_BG||eventToPause==NEXT_FLUSH_TRI||eventToPause==NEXT_OBJ_TXT_CMD)),
		{
			TRACE0("Pause BG 1CYC");
		}
	);
}


void CDaedalusRender::DrawSprite(uObjTxSprite &sprite, bool rectR)	//Without Ratation
{
	InitCombinerAndBlenderMode();
	DaedalusColor speColor = PostProcessSpecularColor();
	DaedalusColor difColor = PostProcessDiffuseColor(0xffffffff);


	float objX = sprite.sprite.objX/4.0f;
	float objY = sprite.sprite.objY/4.0f;
	float width = sprite.sprite.imageW / 32.0f;
	float high = sprite.sprite.imageH / 32.0f;
	float scaleW = sprite.sprite.scaleW/1024.0f;
	float scaleH = sprite.sprite.scaleH/1024.0f;

	if( g_curRomInfo.bIncTexRectEdge )
	{
		width++;
		high++;
	}

	float x0, y0, x1, y1;
	if( rectR )
	{
		// Upper-left coordinate
		// ( X + objX / BaseScaleX, Y+objY/BaseScaleY )
		// Lower-right coordinate
		// ( X + (objX + imageW / scaleW) / BaseScaleX - 1,	Y + (objY + imageH / scaleH) / BaseScaleY - 1 )

		x0 = gObjMtxReal.X + objX/gObjMtxReal.BaseScaleX;
		y0 = gObjMtxReal.Y + objY/gObjMtxReal.BaseScaleY;
		x1 = gObjMtxReal.X + (objX + width / scaleW) / gObjMtxReal.BaseScaleX - 1;
		y1 = gObjMtxReal.Y + (objY + high / scaleH) / gObjMtxReal.BaseScaleY - 1;
	}
	else
	{
		// (objX, objY) - ( objX+imageW/scaleW-1, objY+imageH/scaleH-1)
		x0 = objX;
		y0 = objY;
		x1 = objX + width / scaleW - 1;
		y1 = objY + high / scaleH - 1;

		if( (sprite.sprite.imageFlags&1) ) // flipX
		{
			float temp = x0;
			x0 = x1;
			x1 = temp;
		}

		if( (sprite.sprite.imageFlags&0x10) ) // flipY
		{
			float temp = y0;
			y0 = y1;
			y1 = temp;
		}
	}

	float depth = (gRDP.otherMode.depth_source == 1 ? gRDP.fPrimitiveDepth : 0.0f);
	CTexture *pTexture = g_textures[0].m_pCTexture;
	DrawSimple2DTexture(x0, y0, x1, y1, 0, 0, 1/pTexture->m_fXScale, 1/pTexture->m_fYScale, 
		difColor, speColor, depth, 1);
}


void CDaedalusRender::LoadObjBG1CYC(uObjScaleBg &bg)
{
	u32 imageWidth = bg.imageW/4;
	u32 imageHeight = bg.imageH/4;

	TextureInfo gti;
	gti.Format	= bg.imageFmt;
	gti.Size		= bg.imageSiz;

	BYTE* img = (BYTE*)(g_pu8RamBase+RDPSegAddr(bg.imagePtr));
	
	uint32 palAddr = (uint32)(&g_wRDPPal[0]);
	gti.Address	= RDPSegAddr(bg.imagePtr);

	gti.LeftToLoad		= 0;
	gti.TopToLoad		= 0;

	gti.WidthToCreate		= imageWidth;
	gti.HeightToCreate		= imageHeight;

	gti.clampS = gti.clampT = 1;
	gti.maskS = gti.maskT = 0;

	gti.Palette		= bg.imagePal;
	gti.PalAddress	= palAddr;

	gti.Pitch	= imageWidth << gti.Size >> 1;
	gti.Pitch	= (gti.Pitch>>3)<<3;	// Align to 8 bytes

	if( gti.Address + gti.Pitch*gti.HeightToCreate > g_dwRamSize )
	{
		TRACE0("Skip BG 1CYC loading, memory out of bound");
		return;
	}

	gti.TLutFmt	= G_TT_RGBA16;	//RGBA16
	gti.bSwapped	= FALSE;

	gti.HeightToLoad = gti.HeightToCreate;
	gti.WidthToLoad = gti.WidthToCreate;
	gti.pPhysicalAddress = ((BYTE*)g_pu32RamBase)+gti.Address;
	TextureEntry *pEntry = gTextureCache.GetTexture(&gti,true,true);
	SetCurrentTexture(0,pEntry);

	DEBUGGER_IF_DUMP((pauseAtNext && (eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI||eventToPause == NEXT_OBJ_BG)),
		{
			TRACE0("Load Obj BG 1CYC:\n");
			DebuggerAppendMsg("Addr=0x%08X, W=%d, H=%d, Left=%d, Top=%d\n", 
				gti.Address, gti.WidthToCreate, gti.HeightToCreate, gti.LeftToLoad, gti.TopToLoad);
			DebuggerAppendMsg("Fmt=%s-%db, Pal=%d\n",
				pszImgFormat[gti.Format], pnImgSize[gti.Size], gti.Palette);
		}
	);
}

void CDaedalusRender::LoadObjSprite(uObjTxSprite &sprite, bool useTIAddr)
{
	TextureInfo gti;
	gti.Format	= sprite.sprite.imageFmt;
	gti.Size	= sprite.sprite.imageSiz;

	BYTE* img;
	if( useTIAddr )
	{
		extern SetImgInfo g_TI;
		img = (BYTE*)(g_pu8RamBase+RDPSegAddr(g_TI.dwAddr));
	}
	else
		img = (BYTE*)(g_pu8RamBase+RDPSegAddr(sprite.txtr.block.image));
	u32 palAddr = (uint32)(&g_wRDPPal[0]);

	gti.Address	= RDPSegAddr(sprite.txtr.block.image);
	gti.Address += sprite.sprite.imageAdrs<<3;
	gti.LeftToLoad		= 0;
	gti.TopToLoad		= 0;
	gti.Palette	= sprite.sprite.imagePal;
	gti.PalAddress = palAddr;

	if( sprite.txtr.block.type == G_OBJLT_TXTRBLOCK )
	{
		gti.WidthToCreate		= sprite.sprite.imageW/32;
		if( sprite.sprite.imageW >= 0x8000 )
		{
			gti.WidthToCreate = (0x10000-sprite.sprite.imageW)/32;
		}
		gti.HeightToCreate	= sprite.sprite.imageH/32;
		if( sprite.sprite.imageH >= 0x8000 )
		{
			gti.HeightToCreate	= (0x10000-sprite.sprite.imageH)/32;
		}
		gti.Pitch		= (2047/(sprite.txtr.block.tline-1)) << 3;
	}
	else if( sprite.txtr.block.type == G_OBJLT_TXTRTILE )
	{
//#define	GS_PIX2TMEM(pix, siz)	((pix)>>(4-(siz)))
//#define	GS_TT_TWIDTH(pix,siz)	((GS_PIX2TMEM((pix), (siz))<<2)-1)
//#define	GS_TT_THEIGHT(pix,siz)	(((pix)<<2)-1)

		gti.WidthToCreate		= ((sprite.txtr.tile.twidth+1)>>2)<<(4-gti.Size);
		gti.HeightToCreate	= (sprite.txtr.tile.theight+1)>>2;

		if( gti.Size == G_IM_SIZ_4b )
		{
			gti.Pitch = gti.WidthToCreate >> 1;
		}
		else
			//gti.Pitch		= (sprite.txtr.tile.twidth+1) << 3;
			gti.Pitch		= gti.WidthToCreate << (gti.Size-1);
	}

	if( gti.Address + gti.Pitch*gti.HeightToCreate > g_dwRamSize )
	{
		TRACE0("Skip Obj sprite loading, memory out of bound");
		return;
	}

	gti.TLutFmt	= G_TT_RGBA16;	//RGBA16
	gti.bSwapped	= FALSE;

	gti.HeightToLoad = gti.HeightToCreate;
	gti.WidthToLoad = gti.WidthToCreate;
		gti.pPhysicalAddress = ((BYTE*)g_pu32RamBase)+gti.Address;
	TextureEntry *pEntry = gTextureCache.GetTexture(&gti);
	SetCurrentTexture(0,pEntry);
}

