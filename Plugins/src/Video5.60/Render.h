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

#ifndef _RICE_RENDER_H
#define _RICE_RENDER_H

#include "stdafx.h"
#include "ExtendedRender.h"

class CRender : public CExtendedRender
{
protected:
	CRender();

	TextureUVFlag TileUFlags[8];
	TextureUVFlag TileVFlags[8];

public:

	float m_fScreenViewportMultX;
	float m_fScreenViewportMultY;


	DWORD	m_dwTexturePerspective;
	BOOL	m_bAlphaTestEnable;

	BOOL	m_bZUpdate;
	BOOL	m_bZCompare;
	DWORD	m_dwZBias;

	TextureFilter	m_dwMinFilter;
	TextureFilter	m_dwMagFilter;

	DWORD	m_dwAlpha;

	uint64		m_Mux;
	BOOL	m_bBlendModeValid;

	CColorCombiner *m_pColorCombiner;
	CBlender *m_pAlphaBlender;
	
	
	virtual ~CRender();
	
	inline bool IsTexel0Enable() {return m_pColorCombiner->m_bTex0Enabled;}
	inline bool IsTexel1Enable() {return m_pColorCombiner->m_bTex1Enabled;}
	inline bool CRender::IsTextureEnabled() { return (m_pColorCombiner->m_bTex0Enabled||m_pColorCombiner->m_bTex1Enabled); }

	inline RenderTexture& GetCurrentTexture() { return g_textures[gRSP.curTile]; }
	inline RenderTexture& GetTexture(DWORD dwTile) { return g_textures[dwTile]; }
	void SetViewport(int nLeft, int nTop, int nRight, int nBottom, int maxZ);
	virtual void SetViewportRender() {}
	virtual void SetClipRatio(DWORD type, DWORD value);
	virtual void UpdateScissor() {}
	virtual void ApplyRDPScissor() {}
	virtual void UpdateClipRectangle();
	virtual void UpdateScissorWithClipRatio();
	virtual void ApplyScissorWithClipRatio() {}
	virtual void CaptureScreen(char *folder) {}

	void SetTextureEnableAndScale(int dwTile, bool enable, float fScaleX, float fScaleY);
	
	virtual void SetFogEnable(BOOL bEnable) { gRSP.bFogEnabled = bEnable&&options.bEnableFog; }
	virtual void SetFogMinMax(float fMin, float fMax, float fMul, float fOffset) = 0;
	virtual void TurnFogOnOff(BOOL flag)=0;
	BOOL m_bFogStateSave;
	void SetFogFlagForNegativeW();
	void RestoreFogFlag();

	virtual void SetFogColor(DWORD r, DWORD g, DWORD b, DWORD a) 
	{ 
		gRDP.fogColor = COLOR_RGBA(r, g, b, a); 
	}
	DWORD GetFogColor() { return gRDP.fogColor; }

	void SetProjection(const Matrix & mat, BOOL bPush, LONG nLoadReplace);
	void SetWorldView(const Matrix & mat, BOOL bPush, LONG nLoadReplace);
	inline int GetProjectMatrixLevel(void) { return gRSP.projectionMtxTop; }
	inline int GetWorldViewMatrixLevel(void) { return gRSP.modelViewMtxTop; }

	inline void PopProjection()
	{
		if (gRSP.projectionMtxTop > 0)
			gRSP.projectionMtxTop--;
		else
			TRACE0("Popping past projection stack limits");
	}

	void PopWorldView();
	Matrix & GetWorldProjectMatrix(void);
	void SetWorldProjectMatrix(Matrix &mtx);
	
	void ResetMatrices();

	inline RenderShadeMode GetShadeMode() { return gRSP.shadeMode; }

	void SetVtxTextureCoord(DWORD dwV, float tu, float tv)
	{
		g_fVtxTxtCoords[dwV].x = tu;
		g_fVtxTxtCoords[dwV].y = tv;
	}

	virtual void RenderReset();
	virtual void SetCombinerAndBlender();
	virtual void SetMux(DWORD dwMux0, DWORD dwMux1);
	virtual void SetCullMode(bool bCullFront, bool bCullBack) { gRSP.bCullFront = bCullFront; gRSP.bCullBack = bCullBack; }

	virtual void BeginRendering(void) {}		// For DirectX only
	virtual void EndRendering(void) {}			// For DirectX only

	virtual void ClearBuffer(bool cbuffer, bool zbuffer)=0;
	virtual void ClearZBuffer(float depth)=0;
	virtual void ClearBuffer(bool cbuffer, bool zbuffer, D3DRECT &rect) 
	{
		ClearBuffer(cbuffer, zbuffer);
	}
	virtual void ZBufferEnable(BOOL bZBuffer)=0;
	virtual void SetZCompare(BOOL bZCompare)=0;
	virtual void SetZUpdate(BOOL bZUpdate)=0;
	virtual void SetZBias(int bias)=0;
	virtual void SetAlphaTestEnable(BOOL bAlphaTestEnable)=0;

	void SetTextureFilter(DWORD dwFilter);
	virtual void ApplyTextureFilter() {}
	
	virtual void SetShadeMode(RenderShadeMode mode)=0;

	virtual void SetAlphaRef(DWORD dwAlpha)=0;
	virtual void ForceAlphaRef(DWORD dwAlpha)=0;

