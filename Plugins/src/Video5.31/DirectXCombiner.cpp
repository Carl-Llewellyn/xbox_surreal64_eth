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

static std::vector<u64> g_UnkMux;
extern char* constStr(DWORD op);
int g_dwDirectXCombinerType=DX_BEST_FIT;
DWORD constFactorMap[2];		//1 map to TFACTOR, 2 map to SPECULAR

char * BlendFuncStr[] =
{
	"ENABLE_BOTH",
	"DISABLE_ALPHA",
	"DISABLE_COLOR",
	"DISABLE_BOTH",
	"COLOR_ONE",
	"ALPHA_ONE",
};

char* opStrs[] = {
	"D3DTOP_DISABLE",
	"D3DTOP_SELECTARG1",
	"D3DTOP_SELECTARG2",
	"D3DTOP_MODULATE",
	"D3DTOP_MODULATE2X",
	"D3DTOP_MODULATE4X",
	"D3DTOP_ADD",
	"D3DTOP_ADDSIGNED",
	"D3DTOP_ADDSIGNED2X",
	"D3DTOP_SUBTRACT",
	"D3DTOP_ADDSMOOTH",
	"D3DTOP_BLENDDIFFUSEALPHA",
	"D3DTOP_BLENDTEXTUREALPHA",
	"D3DTOP_BLENDFACTORALPHA",
	"D3DTOP_BLENDTEXTUREALPHAPM",
	"D3DTOP_BLENDCURRENTALPHA",
	"D3DTOP_PREMODULATE",
	"D3DTOP_MODULATEALPHA_ADDCOLOR",
	"D3DTOP_MODULATECOLOR_ADDALPHA",
	"D3DTOP_MODULATEINVALPHA_ADDCOLOR",
	"D3DTOP_MODULATEINVCOLOR_ADDALPHA",
	"D3DTOP_BUMPENVMAP",
	"D3DTOP_BUMPENVMAPLUMINANCE",
	"D3DTOP_DOTPRODUCT3",
	"D3DTOP_MULTIPLYADD",
	"D3DTOP_LERP"
};

char *oprandStrs[] = {
	"D3DTA_DIFFUSE",
	"D3DTA_CURRENT",
	"D3DTA_TEXTURE",
	"D3DTA_TFACTOR",
	"D3DTA_SPECULAR",
	"D3DTA_TEMP",
};

char* OpStr(DWORD op, bool detail=false)
{
	if( op <= D3DTOP_LERP && op >= 1 )
	{
		if( detail )
			return opStrs[op-1];
		else
			return opStrs[op-1]+7;
	}
	else
	{
		return "Invalid-Op";
	}
}

char* OprandStr(DWORD op, char *buf, bool detail=false)
{
	if( (op&7) <= 5 )
	{
		if( detail )
			strcpy(buf, oprandStrs[op&7]);
		else
			strcpy(buf, oprandStrs[op&7]+6);

		if( op&D3DTA_COMPLEMENT )
		{
			if( detail )
				strcat(buf, "|D3DTA_COMPLEMENT");
			else
				strcat(buf,"|C");
		}
		if( op&D3DTA_ALPHAREPLICATE )
		{
			if( detail )
				strcat(buf, "|D3DTA_ALPHAREPLICATE");	
			else
				strcat(buf, "|A");
		}
		return buf;
	}
	else
	{
		if( op == D3DTA_IGNORE )
		{
			return "_";
		}
		else
		{
			return "Invalid-Oprand";
		}
	}
}




//========================================================================
extern D3DCAPS8 g_D3DDeviceCaps;

