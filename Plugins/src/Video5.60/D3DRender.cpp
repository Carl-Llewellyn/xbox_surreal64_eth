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

#define RICEFVF_TEXRECTFVERTEX ( D3DFVF_XYZRHW | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX2 )
#define RICE_FVF_FILLRECTVERTEX ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE  )

extern FiddledVtx * g_pVtxBase;

inline float round( float x )
{
	return (float)(s32)( x + 0.5f );
}

UVFlagMap DirectXUVFlagMaps[] =
{
	TEXTURE_UV_FLAG_WRAP,	D3DTADDRESS_WRAP,
	TEXTURE_UV_FLAG_MIRROR,	D3DTADDRESS_MIRROR,
	TEXTURE_UV_FLAG_CLAMP,	D3DTADDRESS_CLAMP,
};


//*****************************************************************************
// Creator function for singleton
//*****************************************************************************
D3DRender::D3DRender() :
	m_dwTextureFactor(COLOR_RGBA(255,255,255,255))
{
	m_dwrsZEnable=D3DZB_FALSE;
	m_dwrsZWriteEnable=FALSE;

	memset(&m_D3DCombStages, 0, sizeof(D3DCombinerStage)*8);

	for (int i = 0; i < 8; i++)
	{
		m_D3DCombStages[i].dwColorOp = D3DTOP_DISABLE;
		m_D3DCombStages[i].dwColorArg1 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwColorArg2 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwColorArg0 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwAlphaOp = D3DTOP_DISABLE;
		m_D3DCombStages[i].dwAlphaArg1 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwAlphaArg2 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwAlphaArg0 = D3DTA_CURRENT;

		m_D3DCombStages[i].dwMinFilter = D3DTEXF_NONE;
		m_D3DCombStages[i].dwMagFilter = D3DTEXF_NONE;

		m_D3DCombStages[i].dwAddressUMode = 0;
		m_D3DCombStages[i].dwAddressVMode = 0;
		m_D3DCombStages[i].dwAddressW = 0xFFFF;

		m_D3DCombStages[i].dwTexCoordIndex = 0xFFFF;
		m_D3DCombStages[i].pTexture = NULL;
	}

	m_Mux = 0;
	memset(&m_curCombineInfo, 0, sizeof( m_curCombineInfo) );

	m_dwrsZEnable = 0xEEEE;
	m_dwrsZWriteEnable = 0xEEEE;
	m_dwrsSrcBlend = 0xEEEE;
	m_dwrsDestBlend = 0xEEEE;
	m_dwrsAlphaBlendEnable = 0xEEEE;
	m_dwrsAlphaTestEnable = 0xEEEE;
	m_dwrsAlphaRef = 0xEEEE;
	m_dwrsZBias = 0xEEEE;
}

D3DRender::~D3DRender()
{	
	ClearDeviceObjects();
	g_pD3DDev->SetPixelShader(NULL);
}

bool D3DRender::ClearDeviceObjects()
{
	for( int i=0; i<MAX_TEXTURES; i++)
	{
		if (g_textures[i].m_lpsTexturePtr)		// We keep a reference to the most recently selected texture
		{
			LPDIRECT3DTEXTURE8(g_textures[i].m_lpsTexturePtr)->Release();
			g_textures[i].m_lpsTexturePtr = NULL;
			D3DSetTexture( i, NULL );
		}
	}

	return true;
}
	

bool D3DRender::InitDeviceObjects()
{
	// We never change these

	g_pD3DDev->SetRenderState( D3DRS_DITHERENABLE, TRUE ); // Re-enabled by Orkin - Makes 16-bit look nicer

	// We do our own culling
	g_pD3DDev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_NONE );

	// We do our own lighting
	g_pD3DDev->SetRenderState(  D3DRS_AMBIENT, COLOR_RGBA(255,255,255,255) );
	g_pD3DDev->SetRenderState( D3DRS_LIGHTING,	  FALSE);


	SetD3DRSAlphaBlendEnable(TRUE );
	SetD3DRSSrcBlend( D3DBLEND_SRCALPHA);
	SetD3DRSDestBlend(D3DBLEND_INVSRCALPHA);

	g_pD3DDev->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	SetD3DRSZWriteEnable( TRUE);

	//weinersch - do xbox antialiasing here
