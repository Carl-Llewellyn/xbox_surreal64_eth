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

#ifndef __DAEDALUS_D3DRENDER_H__
#define __DAEDALUS_D3DRENDER_H__

#include <d3dx8.h>
#include <d3d8types.h>

#include "stdafx.h"
#include "DaedalusGraphics.h"
#include "DLParser.h"		// DL_PF
#include "Combiner.h"
#include "DirectXCombiner.h"
#include "RDP_S2DEX.h"
#include "textureCache.h"

#include "Unique.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)  { if(p) { (p)->Release();     (p)=NULL; } }
#endif

class ofstream;
extern LPDIRECT3DDEVICE8 g_pD3DDev;

class D3DRender : public CDaedalusRender
{
	friend class DirectXDeviceBuilder;
	friend class CDeviceBuilder;
	friend class CDirectXColorCombiner;
	friend class CDirectXColorCombinerLowestEnd;
	friend class CDirectXColorCombinerLowerEnd;
	friend class CDirectXColorCombinerMiddleLevel;
	friend class CDirectXColorCombinerHigherEnd;
	friend class CDirectXColorCombinerNVidiaTNT;
	friend class CDirectXColorCombinerNVidiaGeforcePlus;
	friend class CDirectXColorCombinerATIDadeonPlus;
	//friend class CNvTNTDirectXCombiner;
	friend class CDirectXBlender;
	
protected:
	D3DRender();
	~D3DRender();

public:
	void BeginRendering(void); 
	void EndRendering(void) {g_pD3DDev->EndScene();}

	bool RestoreDeviceObjects();
	bool InvalidateDeviceObjects();

	void CaptureScreen(char *filename);

	void ApplyTextureFilter();

	void SetAddressUAllStages(DWORD dwTile, TextureUVFlag dwFlag);
	void SetAddressVAllStages(DWORD dwTile, TextureUVFlag dwFlag);

	void SetTextureUFlag(TextureUVFlag dwFlag, DWORD tile);
	void SetTextureVFlag(TextureUVFlag dwFlag, DWORD tile);

	inline void SetTextureFactor(DWORD dwCol)
	{
		if (m_dwTextureFactor != dwCol)
		{
			m_dwTextureFactor = dwCol;
			g_pD3DDev->SetRenderState(D3DRS_TEXTUREFACTOR, dwCol);
		}
	}


	void SetShadeMode(RenderShadeMode mode);
	void ZBufferEnable(BOOL bZBuffer);
	void ClearBuffer(bool cbuffer, bool zbuffer);
	void SetZCompare(BOOL bZCompare);
	void SetZUpdate(BOOL bZUpdate);
	void SetZBias(int bias);
	void SetAlphaRef(DWORD dwAlpha);
	void ForceAlphaRef(DWORD dwAlpha);
	void SetFillMode(DaedalusRenderFillMode mode);
	void SetAlphaTestEnable(BOOL bAlphaTestEnable);

	bool SetCurrentTexture(int tile, CTexture *handler,DWORD dwTileWidth, DWORD dwTileHeight);
	bool SetCurrentTexture(int tile, TextureEntry *pTextureEntry);

	void DrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, DaedalusColor dif, DaedalusColor spe, float z, float rhw);
	void DrawSimpleRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor, float depth, float rhw);
	void DrawText(const char* str, RECT *rect);

	void SetFogMinMax(float fMin, float fMax, float fMul, float fOffset);
	void SetFogEnable(BOOL bEnable);
	void TurnFogOnOff(BOOL flag);
	void SetFogColor(DWORD r, DWORD g, DWORD b, DWORD a);
	void UpdateScissor();
	void ApplyRDPScissor();
	void ApplyScissorWithClipRatio();

#ifdef _DEBUG
	bool DrawTexture(int tex, bool alphaOnly=false);
	void DrawTlutAsImage(WORD*);
	void SaveTextureToFile(int tex, bool alphaOnly=false);
	void SaveSurfaceToFile(LPDIRECT3DSURFACE8 surf);
#endif


protected:
	TSSInfo m_TSSInfo[8];
	DWORD m_dwTextureFactor;

	DWORD m_dwrsZEnable;
	DWORD m_dwrsZWriteEnable;
	DWORD m_dwrsSrcBlend;
	DWORD m_dwrsDestBlend;
	DWORD m_dwrsAlphaBlendEnable;
	DWORD m_dwrsAlphaTestEnable;
	DWORD m_dwrsAlphaRef;
	DWORD m_dwrsZBias;
	
	SetCombineInfo m_curCombineInfo;