CDirectXColorCombiner::CDirectXColorCombiner(CDaedalusRender *pRender)
	:CColorCombiner(pRender), m_pD3DRender((D3DRender*)pRender)
{
	m_pDecodedMux = new CDirectXDecodedMux;
	m_ppGeneralDecodedMux = &m_pDecodedMux;

	m_lastMux = 0;
	m_lastIndex = 0;

	m_bCapsTxtOpAdd = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_ADD)!=0;
	m_bCapsTxtOpAddSmooth = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH)!=0;
	m_bCapsTxtOpBlendCurAlpha = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA )!=0;
	m_bCapsTxtOpBlendDifAlpha = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_BLENDDIFFUSEALPHA )!=0;
	m_bCapsTxtOpBlendFacAlpha = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA )!=0;
	m_bCapsTxtOpBlendTxtAlpha = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA )!=0;
	m_bCapsTxtOpBlendTxtAlphaPm = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHAPM )!=0;
	m_bCapsTxtOpLerp = true; //(g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_LERP )!=0;
	m_bCapsTxtOpModAlphaAddColor = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR )!=0;
	m_bCapsTxtOpModColorAddAlpha = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA )!=0;
	m_bCapsTxtOpModInvAlphaAddColor = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR )!=0;
	m_bCapsTxtOpModInvColorAddAlpha = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA )!=0;
	m_bCapsTxtOpMulAdd = true; // (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD )!=0;
	m_bCapsTxtOpPreMod = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_PREMODULATE )!=0;
	m_bCapsTxtOpSub = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_SUBTRACT )!=0;
	
	m_dwCapsMaxTxtWidth=g_D3DDeviceCaps.MaxTextureWidth;
	m_dwCapsMaxTxtHeight=g_D3DDeviceCaps.MaxTextureHeight;

	m_dwCapsMaxTxt=g_D3DDeviceCaps.MaxSimultaneousTextures;
	m_dwCapsMaxStages=g_D3DDeviceCaps.MaxTextureBlendStages;

	if( m_dwCapsMaxTxt == 1 )
	{
		m_pDecodedMux->m_maxTextures = 1;
	}


	char deviceStr[200];
	strcpy(deviceStr, CGraphicsContext::g_pGraphicsContext->GetDeviceStr());
	strlwr(deviceStr);

	// Check for Nvidia video cards
	if( strstr(deviceStr, "tnt") != NULL )
	{
		//m_dwCapsMaxStages = 2;
		//m_bCapsTxtOpMulAdd = false;
		//m_bCapsTxtOpLerp = false;
	}
	else if( strstr(deviceStr, "geforce") != NULL && strstr(deviceStr,"ti") == NULL && strstr(deviceStr,"geforce3") == NULL )
	{
		// Geforce 2, Geforce 256, but not geforce 2/3/4 ti, Geforce 2 MX, Geforce 4 MX
		//m_dwCapsMaxStages = 2;
		//m_bCapsTxtOpMulAdd = false;
		//m_bCapsTxtOpLerp = false;
	}

	m_supportedStages = m_dwCapsMaxStages;


	m_bTxtOpAdd = m_bCapsTxtOpAdd;
	m_bTxtOpSub = m_bCapsTxtOpSub;
	m_bTxtOpLerp = m_bCapsTxtOpLerp;				

	m_bTxtOpAddSmooth = m_bCapsTxtOpAddSmooth;			
	m_bTxtOpBlendCurAlpha = m_bCapsTxtOpBlendCurAlpha;		
	m_bTxtOpBlendDifAlpha = m_bCapsTxtOpBlendDifAlpha;		
	m_bTxtOpBlendFacAlpha = m_bCapsTxtOpBlendFacAlpha;		
	m_bTxtOpBlendTxtAlpha = m_bCapsTxtOpBlendTxtAlpha;		
	m_bTxtOpMulAdd = m_bCapsTxtOpMulAdd;			

	///m_supportedStages = m_dwCapsMaxStages = 3;
	///m_bCapsTxtOpMulAdd = false;
	///m_bCapsTxtOpLerp = false;

	m_dwGeneralMaxStages = m_supportedStages;

	TRACE0("Create The default DirectX Combiner");
}

CDirectXColorCombiner::~CDirectXColorCombiner()
{
	for( int k=0; k<m_GeneratedMuxs.size(); k++ )
	{
		if( m_GeneratedMuxs[k] != NULL )
			delete m_GeneratedMuxs[k];
	}
	m_GeneratedMuxs.clear();
}

bool CDirectXColorCombiner::Initialize(void)
{
	return true;
}

void CDirectXColorCombiner::SetNumStages(DWORD dwMaxStage)
{
	// Number of stages has been reduced
	// Disable dwNum+1
	if (dwMaxStage < 7)
	{
		m_pD3DRender->D3DSetColorOp( dwMaxStage+1, D3DTOP_DISABLE );
		m_pD3DRender->D3DSetAlphaOp( dwMaxStage+1, D3DTOP_DISABLE );
	}
}