#ifdef _XBOX

	if(AntiAliasMode>1){
		g_pD3DDev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , TRUE);
	}else if(AntiAliasMode==1){
		g_pD3DDev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , FALSE);
		g_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE , TRUE);
		g_pD3DDev->SetRenderState( D3DRS_EDGEANTIALIAS , TRUE);
	}else{
		g_pD3DDev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , FALSE);
	}

#else	
	
	if( ((CDXGraphicsContext*)CGraphicsContext::g_pGraphicsContext)->IsFSAAEnable() )
		g_pD3DDev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , TRUE);
	else
		g_pD3DDev->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE);

#endif

	// Initialize all the renderstate to our defaults.
	SetShadeMode( gRSP.shadeMode );
	g_pD3DDev->SetRenderState( D3DRS_TEXTUREFACTOR, m_dwTextureFactor );

	// weinersch - use old fog rendering for xbox - test this out
/*
	//Rice 5.60
	g_pD3DDev->SetRenderState( D3DRS_FOGENABLE, FALSE);
	float density = 1.0f;
	g_pD3DDev->SetRenderState(D3DRS_FOGDENSITY,   *(DWORD *)(&density));
	g_pD3DDev->SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);
	g_pD3DDev->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
*/
	//Rice 5.10
	g_pD3DDev->SetRenderState( D3DRS_FOGENABLE, FALSE);
	g_pD3DDev->SetRenderState( D3DRS_FOGCOLOR, gRDP.fogColor ); 
//	g_pD3DDev->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR );
   
	// Dafault is ZBuffer disabled
	SetD3DRSZEnable( m_dwrsZEnable );

    SetD3DRSAlphaTestEnable(TRUE );
    SetD3DRSAlphaRef(0x04 );
    g_pD3DDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

	m_dwrsZEnable=D3DZB_FALSE;
	m_dwrsZWriteEnable=FALSE;

	memset(&m_D3DCombStages, 0, sizeof(D3DCombinerStage)*8);

	for (int i = 0; i < 8; i++)
	{
		m_D3DCombStages[i].dwColorOp = D3DTOP_DISABLE;
		m_D3DCombStages[i].dwColorArg1 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwColorArg2 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwColorArg0 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwAlphaOp = D3DTOP_DISABLE;
		m_D3DCombStages[i].dwAlphaArg1 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwAlphaArg2 = D3DTA_CURRENT;
		m_D3DCombStages[i].dwAlphaArg0 = D3DTA_CURRENT;

		m_D3DCombStages[i].dwMinFilter = D3DTEXF_NONE;
		m_D3DCombStages[i].dwMagFilter = D3DTEXF_NONE;

		m_D3DCombStages[i].dwAddressUMode = 0xFFFF;
		m_D3DCombStages[i].dwAddressVMode = 0xFFFF;
		m_D3DCombStages[i].dwAddressW = 0xFFFF;

		m_D3DCombStages[i].dwTexCoordIndex = 0xFFFF;
	}

	m_Mux = 0;
	memset(&m_curCombineInfo, 0, sizeof( m_curCombineInfo) );


	m_dwrsZEnable = 0xEEEE;
	m_dwrsZWriteEnable = 0xEEEE;
	m_dwrsSrcBlend = 0xEEEE;
	m_dwrsDestBlend = 0xEEEE;
	m_dwrsAlphaBlendEnable = 0xEEEE;
	m_dwrsAlphaTestEnable = 0xEEEE;
	m_dwrsAlphaRef = 0xEEEE;
	m_dwrsZBias = 0xEEEE;

	for(int n = 0; n < 4; n++) 
	{ 
		// Texturing stuff 
		D3DSetMinFilter( n, D3DTEXF_LINEAR ); 
		D3DSetMagFilter( n, D3DTEXF_LINEAR ); 
		
		D3DSetAddressU( n, D3DTADDRESS_WRAP ); 
		D3DSetAddressV( n, D3DTADDRESS_WRAP ); 
		
		D3DSetColorArg1( n, D3DTA_TEXTURE ); 
		D3DSetColorArg2( n, D3DTA_DIFFUSE ); 
		D3DSetColorArg0( n, D3DTA_DIFFUSE ); 
		D3DSetColorOp( n, D3DTOP_MODULATE ); 
		D3DSetAlphaArg1( n, D3DTA_TEXTURE ); 
		D3DSetAlphaArg2( n, D3DTA_DIFFUSE ); 
		D3DSetAlphaArg0( n, D3DTA_DIFFUSE ); 
		D3DSetAlphaOp( n, D3DTOP_MODULATE ); 

		D3DSetTexture( i, NULL );
	} 
	
	((CDirectXColorCombiner*)m_pColorCombiner)->Initialize();

	//freakdave
	g_pD3DDev->SetTextureStageState(0, D3DTSS_MINFILTER, TextureMode);
	g_pD3DDev->SetTextureStageState(0, D3DTSS_MAGFILTER, TextureMode);
	g_pD3DDev->SetFlickerFilter(FlickerFilter);
	g_pD3DDev->SetSoftDisplayFilter(SoftDisplayFilter);
	
	return true;	
}

