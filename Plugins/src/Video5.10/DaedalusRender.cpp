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

extern FiddledVtx * g_pVtxBase;
CDaedalusRender * CDaedalusRender::g_pRender=NULL;
extern SetImgInfo g_CI,g_DI;
extern FakeFrameBufferInfo g_FakeFrameBufferInfo;

//========================================================================
CDaedalusRender * CDaedalusRender::GetRender(void)
{
	if( CDaedalusRender::g_pRender == NULL )
	{
		ErrorMsg("g_pRender is NULL");
		exit(0);
	}
	else
		return CDaedalusRender::g_pRender;
}
bool CDaedalusRender::IsAvailable()
{
	return CDaedalusRender::g_pRender != NULL;
}

CDaedalusRender::CDaedalusRender() :
	m_fScreenViewportMultX(2.0f),
	m_fScreenViewportMultY(2.0f),
	m_bZUpdate(FALSE),
	m_bZCompare(FALSE),
	m_dwZBias(0),

	m_dwTexturePerspective(FALSE),
	m_bAlphaTestEnable(FALSE),

	m_dwAlpha(0xFF),

	m_bBlendModeValid(FALSE),
	
	m_dwMinFilter(FILTER_POINT),
	m_dwMagFilter(FILTER_POINT)
{
	int i;
	InitRenderBase();

	for( i=0; i<MAX_TEXTURES; i++ )
	{
		g_textures[i].m_lpsTexturePtr = NULL;
		g_textures[i].m_pCTexture = NULL;
		
		g_textures[i].m_dwTileWidth = 64;		// Value doesn't really matter, as tex not set
		g_textures[i].m_dwTileHeight = 64;
		g_textures[i].m_fTexWidth = 64.0f;		// Value doesn't really matter, as tex not set
		g_textures[i].m_fTexHeight = 64.0f;
		
		TileUFlags[i] = TileVFlags[i] = TEXTURE_UV_FLAG_CLAMP;
	}


	for( i=0; i<MAX_VERTS; i++)
	{
		g_dwVecFlags[i] = 0;
	}
	
	m_pColorCombiner = CDeviceBuilder::GetBuilder()->CreateColorCombiner(this);
	m_pColorCombiner->Initialize();

	m_pAlphaBlender = CDeviceBuilder::GetBuilder()->CreateAlphaBlender(this);

}

CDaedalusRender::~CDaedalusRender()
{
	CDeviceBuilder::GetBuilder()->DeleteColorCombiner();
	m_pColorCombiner = NULL;
	
	CDeviceBuilder::GetBuilder()->DeleteAlphaBlender();
	m_pAlphaBlender = NULL;
}


extern float statusBarRatio;

void CDaedalusRender::LoadSprite2D(Sprite2DInfo &info, DWORD ucode)
{
	TextureInfo gti;

	gti.Format	= info.spritePtr->SourceImageType;
	gti.Size	= info.spritePtr->SourceImageBitSize;
	
	gti.Address	= RDPSegAddr(info.spritePtr->SourceImagePointer);
	gti.Palette	= 0;
	gti.PalAddress = (uint32)(g_pu8RamBase+RDPSegAddr(info.spritePtr->TlutPointer));

	gti.WidthToCreate	= info.spritePtr->SubImageWidth;
	gti.HeightToCreate	= info.spritePtr->SubImageHeight;
	gti.LeftToLoad		= info.spritePtr->SourceImageOffsetS;
	gti.TopToLoad		= info.spritePtr->SourceImageOffsetT;
	
	gti.Pitch	= info.spritePtr->Stride << gti.Size >> 1;

	if( gti.Address + gti.Pitch*gti.HeightToCreate > g_dwRamSize )
	{
		TRACE0("Skip Sprite image decompress, memory out of bound");
		return;
	}
	
	gti.HeightToLoad = gti.HeightToCreate;
	gti.WidthToLoad = gti.WidthToCreate;

	gti.TLutFmt		= G_TT_RGBA16;	//RGBA16
	gti.bSwapped	= FALSE;

	gti.pPhysicalAddress = ((BYTE*)g_pu32RamBase)+gti.Address;
	TextureEntry *pEntry = gTextureCache.GetTexture(&gti);
	SetCurrentTexture(0,pEntry);

	DEBUGGER_IF_DUMP((pauseAtNext && (eventToPause == NEXT_OBJ_TXT_CMD||eventToPause == NEXT_FLUSH_TRI||eventToPause == NEXT_SPRITE_2D)),
	{
		TRACE0("Load Sprite 2D\n");
		DebuggerAppendMsg("Addr=0x%08X, W=%d, H=%d, Left=%d, Top=%d\n", 
			gti.Address, gti.WidthToCreate, gti.HeightToCreate, gti.LeftToLoad, gti.TopToLoad);
		DebuggerAppendMsg("Fmt=%s-%db, Pal=%d\n",
			pszImgFormat[gti.Format], pnImgSize[gti.Size], gti.Palette);
	}
	);
}

void CDaedalusRender::ResetMatrices()
{
	DaedalusMatrix mat;

	mat.m[0][1] = mat.m[0][2] = mat.m[0][3] =
	mat.m[1][0] = mat.m[1][2] = mat.m[1][3] =
	mat.m[2][0] = mat.m[2][1] = mat.m[2][3] =
	mat.m[3][0] = mat.m[3][1] = mat.m[3][2] = 0.0f;

	mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.0f;

	gRSP.projectionMtxTop = 0;
	gRSP.modelViewMtxTop = 0;
	gRSP.projectionMtxs[0] = mat;
	gRSP.modelviewMtxs[0] = mat;
}

#ifdef USING_INT_MATRIX
DaedalusMatrix dif;
void CDaedalusRender::SetProjection(const DaedalusMatrix & mat, const N64IntMatrix &intMtx, BOOL bPush, LONG nLoadReplace)
#else
void CDaedalusRender::SetProjection(const DaedalusMatrix & mat, BOOL bPush, LONG nLoadReplace) 
#endif
{
	if (bPush)
	{
		if (gRSP.projectionMtxTop >= (DAEDALUS_MATRIX_STACK-1))
		{
			TRACE0("Pushing past proj stack limits!");
		}
		else
			gRSP.projectionMtxTop++;

		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat;
#ifdef USING_INT_MATRIX
			gRSP.projectionIntMtxs[gRSP.projectionMtxTop] = intMtx;
#endif
		}
		else
		{
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat * gRSP.projectionMtxs[gRSP.projectionMtxTop-1];
#ifdef USING_INT_MATRIX
			gRSP.projectionIntMtxs[gRSP.projectionMtxTop] = intMtx * gRSP.projectionIntMtxs[gRSP.projectionMtxTop-1];
#endif
		}
		
	}
	else
	{
		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat;
#ifdef USING_INT_MATRIX
			gRSP.projectionIntMtxs[gRSP.projectionMtxTop] = intMtx;
#endif
		}
		else
		{
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat * gRSP.projectionMtxs[gRSP.projectionMtxTop];
#ifdef USING_INT_MATRIX
			gRSP.projectionIntMtxs[gRSP.projectionMtxTop] = intMtx * gRSP.projectionIntMtxs[gRSP.projectionMtxTop];
#endif
		}

	}
	
	gRSPworldProject = gRSP.modelviewMtxs[gRSP.modelViewMtxTop] * gRSP.projectionMtxs[gRSP.projectionMtxTop];
	if( status.isSSEEnabled )
	{
		D3DXMatrixTranspose(&gRSPworldProjectTransported, &gRSPworldProject);
		D3DXMatrixTranspose(&gRSPmodelViewTopTranspose, &gRSPmodelViewTop);
	}

	DumpMatrix(mat,"Set WorldView Matrix");