void CDirectXColorCombiner::DisableCombiner(void)
{
	if( m_dwCapsMaxStages == 1 || !m_bTex1Enabled || !m_bTex0Enabled || options.bWinFrameMode )
	{
		m_pD3DRender->D3DSetColorOp( 1, D3DTOP_DISABLE );
		m_pD3DRender->D3DSetAlphaOp( 1, D3DTOP_DISABLE );

		m_pD3DRender->m_curCombineInfo.m_dwShadeColorChannelFlag = 0;
		m_pD3DRender->m_curCombineInfo.m_dwShadeAlphaChannelFlag = 0;
		m_pD3DRender->m_curCombineInfo.specularPostOp = 0;
		m_pD3DRender->m_curCombineInfo.nStages = 1;

		if( m_bTexelsEnable && !options.bWinFrameMode )
		{
			if( m_pDecodedMux->isUsed(MUX_SHADE) )
			{
				m_pD3DRender->D3DSetColorOp( 0, D3DTOP_MODULATE );
				m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TEXTURE );
				m_pD3DRender->D3DSetColorArg2( 0, D3DTA_DIFFUSE );

				m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_MODULATE );
				m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_TEXTURE );
				m_pD3DRender->D3DSetAlphaArg2( 0, D3DTA_DIFFUSE );
			}
			else
			{
				m_pD3DRender->D3DSetColorOp( 0, D3DTOP_SELECTARG1 );
				m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TEXTURE );

				m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_SELECTARG1 );
				m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_TEXTURE );
			}

			if( m_bTex0Enabled )
			{
				m_pD3DRender->D3DSetTexture( 0, LPDIRECT3DTEXTURE8(g_textures[gRSP.curTile].m_lpsTexturePtr) );
				m_pD3DRender->SetTexelRepeatFlags(gRSP.curTile);
			}
			else
			{
				m_pD3DRender->D3DSetTexture( 0, LPDIRECT3DTEXTURE8(g_textures[(gRSP.curTile+1)&7].m_lpsTexturePtr) );
				m_pD3DRender->SetTexelRepeatFlags((gRSP.curTile+1)&7);
			}

			m_pD3DRender->m_curCombineInfo.stages[0].bTextureUsed = true;
		}
		else
		{
			m_pD3DRender->D3DSetColorOp( 0, D3DTOP_SELECTARG1 );
			m_pD3DRender->D3DSetColorArg1( 0, D3DTA_DIFFUSE );

			m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_SELECTARG1 );
			m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_DIFFUSE );
			m_pD3DRender->D3DSetTexture( 0, NULL );

			m_pD3DRender->m_curCombineInfo.stages[0].bTextureUsed = false;
		}
	}
	else
	{
		// Both textures are enabled, and we have at least 2 combiner stages

		m_pD3DRender->D3DSetColorOp( 2, D3DTOP_DISABLE );
		m_pD3DRender->D3DSetAlphaOp( 2, D3DTOP_DISABLE );
		m_pD3DRender->m_curCombineInfo.nStages = 2;

		m_pD3DRender->m_curCombineInfo.specularPostOp = 0;

		BYTE mask = ~MUX_COMPLEMENT;

		if( m_pDecodedMux->isUsedInColorChannel(MUX_SHADE,mask) )
		{
			m_pD3DRender->m_curCombineInfo.m_dwShadeColorChannelFlag = 0;
		}
		else if( m_pDecodedMux->isUsedInColorChannel(MUX_PRIM,mask) )
		{
			m_pD3DRender->m_curCombineInfo.m_dwShadeColorChannelFlag = MUX_PRIM;
		}
		else if( m_pDecodedMux->isUsedInColorChannel(MUX_ENV,mask) )
		{
			m_pD3DRender->m_curCombineInfo.m_dwShadeColorChannelFlag = MUX_ENV;
		}

		if( m_pDecodedMux->isUsedInColorChannel(MUX_SHADE|MUX_ALPHAREPLICATE,mask) ||
			m_pDecodedMux->isUsedInAlphaChannel(MUX_SHADE,mask) )
		{
			m_pD3DRender->m_curCombineInfo.m_dwShadeAlphaChannelFlag = 0;
		}
		else if( m_pDecodedMux->isUsedInColorChannel(MUX_PRIM|MUX_ALPHAREPLICATE,mask) ||
				 m_pDecodedMux->isUsedInAlphaChannel(MUX_PRIM,mask) )
		{
			m_pD3DRender->m_curCombineInfo.m_dwShadeAlphaChannelFlag = MUX_PRIM;
		}
		else if( m_pDecodedMux->isUsedInColorChannel(MUX_ENV|MUX_ALPHAREPLICATE,mask) ||
				 m_pDecodedMux->isUsedInAlphaChannel(MUX_ENV,mask) )
		{
			m_pD3DRender->m_curCombineInfo.m_dwShadeAlphaChannelFlag = MUX_ENV;
		}

		if( !options.bWinFrameMode )
		{
			if( m_pDecodedMux->isUsedInColorChannel(MUX_SHADE, mask) || m_pD3DRender->m_curCombineInfo.m_dwShadeColorChannelFlag != 0 )
			{
				m_pD3DRender->D3DSetColorOp( 0, D3DTOP_MODULATE );
				m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TEXTURE );
				m_pD3DRender->D3DSetColorArg2( 0, D3DTA_DIFFUSE );
			}
			else if( m_pDecodedMux->isUsedInColorChannel(MUX_SHADE|MUX_ALPHAREPLICATE, mask) ||
				m_pDecodedMux->isUsedInColorChannel(MUX_PRIM|MUX_ALPHAREPLICATE, mask) ||
				m_pDecodedMux->isUsedInColorChannel(MUX_ENV|MUX_ALPHAREPLICATE, mask))
			{
				m_pD3DRender->D3DSetColorOp( 0, D3DTOP_MODULATE );
				m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TEXTURE );
				m_pD3DRender->D3DSetColorArg2( 0, D3DTA_DIFFUSE|D3DTA_ALPHAREPLICATE );
			}
			else
			{
				m_pD3DRender->D3DSetColorOp( 0, D3DTOP_SELECTARG1 );
				m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TEXTURE );
			}

			if( m_pDecodedMux->isUsedInAlphaChannel(MUX_SHADE) || m_pD3DRender->m_curCombineInfo.m_dwShadeAlphaChannelFlag != 0 )
			{
				m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_MODULATE );
				m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_TEXTURE );
				m_pD3DRender->D3DSetAlphaArg2( 0, D3DTA_DIFFUSE );
			}
			else
			{
				m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_SELECTARG1 );
				m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_TEXTURE );
			}

			m_pD3DRender->D3DSetColorOp( 1, D3DTOP_MODULATE );
			m_pD3DRender->D3DSetColorArg1( 1, D3DTA_TEXTURE );
			m_pD3DRender->D3DSetColorArg2( 1, D3DTA_CURRENT );

			m_pD3DRender->D3DSetAlphaOp( 1, D3DTOP_MODULATE );
			m_pD3DRender->D3DSetAlphaArg1( 1, D3DTA_TEXTURE );
			m_pD3DRender->D3DSetAlphaArg2( 1, D3DTA_CURRENT );

			m_pD3DRender->D3DSetTexture( 0, LPDIRECT3DTEXTURE8(g_textures[gRSP.curTile].m_lpsTexturePtr) );
			m_pD3DRender->D3DSetTexture( 1, LPDIRECT3DTEXTURE8(g_textures[(gRSP.curTile+1)&7].m_lpsTexturePtr) );

			m_pD3DRender->SetTexelRepeatFlags(gRSP.curTile);
			m_pD3DRender->SetTexelRepeatFlags((gRSP.curTile+1)&7);

			m_pD3DRender->m_curCombineInfo.stages[0].bTextureUsed = true;
			m_pD3DRender->m_curCombineInfo.stages[1].bTextureUsed = true;
		}
	}
}