bool D3DRender::RenderTexRect()
{
	SetD3DRSZBias(0);
	WORD wIndices[2*3] = {1,0,2, 2,0,3};
	g_pD3DDev->SetVertexShader(RICE_FVF_TLITVERTEX);
	return S_OK == g_pD3DDev->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2, wIndices, D3DFMT_INDEX16, m_texRectTVtx, sizeof(TLITVERTEX));
}

bool D3DRender::RenderFillRect(DWORD dwColor, float depth)
{
	FILLRECTVERTEX frv[4] = {   {m_fillRectVtx[0].x, m_fillRectVtx[0].y, depth, 1, dwColor},
								{m_fillRectVtx[1].x, m_fillRectVtx[0].y, depth, 1, dwColor},
								{m_fillRectVtx[1].x, m_fillRectVtx[1].y, depth, 1, dwColor},
								{m_fillRectVtx[0].x, m_fillRectVtx[1].y, depth, 1, dwColor}  };
	static WORD wIndices[2*3] = {1,0,2, 2,0,3};

	SetD3DRSZBias(0);
	g_pD3DDev->SetVertexShader(RICE_FVF_FILLRECTVERTEX);
	return S_OK ==     g_pD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, wIndices, D3DFMT_INDEX16, frv, sizeof(FILLRECTVERTEX));
}

bool D3DRender::RenderFlushTris()
{
	SetD3DRSZBias(m_dwZBias*8);

	g_pD3DDev->SetVertexShader(RICE_FVF_TLITVERTEX);
	if( options.bForceSoftwareClipper )
	{
		ClipVertexes();

		if (g_clippedVtxCount)
			g_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, g_clippedVtxCount/3, g_clippedVtxBuffer, sizeof(TLITVERTEX));
	}
	else
	{
		g_pD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, gRSP.numVertices/3, g_vtxBuffer, sizeof(TLITVERTEX));
	}

	return true;
}

bool D3DRender::RenderLine3D()
{
	FILLRECTVERTEX frv[4] = {   {m_line3DVector[0].x, m_line3DVector[0].y, m_line3DVtx[0].z, m_line3DVtx[0].rhw, m_line3DVtx[0].dcDiffuse},
								{m_line3DVector[1].x, m_line3DVector[1].y, m_line3DVtx[0].z, m_line3DVtx[0].rhw, m_line3DVtx[0].dcDiffuse},
								{m_line3DVector[2].x, m_line3DVector[2].y, m_line3DVtx[0].z, m_line3DVtx[0].rhw, m_line3DVtx[1].dcDiffuse},
								{m_line3DVector[3].x, m_line3DVector[3].y, m_line3DVtx[0].z, m_line3DVtx[0].rhw, m_line3DVtx[1].dcDiffuse}  };

	static WORD wIndices[2*3] = {1,0,2, 2,1,3};

	SetD3DRSZBias(0);
	g_pD3DDev->SetVertexShader(RICE_FVF_FILLRECTVERTEX);
	HRESULT hr = g_pD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, wIndices, D3DFMT_INDEX16, frv, sizeof(FILLRECTVERTEX));

	return hr == S_OK;
}


LPDIRECT3DSURFACE8 g_pLockableBackBuffer=NULL;

void D3DRender::DrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, D3DCOLOR dif, D3DCOLOR spe, float z, float rhw)
{
	if( status.bVIOriginIsUpdated == true && currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=false;
		CGraphicsContext::Get()->UpdateFrame();
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG,{TRACE0("Screen Update at 1st Simple2DTexture");});
	}

	StartDrawSimple2DTexture(x0, y0, x1, y1, u0, v0, u1, v1, dif, spe, z, rhw);
	
	WORD wIndices[2*3] = {1,0,2, 2,0,3};
	g_pD3DDev->SetVertexShader(RICE_FVF_TLITVERTEX);
	g_pD3DDev->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 4, 2, wIndices, D3DFMT_INDEX16, m_texRectTVtx, sizeof(TLITVERTEX));
}