protected:

	DaedalusColor PostProcessDiffuseColor(DaedalusColor curDiffuseColor);
	DaedalusColor PostProcessSpecularColor();

	// Basic render drawing functions
	bool RenderFlushTris();
	bool RenderTexRect();
	bool RenderFillRect(DWORD dwColor, float depth);
	bool RenderLine3D();

	// Combiner and blender functions
	void SetBlendFactor(DWORD flag);

	inline void D3DSetColorOp(DWORD dwStage, DWORD dwOp)
	{
		if (m_TSSInfo[dwStage].dwColorOp != dwOp)
		{
			m_TSSInfo[dwStage].dwColorOp = dwOp;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_COLOROP, dwOp );
		}
	}
	inline void D3DSetColorArg1(DWORD dwStage, DWORD dwArg1)
	{
		if (m_TSSInfo[dwStage].dwColorArg1 != dwArg1 && dwArg1 != ~0)
		{
			m_TSSInfo[dwStage].dwColorArg1 = dwArg1;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_COLORARG1, dwArg1 );
		}
	}
	inline void D3DSetColorArg0(DWORD dwStage, DWORD dwArg0)
	{
		if (m_TSSInfo[dwStage].dwColorArg0 != dwArg0 && dwArg0 != ~0)
		{
			m_TSSInfo[dwStage].dwColorArg0 = dwArg0;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_COLORARG0, dwArg0 );
		}
	}
	inline void D3DSetColorArg2(DWORD dwStage, DWORD dwArg2)
	{
		if (m_TSSInfo[dwStage].dwColorArg2 != dwArg2 && dwArg2 != ~0)
		{
			m_TSSInfo[dwStage].dwColorArg2 = dwArg2;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_COLORARG2, dwArg2 );
		}
	}

	inline void D3DSetAlphaOp(DWORD dwStage, DWORD dwOp)
	{
		if (m_TSSInfo[dwStage].dwAlphaOp != dwOp)
		{
			m_TSSInfo[dwStage].dwAlphaOp = dwOp;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_ALPHAOP, dwOp );
		}
	}

	inline void D3DSetAlphaArg1(DWORD dwStage, DWORD dwArg1)
	{
		if (m_TSSInfo[dwStage].dwAlphaArg1 != dwArg1 && dwArg1 != ~0)
		{
			m_TSSInfo[dwStage].dwAlphaArg1 = dwArg1;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_ALPHAARG1, dwArg1 );
		}
	}

	inline void D3DSetAlphaArg2(DWORD dwStage, DWORD dwArg2)
	{
		if (m_TSSInfo[dwStage].dwAlphaArg2 != dwArg2 && dwArg2 != ~0)
		{
			m_TSSInfo[dwStage].dwAlphaArg2 = dwArg2;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_ALPHAARG2, dwArg2 );
		}
	}

	inline void D3DSetAlphaArg0(DWORD dwStage, DWORD dwArg0)
	{
		if (m_TSSInfo[dwStage].dwAlphaArg0 != dwArg0 && dwArg0 != ~0)
		{
			m_TSSInfo[dwStage].dwAlphaArg0 = dwArg0;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_ALPHAARG0, dwArg0 );
		}
	}

	inline void D3DSetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE8 pTexture)
	{
		if (m_TSSInfo[dwStage].pTexture != pTexture && m_curCombineInfo.stages[dwStage].bTextureUsed )
			{
			m_TSSInfo[dwStage].pTexture = pTexture;
			g_pD3DDev->SetTexture( dwStage, pTexture );
		}
	}

	inline void D3DSetAddressU(DWORD dwStage, DWORD flag)
	{
		if (m_TSSInfo[dwStage].dwAddressUMode != flag)
		{
			m_TSSInfo[dwStage].dwAddressUMode = flag;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_ADDRESSU, flag );
		}
	}

	inline void D3DSetAddressV(DWORD dwStage, DWORD flag)
	{
		if (m_TSSInfo[dwStage].dwAddressVMode != flag)
		{
			m_TSSInfo[dwStage].dwAddressVMode = flag;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_ADDRESSV, flag );
		}
	}

	inline void D3DSetAddressW(DWORD dwStage, DWORD flag)
	{
		if (m_TSSInfo[dwStage].dwAddressW != flag)
		{
			m_TSSInfo[dwStage].dwAddressW = flag;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_ADDRESSW, flag );
		}
	}

	inline void D3DSetMinFilter(DWORD dwStage, DWORD filter)
	{
		if (m_TSSInfo[dwStage].dwMinFilter != filter)
		{
			m_TSSInfo[dwStage].dwMinFilter = filter;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MINFILTER, filter );
		}
	}

	inline void D3DSetMagFilter(DWORD dwStage, DWORD filter)
	{
		if (m_TSSInfo[dwStage].dwMagFilter != filter)
		{
			m_TSSInfo[dwStage].dwMagFilter = filter;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_MAGFILTER, filter );
		}
	}


	inline void SetD3DRSZEnable(DWORD dwrsZEnable)
	{
		if (m_dwrsZEnable != dwrsZEnable)
		{
			m_dwrsZEnable = dwrsZEnable;
			g_pD3DDev->SetRenderState( D3DRS_ZENABLE, dwrsZEnable );	
		}
	}
	inline void SetD3DRSZWriteEnable(DWORD dwrsZWriteEnable)
	{
		if (m_dwrsZWriteEnable != dwrsZWriteEnable)
		{
			m_dwrsZWriteEnable = dwrsZWriteEnable;
			g_pD3DDev->SetRenderState( D3DRS_ZWRITEENABLE, dwrsZWriteEnable );	
		}
	}
	
	inline void SetD3DRSSrcBlend(DWORD dwrsSrcBlend)
	{
		if (m_dwrsSrcBlend != dwrsSrcBlend)
		{
			m_dwrsSrcBlend = dwrsSrcBlend;
			g_pD3DDev->SetRenderState( D3DRS_SRCBLEND, m_dwrsSrcBlend );	
		}
	}

	inline void SetD3DRSDestBlend(DWORD dwrsDestBlend)
	{
		if (m_dwrsDestBlend != dwrsDestBlend)
		{
			m_dwrsDestBlend = dwrsDestBlend;
			g_pD3DDev->SetRenderState( D3DRS_DESTBLEND, dwrsDestBlend );	
		}
	}

	inline void SetD3DRSAlphaBlendEnable(DWORD dwrsAlphaBlendEnable)
	{
		if (m_dwrsAlphaBlendEnable != dwrsAlphaBlendEnable)
		{
			m_dwrsAlphaBlendEnable = dwrsAlphaBlendEnable;
			g_pD3DDev->SetRenderState( D3DRS_ALPHABLENDENABLE, dwrsAlphaBlendEnable );	
		}
	}

	inline void SetD3DRSAlphaTestEnable(DWORD dwrsAlphaBlendEnable)
	{
		if (m_dwrsAlphaTestEnable != dwrsAlphaBlendEnable)
		{
			m_dwrsAlphaTestEnable = dwrsAlphaBlendEnable;
			g_pD3DDev->SetRenderState( D3DRS_ALPHATESTENABLE, dwrsAlphaBlendEnable );	
		}
	}

	inline void SetD3DRSAlphaRef(DWORD dwrsAlphaRef)
	{
		if (m_dwrsAlphaRef != dwrsAlphaRef)
		{
			m_dwrsAlphaRef = dwrsAlphaRef;
			g_pD3DDev->SetRenderState( D3DRS_ALPHAREF, dwrsAlphaRef );	
		}
	}

	inline void SetD3DRSZBias(DWORD dwrsZBias)
	{
		if (m_dwrsZBias != dwrsZBias)
		{
			m_dwrsZBias = dwrsZBias;
			g_pD3DDev->SetRenderState( D3DRS_ZBIAS, dwrsZBias );	
		}
	}

	inline void D3DSetTexCoordIndex(DWORD dwStage, DWORD index)
	{
		if (m_TSSInfo[dwStage].dwTexCoordIndex != index)
		{
			m_TSSInfo[dwStage].dwTexCoordIndex = index;
			g_pD3DDev->SetTextureStageState( dwStage, D3DTSS_TEXCOORDINDEX, index );
		}
	}

	inline void SetAllTextureStageToThisState(D3DTEXTURESTAGESTATETYPE state, DWORD val)
	{
		for( int i=0; i<m_curCombineInfo.nStages; i++ )
		{
			g_pD3DDev->SetTextureStageState(i, state,val);
		}
	}


	// Extended render functions
public:
	// Frame buffer functions
	void DrawFakeFrameBufferIntoD3DRenderTarget(void);

	// Sprite2D functions
	LPD3DXSPRITE InitSpriteDraw(void);
	void FinishSpriteDraw(LPD3DXSPRITE d3dSprite);
	void DrawSpriteR_Render();

	// S2DEX GBI2 function
	void DrawObjBGCopy(uObjBg &info);
};

#endif // __DAEDALUS_D3DRENDER_H__