void CDirectXColorCombiner::InitCombinerCycleCopy(void)
{
	m_pD3DRender->D3DSetColorOp( 1, D3DTOP_DISABLE );
	m_pD3DRender->D3DSetAlphaOp( 1, D3DTOP_DISABLE );

	m_pD3DRender->SetD3DRSAlphaBlendEnable(FALSE);
	m_pD3DRender->SetD3DRSAlphaTestEnable(TRUE);
	
	SetNumStages(0);
	m_pD3DRender->m_curCombineInfo.stages[0].bTextureUsed = true;
	m_pD3DRender->m_curCombineInfo.nStages = 1;

	m_pD3DRender->D3DSetTexture( 0, LPDIRECT3DTEXTURE8(g_textures[gRSP.curTile].m_lpsTexturePtr) );
	m_pD3DRender->SetTexelRepeatFlags(gRSP.curTile);
	m_pD3DRender->D3DSetTexture( 1, NULL );
	g_pD3DDev->SetPixelShader( NULL );

	m_pD3DRender->D3DSetColorOp( 0, D3DTOP_SELECTARG1 );
	m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TEXTURE );

	m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_SELECTARG1 );
	m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_TEXTURE );
}

void CDirectXColorCombiner::InitCombinerCycle12(void)
{
	bool two_cycles = (gRDP.otherMode.cycle_type == CYCTYPE_1CYCLE);
	bool combinerChanged = FindAndFillCombineMode(m_pD3DRender->m_curCombineInfo) || m_bCycleChanged;

	BOOL AlphaIsEnable = (m_pD3DRender->m_curCombineInfo.blendingFunc!=DISABLE_ALPHA&&m_pD3DRender->m_curCombineInfo.blendingFunc!=DISABLE_BOTH);

	if( !AlphaIsEnable ) 
		m_pD3DRender->SetD3DRSAlphaBlendEnable(AlphaIsEnable);

	if( m_pD3DRender->m_curCombineInfo.blendingFunc==DISABLE_ALPHA )	//Disable Alpha Channel
	{
		//Alpha = 1
		m_pD3DRender->SetD3DRSSrcBlend( D3DBLEND_ONE);
		m_pD3DRender->SetD3DRSDestBlend(D3DBLEND_ZERO);
	}
	else if( m_pD3DRender->m_curCombineInfo.blendingFunc==DISABLE_COLOR)	//Disable Color Channel
	{
		//Color = 0
		m_pD3DRender->SetD3DRSSrcBlend(D3DBLEND_SRCALPHA);
		m_pD3DRender->SetD3DRSDestBlend(D3DBLEND_INVSRCALPHA);
	}
	else if( m_pD3DRender->m_curCombineInfo.blendingFunc==DISABLE_BOTH)	//Disable Color and Alpha Channel
	{
		//Color = 0
		m_pD3DRender->SetD3DRSSrcBlend( D3DBLEND_ZERO);
		m_pD3DRender->SetD3DRSDestBlend(D3DBLEND_ZERO);
	}

	// Allows a combine mode to select what TFACTOR should be
	if( m_pD3DRender->m_curCombineInfo.TFactor != MUX_0 )
	{
		m_pD3DRender->SetBlendFactor(m_pD3DRender->m_curCombineInfo.TFactor);
	}

	if( m_bTex0Enabled )	m_pD3DRender->SetTexelRepeatFlags(gRSP.curTile);
	if( m_bTex1Enabled )			m_pD3DRender->SetTexelRepeatFlags((gRSP.curTile+1)&7);

	for (int i = 0; i < m_pD3DRender->m_curCombineInfo.nStages; i++)
	{
		if( gRDP.texturesAreReloaded || combinerChanged || m_bCycleChanged )
		{
			bool usingTextureForColor = m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg1 == D3DTA_TEXTURE || m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg2 == D3DTA_TEXTURE || m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg0 == D3DTA_TEXTURE;
			bool usingTextureForAlpha = m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg1 == D3DTA_TEXTURE || m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg2 == D3DTA_TEXTURE || m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg0 == D3DTA_TEXTURE;
			if( usingTextureForColor || ( usingTextureForAlpha && AlphaIsEnable ) )
			{
				if( m_pD3DRender->m_curCombineInfo.stages[i].dwTexture == 0 )	//Use Texel0
				{
					if( g_textures[gRSP.curTile].m_pCTexture != NULL )
						m_pD3DRender->D3DSetTexture( i, LPDIRECT3DTEXTURE8(g_textures[gRSP.curTile].m_lpsTexturePtr) );
					else
					{
						// This makes sure that the stage has a valid texture binded to
						m_pD3DRender->D3DSetTexture( i, LPDIRECT3DTEXTURE8(g_textures[(gRSP.curTile+1)&7].m_lpsTexturePtr) );
					}
				}
				else
				{
					m_pD3DRender->D3DSetTexture( i, LPDIRECT3DTEXTURE8(g_textures[(gRSP.curTile+1)&7].m_lpsTexturePtr) );
				}
			}
			else
			{
				m_pD3DRender->D3DSetTexture( i, NULL );
			}
		}
		
		//g_pD3DDev->SetTextureStageState( i, D3DTSS_TEXCOORDINDEX, m_pD3DRender->m_curCombineInfo.stages[i].dwTexture );

		if( combinerChanged || m_bCycleChanged)
		{
			m_pD3DRender->D3DSetTexCoordIndex( i, m_pD3DRender->m_curCombineInfo.stages[i].dwTexture );

			m_pD3DRender->D3DSetColorOp( i, m_pD3DRender->m_curCombineInfo.stages[i].colorOp.op );
			m_pD3DRender->D3DSetColorArg1( i, m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg1 );

			if( m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg2 != D3DTA_IGNORE )
			{
				m_pD3DRender->D3DSetColorArg2( i, m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg2 );
			}

			if( m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg0 != 0 )
			{
				m_pD3DRender->D3DSetColorArg0( i, m_pD3DRender->m_curCombineInfo.stages[i].colorOp.Arg0 );
			}


			if (AlphaIsEnable)
			{
				m_pD3DRender->D3DSetAlphaOp( i, m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.op );
				m_pD3DRender->D3DSetAlphaArg1( i, m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg1 );

				if( m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg2 != D3DTA_IGNORE )
				{
					m_pD3DRender->D3DSetAlphaArg2( i, m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg2 );
				}

				if( m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg0 != 0 )
				{
					m_pD3DRender->D3DSetAlphaArg0( i, m_pD3DRender->m_curCombineInfo.stages[i].alphaOp.Arg0 );
				}
			}
		}
	}

	gRDP.texturesAreReloaded = false;

	// Cap off the texture stages
	m_pD3DRender->D3DSetColorOp(m_pD3DRender->m_curCombineInfo.nStages, D3DTOP_DISABLE);
	m_pD3DRender->D3DSetAlphaOp(m_pD3DRender->m_curCombineInfo.nStages, D3DTOP_DISABLE);

	if( m_pD3DRender->m_curCombineInfo.blendingFunc==DISABLE_COLOR )
	{
		m_pD3DRender->SetBlendFactor(0);
		m_pD3DRender->D3DSetColorOp( 0, D3DTOP_SELECTARG1 );
		m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TFACTOR );
		//m_pD3DRender->D3DSetColorArg1( 0, D3DTA_DIFFUSE );
	}
}