#ifdef USING_INT_MATRIX
	gRSPworldProjectInt = gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop] * gRSP.projectionIntMtxs[gRSP.projectionMtxTop];
#endif

	//D3DXMATRIX temp(windowSetting.vpWidthW/2,0,0,0,0,windowSetting.vpHeightW/2,0,0,0,0,0.5,0,0,0,0,1);
	//gRSPworldProject = gRSPworldProject*temp;
}

#ifdef USING_INT_MATRIX
void CDaedalusRender::SetWorldView(const DaedalusMatrix & mat, const N64IntMatrix &intMtx, BOOL bPush, LONG nLoadReplace)
#else
void CDaedalusRender::SetWorldView(const DaedalusMatrix & mat, BOOL bPush, LONG nLoadReplace)
#endif
{
	// ModelView
	if (bPush)
	{
		if (gRSP.modelViewMtxTop >= (DAEDALUS_MATRIX_STACK-1))
			DebuggerAppendMsg("Pushing past modelview stack limits! %s", nLoadReplace==RENDER_LOAD_MATRIX?"Load":"Mul");
		else
			gRSP.modelViewMtxTop++;

		// We should store the current projection matrix...
		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat;
#ifdef USING_INT_MATRIX
			gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop] = intMtx;
#endif
		}
		else			// Multiply projection matrix
		{
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat * gRSP.modelviewMtxs[gRSP.modelViewMtxTop-1];
#ifdef USING_INT_MATRIX
			gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop] = intMtx * gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop-1];
#endif
		}
	}
	else	// NoPush
	{
		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat;
#ifdef USING_INT_MATRIX
			gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop] = intMtx;
#endif
		}
		else
		{
			// Multiply projection matrix
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat * gRSP.modelviewMtxs[gRSP.modelViewMtxTop];
#ifdef USING_INT_MATRIX
			gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop] = intMtx * gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop];
#endif
		}
	}

	gRSPmodelViewTop = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];
	gRSPworldProject = gRSP.modelviewMtxs[gRSP.modelViewMtxTop] * gRSP.projectionMtxs[gRSP.projectionMtxTop];
	if( status.isSSEEnabled )
	{
		D3DXMatrixTranspose(&gRSPworldProjectTransported, &gRSPworldProject);
		D3DXMatrixTranspose(&gRSPmodelViewTopTranspose, &gRSPmodelViewTop);
	}

#ifdef USING_INT_MATRIX
	gRSP.modelViewIntTop = gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop];
	gRSPworldProjectInt = gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop] * gRSP.projectionIntMtxs[gRSP.projectionMtxTop];
#endif
	DumpMatrix(mat,"Set WorldView Matrix");
}

void CDaedalusRender::PopWorldView()
{
	if (gRSP.modelViewMtxTop > 0)
	{
		gRSP.modelViewMtxTop--;
		gRSPworldProject = gRSP.modelviewMtxs[gRSP.modelViewMtxTop] * gRSP.projectionMtxs[gRSP.projectionMtxTop];
		if( status.isSSEEnabled )
		{
			D3DXMatrixTranspose(&gRSPworldProjectTransported, &gRSPworldProject);
			D3DXMatrixTranspose(&gRSPmodelViewTopTranspose, &gRSPmodelViewTop);
		}

		gRSPmodelViewTop = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];
#ifdef USING_INT_MATRIX
		gRSP.modelViewIntTop = gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop];
		gRSPworldProjectInt = gRSP.modelviewIntMtxs[gRSP.modelViewMtxTop] * gRSP.projectionIntMtxs[gRSP.projectionMtxTop];
#endif
	}
	else
	{
#ifdef _DEBUG
		if( pauseAtNext )
			TRACE0("Popping past worldview stack limits");
#endif
	}
}


DaedalusMatrix & CDaedalusRender::GetWorldProjectMatrix(void)
{
	return gRSPworldProject;
}

#ifdef USING_INT_MATRIX
N64IntMatrix& CDaedalusRender::GetWorldProjectIntMatrix(void)
{
	return gRSPworldProjectInt;
}

//Called by Force Matrix opcode
void CDaedalusRender::SetWorldProjectMatrix(DaedalusMatrix &mtx, N64IntMatrix &intMtx)
#else
void CDaedalusRender::SetWorldProjectMatrix(DaedalusMatrix &mtx)
#endif
{
#ifdef _DEBUG
	if( pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_FLUSH_TRI || eventToPause == NEXT_MATRIX_CMD ) )
	{
		extern DList	g_dwPCStack[];
		extern int		g_dwPCindex;
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DebuggerAppendMsg("Force Matrix: pc=%08X", dwPC);
		DumpMatrix(mtx, "Force Matrix, loading new world-project matrix");
	}
#endif
	gRSPworldProject = mtx;
	if( status.isSSEEnabled )
	{
		D3DXMatrixTranspose(&gRSPworldProjectTransported, &gRSPworldProject);
	}

#ifdef USING_INT_MATRIX
	gRSPworldProjectInt = intMtx;
	gRSPworldProjectConverted = gRSPworldProjectInt.convert();
#endif
}

void CDaedalusRender::SetMux(DWORD dwMux0, DWORD dwMux1)
{
	u64 tempmux = (((u64)dwMux0) << 32) | (u64)dwMux1;
	if( m_Mux != tempmux )
	{
		m_Mux = tempmux;
		m_bBlendModeValid = FALSE;
		m_pColorCombiner->UpdateCombiner(dwMux0, dwMux1);
	}
}


void CDaedalusRender::InitCombinerAndBlenderMode()
{
	StartProfiler(PROFILE_COMBINER);
	InitOtherModes();

	if( currentRomOptions.normalAlphaBlender )
		m_pAlphaBlender->NormalAlphaBlender();
	else
		m_pAlphaBlender->InitBlenderMode();

	m_pColorCombiner->InitCombinerMode();
	StopProfiler(PROFILE_COMBINER);
}

void CDaedalusRender::RenderReset()
{
	UpdateClipRectangle();
	ResetMatrices();
	SetZBias(0);
	gRSP.numVertices = 0;
	gRSP.curTile = 0;
	gRSP.fTexScaleX = 1/32.0f;;
	gRSP.fTexScaleY = 1/32.0f;
}

bool CDaedalusRender::FillRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor)
{
	DL_PF("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%8X", nX0, nY0, nX1, nY1, dwColor);

	if( g_CI.dwSize != G_IM_SIZ_16b && currentRomOptions.N64FrameBufferEmuType == FRM_DISABLE)
		return true;
	
	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->SwapBuffer();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st FillRectangle");});
	}

#ifdef _DEBUG
	SetFillMode(DAEDALUS_FILLMODE_SOLID);
