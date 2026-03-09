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
#include "../../../config.h"
extern FiddledVtx * g_pVtxBase;
CRender * CRender::g_pRender=NULL;

D3DXMATRIX reverseXY(-1,0,0,0,0,-1,0,0,0,0,1,0,0,0,0,1);
D3DXMATRIX reverseY(1,0,0,0,0,-1,0,0,0,0,1,0,0,0,0,1);

//========================================================================
CRender * CRender::GetRender(void)
{
	if( CRender::g_pRender == NULL )
	{
		ErrorMsg("g_pRender is NULL");
		exit(0);
	}
	else
		return CRender::g_pRender;
}
bool CRender::IsAvailable()
{
	return CRender::g_pRender != NULL;
}

CRender::CRender() :
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
		g_dwVtxFlags[i] = 0;
	}
	
	m_pColorCombiner = CDeviceBuilder::GetBuilder()->CreateColorCombiner(this);
	m_pColorCombiner->Initialize();

	m_pAlphaBlender = CDeviceBuilder::GetBuilder()->CreateAlphaBlender(this);
}

CRender::~CRender()
{
	if( m_pColorCombiner != NULL )
	{
		CDeviceBuilder::GetBuilder()->DeleteColorCombiner();
		m_pColorCombiner = NULL;
	}
	
	if( m_pAlphaBlender != NULL )
	{
		CDeviceBuilder::GetBuilder()->DeleteAlphaBlender();
		m_pAlphaBlender = NULL;
	}
}

void CRender::ResetMatrices()
{
	Matrix mat;

	mat.m[0][1] = mat.m[0][2] = mat.m[0][3] =
	mat.m[1][0] = mat.m[1][2] = mat.m[1][3] =
	mat.m[2][0] = mat.m[2][1] = mat.m[2][3] =
	mat.m[3][0] = mat.m[3][1] = mat.m[3][2] = 0.0f;

	mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.0f;

	gRSP.projectionMtxTop = 0;
	gRSP.modelViewMtxTop = 0;
	gRSP.projectionMtxs[0] = mat;
	gRSP.modelviewMtxs[0] = mat;

	gRSP.bMatrixIsUpdated = true;
	UpdateCombinedMatrix();
}

void CRender::SetProjection(const Matrix & mat, BOOL bPush, LONG nLoadReplace) 
{
	if (bPush)
	{
		if (gRSP.projectionMtxTop >= (RICE_MATRIX_STACK-1))
		{
			TRACE0("Pushing past proj stack limits!");
		}
		else
			gRSP.projectionMtxTop++;

		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat;
		}
		else
		{
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat * gRSP.projectionMtxs[gRSP.projectionMtxTop-1];
		}
		
	}
	else
	{
		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat;
		}
		else
		{
			gRSP.projectionMtxs[gRSP.projectionMtxTop] = mat * gRSP.projectionMtxs[gRSP.projectionMtxTop];
		}

	}
	
	gRSP.bMatrixIsUpdated = true;

	DumpMatrix(mat,"Set Projection Matrix");
}

bool mtxPopUpError = false;
void CRender::SetWorldView(const Matrix & mat, BOOL bPush, LONG nLoadReplace)
{
	// ModelView
	if (bPush)
	{
		//if( mtxPopUpError )
		//{
		//	mtxPopUpError = false;
		//	return;
		//}

		if (gRSP.modelViewMtxTop >= (RICE_MATRIX_STACK-1))
			DebuggerAppendMsg("Pushing past modelview stack limits! %s", nLoadReplace==RENDER_LOAD_MATRIX?"Load":"Mul");
		else
			gRSP.modelViewMtxTop++;

		// We should store the current projection matrix...
		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat;
		}
		else			// Multiply projection matrix
		{
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat * gRSP.modelviewMtxs[gRSP.modelViewMtxTop-1];
		}
	}
	else	// NoPush
	{
		if (nLoadReplace == RENDER_LOAD_MATRIX)
		{
			// Load projection matrix
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat;
		}
		else
		{
			// Multiply projection matrix
			gRSP.modelviewMtxs[gRSP.modelViewMtxTop] = mat * gRSP.modelviewMtxs[gRSP.modelViewMtxTop];
		}
	}

	gRSPmodelViewTop = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];
	gRSP.bMatrixIsUpdated = true;

	DumpMatrix(mat,"Set WorldView Matrix");
}


void CRender::PopWorldView()
{
	if (gRSP.modelViewMtxTop > 0)
	{
		gRSP.modelViewMtxTop--;
		gRSPmodelViewTop = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];
		gRSP.bMatrixIsUpdated = true;
	}
	else
	{
#ifdef _DEBUG
		if( pauseAtNext )
			TRACE0("Popping past worldview stack limits");
#endif
		mtxPopUpError = true;
	}
}


Matrix & CRender::GetWorldProjectMatrix(void)
{
	return gRSPworldProject;
}

void CRender::SetWorldProjectMatrix(Matrix &mtx)
{
#ifdef _DEBUG
	if( pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_FLUSH_TRI || eventToPause == NEXT_MATRIX_CMD ) )
	{
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DebuggerAppendMsg("Force Matrix: pc=%08X", dwPC);
		DumpMatrix(mtx, "Force Matrix, loading new world-project matrix");
	}
#endif
	gRSPworldProject = mtx;

	gRSP.bMatrixIsUpdated = false;
	gRSP.bCombinedMatrixIsUpdated = true;
}

void CRender::SetMux(DWORD dwMux0, DWORD dwMux1)
{
	uint64 tempmux = (((uint64)dwMux0) << 32) | (uint64)dwMux1;
	if( m_Mux != tempmux )
	{
		m_Mux = tempmux;
		m_bBlendModeValid = FALSE;
		m_pColorCombiner->UpdateCombiner(dwMux0, dwMux1);
	}
}