void CDirectXColorCombiner::InitCombinerCycleFill(void)
{
	m_pD3DRender->D3DSetColorOp( 1, D3DTOP_DISABLE );
	m_pD3DRender->D3DSetAlphaOp( 1, D3DTOP_DISABLE );

	SetNumStages(0);
	m_pD3DRender->m_curCombineInfo.stages[0].bTextureUsed = true;

	m_pD3DRender->D3DSetTexture( 0, NULL );
	m_pD3DRender->D3DSetTexture( 1, NULL );
	g_pD3DDev->SetPixelShader( NULL );

	m_pD3DRender->D3DSetColorOp( 0, D3DTOP_SELECTARG1 );
	m_pD3DRender->D3DSetColorArg1( 0, D3DTA_DIFFUSE );

	m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_SELECTARG1 );
	m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_DIFFUSE );
}

bool CDirectXColorCombiner::FindAndFillCombineMode(SetCombineInfo & sci)
{
#ifdef _DEBUG
	if( debuggerDropCombinerInfos )
	{
		debuggerDropCombinerInfos = false;
		for( int k=0; k<m_GeneratedMuxs.size(); k++ )
		{
			if( m_GeneratedMuxs[k] != NULL )
				delete m_GeneratedMuxs[k];
		}
		m_GeneratedMuxs.clear();
	}
#endif

	//Now search the GenerateMux array
	if( sci.mux == m_pD3DRender->m_Mux )
		return false;
	else
	{
		int index = m_GeneratedMuxs.find(m_pD3DRender->m_Mux);
		if( index >= 0 )
		{
			sci = *m_GeneratedMuxs[index];
			gRSP.bProcessSpecularColor = (sci.specularPostOp!=0);
			return true;
		}
		else
		{
			SetCombineInfo *pinfo = new SetCombineInfo;
			GenerateD3DCombineInfo(*pinfo);
			m_GeneratedMuxs.add(m_pD3DRender->m_Mux, pinfo);
			pinfo->mux = m_pD3DRender->m_Mux;
			sci = *pinfo;
			gRSP.bProcessSpecularColor = (sci.specularPostOp!=0);
			return true;
		}
	}
}	