void D3DRender::DrawSimpleRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor, float depth, float rhw)
{
	StartDrawSimpleRect(nX0, nY0, nX1, nY1, dwColor, depth, rhw);

	// Divide by 2 instead of 4, because screen is magnified by two
	FILLRECTVERTEX frv[4] = {   {m_simpleRectVtx[0].x, m_simpleRectVtx[0].y, depth, rhw, dwColor},
								{m_simpleRectVtx[1].x, m_simpleRectVtx[0].y, depth, rhw, dwColor},
								{m_simpleRectVtx[1].x, m_simpleRectVtx[1].y, depth, rhw, dwColor},
								{m_simpleRectVtx[0].x, m_simpleRectVtx[1].y, depth, rhw, dwColor}  };

	static WORD wIndices[2*3] = {1,0,2, 2,0,3};

	g_pD3DDev->SetVertexShader(RICE_FVF_FILLRECTVERTEX);
	g_pD3DDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, wIndices, D3DFMT_INDEX16, frv, sizeof(FILLRECTVERTEX));
}

void D3DRender::SetTextureUFlag(TextureUVFlag dwFlag, DWORD tile)
{
	TileUFlags[tile] = dwFlag;
	if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
	{
		D3DSetAddressU( 0, DirectXUVFlagMaps[dwFlag].realFlag );
	}
	else
	{
		for( int i=0; i<m_curCombineInfo.nStages; i++ )
		{
			if( m_curCombineInfo.stages[i].dwTexture == tile-gRSP.curTile )
			{
				D3DSetAddressU( i, DirectXUVFlagMaps[dwFlag].realFlag );
			}
		}
	}
}

void D3DRender::SetTextureVFlag(TextureUVFlag dwFlag, DWORD tile)
{
	TileVFlags[tile] = dwFlag;
	if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
	{
		D3DSetAddressV(0, DirectXUVFlagMaps[dwFlag].realFlag );
	}
	else
	{
		for( int i=0; i<m_curCombineInfo.nStages; i++ )
		{
			if( m_curCombineInfo.stages[i].dwTexture == tile-gRSP.curTile )
			{
				D3DSetAddressV( i, DirectXUVFlagMaps[dwFlag].realFlag );
			}
		}
	}
}

void D3DRender::SetAddressUAllStages(DWORD dwTile, TextureUVFlag dwFlag)
{
	SetTextureUFlag(dwFlag, dwTile);
}

void D3DRender::SetAddressVAllStages(DWORD dwTile, TextureUVFlag dwFlag)
{
	SetTextureVFlag(dwFlag, dwTile);
}

#define RSP_ZELDA_FOG 0x00010000
void D3DRender::ZBufferEnable(BOOL bZBuffer)
{
	if( g_curRomInfo.bForceDepthBuffer )
		bZBuffer = TRUE;

	SetZCompare(bZBuffer);
	SetZUpdate(bZBuffer);
}

#define RSP_ZELDA_ZBUFFER 0x00000001 
void D3DRender::SetZCompare(BOOL bZCompare)
{
	if( g_curRomInfo.bForceDepthBuffer )
		bZCompare = TRUE;

	gRSP.bZBufferEnabled = bZCompare;
	m_bZCompare = bZCompare;
	SetD3DRSZEnable( bZCompare ? D3DZB_TRUE : D3DZB_FALSE );
}

void D3DRender::SetZUpdate(BOOL bZUpdate)
{
	if( g_curRomInfo.bForceDepthBuffer )
		bZUpdate = TRUE;

	m_bZUpdate = bZUpdate;
	if( bZUpdate )	
	{
		SetD3DRSZEnable(  D3DZB_TRUE );
	}
	SetD3DRSZWriteEnable( bZUpdate );
}

void D3DRender::SetZBias(int bias)
{
	if (m_dwZBias != bias)
	{
		m_dwZBias = bias;
		if( bias == 0 )
			SetD3DRSZBias(0);
		else
			SetD3DRSZBias(16);
	}
}