void CRender::SetCombinerAndBlender()
{
	StartProfiler(PROFILE_COMBINER);
	InitOtherModes();

	if( g_curRomInfo.bDisableBlender )
		m_pAlphaBlender->DisableAlphaBlender();
	else if( currentRomOptions.bNormalBlender )
		m_pAlphaBlender->NormalAlphaBlender();
	else
		m_pAlphaBlender->InitBlenderMode();

	m_pColorCombiner->InitCombinerMode();
	StopProfiler(PROFILE_COMBINER);
}

void CRender::RenderReset()
{
	UpdateClipRectangle();
	ResetMatrices();
	SetZBias(0);
	gRSP.numVertices = 0;
	gRSP.curTile = 0;
	gRSP.fTexScaleX = 1/32.0f;;
	gRSP.fTexScaleY = 1/32.0f;
}

bool CRender::FillRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor)
{
	LOG_DL("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%8X", nX0, nY0, nX1, nY1, dwColor);

	if( g_CI.dwSize != TXT_SIZE_16b && frameBufferOptions.bIgnore )
		return true;

	if( status.bHandleN64TextureBuffer && !status.bDirectWriteIntoRDRAM )	status.bFrameBufferIsDrawn = true;

	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->UpdateFrame();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st FillRectangle");});
	}

	if( status.bCIBufferIsRendered && status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_BEFORE_SCREEN_CLEAR )
	{
		if( (nX0==0 && nY0 == 0 && (nX1 == g_CI.dwWidth ||nX1 == g_CI.dwWidth-1) ) ||
			(nX0==gRDP.scissor.left && nY0 == gRDP.scissor.top && (nX1 == gRDP.scissor.right || nX1 == gRDP.scissor.right-1)) ||
			((nX0+nX1 == g_CI.dwWidth || nX0+nX1 == g_CI.dwWidth-1 || nX0+nX1 == gRDP.scissor.left+gRDP.scissor.right || nX0+nX1 == gRDP.scissor.left+gRDP.scissor.right-1) &&
			(nY0 == gRDP.scissor.top || nY0 == 0 || nY0+nY1 == gRDP.scissor.top+gRDP.scissor.bottom || nY0+nY1 == gRDP.scissor.top+gRDP.scissor.bottom-1 ) ))
		{
			status.bVIOriginIsUpdated=false;
			CGraphicsContext::Get()->UpdateFrame();
			DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update Before Screen Clear");});
		}
	}


	SetFillMode(RICE_FILLMODE_SOLID);

	StartProfiler(PROFILE_DRAWING);

	bool res=true;

	/*
	// I don't know why this does not work for OpenGL
	if( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL && nX0 == 0 && nY0 == 0 && ((nX1==windowSetting.uViWidth && nY1==windowSetting.uViHeight)||(nX1==windowSetting.uViWidth-1 && nY1==windowSetting.uViHeight-1)) )
	{
		CGraphicsContext::g_pGraphicsContext->Clear(CLEAR_COLOR_BUFFER,dwColor);
	}
	else
	*/
	{
		BOOL m_savedZBufferFlag = gRSP.bZBufferEnabled;	// Save ZBuffer state
		ZBufferEnable( FALSE );

		m_fillRectVtx[0].x = ViewPortTranslatei_x(nX0);
		m_fillRectVtx[0].y = ViewPortTranslatei_y(nY0);
		m_fillRectVtx[1].x = ViewPortTranslatei_x(nX1);
		m_fillRectVtx[1].y = ViewPortTranslatei_y(nY1);

		SetCombinerAndBlender();

		if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
		{
			ZBufferEnable(FALSE);
		}
		else
		{
			dwColor = PostProcessDiffuseColor(0);
		}

		float depth = (gRDP.otherMode.depth_source == 1 ? gRDP.fPrimitiveDepth : 0 );

		ApplyRDPScissor();
		TurnFogOnOff(FALSE);
		res = RenderFillRect(dwColor, depth);
		TurnFogOnOff(gRSP.bFogEnabled);
		ApplyScissorWithClipRatio();

		if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
		{
			ZBufferEnable(gRSP.bZBufferEnabled);
		}
	}

	if( options.bWinFrameMode )	SetFillMode(RICE_FILLMODE_WINFRAME );

	StopProfiler(PROFILE_DRAWING);

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FILLRECT, {DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", nX0, nY0, nX1, nY1, dwColor);
			DebuggerAppendMsg("Pause after FillRect: Color=%08X\n", dwColor);if( logCombiners ) m_pColorCombiner->DisplayMuxString();});
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FLUSH_TRI, {DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", nX0, nY0, nX1, nY1, dwColor);
			DebuggerAppendMsg("Pause after FillRect: Color=%08X\n", dwColor);if( logCombiners ) m_pColorCombiner->DisplayMuxString();});

	return res;
}