#endif

	StartProfiler(PROFILE_DRAWING);

	bool res=true;

	if( gRDP.otherMode.cycle_type == CYCTYPE_FILL && nX0 == 0 && nY0 == 0 && ((nX1==windowSetting.uViWidth && nY1==windowSetting.uViHeight)||(nX1==windowSetting.uViWidth-1 && nY1==windowSetting.uViHeight-1)) )
	{
		CGraphicsContext::g_pGraphicsContext->Clear(CLEAR_COLOR_BUFFER,dwColor);
	}
	else
	{
		BOOL m_savedZBufferFlag = gRSP.bZBufferEnabled;	// Save ZBuffer state
		ZBufferEnable( FALSE );

		m_fillRectVtx[0].x = ViewPortTranslatei_x(nX0);
		m_fillRectVtx[1].x = ViewPortTranslatei_x(nX1);
		m_fillRectVtx[0].y = ViewPortTranslatei_y(nY0);
		m_fillRectVtx[1].y = ViewPortTranslatei_y(nY1);

		InitCombinerAndBlenderMode();

		if( gRDP.otherMode.cycle_type  >= CYCTYPE_COPY )
		{
			ZBufferEnable(FALSE);
		}
		else
		{
			dwColor = PostProcessDiffuseColor(0);
		}

		float depth = (gRDP.otherMode.depth_source == 1 ? gRDP.fPrimitiveDepth : 0 );

		ApplyRDPScissor();
		res = RenderFillRect(dwColor, depth);
		ApplyScissorWithClipRatio();

		if( gRDP.otherMode.cycle_type  >= CYCTYPE_COPY )
		{
			ZBufferEnable(gRSP.bZBufferEnabled);
		}
	}

#ifdef _DEBUG
	if( options.bWinFrameMode )	SetFillMode(DAEDALUS_FILLMODE_WINFRAME );
#endif

	StopProfiler(PROFILE_DRAWING);

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FILLRECT, {DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", nX0, nY0, nX1, nY1, dwColor);
			DebuggerAppendMsg("Pause after FillRect: Color=%08X\n", dwColor);if( logTriDetails ) m_pColorCombiner->DisplayMuxString();});
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FLUSH_TRI, {DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", nX0, nY0, nX1, nY1, dwColor);
			DebuggerAppendMsg("Pause after FillRect: Color=%08X\n", dwColor);if( logTriDetails ) m_pColorCombiner->DisplayMuxString();});

	return res;
}


bool CDaedalusRender::Line3D(DWORD dwV0, DWORD dwV1, DWORD dwWidth)
{
	DL_PF("Line3D: Vtx0=%d, Vtx1=%d, Width=%d", dwV0, dwV1, dwWidth);

	StartProfiler(PROFILE_DRAWING);

	m_line3DVtx[0].z = (g_vecProjected[dwV0].z + 1.0f) * 0.5f;
	m_line3DVtx[1].z = (g_vecProjected[dwV1].z + 1.0f) * 0.5f;

	if( m_line3DVtx[0].z != m_line3DVtx[1].z )  
		return false;

	m_line3DVtx[0].x = ViewPortTranslatef_x(g_vecProjected[dwV0].x);
	m_line3DVtx[0].y = ViewPortTranslatef_y(g_vecProjected[dwV0].y);
	m_line3DVtx[0].rhw = g_vecProjected[dwV0].w;
	m_line3DVtx[0].dcDiffuse = PostProcessDiffuseColor(g_dwVecDiffuseCol[dwV0]);
	m_line3DVtx[0].dcSpecular = PostProcessSpecularColor();

	m_line3DVtx[1].x = ViewPortTranslatef_x(g_vecProjected[dwV1].x);
	m_line3DVtx[1].y = ViewPortTranslatef_y(g_vecProjected[dwV1].y);
	m_line3DVtx[1].rhw = g_vecProjected[dwV1].w;
	m_line3DVtx[1].dcDiffuse = PostProcessDiffuseColor(g_dwVecDiffuseCol[dwV1]);
	m_line3DVtx[1].dcSpecular = m_line3DVtx[0].dcSpecular;

	float width = dwWidth*0.5f+1.5f;

	if( m_line3DVtx[0].y == m_line3DVtx[1].y )
	{
		m_line3DVector[0].x = m_line3DVector[1].x = m_line3DVtx[0].x;
		m_line3DVector[2].x = m_line3DVector[3].x = m_line3DVtx[1].x;

		m_line3DVector[0].y = m_line3DVector[2].y = m_line3DVtx[0].y-width/2*windowSetting.fMultY;
		m_line3DVector[1].y = m_line3DVector[3].y = m_line3DVtx[0].y+width/2*windowSetting.fMultY;
	}
	else
	{
		m_line3DVector[0].y = m_line3DVector[1].y = m_line3DVtx[0].y;
		m_line3DVector[2].y = m_line3DVector[3].y = m_line3DVtx[1].y;

		m_line3DVector[0].x = m_line3DVector[2].x = m_line3DVtx[0].x-width/2*windowSetting.fMultX;
		m_line3DVector[1].x = m_line3DVector[3].x = m_line3DVtx[0].x+width/2*windowSetting.fMultX;
	}

	InitCombinerAndBlenderMode();

	bool res=RenderLine3D();

	StopProfiler(PROFILE_DRAWING);

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_FLUSH_TRI, {
		DebuggerAppendMsg("Pause after Line3D: v%d(%f,%f,%f), v%d(%f,%f,%f), Width=%d\n", dwV0, m_line3DVtx[0].x, m_line3DVtx[0].y, m_line3DVtx[0].z, 
			dwV1, m_line3DVtx[1].x, m_line3DVtx[1].y, m_line3DVtx[1].z, dwWidth);
	});

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_OBJ_TXT_CMD, {
		DebuggerAppendMsg("Pause after Line3D: v%d(%f,%f,%f), v%d(%f,%f,%f), Width=%d\n", dwV0, m_line3DVtx[0].x, m_line3DVtx[0].y, m_line3DVtx[0].z, 
			dwV1, m_line3DVtx[1].x, m_line3DVtx[1].y, m_line3DVtx[1].z, dwWidth);
	});

	return res;
}

bool CDaedalusRender::RemapTextureCoordinate
	(float t0, float t1, u32 tileWidth, u32 mask, float textureWidth, float &u0, float &u1)
{
	int s0 = (int)t0;
	int s1 = (int)t1;
	int width = mask>0 ? (1<<mask) : tileWidth;
	if( width == 0 ) return false;

	int divs0 = s0/width; if( divs0*width > s0 )	divs0--;
	int divs1 = s1/width; if( divs1*width > s1 )	divs1--;

	if( divs0 == divs1 )
	{
		s0 -= divs0*width;
		s1 -= divs1*width;
		//if( s0 > s1 )	
		//	s0++;
		//else if( s1 > s0 )	
		//	s1++;
		u0 = s0/textureWidth;
		u1 = s1/textureWidth;

		return true;
	}
	else if( divs0+1 == divs1 && s0%width==0 && s1%width == 0 )
	{
		u0 = 0;
		u1 = tileWidth/textureWidth;
		return true;
	}
	else if( divs0 == divs1+1 && s0%width==0 && s1%width == 0 )
	{
		u1 = 0;
		u0 = tileWidth/textureWidth;
		return true;
	}
	else
	{
		//if( s0 > s1 )	
		//{
			//s0++;
		//	u0 = s0/textureWidth;
		//}
		//else if( s1 > s0 )	
		//{
			//s1++;
		//	u1 = s1/textureWidth;
		//}

		return false;
	}
}