void DisplayBlendingStageInfo(SetCombineInfo &info)
{
#ifdef _DEBUG
	char buf0[100], buf1[100], buf2[100];
	DebuggerAppendMsg("\nStages:%d, Alpha:%s, Factor:%s, Specular:%s Dif Color:0x%X Dif Alpha:0x%X\n", 
		info.nStages, BlendFuncStr[info.blendingFunc], constStr(info.TFactor),
		constStr(info.specularPostOp), info.m_dwShadeColorChannelFlag, info.m_dwShadeAlphaChannelFlag);

	for( int i=0; i<info.nStages; i++ )
	{
		CombineStage &s = info.stages[i];
		DebuggerAppendMsg("%d:Color: %s - %s, %s, %s%s\n", i,
			OpStr(s.colorOp.op), OprandStr(s.colorOp.Arg1, buf1), OprandStr(s.colorOp.Arg2, buf2), 
			s.colorOp.Arg0==MUX_0?"":OprandStr(s.colorOp.Arg0, buf0),
			s.dwTexture!=0?" -Tex1":"");
	}
	for( i=0; i<info.nStages; i++ )
	{
		CombineStage &s = info.stages[i];
		DebuggerAppendMsg("%d:Alpha: %s - %s, %s, %s%s\n", i,
			OpStr(s.alphaOp.op), OprandStr(s.alphaOp.Arg1, buf1), OprandStr(s.alphaOp.Arg2, buf2),
			s.alphaOp.Arg0==MUX_0?"":OprandStr(s.alphaOp.Arg0, buf0),
			s.dwTexture!=0?" -Tex1":"");
	}
	TRACE0("\n\n");
#endif
}

void CDirectXColorCombiner::DisplayBlendingStageInfo(void)
{
	::DisplayBlendingStageInfo(m_pD3DRender->m_curCombineInfo);
}


void D3DRender::SetBlendFactor(DWORD flag)
{
	SetTextureFactor(m_pColorCombiner->GetConstFactor(flag, flag));
}


DaedalusColor D3DRender::PostProcessDiffuseColor(DaedalusColor curDiffuseColor)
{
	DWORD mask = (m_curCombineInfo.blendingFunc != DISABLE_COLOR?0xFFFFFFFF:0xFF000000);

	if( m_curCombineInfo.m_dwShadeColorChannelFlag + m_curCombineInfo.m_dwShadeAlphaChannelFlag == 0 )
	{
		return (curDiffuseColor&mask);
	}

	if( (m_curCombineInfo.m_dwShadeColorChannelFlag & 0xFFFFFF00) == 0 )
	{
		return (m_pColorCombiner->GetConstFactor(m_curCombineInfo.m_dwShadeColorChannelFlag, m_curCombineInfo.m_dwShadeAlphaChannelFlag, curDiffuseColor)&mask);
	}
	else
		return (CalculateConstFactor(m_curCombineInfo.m_dwShadeColorChannelFlag, m_curCombineInfo.m_dwShadeAlphaChannelFlag, curDiffuseColor)&mask);
}

DaedalusColor D3DRender::PostProcessSpecularColor()
{
	if( m_curCombineInfo.specularPostOp == MUX_0 )
	{
		return 0xFFFFFFFF;
	}
	else
	{
		return m_pColorCombiner->GetConstFactor(m_curCombineInfo.specularPostOp, m_curCombineInfo.specularPostOp);
	}
}

void CDirectXColorCombiner::InitCombinerBlenderForSimpleTextureDraw(DWORD tile)
{
	m_pD3DRender->ZBufferEnable( FALSE );

	m_pD3DRender->D3DSetColorOp( 1, D3DTOP_DISABLE );
	m_pD3DRender->D3DSetAlphaOp( 1, D3DTOP_DISABLE );

	m_pD3DRender->SetD3DRSSrcBlend( D3DBLEND_ONE);
	m_pD3DRender->SetD3DRSDestBlend(D3DBLEND_ZERO);
	m_pD3DRender->SetD3DRSAlphaBlendEnable(TRUE);
	//m_pD3DRender->SetD3DRSAlphaBlendEnable(FALSE);
	m_pD3DRender->SetD3DRSAlphaTestEnable(TRUE);
	//m_pD3DRender->SetD3DRSAlphaTestEnable(FALSE);
	
	SetNumStages(0);
	m_pD3DRender->m_curCombineInfo.stages[0].bTextureUsed = true;

	m_pD3DRender->D3DSetTexture( 0, LPDIRECT3DTEXTURE8(g_textures[tile].m_lpsTexturePtr) );
	m_pD3DRender->D3DSetColorOp( 0, D3DTOP_SELECTARG1 );
	m_pD3DRender->D3DSetColorArg1( 0, D3DTA_TEXTURE );

	m_pD3DRender->D3DSetAlphaOp( 0, D3DTOP_SELECTARG1 );
	m_pD3DRender->D3DSetAlphaArg1( 0, D3DTA_TEXTURE );

	m_pD3DRender->m_curCombineInfo.m_dwShadeColorChannelFlag = 0;
	m_pD3DRender->m_curCombineInfo.m_dwShadeAlphaChannelFlag = 0;
	m_pD3DRender->m_curCombineInfo.specularPostOp = 0;

	m_pD3DRender->SetAddressUAllStages( 0, TEXTURE_UV_FLAG_CLAMP );
	m_pD3DRender->SetAddressVAllStages( 0, TEXTURE_UV_FLAG_CLAMP );
}