bool CRender::Line3D(DWORD dwV0, DWORD dwV1, DWORD dwWidth)
{
	LOG_DL("Line3D: Vtx0=%d, Vtx1=%d, Width=%d", dwV0, dwV1, dwWidth);
	status.bCIBufferIsRendered = true;

	StartProfiler(PROFILE_DRAWING);

	m_line3DVtx[0].z = (g_vecProjected[dwV0].z + 1.0f) * 0.5f;
	m_line3DVtx[1].z = (g_vecProjected[dwV1].z + 1.0f) * 0.5f;

	if( m_line3DVtx[0].z != m_line3DVtx[1].z )  
		return false;

	if( status.bHandleN64TextureBuffer && !status.bDirectWriteIntoRDRAM )	status.bFrameBufferIsDrawn = true;
	if( status.bHandleN64TextureBuffer ) 
	{
		g_pTextureBufferInfo->maxUsedHeight = g_pTextureBufferInfo->N64Height;
		if( status.bHandleN64TextureBuffer && !status.bDirectWriteIntoRDRAM )	
		{
			status.bFrameBufferIsDrawn = true;
			status.bFrameBufferDrawnByTriangles = true;
		}
	}

	m_line3DVtx[0].x = ViewPortTranslatef_x(g_vecProjected[dwV0].x);
	m_line3DVtx[0].y = ViewPortTranslatef_y(g_vecProjected[dwV0].y);
	m_line3DVtx[0].rhw = g_vecProjected[dwV0].w;
	m_line3DVtx[0].dcDiffuse = PostProcessDiffuseColor(g_dwVtxDifColor[dwV0]);
	m_line3DVtx[0].dcSpecular = PostProcessSpecularColor();

	m_line3DVtx[1].x = ViewPortTranslatef_x(g_vecProjected[dwV1].x);
	m_line3DVtx[1].y = ViewPortTranslatef_y(g_vecProjected[dwV1].y);
	m_line3DVtx[1].rhw = g_vecProjected[dwV1].w;
	m_line3DVtx[1].dcDiffuse = PostProcessDiffuseColor(g_dwVtxDifColor[dwV1]);
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

	SetCombinerAndBlender();

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

bool CRender::RemapTextureCoordinate
	(float t0, float t1, DWORD tileWidth, DWORD mask, float textureWidth, float &u0, float &u1)
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

bool CRender::TexRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, float fS0, float fT0, float fScaleS, float fScaleT, bool colorFlag, DWORD diffuseColor)
{
	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->UpdateFrame();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st textRect");});
	}

	if( options.enableHackForGames == HACK_FOR_BANJO_TOOIE )
	{
		// Hack for Banjo shadow in Banjo Tooie
		if( g_TI.dwWidth == g_CI.dwWidth && g_TI.dwFormat == TXT_FMT_CI && g_TI.dwSize == TXT_SIZE_8b )
		{
			if( nX0 == fS0 && nY0 == fT0 )//&& nX0 > 90 && nY0 > 130 && nX1-nX0 > 80 && nY1-nY0 > 20 )
			{
				// Skip the text rect
				return true;
			}
		}
	}

	if( status.bN64IsDrawingTextureBuffer && frameBufferOptions.bIgnore )
	{
		return true;
	}

	PrepareTextures();

	if( options.enableHackForGames == HACK_FOR_DR_MARIO )
	{
		// Hack for Banjo shadow in Dr. Mario
		if( g_CI.dwWidth  == 320 && nX1 == 328 && g_TI.dwFormat == TXT_FMT_RGBA && g_TI.dwSize == TXT_SIZE_16b &&
			nX0 == 0 )
		{
			fScaleS = 328.0f / 320.0f;
			nX1 = 320;
		}
	}


	if( status.bHandleN64TextureBuffer && g_pTextureBufferInfo->CI_Info.dwSize == TXT_SIZE_8b )	
	{
		return true;
	}

	if( !IsTextureEnabled() &&  gRDP.otherMode.cycle_type  != CYCLE_TYPE_COPY )
	{
		FillRect(nX0, nY0, nX1, nY1, gRDP.primitiveColor);
		return true;
	}

	if( IsUsedAsDI(g_CI.dwAddr) && !status.bHandleN64TextureBuffer )
	{
		//status.bHandleN64TextureBuffer = true;
		//CGraphicsContext::g_pGraphicsContext->SetTextureBuffer(g_CI);	// Set CI=ZI as texture buffer
		status.bFrameBufferIsDrawn = true;
		//status.bFrameBufferDrawnByTriangles = true;
	}

	if( status.bHandleN64TextureBuffer && !status.bDirectWriteIntoRDRAM )	status.bFrameBufferIsDrawn = true;

	LOG_DL("TexRect: X0=%d, Y0=%d, X1=%d, Y1=%d,\n\t\tfS0=%f, fT0=%f, ScaleS=%f, ScaleT=%f ",
		nX0, nY0, nX1, nY1, fS0, fT0, fScaleS, fScaleT);

	StartProfiler(PROFILE_DRAWING);

	if( options.bEnableHacks )
	{
		// Goldeneye HACK
		if( options.bEnableHacks && nY1 - nY0 < 2 ) nY1 = nY1+2;
	}

	// Scale to Actual texture coords
	// The two cases are to handle the oversized textures hack on voodoos

	SetCombinerAndBlender();
	

	if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY || !gRDP.otherMode.z_cmp )
	{
		ZBufferEnable(FALSE);
	}

	BOOL accurate = currentRomOptions.bAccurateTextureMapping;

	CTexture *surf = g_textures[gRSP.curTile].m_pCTexture;
	RenderTexture &tex0 = g_textures[gRSP.curTile];
	Tile &tile0 = gRDP.tiles[gRSP.curTile];

	float widthDiv = tex0.m_fTexWidth;
	float heightDiv = tex0.m_fTexHeight;
	float t0u0 = (fS0 * tile0.fShiftScaleS-tile0.fhilite_sl);
	float t0u1;
	if( accurate && gRDP.otherMode.cycle_type >= CYCLE_TYPE_COPY )
	{
		t0u1 = t0u0 + (fScaleS * (nX1 - nX0 - 1))*tile0.fShiftScaleS;
	}
	else
	{
		t0u1 = t0u0 + (fScaleS * (nX1 - nX0))*tile0.fShiftScaleS;
	}


	if( status.UseLargerTile[0] )
	{
		m_texRectTex1UV[0].u = (t0u0+status.LargerTileRealLeft[0])/widthDiv;
		m_texRectTex1UV[1].u = (t0u1+status.LargerTileRealLeft[0])/widthDiv;
	}
	else
	{
		m_texRectTex1UV[0].u = t0u0/widthDiv;
		m_texRectTex1UV[1].u = t0u1/widthDiv;
		if( accurate && !tile0.bMirrorS && RemapTextureCoordinate(t0u0, t0u1, tex0.m_dwTileWidth, tile0.dwMaskS, widthDiv, m_texRectTex1UV[0].u, m_texRectTex1UV[1].u) )
			SetTextureUFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile);
	}

	float t0v0 = (fT0 * tile0.fShiftScaleT-tile0.fhilite_tl);
	float t0v1;
	if ( accurate && gRDP.otherMode.cycle_type >= CYCLE_TYPE_COPY)
	{
		t0v1 = t0v0 + (fScaleT * (nY1 - nY0-1))*tile0.fShiftScaleT;
	}
	else
	{
		t0v1 = t0v0 + (fScaleT * (nY1 - nY0))*tile0.fShiftScaleT;
	}

	m_texRectTex1UV[0].v = t0v0/heightDiv;
	m_texRectTex1UV[1].v = t0v1/heightDiv;
	if( accurate && !tile0.bMirrorT && RemapTextureCoordinate(t0v0, t0v1, tex0.m_dwTileHeight, tile0.dwMaskT, heightDiv, m_texRectTex1UV[0].v, m_texRectTex1UV[1].v) )
		SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, gRSP.curTile);
	
	D3DCOLOR speColor = PostProcessSpecularColor();
	D3DCOLOR difColor;
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
		RenderTexture &tex1 = g_textures[(gRSP.curTile+1)&7];
		Tile &tile1 = gRDP.tiles[(gRSP.curTile+1)&7];

		widthDiv = tex1.m_fTexWidth;
		heightDiv = tex1.m_fTexHeight;
		//if( tile1.dwMaskS == 0 )	widthDiv = tile1.dwWidth;
		//if( tile1.dwMaskT == 0 )	heightDiv = tile1.dwHeight;

		float t0u0 = fS0 * tile1.fShiftScaleS -tile1.fhilite_sl;
		float t0v0 = fT0 * tile1.fShiftScaleT -tile1.fhilite_tl;
		float t0u1;
		float t0v1;
		if ( accurate && gRDP.otherMode.cycle_type >= CYCLE_TYPE_COPY)
		{
			t0u1 = t0u0 + (fScaleS * (nX1 - nX0 - 1))*tile1.fShiftScaleS;
			t0v1 = t0v0 + (fScaleT * (nY1 - nY0 - 1))*tile1.fShiftScaleT;
		}
		else
		{
			t0u1 = t0u0 + (fScaleS * (nX1 - nX0))*tile1.fShiftScaleS;
			t0v1 = t0v0 + (fScaleT * (nY1 - nY0))*tile1.fShiftScaleT;
		}

		if( status.UseLargerTile[1] )
		{
			m_texRectTex2UV[0].u = (t0u0+status.LargerTileRealLeft[1])/widthDiv;
			m_texRectTex2UV[1].u = (t0u1+status.LargerTileRealLeft[1])/widthDiv;
		}
		else
		{
			m_texRectTex2UV[0].u = t0u0/widthDiv;
			m_texRectTex2UV[1].u = t0u1/widthDiv;
			if( accurate && !tile1.bMirrorS && RemapTextureCoordinate(t0u0, t0u1, tex1.m_dwTileWidth, tile1.dwMaskS, widthDiv, m_texRectTex2UV[0].u, m_texRectTex2UV[1].u) )
				SetTextureUFlag(TEXTURE_UV_FLAG_CLAMP, (gRSP.curTile+1)&7);
		}

		m_texRectTex2UV[0].v = t0v0/heightDiv;
		m_texRectTex2UV[1].v = t0v1/heightDiv;

		if( accurate && !tile1.bMirrorT && RemapTextureCoordinate(t0v0, t0v1, tex1.m_dwTileHeight, tile1.dwMaskT, heightDiv, m_texRectTex2UV[0].v, m_texRectTex2UV[1].v) )
			SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, (gRSP.curTile+1)&7);

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
	TurnFogOnOff(FALSE);
	if( TileUFlags[gRSP.curTile]==TEXTURE_UV_FLAG_CLAMP && TileVFlags[gRSP.curTile]==TEXTURE_UV_FLAG_CLAMP && options.forceTextureFilter == FORCE_DEFAULT_FILTER )
	{
		TextureFilter dwFilter = m_dwMagFilter;
		m_dwMagFilter = m_dwMinFilter = FILTER_LINEAR;
		ApplyTextureFilter();
		ApplyRDPScissor();
		res = RenderTexRect();
		ApplyScissorWithClipRatio();
		m_dwMagFilter = m_dwMinFilter = dwFilter;
		ApplyTextureFilter();
	}
	else if( fScaleS >= 1 && fScaleT >= 1 && options.forceTextureFilter == FORCE_DEFAULT_FILTER )
	{
		TextureFilter dwFilter = m_dwMagFilter;
		m_dwMagFilter = m_dwMinFilter = FILTER_POINT;
		ApplyTextureFilter();
		ApplyRDPScissor();
		res = RenderTexRect();
		ApplyScissorWithClipRatio();
		m_dwMagFilter = m_dwMinFilter = dwFilter;
		ApplyTextureFilter();
	}
	else
	{
		ApplyRDPScissor();
		res = RenderTexRect();
		ApplyScissorWithClipRatio();
	}
	TurnFogOnOff(gRSP.bFogEnabled);

	if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY || !gRDP.otherMode.z_cmp  )
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
		if( logCombiners ) m_pColorCombiner->DisplayMuxString();
	});

	StopProfiler(PROFILE_DRAWING);

	return res;
}