	virtual void InitOtherModes(void);

	void SetVertexTextureUVCoord(TLITVERTEX &v, float fTex0S, float fTex0T, float fTex1S, float fTex1T);
	void SetVertexTextureUVCoord(TLITVERTEX &v, float fTex0S, float fTex0T);
	virtual D3DCOLOR PostProcessDiffuseColor(D3DCOLOR curDiffuseColor)=0;
	virtual D3DCOLOR PostProcessSpecularColor()=0;
	
	bool FlushTris();
	virtual bool RenderFlushTris()=0;

	bool TexRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, float fS0, float fT0, float fScaleS, float fScaleT, bool colorFlag=false, DWORD difcolor=0xFFFFFFFF);
	bool TexRectFlip(LONG nX0, LONG nY0, LONG nX1, LONG nY1, float fS0, float fT0, float fS1, float fT1);
	bool FillRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor);
	bool Line3D(DWORD dwV0, DWORD dwV1, DWORD dwWidth);

	virtual void SetAddressUAllStages(DWORD dwTile, TextureUVFlag dwFlag);	// For DirectX only, fix me
	virtual void SetAddressVAllStages(DWORD dwTile, TextureUVFlag dwFlag);	// For DirectX only, fix me
	virtual void SetTextureUFlag(TextureUVFlag dwFlag, DWORD tile)=0;
	virtual void SetTextureVFlag(TextureUVFlag dwFlag, DWORD tile)=0;
	virtual void SetTexelRepeatFlags(DWORD dwTile);
	virtual void SetAllTexelRepeatFlag();
	
	virtual bool SetCurrentTexture(int tile, TextureEntry *pTextureEntry)=0;
	virtual bool SetCurrentTexture(int tile, CTexture *handler, DWORD dwTileWidth, DWORD dwTileHeight, TextureEntry *pTextureEntry) = 0;

	virtual bool InitDeviceObjects()=0;
	virtual bool ClearDeviceObjects()=0;
	virtual void Initialize(void);
	virtual void CleanUp(void);
	
	virtual void SetFillMode(FillMode mode)=0;

#ifdef _DEBUG
	virtual bool DrawTexture(int tex, bool alphaOnly=false)=0;
	virtual void DrawTlutAsImage(WORD*)=0;
	virtual void SaveTextureToFile(int tex, bool alphaOnly=false);
#endif

	void LoadSprite2D(Sprite2DInfo &info, DWORD ucode);
	void LoadObjBGCopy(uObjBg &info);
	void LoadObjBG1CYC(uObjScaleBg &info);
	void LoadObjSprite(uObjTxSprite &info, bool useTIAddr=false);

	void LoadFrameBuffer(bool useVIreg=false, DWORD left=0, DWORD top=0, DWORD width=0, DWORD height=0);
	void LoadTextureFromMemory(void *buf, DWORD left, DWORD top, DWORD width, DWORD height, DWORD pitch, DWORD format);
	void LoadTxtrBufIntoTexture(void);
	void DrawSprite2D(Sprite2DInfo &info, DWORD ucode);
	void DrawSpriteR(uObjTxSprite &sprite, bool initCombiner=true, DWORD tile=0, DWORD left=0, DWORD top=0, DWORD width=0, DWORD height=0);
	void DrawSprite(uObjTxSprite &sprite, bool rectR = true);
	void DrawObjBGCopy(uObjBg &info);
	virtual void DrawSpriteR_Render(){};
	virtual void DrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, D3DCOLOR dif, D3DCOLOR spe, float z, float rhw)=0;
	void DrawFrameBuffer(bool useVIreg=false, DWORD left=0, DWORD top=0, DWORD width=0, DWORD height=0);
	void DrawObjBG1CYC(uObjScaleBg &bg, bool scaled=true);

	static CRender * g_pRender;
	static CRender * GetRender(void);
	static bool IsAvailable();


protected:
	BOOL			m_savedZBufferFlag;
	DWORD			m_savedMinFilter;
	DWORD			m_savedMagFilter;

	// FillRect
	virtual bool	RenderFillRect(DWORD dwColor, float depth)=0;
	VECTOR2			m_fillRectVtx[2];
	
	// Line3D
	virtual bool	RenderLine3D()=0;

	LITVERTEX		m_line3DVtx[2];
	VECTOR2			m_line3DVector[4];
	
	// TexRect
	virtual bool	RenderTexRect()=0;

	TLITVERTEX		m_texRectTVtx[4];
	TexCord			m_texRectTex1UV[2];
	TexCord			m_texRectTex2UV[2];

	// DrawSimple2DTexture
	virtual void	StartDrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, D3DCOLOR dif, D3DCOLOR spe, float z, float rhw);

	// DrawSimpleRect
	virtual void	StartDrawSimpleRect(LONG nX0, LONG nY0, LONG nX1, LONG nY1, DWORD dwColor, float depth, float rhw);
	VECTOR2			m_simpleRectVtx[2];

	bool			RemapTextureCoordinate(float s0, float s1, DWORD tileWidth, DWORD mask, float textureWidth,
											float &u0, float &u1);

};

#define ffloor(a) (((int(a))<=(a))?(float)(int(a)):((float)(int(a))-1))
#endif	//_RICE_RENDER_H