//freakdave
TextureFilterMap DXTexFilterMap[6]=
{
	{FILTER_NONE, D3DTEXF_NONE},
	{FILTER_POINT, D3DTEXF_POINT},
	{FILTER_LINEAR, D3DTEXF_LINEAR},
	{FILTER_ANISOTROPIC, D3DTEXF_ANISOTROPIC},
	//{FILTER_FLATCUBIC, D3DTEXF_FLATCUBIC}, //wtf is going on here ??
	{FILTER_FLATCUBIC, 4}, //it works this way though,hm,strange...
	{FILTER_GAUSSIANCUBIC, D3DTEXF_GAUSSIANCUBIC},
};

void D3DRender::ApplyTextureFilter()
{
	if( gRDP.otherMode.cycle_type  >= CYCLE_TYPE_COPY )
	{
		D3DSetMinFilter( 0, DXTexFilterMap[m_dwMinFilter].realFilter );
		D3DSetMagFilter( 0, DXTexFilterMap[m_dwMagFilter].realFilter );
	}
	else
	{
		for( int i=0; i<m_curCombineInfo.nStages; i++ )
		{
			D3DSetMinFilter( i, DXTexFilterMap[m_dwMinFilter].realFilter );
			D3DSetMagFilter( i, DXTexFilterMap[m_dwMagFilter].realFilter );
		}
	}
}

void D3DRender::SetShadeMode(RenderShadeMode mode)
{
	if (gRSP.shadeMode != mode)
	{
		gRSP.shadeMode = mode;
		if( mode == SHADE_DISABLED || mode == SHADE_FLAT )	//Shade is disabled, use Primitive color for flat shade
		{
			g_pD3DDev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
		}
		else
			g_pD3DDev->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	}
}

void D3DRender::SetAlphaRef(DWORD dwAlpha)
{
	if (m_dwAlpha != dwAlpha)
	{
		m_dwAlpha = dwAlpha;
		SetD3DRSAlphaRef(dwAlpha);
	}
}

void D3DRender::ForceAlphaRef(DWORD dwAlpha)
{
	SetD3DRSAlphaRef(dwAlpha);
	m_dwrsAlphaRef = dwAlpha;
	g_pD3DDev->SetRenderState( D3DRS_ALPHAREF, dwAlpha );	
}

bool D3DRender::SetCurrentTexture(int tile, CTexture *handler, DWORD dwTileWidth, DWORD dwTileHeight, TextureEntry *pTextureEntry)
{
	RenderTexture &texture = g_textures[tile];
	texture.pTextureEntry = pTextureEntry;

	if( handler != NULL  && texture.m_lpsTexturePtr != handler->GetTexture() )
	{
		if( texture.m_lpsTexturePtr )
		{
			LPDIRECT3DTEXTURE8(texture.m_lpsTexturePtr)->Release();
			texture.m_lpsTexturePtr = NULL;
		}

		texture.m_lpsTexturePtr = LPDIRECT3DTEXTURE8(handler->GetTexture());
		texture.m_pCTexture = handler;

		if (texture.m_lpsTexturePtr != NULL)
		{
			LPDIRECT3DTEXTURE8(texture.m_lpsTexturePtr)->AddRef();
		}
			
		texture.m_dwTileWidth = dwTileWidth;
		texture.m_dwTileHeight = dwTileHeight;

		texture.m_fTexWidth = (float)handler->m_dwCreatedTextureWidth;
		texture.m_fTexHeight = (float)handler->m_dwCreatedTextureHeight;
	}

	return true;
}

bool D3DRender::SetCurrentTexture(int tile, TextureEntry *pEntry)
{
	if (pEntry != NULL && pEntry->pTexture != NULL)
	{	
		SetCurrentTexture( tile, pEntry->pTexture,pEntry->ti.WidthToCreate, pEntry->ti.HeightToCreate, pEntry);
		return true;
	}
	else
	{
		SetCurrentTexture( tile, NULL, 64, 64, NULL );
		return false;
	}
}


void D3DRender::SetFillMode(FillMode mode)
{
	switch( mode )
	{
	case RICE_FILLMODE_WINFRAME:
		g_pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		break;
	case RICE_FILLMODE_SOLID:
		g_pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID );
		break;
	}
}

void D3DRender::SetAlphaTestEnable(BOOL bAlphaTestEnable)
{
#ifdef _DEBUG
	SetD3DRSAlphaTestEnable((bAlphaTestEnable && debuggerEnableAlphaTest));
#else
	SetD3DRSAlphaTestEnable((bAlphaTestEnable));
#endif
}