bool CRender::TexRectFlip(LONG nX0, LONG nY0, LONG nX1, LONG nY1, float fS0, float fT0, float fS1, float fT1)
{
	LOG_DL("TexRectFlip: X0=%d, Y0=%d, X1=%d, Y1=%d,\n\t\tfS0=%f, fT0=%f, fS1=%f, fT1=%f ",
			nX0, nY0, nX1, nY1, fS0, fT0, fS1, fT1);

	StartProfiler(PROFILE_DRAWING);
	if( status.bHandleN64TextureBuffer && !status.bDirectWriteIntoRDRAM )	
	{
		status.bFrameBufferIsDrawn = true;
		status.bFrameBufferDrawnByTriangles = true;
	}

	PrepareTextures();

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

	SetCombinerAndBlender();

	D3DCOLOR speColor = PostProcessSpecularColor();
	D3DCOLOR difColor = PostProcessDiffuseColor(gRDP.primitiveColor);

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

	TurnFogOnOff(FALSE);
	bool res = RenderTexRect();
	TurnFogOnOff(gRSP.bFogEnabled);

	// Restore state
	ZBufferEnable( m_savedZBufferFlag );

	StopProfiler(PROFILE_DRAWING);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((eventToPause == NEXT_FLUSH_TRI || eventToPause == NEXT_TEXTRECT), {
		DebuggerAppendMsg("TexRectFlip: tile=%d, X0=%d, Y0=%d, X1=%d, Y1=%d,\nfS0=%f, fT0=%f, nfS1=%f, fT1=%f\n",
			gRSP.curTile, nX0, nY0, nX1, nY1, fS0, fT0, fS1, fT1);
		DebuggerAppendMsg("       : x0=%f, y0=%f, x1=%f, y1=%f\n",	m_texRectTVtx[0].x, m_texRectTVtx[0].y, m_texRectTVtx[2].x, m_texRectTVtx[2].y);
		DebuggerAppendMsg("   Tex0: u0=%f, v0=%f, u1=%f, v1=%f\n",	m_texRectTVtx[0].tcord[0].u, m_texRectTVtx[0].tcord[0].v, m_texRectTVtx[2].tcord[0].u, m_texRectTVtx[2].tcord[0].v);
		TRACE0("Pause after TexRectFlip\n");
		if( logCombiners ) m_pColorCombiner->DisplayMuxString();
	});

	return res;
}