#ifdef _DEBUG
void CDirectXColorCombiner::DisplaySimpleMuxString(void)
{
	CColorCombiner::DisplaySimpleMuxString();
	TRACE0("\n");

	SetCombineInfo info;
	GenerateD3DCombineInfo(info);
	info.mux = m_pRender->m_Mux;
	::DisplayBlendingStageInfo(info);
	TRACE0("\n\n");
}

#endif

bool CDirectXColorCombiner::IsTextureUsedInStage(CombineStage &stage)
{
	if( (stage.colorOp.Arg1&D3DTA_SELECTMASK)==D3DTA_TEXTURE || (stage.colorOp.Arg2&D3DTA_SELECTMASK)==D3DTA_TEXTURE || (stage.colorOp.Arg0 &D3DTA_SELECTMASK)==D3DTA_TEXTURE ||
		(stage.alphaOp.Arg1&D3DTA_SELECTMASK)==D3DTA_TEXTURE || (stage.alphaOp.Arg2&D3DTA_SELECTMASK)==D3DTA_TEXTURE || (stage.alphaOp.Arg0 &D3DTA_SELECTMASK)==D3DTA_TEXTURE )
	{
		return true;
	}
	else
		return false;
}


DWORD MuxToD3DOpMap[] = {
	D3DTOP_SELECTARG1,	//CM_REPLACE,
	D3DTOP_MODULATE,	//CM_MODULATE,
	D3DTOP_ADD,			//CM_ADD,
	D3DTOP_SUBTRACT,	//CM_SUBTRACT,
	D3DTOP_LERP,		//CM_INTERPOLATE,		

	D3DTOP_ADDSMOOTH,			//CM_ADDSMOOTH,			
	D3DTOP_BLENDCURRENTALPHA,	//CM_BLENDCURRENTALPHA,	
	D3DTOP_BLENDDIFFUSEALPHA,	//CM_BLENDDIFFUSEALPHA,	
	D3DTOP_BLENDFACTORALPHA,	//CM_BLENDFACTORALPHA,	
	D3DTOP_BLENDTEXTUREALPHA,	//CM_BLENDTEXTUREALPHA,	
	D3DTOP_MULTIPLYADD,			//CM_MULTIPLYADD,		
};


DWORD GeneralToD3DMap[] = {
	D3DTA_TFACTOR,	//MUX_0 = 0,
	D3DTA_TFACTOR,	//MUX_1,
	D3DTA_CURRENT,	//MUX_COMBINED,
	D3DTA_TEXTURE,	//MUX_TEXEL0,
	D3DTA_TEXTURE,	//MUX_TEXEL1,
	D3DTA_TFACTOR,	//MUX_PRIM,
	D3DTA_DIFFUSE,	//MUX_SHADE,
	D3DTA_TFACTOR,	//MUX_ENV,
	D3DTA_CURRENT,	//MUX_COMBALPHA,
	D3DTA_TEXTURE,	//MUX_T0_ALPHA,
	D3DTA_TEXTURE,	//MUX_T1_ALPHA,
	D3DTA_TFACTOR,	//MUX_PRIM_ALPHA,
	D3DTA_DIFFUSE,	//MUX_SHADE_ALPHA,
	D3DTA_TFACTOR,	//MUX_ENV_ALPHA,
	D3DTA_TFACTOR,	//MUX_LODFRAC,
	D3DTA_TFACTOR,	//MUX_PRIMLODFRAC,
};