bool CDaedalusRender::TexRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, float fS0, float fT0, float fScaleS, float fScaleT, bool colorFlag, DWORD diffuseColor)
{
	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->SwapBuffer();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st textRect");});
	}

	SetupTextures();

	if( g_bUsingFakeCI && g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_8b )	
	{
		return true;
	}

	if( !IsTextureEnabled() &&  gRDP.otherMode.cycle_type  != CYCTYPE_COPY )
	{
		FillRect(nX0, nY0, nX1, nY1, gRDP.primitiveColor);
		return true;
	}

	DL_PF("TexRect: X0=%d, Y0=%d, X1=%d, Y1=%d,\n\t\tfS0=%f, fT0=%f, ScaleS=%f, ScaleT=%f ",
		nX0, nY0, nX1, nY1, fS0, fT0, fScaleS, fScaleT);

	StartProfiler(PROFILE_DRAWING);

	if( options.enableHacks )
	{
		// Goldeneye HACK
		if( options.enableHacks && nY1 - nY0 < 2 ) nY1 = nY1+2;
	}

	// Scale to Actual texture coords
	// The two cases are to handle the oversized textures hack on voodoos

	InitCombinerAndBlenderMode();
	

	if( gRDP.otherMode.cycle_type  >= CYCTYPE_COPY )
	{
		ZBufferEnable(FALSE);
	}

	BOOL accurate = currentRomOptions.accurateTextureMapping;

	CTexture *surf = g_textures[gRSP.curTile].m_pCTexture;
	DaedalusRenderTexture &tex0 = g_textures[gRSP.curTile];
	Tile &tile0 = gRDP.tiles[gRSP.curTile];

	float widthDiv = tex0.m_fTexWidth;
	float heightDiv = tex0.m_fTexHeight;
	float t0u0 = (fS0 * tile0.fShiftScaleS-tile0.hilite_sl);
	float t0u1 = t0u0 + (fScaleS * (nX1 - nX0))*tile0.fShiftScaleS;

	if( status.UseLargerTile[0] )
	{
		m_texRectTex1UV[0].u = (t0u0+status.LargerTileRealLeft[0])/widthDiv;
		m_texRectTex1UV[1].u = (t0u1+status.LargerTileRealLeft[0])/widthDiv;
	}
	else
	{
		m_texRectTex1UV[0].u = t0u0/widthDiv;
		m_texRectTex1UV[1].u = t0u1/widthDiv;
		//if( accurate && !tile0.bMirrorS && RemapTextureCoordinate(t0u0, t0u1, tex0.m_dwTileWidth, tile0.dwMaskS, widthDiv, m_texRectTex1UV[0].u, m_texRectTex1UV[1].u) )
		//	SetTextureUFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile);
	}

	float t0v0 = (fT0 * tile0.fShiftScaleT-tile0.hilite_tl);
	float t0v1 = t0v0 + (fScaleT * (nY1 - nY0))*tile0.fShiftScaleT;

	m_texRectTex1UV[0].v = t0v0/heightDiv;
	m_texRectTex1UV[1].v = t0v1/heightDiv;
	//if( accurate && !tile0.bMirrorT && RemapTextureCoordinate(t0v0, t0v1, tex0.m_dwTileHeight, tile0.dwMaskT, heightDiv, m_texRectTex1UV[0].v, m_texRectTex1UV[1].v) )
	//	SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile);
	
	DaedalusColor speColor = PostProcessSpecularColor();
	DaedalusColor difColor;
	if( colorFlag )
		difColor = PostProcessDiffuseColor(diffuseColor);
	else
		difColor = PostProcessDiffuseColor(0);

	if( g_curRomInfo.bIncTexRectEdge )
	{
		nX1++;
		nY1++;
	}
	
	m_texRectTVtx[0].x = ViewPortTranslatei_x(nX0);
	m_texRectTVtx[0].y = ViewPortTranslatei_y(nY0);
	m_texRectTVtx[0].dcDiffuse = difColor;
	m_texRectTVtx[0].dcSpecular = speColor;

	m_texRectTVtx[1].x = ViewPortTranslatei_x(nX1);
	m_texRectTVtx[1].y = ViewPortTranslatei_y(nY0);
	m_texRectTVtx[1].dcDiffuse = difColor;
	m_texRectTVtx[1].dcSpecular = speColor;

	m_texRectTVtx[2].x = ViewPortTranslatei_x(nX1);
	m_texRectTVtx[2].y = ViewPortTranslatei_y(nY1);
	m_texRectTVtx[2].dcDiffuse = difColor;
	m_texRectTVtx[2].dcSpecular = speColor;

	m_texRectTVtx[3].x = ViewPortTranslatei_x(nX0);
	m_texRectTVtx[3].y = ViewPortTranslatei_y(nY1);
	m_texRectTVtx[3].dcDiffuse = difColor;
	m_texRectTVtx[3].dcSpecular = speColor;

	float depth = (gRDP.otherMode.depth_source == 1 ? gRDP.fPrimitiveDepth : 0 );

	m_texRectTVtx[0].z = m_texRectTVtx[1].z = m_texRectTVtx[2].z = m_texRectTVtx[3].z = depth;
	m_texRectTVtx[0].rhw = m_texRectTVtx[1].rhw = m_texRectTVtx[2].rhw = m_texRectTVtx[3].rhw = 1;

	if( IsTexel1Enable() )
	{
		surf = g_textures[(gRSP.curTile+1)&7].m_pCTexture;
		DaedalusRenderTexture &tex1 = g_textures[(gRSP.curTile+1)&7];
		Tile &tile1 = gRDP.tiles[(gRSP.curTile+1)&7];

		widthDiv = tex1.m_fTexWidth;
		heightDiv = tex1.m_fTexHeight;
		//if( tile1.dwMaskS == 0 )	widthDiv = tile1.dwWidth;
		//if( tile1.dwMaskT == 0 )	heightDiv = tile1.dwHeight;

		float t0u0 = fS0 * tile1.fShiftScaleS -tile1.hilite_sl;
		float t0v0 = fT0 * tile1.fShiftScaleT -tile1.hilite_tl;
		float t0u1 = t0u0 + (fScaleS * (nX1 - nX0))*tile1.fShiftScaleS;
		float t0v1 = t0v0 + (fScaleT * (nY1 - nY0))*tile1.fShiftScaleT;

		if( status.UseLargerTile[1] )
		{
			m_texRectTex2UV[0].u = (t0u0+status.LargerTileRealLeft[1])/widthDiv;
			m_texRectTex2UV[1].u = (t0u1+status.LargerTileRealLeft[1])/widthDiv;
		}
		else
		{
			m_texRectTex2UV[0].u = t0u0/widthDiv;
			m_texRectTex2UV[1].u = t0u1/widthDiv;
			//if( accurate && !tile1.bMirrorS && RemapTextureCoordinate(t0u0, t0u1, tex1.m_dwTileWidth, tile1.dwMaskS, widthDiv, m_texRectTex2UV[0].u, m_texRectTex2UV[1].u) )
			//	SetTextureUFlag(TEXTURE_UV_FLAG_CLAMP, (gRSP.curTile+1)&7);
		}

		m_texRectTex2UV[0].v = t0v0/heightDiv;
		m_texRectTex2UV[1].v = t0v1/heightDiv;

		//if( accurate && !tile1.bMirrorT && RemapTextureCoordinate(t0v0, t0v1, tex1.m_dwTileHeight, tile1.dwMaskT, heightDiv, m_texRectTex2UV[0].v, m_texRectTex2UV[1].v) )
		//	SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, (gRSP.curTile+1)&7);

		SetVertexTextureUVCoord(m_texRectTVtx[0], m_texRectTex1UV[0].u, m_texRectTex1UV[0].v, m_texRectTex2UV[0].u, m_texRectTex2UV[0].v);
		SetVertexTextureUVCoord(m_texRectTVtx[1], m_texRectTex1UV[1].u, m_texRectTex1UV[0].v, m_texRectTex2UV[1].u, m_texRectTex2UV[0].v);
		SetVertexTextureUVCoord(m_texRectTVtx[2], m_texRectTex1UV[1].u, m_texRectTex1UV[1].v, m_texRectTex2UV[1].u, m_texRectTex2UV[1].v);
		SetVertexTextureUVCoord(m_texRectTVtx[3], m_texRectTex1UV[0].u, m_texRectTex1UV[1].v, m_texRectTex2UV[0].u, m_texRectTex2UV[1].v);
	}
	else
	{
		SetVertexTextureUVCoord(m_texRectTVtx[0], m_texRectTex1UV[0].u, m_texRectTex1UV[0].v);
		SetVertexTextureUVCoord(m_texRectTVtx[1], m_texRectTex1UV[1].u, m_texRectTex1UV[0].v);
		SetVertexTextureUVCoord(m_texRectTVtx[2], m_texRectTex1UV[1].u, m_texRectTex1UV[1].v);
		SetVertexTextureUVCoord(m_texRectTVtx[3], m_texRectTex1UV[0].u, m_texRectTex1UV[1].v);
	}


	bool res;
	if( TileUFlags[gRSP.curTile]==TEXTURE_UV_FLAG_CLAMP && TileVFlags[gRSP.curTile]==TEXTURE_UV_FLAG_CLAMP )
	{
		DaedalusTextureFilter dwFilter = m_dwMagFilter;
		m_dwMagFilter = m_dwMinFilter = FILTER_LINEAR;
		ApplyTextureFilter();
		ApplyRDPScissor();
		res = RenderTexRect();
		ApplyScissorWithClipRatio();
		m_dwMagFilter = m_dwMinFilter = dwFilter;
		ApplyTextureFilter();
	}
	else if( fScaleS >= 1 && fScaleT >= 1)
	{
		DaedalusTextureFilter dwFilter = m_dwMagFilter;
		m_dwMagFilter = m_dwMinFilter = FILTER_POINT;
		ApplyTextureFilter();
		ApplyRDPScissor();
		res = RenderTexRect();
		ApplyScissorWithClipRatio();
		m_dwMagFilter = m_dwMinFilter = dwFilter = FILTER_LINEAR;
		ApplyTextureFilter();
	}
	else
	{
		ApplyRDPScissor();
		res = RenderTexRect();
		ApplyScissorWithClipRatio();
	}

	if( gRDP.otherMode.cycle_type  >= CYCTYPE_COPY )
	{
		ZBufferEnable(gRSP.bZBufferEnabled);
	}


	StopProfiler(PROFILE_DRAWING);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_FLUSH_TRI || eventToPause == NEXT_TEXTRECT), {
		DebuggerAppendMsg("TexRect: tile=%d, X0=%d, Y0=%d, X1=%d, Y1=%d,\nfS0=%f, fT0=%f, ScaleS=%f, ScaleT=%f\n",
			gRSP.curTile, nX0, nY0, nX1, nY1, fS0, fT0, fScaleS, fScaleT);
		DebuggerAppendMsg("       : x0=%f, y0=%f, x1=%f, y1=%f\n",	m_texRectTVtx[0].x, m_texRectTVtx[0].y, m_texRectTVtx[1].x, m_texRectTVtx[1].y);
		DebuggerAppendMsg("   Tex0: u0=%f, v0=%f, u1=%f, v1=%f\n",	m_texRectTex1UV[0].u, m_texRectTex1UV[0].v, m_texRectTex1UV[1].u, m_texRectTex1UV[1].v);
		if( IsTexel1Enable() )
		{
			DebuggerAppendMsg("   Tex1: u0=%f, v0=%f, u1=%f, v1=%f\n",	m_texRectTex2UV[0].u, m_texRectTex2UV[0].v, m_texRectTex2UV[1].u, m_texRectTex2UV[1].v);
		}
		DebuggerAppendMsg("color=%08X, %08X\n",	m_texRectTVtx[0].dcDiffuse, m_texRectTVtx[0].dcSpecular);
		DebuggerAppendMsg("Pause after TexRect\n");
		if( logTriDetails ) m_pColorCombiner->DisplayMuxString();
	});

	StopProfiler(PROFILE_DRAWING);

	return res;
}