void CRender::StartDrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, D3DCOLOR dif, D3DCOLOR spe, float z, float rhw)
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

	RenderTexture &txtr = g_textures[0];
	if( txtr.pTextureEntry && txtr.pTextureEntry->txtrBufIdx > 0 )
	{
		TextureBufferInfo &info = gTextureBufferInfos[txtr.pTextureEntry->txtrBufIdx-1];
		m_texRectTVtx[0].tcord[0].u *= info.scaleX;
		m_texRectTVtx[0].tcord[0].v *= info.scaleY;
		m_texRectTVtx[1].tcord[0].u *= info.scaleX;
		m_texRectTVtx[1].tcord[0].v *= info.scaleY;
		m_texRectTVtx[2].tcord[0].u *= info.scaleX;
		m_texRectTVtx[2].tcord[0].v *= info.scaleY;
		m_texRectTVtx[3].tcord[0].u *= info.scaleX;
		m_texRectTVtx[3].tcord[0].v *= info.scaleY;
	}

	m_texRectTVtx[0].z = m_texRectTVtx[1].z = m_texRectTVtx[2].z = m_texRectTVtx[3].z = z;
	m_texRectTVtx[0].rhw = m_texRectTVtx[1].rhw = m_texRectTVtx[2].rhw = m_texRectTVtx[3].rhw = rhw;
}

void CRender::StartDrawSimpleRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor, float depth, float rhw)
{
	m_simpleRectVtx[0].x = ViewPortTranslatei_x(nX0);
	m_simpleRectVtx[1].x = ViewPortTranslatei_x(nX1);
	m_simpleRectVtx[0].y = ViewPortTranslatei_y(nY0);
	m_simpleRectVtx[1].y = ViewPortTranslatei_y(nY1);
}

void CRender::SetAddressUAllStages(DWORD dwTile, TextureUVFlag dwFlag)
{
}

void CRender::SetAddressVAllStages(DWORD dwTile, TextureUVFlag dwFlag)
{
}

void CRender::SetAllTexelRepeatFlag()
{
	if( IsTextureEnabled() )
	{
		if( IsTexel0Enable() || gRDP.otherMode.cycle_type  == CYCLE_TYPE_COPY )
			SetTexelRepeatFlags(gRSP.curTile);
		if( IsTexel1Enable() )
			SetTexelRepeatFlags((gRSP.curTile+1)&7);
	}
}


void CRender::SetTexelRepeatFlags(DWORD dwTile)
{

	if( gRDP.tiles[dwTile].dwMaskS == 0 || gRDP.tiles[dwTile].bClampS )
	{
		if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
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
		if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
			SetTextureVFlag(TEXTURE_UV_FLAG_WRAP, dwTile);	// Can not clamp in COPY/FILL mode
		else
			SetTextureVFlag(TEXTURE_UV_FLAG_CLAMP, dwTile);
	}
	else if (gRDP.tiles[dwTile].bMirrorT )
		SetTextureVFlag(TEXTURE_UV_FLAG_MIRROR, dwTile);
	else								
		SetTextureVFlag(TEXTURE_UV_FLAG_WRAP, dwTile);
}