void D3DRender::SetFogMinMax(float fMin, float fMax, float fMul, float fOffset)
{
	
	::SetFogMinMax(fMin, fMax, fMul, fOffset);
	float fmin = fMin/1000;
	float fmax = fMax/1000;
	g_pD3DDev->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&fmin));
	g_pD3DDev->SetRenderState(D3DRS_FOGEND,   *(DWORD *)(&fmax));
	

	//g_pD3DDev->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&gRSPfFogMin));
	//g_pD3DDev->SetRenderState(D3DRS_FOGEND,   *(DWORD *)(&gRSPfFogMax));
}

void D3DRender::TurnFogOnOff(BOOL flag)
{
	g_pD3DDev->SetRenderState( D3DRS_FOGENABLE, flag);
}

#define RSP_ZELDA_CULL_FRONT 0x00000400
void D3DRender::SetFogEnable(BOOL bEnable)
{
	if( options.enableHackForGames == HACK_FOR_TWINE && gRSP.bFogEnabled == FALSE && bEnable == FALSE && (gRDP.geometryMode & RSP_ZELDA_CULL_FRONT) )
	{
		g_pD3DDev->Clear(1, NULL, D3DCLEAR_ZBUFFER, 0xFF000000, 1.0, 0);
	}



	gRSP.bFogEnabled = bEnable&&options.bEnableFog;
	//DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("Set Fog %s", bEnable?"enable":"disable");});
	
	//g_pD3DDev->SetRenderState( D3DRS_FOGENABLE, FALSE);
	//return;		//Fog does work, need to fix

	if( gRSP.bFogEnabled )
	{
		g_pD3DDev->SetRenderState( D3DRS_FOGENABLE, TRUE);
		g_pD3DDev->SetRenderState(D3DRS_FOGCOLOR, gRDP.fogColor);

		//g_pD3DDev->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&gRSPfFogMin));
		//g_pD3DDev->SetRenderState(D3DRS_FOGEND,   *(DWORD *)(&gRSPfFogMax));

		g_pD3DDev->SetRenderState(D3DRS_RANGEFOGENABLE, TRUE); // weinersch - use 5.10 method
	}
	else
	{
		g_pD3DDev->SetRenderState( D3DRS_FOGENABLE, FALSE);
	}
}

void D3DRender::SetFogColor(DWORD r, DWORD g, DWORD b, DWORD a)
{
	gRDP.fogColor = COLOR_RGBA(r, g, b, a); 
    g_pD3DDev->SetRenderState(D3DRS_FOGCOLOR, gRDP.fogColor);
}

void D3DRender::ClearBuffer(bool cbuffer, bool zbuffer)
{
	float depth = ((gRDP.originalFillColor&0xFFFF)>>2)/(float)0x3FFF;
	if( cbuffer ) g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, depth, 0);
	if( zbuffer ) g_pD3DDev->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0xFF000000, depth, 0);
}

void D3DRender::ClearZBuffer(float depth)
{
	g_pD3DDev->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0xFF000000, depth, 0);
}

void D3DRender::ClearBuffer(bool cbuffer, bool zbuffer, D3DRECT &rect)
{
	float depth = ((gRDP.originalFillColor&0xFFFF)>>2)/(float)0x3FFF;
	if( cbuffer ) g_pD3DDev->Clear(1, &rect, D3DCLEAR_TARGET, 0xFF000000, depth, 0);
	if( zbuffer ) g_pD3DDev->Clear(1, &rect, D3DCLEAR_ZBUFFER, 0xFF000000, depth, 0);
}


void D3DRender::UpdateScissor()
{
	if( options.bEnableHacks && g_CI.dwWidth == 0x200 && gRDP.scissor.right == 0x200 && g_CI.dwWidth>(*g_GraphicsInfo.VI_WIDTH_REG & 0xFFF) )
	{
		// Hack for RE2
		DWORD width = *g_GraphicsInfo.VI_WIDTH_REG & 0xFFF;
		DWORD height = (gRDP.scissor.right*gRDP.scissor.bottom)/width;
		D3DVIEWPORT8 vp = {0, 0, (DWORD)(width*windowSetting.fMultX), (DWORD)(height*windowSetting.fMultY), 0, 1};
		if( !gRSP.bNearClip )
			vp.MinZ = -10000;
		g_pD3DDev->SetViewport(&vp);
	}
	else
	{
		UpdateScissorWithClipRatio();
	}
}