bool CDaedalusRender::TexRectFlip(LONG nX0, LONG nY0, LONG nX1, LONG nY1, float fS0, float fT0, float fS1, float fT1)
{
	DL_PF("TexRectFlip: X0=%d, Y0=%d, X1=%d, Y1=%d,\n\t\tfS0=%f, fT0=%f, fS1=%f, fT1=%f ",
			nX0, nY0, nX1, nY1, fS0, fT0, fS1, fT1);

	StartProfiler(PROFILE_DRAWING);

	SetupTextures();

	// Save ZBuffer state
	m_savedZBufferFlag = gRSP.bZBufferEnabled;
	if( gRDP.otherMode.depth_source == 0 )	ZBufferEnable( FALSE );

	float widthDiv = g_textures[gRSP.curTile].m_fTexWidth;
	float heightDiv = g_textures[gRSP.curTile].m_fTexHeight;

	Tile &tile0 = gRDP.tiles[gRSP.curTile];

	float t0u0 = fS0 / widthDiv;
	float t0v0 = fT0 / heightDiv;

	float t0u1 = (fS1 - fS0)/ widthDiv + t0u0;
	float t0v1 = (fT1 - fT0)/ heightDiv + t0v0;

	float depth = (gRDP.otherMode.depth_source == 1 ? gRDP.fPrimitiveDepth : 0 );

	if( t0u0 >= 0 && t0u1 <= 1 && t0u1 >= t0u0 ) SetTextureUFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile);
	if( t0v0 >= 0 && t0v1 <= 1 && t0v1 >= t0v0 ) SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile);

	InitCombinerAndBlenderMode();

	DaedalusColor speColor = PostProcessSpecularColor();
	DaedalusColor difColor = PostProcessDiffuseColor(gRDP.primitiveColor);

	// Same as TexRect, but with texcoords 0,2 swapped
	m_texRectTVtx[0].x = ViewPortTranslatei_x(nX0);
	m_texRectTVtx[0].y = ViewPortTranslatei_y(nY0);
	m_texRectTVtx[0].dcDiffuse = difColor;
	m_texRectTVtx[0].dcSpecular = speColor;

	m_texRectTVtx[1].x = ViewPortTranslatei_x(nX1);
	m_texRectTVtx[1].y = ViewPortTranslatei_y(nY0);
	m_texRectTVtx[1].dcDiffuse = difColor;
	m_texRectTVtx[1].dcSpecular = speColor;

	m_texRectTVtx[2].x = ViewPortTranslatei_x(nX1);
	m_texRectTVtx[2].y = ViewPortTranslatei_y(nY1);
	m_texRectTVtx[2].dcDiffuse = difColor;
	m_texRectTVtx[2].dcSpecular = speColor;

	m_texRectTVtx[3].x = ViewPortTranslatei_x(nX0);
	m_texRectTVtx[3].y = ViewPortTranslatei_y(nY1);
	m_texRectTVtx[3].dcDiffuse = difColor;
	m_texRectTVtx[3].dcSpecular = speColor;

	m_texRectTVtx[0].z = m_texRectTVtx[1].z = m_texRectTVtx[2].z = m_texRectTVtx[3].z = depth;
	m_texRectTVtx[0].rhw = m_texRectTVtx[1].rhw = m_texRectTVtx[2].rhw = m_texRectTVtx[3].rhw = 1.0f;
	
	SetVertexTextureUVCoord(m_texRectTVtx[0], t0u0, t0v0);
	SetVertexTextureUVCoord(m_texRectTVtx[1], t0u0, t0v1);
	SetVertexTextureUVCoord(m_texRectTVtx[2], t0u1, t0v1);
	SetVertexTextureUVCoord(m_texRectTVtx[3], t0u1, t0v0);

	bool res = RenderTexRect();

	// Restore state
	ZBufferEnable( m_savedZBufferFlag );

	StopProfiler(PROFILE_DRAWING);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_FLUSH_TRI || eventToPause == NEXT_TEXTRECT), {
		DebuggerAppendMsg("TexRectFlip: tile=%d, X0=%d, Y0=%d, X1=%d, Y1=%d,\nfS0=%f, fT0=%f, nfS1=%f, fT1=%f\n",
			gRSP.curTile, nX0, nY0, nX1, nY1, fS0, fT0, fS1, fT1);
		DebuggerAppendMsg("       : x0=%f, y0=%f, x1=%f, y1=%f\n",	m_texRectTVtx[0].x, m_texRectTVtx[0].y, m_texRectTVtx[2].x, m_texRectTVtx[2].y);
		DebuggerAppendMsg("   Tex0: u0=%f, v0=%f, u1=%f, v1=%f\n",	m_texRectTVtx[0].tcord[0].u, m_texRectTVtx[0].tcord[0].v, m_texRectTVtx[2].tcord[0].u, m_texRectTVtx[2].tcord[0].v);
		TRACE0("Pause after TexRectFlip\n");
		if( logTriDetails ) m_pColorCombiner->DisplayMuxString();
	});

	return res;
}