void CRender::Initialize(void)
{
	ClearDeviceObjects();
	InitDeviceObjects();
}

void CRender::CleanUp(void)
{
	m_pColorCombiner->CleanUp();
	ClearDeviceObjects();
}

void myVec3Transform(float *vecout, float *vecin, float* m)
{
	float w = m[3]*vecin[0]+m[7]*vecin[1]+m[11]*vecin[2]+m[15];
	vecout[0] = (m[0]*vecin[0]+m[4]*vecin[1]+m[8]*vecin[2]+m[12])/w;
	vecout[1] = (m[1]*vecin[0]+m[5]*vecin[1]+m[9]*vecin[2]+m[13])/w;
	vecout[2] = (m[2]*vecin[0]+m[6]*vecin[1]+m[10]*vecin[2]+m[14])/w;
}

void CRender::SetTextureEnableAndScale(int dwTile, bool bEnable, float fScaleX, float fScaleY)
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

void CRender::SetFogFlagForNegativeW()
{
	if( !gRSP.bFogEnabled )	return;

	m_bFogStateSave = gRSP.bFogEnabled;

	BOOL flag=gRSP.bFogEnabled;
	
	for (DWORD i = 0; i < gRSP.numVertices; i++) 
	{
		if( g_vtxBuffer[i].rhw < 0 )
			flag = FALSE;
	}

	TurnFogOnOff(flag);
}

void CRender::RestoreFogFlag()
{
	if( !gRSP.bFogEnabled )	return;
	TurnFogOnOff(m_bFogStateSave);
}

void CRender::SetViewport(int nLeft, int nTop, int nRight, int nBottom, int maxZ)
{
	if( status.bHandleN64TextureBuffer )
		return;

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

	LOG_DL("SetViewport (%d,%d - %d,%d)",gRSP.nVPLeftN, gRSP.nVPTopN, gRSP.nVPRightN, gRSP.nVPBottomN);
}

extern bool bHalfTxtScale;
bool CRender::FlushTris()
{
	status.bCIBufferIsRendered = true;

	DEBUGGER_ONLY_IF( (!debuggerEnableZBuffer), {ZBufferEnable( FALSE );} );

	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->UpdateFrame();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{DebuggerAppendMsg("Screen Update at 1st triangle");});
	}

	// Hack for Pilotwings 64 (U) [!].v64
	static bool skipNext=false;
	if( options.enableHackForGames == HACK_FOR_PILOT_WINGS )
	{
		if( IsUsedAsDI(g_CI.dwAddr) && gRDP.otherMode.z_cmp+gRDP.otherMode.z_upd > 0 )
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
	}

	if( status.bN64IsDrawingTextureBuffer && frameBufferOptions.bIgnore )
	{
		gRSP.numVertices = 0;
		return true;
	}

	extern bool bConkerHideShadow;
	if( options.enableHackForGames == HACK_FOR_CONKER && bConkerHideShadow )
	{
		gRSP.numVertices = 0;
		return true;
	}

	if( IsUsedAsDI(g_CI.dwAddr) && !status.bHandleN64TextureBuffer )
	{
		//status.bHandleN64TextureBuffer = true;
		//CGraphicsContext::g_pGraphicsContext->SetTextureBuffer(g_CI);	// Set CI=ZI as texture buffer
		status.bFrameBufferIsDrawn = true;
		//status.bFrameBufferDrawnByTriangles = true;
	}

	/*
	if( status.bHandleN64TextureBuffer && g_pTextureBufferInfo->CI_Info.dwSize == TXT_SIZE_8b )	
	{
		gRSP.numVertices = 0;
		return true;
	}
	*/

	if (gRSP.numVertices == 0)	return true;
	if( status.bHandleN64TextureBuffer )
	{
		g_pTextureBufferInfo->maxUsedHeight = g_pTextureBufferInfo->N64Height;
		if( !status.bDirectWriteIntoRDRAM )	
		{
			status.bFrameBufferIsDrawn = true;
			status.bFrameBufferDrawnByTriangles = true;
		}
	}

	if( !gRDP.bFogEnableInBlender && gRSP.bFogEnabled )
	{
		TurnFogOnOff(FALSE);
	}

	for( int t=0; t<2; t++ )
	{
		float halfscaleS = 1;
		float halfscaleT = 1;

		// This will get rid of the thin black lines
		if( t==0 && !(m_pColorCombiner->m_bTex0Enabled) ) 
			continue;
		else
		{
			if( ( gRDP.tiles[gRSP.curTile].dwSize == TXT_SIZE_32b && options.enableHackForGames == HACK_FOR_RUMBLE ) ||
                (bHalfTxtScale && g_curRomInfo.bTextureScaleHack ) )
			{
				halfscaleS = 0.5;
				halfscaleT = 0.5;
			}
		}
		if( t==1 && !(m_pColorCombiner->m_bTex1Enabled) ) break;

		bool clampS=true;
		bool clampT=true;
		DWORD i;

		if( halfscaleS < 1 )
		{
			for( i=0; i<gRSP.numVertices; i++ )
			{
				if( t == 0 )
				{
					g_vtxBuffer[i].tcord[t].u += gRSP.tex0OffsetX;
					g_vtxBuffer[i].tcord[t].u /= 2;
					g_vtxBuffer[i].tcord[t].u -= gRSP.tex0OffsetX;
					g_vtxBuffer[i].tcord[t].v += gRSP.tex0OffsetY;
					g_vtxBuffer[i].tcord[t].v /= 2;
					g_vtxBuffer[i].tcord[t].v -= gRSP.tex0OffsetY;
				}
				else
				{
					g_vtxBuffer[i].tcord[t].u += gRSP.tex1OffsetX;
					g_vtxBuffer[i].tcord[t].u /= 2;
					g_vtxBuffer[i].tcord[t].u -= gRSP.tex1OffsetX;
					g_vtxBuffer[i].tcord[t].v += gRSP.tex1OffsetY;
					g_vtxBuffer[i].tcord[t].v /= 2;
					g_vtxBuffer[i].tcord[t].v -= gRSP.tex1OffsetY;
				}
			}
		}

		for( i=0; i<gRSP.numVertices; i++ )
		{
			if( g_vtxBuffer[i].tcord[t].u > 1.0 || g_vtxBuffer[i].tcord[t].u < 0.0  )
			{
				clampS = false;
				break;
			}
		}

		for( i=0; i<gRSP.numVertices; i++ )
		{
			if( g_vtxBuffer[i].tcord[t].v > 1.0 || g_vtxBuffer[i].tcord[t].v < 0.0  )
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

	if( status.bHandleN64TextureBuffer && g_pTextureBufferInfo->CI_Info.dwSize == TXT_SIZE_8b )
	{
		ZBufferEnable(FALSE);
	}

	bool res = RenderFlushTris();
	g_clippedVtxCount = 0;

	LOG_DL("FlushTris: Draw %d Triangles", gRSP.numVertices/3);
	
	gRSP.numVertices = 0;	// Reset index

	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_FLUSH_TRI, {
		TRACE0("Pause after FlushTris\n");
		if( logCombiners ) m_pColorCombiner->DisplayMuxString();
	});

	if( !gRDP.bFogEnableInBlender && gRSP.bFogEnabled )
	{
		TurnFogOnOff(TRUE);
	}

	return res;
}