void D3DRender::ApplyRDPScissor()
{
	if( options.bEnableHacks && g_CI.dwWidth == 0x200 && gRDP.scissor.right == 0x200 && g_CI.dwWidth>(*g_GraphicsInfo.VI_WIDTH_REG & 0xFFF) )
	{
		// Hack for RE2
		DWORD width = *g_GraphicsInfo.VI_WIDTH_REG & 0xFFF;
		DWORD height = (gRDP.scissor.right*gRDP.scissor.bottom)/width;
		D3DVIEWPORT8 vp = {0, 0, (DWORD)(width*windowSetting.fMultX), (DWORD)(height*windowSetting.fMultY), 0, 1};
		if( !gRSP.bNearClip )
			vp.MinZ = -10000;
		g_pD3DDev->SetViewport(&vp);
	}
	else
	{
		D3DVIEWPORT8 vp = {
			(DWORD)(gRDP.scissor.left*windowSetting.fMultX), 
				(DWORD)(gRDP.scissor.top*windowSetting.fMultY), 
				(DWORD)((gRDP.scissor.right-gRDP.scissor.left+1)*windowSetting.fMultX), 
				(DWORD)((gRDP.scissor.bottom-gRDP.scissor.top+1)*windowSetting.fMultY), 0, 1
		};
		if( !gRSP.bNearClip )
			vp.MinZ = -10000;
		g_pD3DDev->SetViewport(&vp);
	}
}

void D3DRender::ApplyScissorWithClipRatio()
{
	D3DVIEWPORT8 vp = {
		(DWORD)(gRSP.real_clip_scissor_left*windowSetting.fMultX), 
			(DWORD)(gRSP.real_clip_scissor_top*windowSetting.fMultY), 
			(DWORD)((gRSP.real_clip_scissor_right-gRSP.real_clip_scissor_left+1)*windowSetting.fMultX), 
			(DWORD)((gRSP.real_clip_scissor_bottom-gRSP.real_clip_scissor_top+1)*windowSetting.fMultY), 0, 1
	};
	if( !gRSP.bNearClip )
		vp.MinZ = -10000;
	g_pD3DDev->SetViewport(&vp);
}

void D3DRender::BeginRendering(void) 
{
	g_pD3DDev->BeginScene();
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	g_pD3DDev->SetTransform( D3DTS_WORLD, &mat );
	g_pD3DDev->SetTransform( D3DTS_PROJECTION, &mat );
	g_pD3DDev->SetTransform( D3DTS_VIEW, &mat );
}

void D3DRender::CaptureScreen(char *filename)
{
	 
}

void D3DRender::SetCullMode(bool bCullFront, bool bCullBack)
{
	CRender::SetCullMode(bCullFront, bCullBack);
	/*
	if( bCullFront && bCullBack )
	{
		g_pD3DDev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );
		g_pD3DDev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
	}
	else if( bCullFront )
	{
		g_pD3DDev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );
	}
	else if( bCullBack )
	{
		g_pD3DDev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
	}
	else
	{
		g_pD3DDev->SetRenderState( D3DRS_CULLMODE,   D3DCULL_NONE );
	}
	*/
}

void D3DRender::D3DSetMinFilter(DWORD dwStage, DWORD filter)
{
	if (m_D3DCombStages[dwStage].dwMinFilter != filter)
	{
		m_D3DCombStages[dwStage].dwMinFilter = filter;

		if( filter == D3DTEXF_LINEAR && options.DirectXAnisotropyValue > 0 )
		{
			// Use Anisotropy filter instead of LINEAR filter
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC  );
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MAXANISOTROPY, min(options.DirectXAnisotropyValue, (DWORD)CGraphicsContext::m_maxAnisotropy) );
		}
		else
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MINFILTER, filter );
	}
}

void D3DRender::D3DSetMagFilter(DWORD dwStage, DWORD filter)
{
	if (m_D3DCombStages[dwStage].dwMagFilter != filter)
	{
		m_D3DCombStages[dwStage].dwMagFilter = filter;
		if( filter == D3DTEXF_LINEAR && options.DirectXAnisotropyValue > 0 )
		{
			// Use Anisotropy filter instead of LINEAR filter
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC  );
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MAXANISOTROPY, min(options.DirectXAnisotropyValue, (DWORD)CGraphicsContext::m_maxAnisotropy) );
		}
		else
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MAGFILTER, filter );
	}
}