DWORD GeneralGetD3DAlphaOp(DWORD op)
{
#ifdef _DEBUG
	if( op >= D3DTOP_BLENDCURRENTALPHA && op <= D3DTOP_BLENDTEXTUREALPHA )
	{
		DebuggerAppendMsg("Check me, D3D alpha op is wrong");
	}
#endif
	return MuxToD3DOpMap[op];

}
DWORD GeneralGetD3DColorOp(DWORD op)
{
	return MuxToD3DOpMap[op];
}
DWORD CDirectXColorCombiner::GetD3DArgument(DWORD val)
{
	if( val == CM_IGNORE )
		return D3DTA_IGNORE;

	DWORD d3dArg = D3DTA_DIFFUSE;

	if( (val&MUX_MASK) == MUX_PRIM || (val&MUX_MASK) == MUX_ENV )
	{
		if( constFactorMap[0] == 0 )
		{
			if( m_pDecodedMux->HowManyConstFactors() < 2 )
			{
				//Only 1 const factor is used, so just map it to TFACTOR
				constFactorMap[0] = (val&MUX_MASK);
				d3dArg = D3DTA_TFACTOR;
			}
			else
			{
				//Be careful here, two factors are used, need to map one to TFACTOR, and the other
				//one to Specular, but Specular can not be used in Alpha channel combine.
				//So need to check which one is not used in Alpha channel, then map that one to Specular
				//and more the other one to TFACTOR
				if( m_pDecodedMux->isUsedInAlphaChannel(MUX_ENV) )
				{
					if( m_pDecodedMux->isUsedInAlphaChannel(MUX_PRIM) )
					{	
						//For cases that both factors are used in Alpha channel
						//I can not do anything here
						//Can I do something here?
					}

					constFactorMap[0] = MUX_ENV;
					constFactorMap[1] = MUX_PRIM;
				}
				else
				{
					constFactorMap[0] = MUX_PRIM;
					constFactorMap[1] = MUX_ENV;
				}
			}
		}

		if( constFactorMap[0] == (val&MUX_MASK) )
			d3dArg = D3DTA_TFACTOR;
		else
			d3dArg = D3DTA_SPECULAR;
	}
	else
	{
		d3dArg = GeneralToD3DMap[val&MUX_MASK];
	}

	if( val&MUX_COMPLEMENT )
	{
		d3dArg |= D3DTA_COMPLEMENT;
	}

	if( val&MUX_ALPHAREPLICATE )
	{
		d3dArg |= D3DTA_ALPHAREPLICATE;
	}

	return d3dArg;
}
bool CDirectXColorCombiner::GenerateD3DCombineInfo(SetCombineInfo &cinfo)
{
	//if( m_pDecodedMux->m_dwMux0 != m_dwLastMux0 || m_pDecodedMux->m_dwMux1 != m_dwLastMux1 )
	{
		m_lastIndex = CGeneralCombiner::FindCompiledMux();
		if( m_lastIndex < 0 )		// Can not found
		{
			m_lastIndex = CGeneralCombiner::ParseDecodedMux();
		}
		//m_dwLastMux0 = m_pDecodedMux->m_dwMux0;
		//m_dwLastMux1 = m_pDecodedMux->m_dwMux1;
	}

	GeneralCombinerInfo &res = m_vCompiledCombinerStages[m_lastIndex];
	constFactorMap[0] = constFactorMap[1] = 0;

	int i;
	for( i=0; i<8; i++ )
	{
		cinfo.stages[i].dwTexture = 0;
		cinfo.stages[i].alphaOp.Arg0 = D3DTA_IGNORE;
		cinfo.stages[i].colorOp.Arg0 = D3DTA_IGNORE;
	}

	for( i=0; i<res.nStages; i++ )
	{
		CombineStage &stage = cinfo.stages[i];
		GeneralCombineStage &gstage = res.stages[i];

		stage.colorOp.op = GeneralGetD3DColorOp(gstage.colorOp.op);
		stage.alphaOp.op = GeneralGetD3DAlphaOp(gstage.alphaOp.op);
		stage.colorOp.Arg0 = GetD3DArgument(gstage.colorOp.Arg0);
		stage.colorOp.Arg1 = GetD3DArgument(gstage.colorOp.Arg1);
		stage.colorOp.Arg2 = GetD3DArgument(gstage.colorOp.Arg2);
		stage.alphaOp.Arg0 = GetD3DArgument(gstage.alphaOp.Arg0);
		stage.alphaOp.Arg1 = GetD3DArgument(gstage.alphaOp.Arg1);
		stage.alphaOp.Arg2 = GetD3DArgument(gstage.alphaOp.Arg2);

		stage.dwTexture = gstage.dwTexture;
		stage.bTextureUsed = gstage.bTextureUsed;
	}

	cinfo.nStages = res.nStages;
	cinfo.blendingFunc = res.blendingFunc;
	cinfo.m_dwShadeAlphaChannelFlag = m_pDecodedMux->m_dwShadeAlphaChannelFlag;
	cinfo.m_dwShadeColorChannelFlag = m_pDecodedMux->m_dwShadeColorChannelFlag;

	if( res.TFactor != MUX_0 || res.specularPostOp != MUX_0 )
	{
		cinfo.TFactor = res.TFactor;
		cinfo.specularPostOp = res.specularPostOp;
	}
	else
	{
		switch(m_pDecodedMux->HowManyConstFactors())
		{
		case 0:
			cinfo.TFactor = 0;
			cinfo.specularPostOp = 0;
			break;
		case 1:
			cinfo.TFactor = constFactorMap[0];
			cinfo.specularPostOp = 0;
			break;
		case 2:
			cinfo.TFactor = constFactorMap[0];
			cinfo.specularPostOp = constFactorMap[1];
			break;
		}	
	}


	return true;
}