#ifdef _DEBUG
void CRender::SaveTextureToFile(int tex, bool alphaOnly)
{
	CTexture *pTexture = g_textures[tex].m_pCTexture;
	if( pTexture == NULL )
	{
		TRACE0("Can't dump null texture");
		return;
	}

	DrawInfo di;
	if( pTexture->StartUpdate(&di) )
	{
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
}
#endif

extern TextureBufferInfo gTextureBufferInfos[];
void SetVertexTextureUVCoord(TexCord &dst, float s, float t, int tile, TextureEntry *pEntry)
{
	RenderTexture &txtr = g_textures[tile];
	TextureBufferInfo &info = gTextureBufferInfos[pEntry->txtrBufIdx-1];

	DWORD addrOffset = g_TI.dwAddr-info.CI_Info.dwAddr;
	DWORD extraTop = (addrOffset>>(info.CI_Info.dwSize-1)) /info.CI_Info.dwWidth;
	DWORD extraLeft = (addrOffset>>(info.CI_Info.dwSize-1))%info.CI_Info.dwWidth;

	if( pEntry->txtrBufIdx > 0  )
	{
		// Loading from texture buffer or back buffer
		s += (extraLeft+pEntry->ti.LeftToLoad)/txtr.m_fTexWidth;
		t += (extraTop+pEntry->ti.TopToLoad)/txtr.m_fTexHeight;

		s *= info.scaleX;
		t *= info.scaleY;
	}

	dst.u = s;
	dst.v = t;
}

void CRender::SetVertexTextureUVCoord(TLITVERTEX &v, float fTex0S, float fTex0T)
{
	RenderTexture &txtr = g_textures[0];
	if( txtr.pTextureEntry && txtr.pTextureEntry->txtrBufIdx > 0 )
	{
		::SetVertexTextureUVCoord(v.tcord[0], fTex0S, fTex0T, 0, txtr.pTextureEntry);
	}
	else
	{
		v.tcord[0].u = fTex0S;
		v.tcord[0].v = fTex0T;
	}
}
void CRender::SetVertexTextureUVCoord(TLITVERTEX &v, float fTex0S, float fTex0T, float fTex1S, float fTex1T)
{
	if( options.enableHackForGames == HACK_FOR_ZELDA && m_Mux == 0x00262a60150c937f && gRSP.curTile == 0 )
	{
		// Hack for Zelda Sun
		Tile &t0 = gRDP.tiles[0];
		Tile &t1 = gRDP.tiles[1];
		if( t0.dwFormat == TXT_FMT_I && t0.dwSize == TXT_SIZE_8b && t0.dwWidth == 64 &&
			t1.dwFormat == TXT_FMT_I && t1.dwSize == TXT_SIZE_8b && t1.dwWidth == 64 &&
			t0.dwHeight == t1.dwHeight )
		{
			fTex0S /= 2;
			fTex0T /= 2;
			fTex1S /= 2;
			fTex1T /= 2;
		}
	}

	RenderTexture &txtr0 = g_textures[0];
	if( txtr0.pTextureEntry && txtr0.pTextureEntry->txtrBufIdx > 0 )
	{
		::SetVertexTextureUVCoord(v.tcord[0], fTex0S, fTex0T, 0, txtr0.pTextureEntry);
	}
	else
	{
		v.tcord[0].u = fTex0S;
		v.tcord[0].v = fTex0T;
	}

	RenderTexture &txtr1 = g_textures[1];
	if( txtr1.pTextureEntry && txtr1.pTextureEntry->txtrBufIdx > 0 )
	{
		::SetVertexTextureUVCoord(v.tcord[1], fTex1S, fTex1T, 1, txtr1.pTextureEntry);
	}
	else
	{
		v.tcord[1].u = fTex1S;
		v.tcord[1].v = fTex1T;
	}
}

void CRender::SetClipRatio(DWORD type, DWORD word1)
{
	bool modified = false;
	switch(type)
	{
	case RSP_MV_WORD_OFFSET_CLIP_RNX:
		LOG_DL("    RSP_MOVE_WORD_CLIP  NegX: %d", (LONG)(short)word1);
		if( gRSP.clip_ratio_negx != (short)word1 )
		{
			gRSP.clip_ratio_negx = (short)word1;
			modified = true;
		}
		break;
	case RSP_MV_WORD_OFFSET_CLIP_RNY:
		LOG_DL("    RSP_MOVE_WORD_CLIP  NegY: %d", (LONG)(short)word1);
		if( gRSP.clip_ratio_negy != (short)word1 )
		{
			gRSP.clip_ratio_negy = (short)word1;
			modified = true;
		}
		break;
	case RSP_MV_WORD_OFFSET_CLIP_RPX:
		LOG_DL("    RSP_MOVE_WORD_CLIP  PosX: %d", (LONG)(short)word1);
		if( gRSP.clip_ratio_posx != -(short)word1 )
		{
			gRSP.clip_ratio_posx = -(short)word1;
			modified = true;
		}
		break;
	case RSP_MV_WORD_OFFSET_CLIP_RPY:
		LOG_DL("    RSP_MOVE_WORD_CLIP  PosY: %d", (LONG)(short)word1);
		if( gRSP.clip_ratio_posy != -(short)word1 )
		{
			gRSP.clip_ratio_posy = -(short)word1;
			modified = true;
		}
		break;
	}

	if( modified )
	{
		UpdateClipRectangle();
	}

}
void CRender::UpdateClipRectangle()
{
	if( status.bHandleN64TextureBuffer )
	{
		//windowSetting.fMultX = windowSetting.fMultY = 1;
		windowSetting.vpLeftW = 0;
		windowSetting.vpTopW = 0;
		windowSetting.vpRightW = g_pTextureBufferInfo->bufferWidth;
		windowSetting.vpBottomW = g_pTextureBufferInfo->bufferHeight;
		windowSetting.vpWidthW = g_pTextureBufferInfo->bufferWidth;
		windowSetting.vpHeightW = g_pTextureBufferInfo->bufferHeight;

		gRSP.vtxXMul = windowSetting.vpWidthW/2.0f;
		gRSP.vtxXAdd = gRSP.vtxXMul + windowSetting.vpLeftW;
		gRSP.vtxYMul = -windowSetting.vpHeightW/2.0f;
		gRSP.vtxYAdd = windowSetting.vpHeightW/2.0f + windowSetting.vpTopW+windowSetting.toolbarHeightToUse;

		// Update clip rectangle by setting scissor

		int halfx = g_pTextureBufferInfo->bufferWidth/2;
		int halfy = g_pTextureBufferInfo->bufferHeight/2;
		int centerx = halfx;
		int centery = halfy;

		gRSP.clip_ratio_left = centerx - halfx * gRSP.clip_ratio_negx;
		gRSP.clip_ratio_top = centery - halfy * gRSP.clip_ratio_negy;
		gRSP.clip_ratio_right = centerx + halfx * gRSP.clip_ratio_posx;
		gRSP.clip_ratio_bottom = centery + halfy * gRSP.clip_ratio_posy;
	}
	else
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
		gRSP.vtxYAdd = windowSetting.vpHeightW/2.0f + windowSetting.vpTopW+windowSetting.toolbarHeightToUse;

		// Update clip rectangle by setting scissor

		int halfx = gRSP.nVPWidthN/2;
		int halfy = gRSP.nVPHeightN/2;
		int centerx = gRSP.nVPLeftN+halfx;
		int centery = gRSP.nVPTopN+halfy;

		gRSP.clip_ratio_left = centerx - halfx * gRSP.clip_ratio_negx;
		gRSP.clip_ratio_top = centery - halfy * gRSP.clip_ratio_negy;
		gRSP.clip_ratio_right = centerx + halfx * gRSP.clip_ratio_posx;
		gRSP.clip_ratio_bottom = centery + halfy * gRSP.clip_ratio_posy;
	}


	UpdateScissorWithClipRatio();
}