void CDaedalusRender::StartDrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, DaedalusColor dif, DaedalusColor spe, float z, float rhw)
{
	m_texRectTVtx[0].x = ViewPortTranslatei_x(x0);	// << Error here, shouldn't divid by 4
	m_texRectTVtx[0].y = ViewPortTranslatei_y(y0);
	m_texRectTVtx[0].dcDiffuse = dif;
	m_texRectTVtx[0].dcSpecular = spe;
	m_texRectTVtx[0].tcord[0].u = u0;
	m_texRectTVtx[0].tcord[0].v = v0;


	m_texRectTVtx[1].x = ViewPortTranslatei_x(x1);
	m_texRectTVtx[1].y = ViewPortTranslatei_y(y0);
	m_texRectTVtx[1].dcDiffuse = dif;
	m_texRectTVtx[1].dcSpecular = spe;
	m_texRectTVtx[1].tcord[0].u = u1;
	m_texRectTVtx[1].tcord[0].v = v0;

	m_texRectTVtx[2].x = ViewPortTranslatei_x(x1);
	m_texRectTVtx[2].y = ViewPortTranslatei_y(y1);
	m_texRectTVtx[2].dcDiffuse = dif;
	m_texRectTVtx[2].dcSpecular = spe;
	m_texRectTVtx[2].tcord[0].u = u1;
	m_texRectTVtx[2].tcord[0].v = v1;

	m_texRectTVtx[3].x = ViewPortTranslatei_x(x0);
	m_texRectTVtx[3].y = ViewPortTranslatei_y(y1);
	m_texRectTVtx[3].dcDiffuse = dif;
	m_texRectTVtx[3].dcSpecular = spe;
	m_texRectTVtx[3].tcord[0].u = u0;
	m_texRectTVtx[3].tcord[0].v = v1;

	m_texRectTVtx[0].z = m_texRectTVtx[1].z = m_texRectTVtx[2].z = m_texRectTVtx[3].z = z;
	m_texRectTVtx[0].rhw = m_texRectTVtx[1].rhw = m_texRectTVtx[2].rhw = m_texRectTVtx[3].rhw = rhw;
}

void CDaedalusRender::StartDrawSimpleRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor, float depth, float rhw)
{
	m_simpleRectVtx[0].x = ViewPortTranslatei_x(nX0);
	m_simpleRectVtx[1].x = ViewPortTranslatei_x(nX1);
	m_simpleRectVtx[0].y = ViewPortTranslatei_y(nY0);
	m_simpleRectVtx[1].y = ViewPortTranslatei_y(nY1);
}

void CDaedalusRender::SetAddressUAllStages(DWORD dwTile, TextureUVFlag dwFlag)
{
}

void CDaedalusRender::SetAddressVAllStages(DWORD dwTile, TextureUVFlag dwFlag)
{
}

void CDaedalusRender::SetAllTexelRepeatFlag()
{
	if( IsTextureEnabled() )
	{
		if( IsTexel0Enable() || gRDP.otherMode.cycle_type  == CYCTYPE_COPY )
			SetTexelRepeatFlags(gRSP.curTile);
		if( IsTexel1Enable() )
			SetTexelRepeatFlags((gRSP.curTile+1)&7);
	}
}


void CDaedalusRender::SetTexelRepeatFlags(DWORD dwTile)
{

	if( gRDP.tiles[dwTile].dwMaskS == 0 || gRDP.tiles[dwTile].bClampS )
	{
		if( gRDP.otherMode.cycle_type  >= CYCTYPE_COPY )
			SetTextureUFlag(TEXTURE_UV_FLAG_WRAP, dwTile);	// Can not clamp in COPY/FILL mode
		else
			SetTextureUFlag(TEXTURE_UV_FLAG_CLAMP, dwTile);
	}
	else if (gRDP.tiles[dwTile].bMirrorS )
		SetTextureUFlag(TEXTURE_UV_FLAG_MIRROR, dwTile);
	else								
		SetTextureUFlag(TEXTURE_UV_FLAG_WRAP, dwTile);
	
	if( gRDP.tiles[dwTile].dwMaskT == 0 || gRDP.tiles[dwTile].bClampT)
	{
		if( gRDP.otherMode.cycle_type  >= CYCTYPE_COPY )
			SetTextureVFlag(TEXTURE_UV_FLAG_WRAP, dwTile);	// Can not clamp in COPY/FILL mode
		else
			SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, dwTile);
	}
	else if (gRDP.tiles[dwTile].bMirrorT )
		SetTextureVFlag(TEXTURE_UV_FLAG_MIRROR, dwTile);
	else								
		SetTextureVFlag(TEXTURE_UV_FLAG_WRAP, dwTile);
}

void CDaedalusRender::Initialize(void)
{
	InvalidateDeviceObjects();
	RestoreDeviceObjects();
}


void myVec3Transform(float *vecout, float *vecin, float* m)
{
	float w = m[3]*vecin[0]+m[7]*vecin[1]+m[11]*vecin[2]+m[15];
	vecout[0] = (m[0]*vecin[0]+m[4]*vecin[1]+m[8]*vecin[2]+m[12])/w;
	vecout[1] = (m[1]*vecin[0]+m[5]*vecin[1]+m[9]*vecin[2]+m[13])/w;
	vecout[2] = (m[2]*vecin[0]+m[6]*vecin[1]+m[10]*vecin[2]+m[14])/w;
}

void CDaedalusRender::SetTextureEnableAndScale(int dwTile, bool bEnable, float fScaleX, float fScaleY)
{
	gRSP.bTextureEnabled = bEnable;

	if( bEnable )
	{
		if( gRSP.curTile != dwTile )
			gRDP.textureIsChanged = true;
		gRSP.curTile = dwTile;

		gRSP.fTexScaleX = fScaleX;
		gRSP.fTexScaleY = fScaleY;

		if( fScaleX == 0 || fScaleY == 0 )
		{
			gRSP.fTexScaleX = 1/32.0f;;
			gRSP.fTexScaleY = 1/32.0f;
		}
	}
}