void CRender::UpdateScissorWithClipRatio()
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


void CRender::InitOtherModes(void)					// Set other modes not covered by color combiner or alpha blender
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
			ForceAlphaRef(128);	// Strange, I have to use value=2 for pixel shader combiner for Nvidia FX5200
								// for other video cards, value=1 is good enough.
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
			// RDP_ALPHA_COMPARE_THRESHOLD || RDP_ALPHA_COMPARE_DITHER
			if(	m_dwAlpha==0 )
				ForceAlphaRef(1);
			else
				ForceAlphaRef(m_dwAlpha);
			SetAlphaTestEnable(TRUE);
		}
	}

	if( options.enableHackForGames == HACK_FOR_SOUTH_PARK_RALLY && m_Mux == 0x00121824ff33ffff &&
		gRSP.bCullFront && gRDP.otherMode.aa_en && gRDP.otherMode.z_cmp && gRDP.otherMode.z_upd )
	{
		SetZCompare(FALSE);
	}


	if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
	{
		// Disable zbuffer for COPY and FILL mode
		SetZCompare(FALSE);
	}
	else
	{
		SetZCompare(gRDP.otherMode.z_cmp);
		SetZUpdate(gRDP.otherMode.z_upd);
	}

	/*
	if( options.enableHackForGames == HACK_FOR_SOUTH_PARK_RALLY && m_Mux == 0x00121824ff33ffff &&
		gRSP.bCullFront && gRDP.otherMode.z_cmp && gRDP.otherMode.z_upd )//&& gRDP.otherMode.aa_en )
	{
		SetZCompare(FALSE);
		SetZUpdate(FALSE);
	}
	*/
}


void CRender::SetTextureFilter(DWORD dwFilter)
{
	/*
	//freakdave
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
		}
	//}

	ApplyTextureFilter();
}