void CDaedalusRender::SetFogFlagForNegativeW()
{
	if( !gRSP.bFogEnabled )	return;

	m_bFogStateSave = gRSP.bFogEnabled;

	BOOL flag=gRSP.bFogEnabled;
	
	for (uint32 i = 0; i < gRSP.numVertices; i++) 
	{
		if( g_ucVertexBuffer[i].rhw < 0 )
			flag = FALSE;
	}

	TurnFogOnOff(flag);
}

void CDaedalusRender::RestoreFogFlag()
{
	if( !gRSP.bFogEnabled )	return;
	TurnFogOnOff(m_bFogStateSave);
}


DWORD GammaCorrection(DWORD color)
{
/*	if( options.gamma_correction == 0 )	return color;
	BYTE* p = (BYTE*)&color;
	//p[0] = g_gammaValues[p[0]];	//a
	p[0] = p[0];
	p[1] = g_gammaValues[p[1]];	//r
	p[2] = g_gammaValues[p[2]];	//g
	p[3] = g_gammaValues[p[3]];	//b */
	return color;
}

void CDaedalusRender::SetViewport(int nLeft, int nTop, int nRight, int nBottom, int maxZ)
{
	static float MultX=0, MultY=0;

	if( gRSP.nVPLeftN == nLeft && gRSP.nVPTopN == nTop &&
		gRSP.nVPRightN == nRight && gRSP.nVPBottomN == nBottom &&
		MultX==windowSetting.fMultX && MultY==windowSetting.fMultY)
	{
		// no changes
		return;
	}

	MultX=windowSetting.fMultX;
	MultY=windowSetting.fMultY;

	gRSP.maxZ = maxZ;
	gRSP.nVPLeftN = nLeft;
	gRSP.nVPTopN = nTop;
	gRSP.nVPRightN = nRight;
	gRSP.nVPBottomN = nBottom;
	gRSP.nVPWidthN = nRight - nLeft + 1;
	gRSP.nVPHeightN = nBottom - nTop + 1;

	UpdateClipRectangle();
	SetViewportRender();

	DL_PF("SetViewport (%d,%d - %d,%d)",gRSP.nVPLeftN, gRSP.nVPTopN, gRSP.nVPRightN, gRSP.nVPBottomN);
}


bool CDaedalusRender::FlushTris()
{
	DEBUGGER_ONLY_IF( (!debuggerEnableZBuffer), {ZBufferEnable( FALSE );} );

	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->SwapBuffer();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st triangle");});
	}

	// Hack for Pilotwings 64 (U) [!].v64
	static bool skipNext=false;
	if( g_CI.dwAddr == g_DI.dwAddr && gRDP.otherMode.z_cmp+gRDP.otherMode.z_upd > 0 )
	{
		TRACE0("Warning: using Flushtris to write Zbuffer" );
		gRSP.numVertices = 0;
		skipNext = true;
		return true;
	}
	else if( skipNext )
	{
		skipNext = false;
		gRSP.numVertices = 0;
		return true;
	}

	/*
	if( g_bUsingFakeCI && g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_8b )	
	{
		gRSP.numVertices = 0;
		return true;
	}
	*/

	if (gRSP.numVertices == 0)	return true;

	if( !gRDP.bFogEnableInBlender && gRSP.bFogEnabled )
	{
		TurnFogOnOff(FALSE);
	}


	for( u32 t=0; t<2; t++ )
	{
		// This will get rid of the thin black lines
		if( t==0 && !(m_pColorCombiner->m_bTex0Enabled) ) continue;
		if( t==1 && !(m_pColorCombiner->m_bTex1Enabled) ) break;

		bool clampS=true;
		bool clampT=true;
		u32 i;
		for( i=0; i<gRSP.numVertices; i++ )
		{
			if( g_ucVertexBuffer[i].tcord[t].u > 1.0 || g_ucVertexBuffer[i].tcord[t].u < 0.0  )
			{
				clampS = false;
				break;
			}
		}

		for( i=0; i<gRSP.numVertices; i++ )
		{
			if( g_ucVertexBuffer[i].tcord[t].v > 1.0 || g_ucVertexBuffer[i].tcord[t].v < 0.0  )
			{
				clampT = false;
				break;
			}
		}

		if( clampS )
		{
			SetTextureUFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile+t);
		}
		if( clampT )
		{
			SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile+t);
		}
	}


	bool res = RenderFlushTris();

	DL_PF("FlushTris: Draw %d Triangles", gRSP.numVertices/3);
	
	gRSP.numVertices = 0;	// Reset index

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_FLUSH_TRI, {
		TRACE0("Pause after FlushTris\n");
		if( logTriDetails ) m_pColorCombiner->DisplayMuxString();
	});

	if( !gRDP.bFogEnableInBlender && gRSP.bFogEnabled )
	{
		TurnFogOnOff(TRUE);
	}

	return res;
}

#ifdef _DEBUG
void CDaedalusRender::SaveTextureToFile(int tex, bool alphaOnly)
{
	CTexture *pTexture = g_textures[tex].m_pCTexture;
	if( pTexture == NULL )
	{
		TRACE0("Can't dump null texture");
		return;
	}

	DrawInfo di;
	pTexture->StartUpdate(&di);
	FILE *pfile = fopen("\\texture.txt", "wt");
	if( pfile == NULL )
	{
		TRACE0("Error to open file");
		return;
	}
	DWORD *p = (DWORD*)(di.lpSurface);
	for( DWORD i=0; i<di.dwHeight; i++ )
	{
		for( DWORD j=0; j<di.dwWidth; j++ )
		{
			fprintf(pfile, "%08X, ", p[i*di.dwCreatedWidth+j]);
		}
		fprintf(pfile, "\n");
	}
	fclose(pfile);
	pTexture->EndUpdate(&di);
}
#endif

void CDaedalusRender::SetVertexTextureUVCoord(TLITVERTEX &v, float fTex0S, float fTex0T)
{
	v.tcord[0].u = fTex0S;
	v.tcord[0].v = fTex0T;
}
void CDaedalusRender::SetVertexTextureUVCoord(TLITVERTEX &v, float fTex0S, float fTex0T, float fTex1S, float fTex1T)
{
	v.tcord[0].u = fTex0S;
	v.tcord[0].v = fTex0T;

	v.tcord[1].u = fTex1S;
	v.tcord[1].v = fTex1T;
}

void CDaedalusRender::SetClipRatio(DWORD type, DWORD dwCmd1)
{
	bool modified = false;
	switch(type)
	{
	case G_MWO_CLIP_RNX:
		DL_PF("    G_MW_CLIP  NegX: %d", (LONG)(SHORT)dwCmd1);
		if( gRSP.clip_ratio_negx != (SHORT)dwCmd1 )
		{
			gRSP.clip_ratio_negx = (SHORT)dwCmd1;
			modified = true;
		}
		break;
	case G_MWO_CLIP_RNY:
		DL_PF("    G_MW_CLIP  NegY: %d", (LONG)(SHORT)dwCmd1);
		if( gRSP.clip_ratio_negy != (SHORT)dwCmd1 )
		{
			gRSP.clip_ratio_negy = (SHORT)dwCmd1;
			modified = true;
		}
		break;
	case G_MWO_CLIP_RPX:
		DL_PF("    G_MW_CLIP  PosX: %d", (LONG)(SHORT)dwCmd1);
		if( gRSP.clip_ratio_posx != -(SHORT)dwCmd1 )
		{
			gRSP.clip_ratio_posx = -(SHORT)dwCmd1;
			modified = true;
		}
		break;
	case G_MWO_CLIP_RPY:
		DL_PF("    G_MW_CLIP  PosY: %d", (LONG)(SHORT)dwCmd1);
		if( gRSP.clip_ratio_posy != -(SHORT)dwCmd1 )
		{
			gRSP.clip_ratio_posy = -(SHORT)dwCmd1;
			modified = true;
		}
		break;
	}

	if( modified )
	{
		UpdateClipRectangle();
	}

}
void CDaedalusRender::UpdateClipRectangle()
{
	windowSetting.vpLeftW = int(gRSP.nVPLeftN * windowSetting.fMultX);
	windowSetting.vpTopW = int(gRSP.nVPTopN  * windowSetting.fMultY);
	windowSetting.vpRightW = int(gRSP.nVPRightN* windowSetting.fMultX);
	windowSetting.vpBottomW = int(gRSP.nVPBottomN* windowSetting.fMultY);
	windowSetting.vpWidthW = int((gRSP.nVPRightN - gRSP.nVPLeftN + 1) * windowSetting.fMultX);
	windowSetting.vpHeightW = int((gRSP.nVPBottomN - gRSP.nVPTopN + 1) * windowSetting.fMultY);

	gRSP.vtxXMul = windowSetting.vpWidthW/2.0f;
	gRSP.vtxXAdd = gRSP.vtxXMul + windowSetting.vpLeftW;
	gRSP.vtxYMul = -windowSetting.vpHeightW/2.0f;
	gRSP.vtxYAdd = windowSetting.vpHeightW/2.0f + windowSetting.vpTopW+toolbarHeightToUse;

	// Update clip rectangle by setting scissor

	int halfx = gRSP.nVPWidthN/2;
	int halfy = gRSP.nVPHeightN/2;
	int centerx = gRSP.nVPLeftN+halfx;
	int centery = gRSP.nVPTopN+halfy;

	gRSP.clip_ratio_left = centerx - halfx * gRSP.clip_ratio_negx;
	gRSP.clip_ratio_top = centery - halfy * gRSP.clip_ratio_negy;
	gRSP.clip_ratio_right = centerx + halfx * gRSP.clip_ratio_posx;
	gRSP.clip_ratio_bottom = centery + halfy * gRSP.clip_ratio_posy;

	UpdateScissorWithClipRatio();
}

void CDaedalusRender::UpdateScissorWithClipRatio()
{
	gRSP.real_clip_scissor_left = max(gRDP.scissor.left, gRSP.clip_ratio_left);
	gRSP.real_clip_scissor_top = max(gRDP.scissor.top, gRSP.clip_ratio_top);
	gRSP.real_clip_scissor_right = min(gRDP.scissor.right,gRSP.clip_ratio_right);
	gRSP.real_clip_scissor_bottom = min(gRDP.scissor.bottom, gRSP.clip_ratio_bottom);

	gRSP.real_clip_scissor_left = max(gRSP.real_clip_scissor_left, 0);
	gRSP.real_clip_scissor_top = max(gRSP.real_clip_scissor_top, 0);
	gRSP.real_clip_scissor_right = min(gRSP.real_clip_scissor_right,windowSetting.uViWidth-1);
	gRSP.real_clip_scissor_bottom = min(gRSP.real_clip_scissor_bottom, windowSetting.uViHeight-1);

	float halfx = gRSP.nVPWidthN/2.0f;
	float halfy = gRSP.nVPHeightN/2.0f;
	float centerx = gRSP.nVPLeftN+halfx;
	float centery = gRSP.nVPTopN+halfy;

	gRSP.real_clip_ratio_negx = (gRSP.real_clip_scissor_left - centerx)/halfx;
	gRSP.real_clip_ratio_negy = (gRSP.real_clip_scissor_top - centery)/halfy;
	gRSP.real_clip_ratio_posx = (gRSP.real_clip_scissor_right - centerx)/halfx;
	gRSP.real_clip_ratio_posy = (gRSP.real_clip_scissor_bottom - centery)/halfy;

	ApplyScissorWithClipRatio();
}


void CDaedalusRender::InitOtherModes(void)					// Set other modes not covered by color combiner or alpha blender
{
	ApplyTextureFilter();

	//
	// I can't think why the hand in mario's menu screen is rendered with an opaque rendermode,
	// and no alpha threshold. We set the alpha reference to 1 to ensure that the transparent pixels
	// don't get rendered. I hope this doesn't fuck anything else up.
	//
	if ( gRDP.otherMode.alpha_compare == 0 )
	{
		if ( gRDP.otherMode.cvg_x_alpha && (gRDP.otherMode.alpha_cvg_sel || gRDP.otherMode.aa_en ) )
		{
			ForceAlphaRef(1);
			SetAlphaTestEnable(TRUE);
		}
		else
		{
			SetAlphaTestEnable(FALSE);
		}
	}
	else
	{
		if( (gRDP.otherMode.alpha_cvg_sel ) && !gRDP.otherMode.cvg_x_alpha )
		{
			// Use CVG for pixel alpha
			SetAlphaTestEnable(FALSE);
		}
		else
		{
			// G_AC_THRESHOLD || G_AC_DITHER
			if(	m_dwAlpha==0 )
				ForceAlphaRef(1);
			else
				ForceAlphaRef(m_dwAlpha);
			SetAlphaTestEnable(TRUE);
		}
	}

	if( gRDP.otherMode.cycle_type  >= CYCTYPE_COPY )
	{
		// Disable zbuffer for COPY and FILL mode
		SetZCompare(FALSE);
	}
	else
	{
		SetZCompare(gRDP.otherMode.z_cmp);
		SetZUpdate(gRDP.otherMode.z_upd);
	}
}

//freakdave
void CDaedalusRender::SetTextureFilter(DWORD dwFilter)
{
	/*
	if( options.forceTextureFilter == FORCE_DEFAULT_FILTER )
	{
		switch(dwFilter)
		{
			case G_TF_AVERAGE:	//?
			case G_TF_BILERP:
				m_dwMinFilter = m_dwMagFilter = FILTER_LINEAR;
				break;
			default:
				m_dwMinFilter = m_dwMagFilter = FILTER_POINT;
				break;
		}
	}
	else
	{*/
		switch( options.forceTextureFilter )
		{
		case FORCE_NONE_FILTER:
			m_dwMinFilter = m_dwMagFilter = FILTER_NONE;
			break;
		case FORCE_POINT_FILTER:
			m_dwMinFilter = m_dwMagFilter = FILTER_POINT;
			break;
		case FORCE_LINEAR_FILTER:
			m_dwMinFilter = m_dwMagFilter = FILTER_LINEAR;
			break;
		/*case FORCE_BILINEAR_FILTER:
			m_dwMinFilter = m_dwMagFilter = FILTER_LINEAR;
			break;*/
		case FORCE_ANISOTROPIC_FILTER:
			m_dwMinFilter = m_dwMagFilter = FILTER_ANISOTROPIC;
			break;
		case FORCE_FLATCUBIC_FILTER:
			m_dwMinFilter = m_dwMagFilter = FILTER_FLATCUBIC;
			break;
		case FORCE_GAUSSIANCUBIC_FILTER:
			m_dwMinFilter = m_dwMagFilter = FILTER_GAUSSIANCUBIC;
			break;
		default:
			m_dwMinFilter = m_dwMagFilter = FILTER_ANISOTROPIC;
			break;
		}
	//}

	ApplyTextureFilter();
}
