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

#ifndef _DAEDALUS_RENDER_BASE_H
#define _DAEDALUS_RENDER_BASE_H

/*
 *	Global variables defined in this file were moved out of DaedalusRender class
 *  to make them be accessed faster
 */

#define DAEDALUS_MATRIX_STACK		60
#define MAX_TEXTURES				8

DWORD GammaCorrection(DWORD color);

enum
{
	RENDER_LOAD_MATRIX = 0,
	RENDER_MUL_MATRIX = 1
};

enum DaedalusRenderFillMode
{
	DAEDALUS_FILLMODE_WINFRAME,
	DAEDALUS_FILLMODE_SOLID,
};

enum { MAX_VERTS = 80 };		// F3DLP.Rej supports up to 80 verts!

//#define USING_INT_MATRIX

#ifdef USING_INT_MATRIX
class N64IntMatrix
{
public:
	int   nums[16];
	u32   decs[16];

public:
	N64IntMatrix& operator *= ( const N64IntMatrix& mtx);
	N64IntMatrix operator * ( const N64IntMatrix& mtx) const;
	N64IntMatrix( const N64IntMatrix& mtx );
	N64IntMatrix(const short new_nums[16], const int new_decs[16]);
	N64IntMatrix(const WORD *new_nums, const WORD *new_decs);
	N64IntMatrix(DWORD addr);
	N64IntMatrix();
	DaedalusMatrix convert() const;
};
#endif

void myVec3Transform(float *vecout, float *vecin, float* m);

// All these arrays are moved out of the class CDaedalusRender
// to be accessed in faster speed
extern D3DXVECTOR4	g_vecProjected[MAX_VERTS];
extern float		g_vtxProjected5[1000][5];
extern float		g_fFogCoord[MAX_VERTS];
extern DWORD		g_dwVecFlags[MAX_VERTS];			// Z_POS Z_NEG etc
extern VECTOR2		g_vecTexture[MAX_VERTS];
extern VECTOR2		g_vecTextureMapped[MAX_VERTS];
extern DWORD		g_dwVecDiffuseCol[MAX_VERTS];
extern DaedalusRenderTexture g_textures[MAX_TEXTURES];
extern TLITVERTEX			g_ucVertexBuffer[1000];
extern unsigned int			g_vtxIndex[1000];
extern unsigned int			g_triVtxIndexes[1000];
extern bool					g_triVtxInitFlags[1000];
extern BYTE					g_oglVtxColors[1000][4];
extern DWORD				g_clipFlag[MAX_VERTS];
extern DWORD				g_clipFlag2[MAX_VERTS];

//#define INIT_VERTEX_METHOD_2

/*
 *	Global variables
 */

/************************************************************************/
/*      Don't move                                                      */
/************************************************************************/

extern DWORD			gRSPnumLights;
extern DaedalusLight	gRSPlights[16];
extern DaedalusMatrix	gRSPworldProjectTransported;
extern DaedalusMatrix	gRSPworldProject;
extern N64Light			gRSPn64lights[16];
extern DaedalusMatrix	gRSPmodelViewTop;
extern DaedalusMatrix	gRSPmodelViewTopTranspose;
extern float	gRSPfFogMin;
extern float	gRSPfFogMax;
extern float	gRSPfFogDivider;

/************************************************************************/
/*      Don't move                                                      */
/************************************************************************/
typedef __declspec(align(16)) struct 
{
	/************************************************************************/
	/*      Don't move                                                      */
	/************************************************************************/
	union {		
		struct {
			float	fAmbientLightR;
			float   fAmbientLightG;
			float	fAmbientLightB;
			float	fAmbientLightA;
		};
		float fAmbientColors[4];
		//__m64 fAmbientLightColor64[2];
	};
	/************************************************************************/
	/*      Don't move above                                                */
	/************************************************************************/
	bool	bTextureEnabled;
	DWORD	curTile;
	float	fTexScaleX;
	float	fTexScaleY;

	RenderShadeMode	shadeMode;
	bool	bCullFront;
	bool	bCullBack;
	bool	bLightingEnable;
	bool	bTextureGen;
	bool	bFogEnabled;
	BOOL	bZBufferEnabled;

	DWORD	ambientLightColor;
	DWORD	ambientLightIndex;
#ifdef USING_INT_MATRIX
	int		iAmbientLightR,iAmbientLightG,iAmbientLightB;
#endif

	float	fFogMul;
	float	fFogOffset;

#ifdef USING_INT_MATRIX
	N64IntMatrix	modelViewIntTop;
	N64IntMatrix	modelViewIntTopTranspose;
	N64IntMatrix	worldProjectInt;
	DaedalusMatrix	worldProjectConverted;
#endif
	DWORD	projectionMtxTop;
	DWORD	modelViewMtxTop;

	DWORD	numVertices;

	int		nVPLeftN, nVPTopN, nVPRightN, nVPBottomN, nVPWidthN, nVPHeightN, maxZ;
	int		clip_ratio_negx,	clip_ratio_negy,	clip_ratio_posx,	clip_ratio_posy;
	int		clip_ratio_left,	clip_ratio_top,	clip_ratio_right,	clip_ratio_bottom;
	int		real_clip_scissor_left,	real_clip_scissor_top,	real_clip_scissor_right,	real_clip_scissor_bottom;
	float	real_clip_ratio_negx,	real_clip_ratio_negy,	real_clip_ratio_posx,	real_clip_ratio_posy;

	DaedalusMatrix	projectionMtxs[DAEDALUS_MATRIX_STACK];
	DaedalusMatrix	modelviewMtxs[DAEDALUS_MATRIX_STACK];
#ifdef USING_INT_MATRIX
	N64IntMatrix	projectionIntMtxs[DAEDALUS_MATRIX_STACK];
	N64IntMatrix	modelviewIntMtxs[DAEDALUS_MATRIX_STACK];
#endif

	DWORD		segments[16];

	int		DKRCMatrixIndex;
	int		DKRVtxCount;
	bool	DKRVtxAddBase;
	DWORD	dwDKRVtxAddr;
	DWORD	dwDKRMatrixAddr;
	DaedalusMatrix	DKRMatrixes[4];
	D3DXVECTOR4		DKRBaseVec;

	int		ucode;
	int		vertexMult;	
	bool	bNearClip;
	bool	bRejectVtx;

	bool	bProcessDiffuseColor;
	bool	bProcessSpecularColor;

	// For DirectX only
	float	vtxXMul;
	float	vtxXAdd;
	float	vtxYMul;
	float	vtxYAdd;

	// Texture coordinates computation constants
	float	tex0scaleX;
	float	tex0scaleY;
	float	tex1scaleX;
	float	tex1scaleY;
	float	tex0OffsetX;
	float	tex0OffsetY;
	float	tex1OffsetX;
	float	tex1OffsetY;
	float	texGenYRatio;
	float	texGenXRatio;

} RSP_Options;

extern RSP_Options gRSP;

typedef __declspec(align(16)) struct {
	DWORD	keyR;
	DWORD	keyG;
	DWORD	keyB;
	DWORD	keyA;
	DWORD	keyRGB;
	DWORD	keyRGBA;
	float	fKeyA;
	
	bool	bFogEnableInBlender;

	DWORD	fogColor;
	DWORD	primitiveColor;
	DWORD	envColor;
	DWORD	primitiveDepth;
	DWORD	primLODMin;
	DWORD	primLODFrac;

	float	fPrimitiveDepth;
	float	fvFogColor[4];
	float	fvPrimitiveColor[4];
	float	fvEnvColor[4];

	DWORD	fillColor;
	DWORD	originalFillColor;

	DWORD	geometryMode;
	DWORD	otherModeL;
	DWORD	otherModeH;
	RDP_OtherMode otherMode;

	Tile	tiles[8];
	ScissorType scissor;

	bool	textureIsChanged;
	bool	texturesAreReloaded;
} RDP_Options;

extern RDP_Options gRDP;

/*
*	Global functions
*/
void InitRenderBase();
void SetFogMinMax(float fMin, float fMax, float fMul, float fOffset);
void InitVertex(TLITVERTEX &v, DWORD dwV, DWORD vtxIndex, bool bTexture, bool openGL = true );
void InitVertexTextureConstants();
void AddTri(DWORD dwV0, DWORD dwV1, DWORD dwV2);
bool TestTri(DWORD dwV0, DWORD dwV1, DWORD dwV2);
extern void (*SetNewVertexInfo)(DWORD dwAddress, DWORD dwV0, DWORD dwNum);
void SetNewVertexInfoSSE(DWORD dwAddress, DWORD dwV0, DWORD dwNum);
void SetNewVertexInfoNoSSE(DWORD dwAddress, DWORD dwV0, DWORD dwNum);
void SetPrimitiveColor(DWORD dwCol, DWORD LODMin, DWORD LODFrac);
void SetPrimitiveDepth(DWORD z, DWORD dwDZ);
void SetVertexXYZ(DWORD vertex, float x, float y, float z);
void ModifyVertexInfo(DWORD where, DWORD vertex, DWORD val);
void SetNewVertexInfoDKR(DWORD dwAddress, DWORD dwV0, DWORD dwNum);
void SetLightCol(DWORD dwLight, DWORD dwCol);
void SetLightDirection(DWORD dwLight, float x, float y, float z);
void ForceMainTextureIndex(int dwTile); 

void LogTextureCoords(float fTex0S, float fTex0T, float fTex1S, float fTex1T);
bool CheckTextureCoords(int tex);
void ResetTextureCoordsLog(float maxs0, float maxt0, float maxs1, float maxt1);

inline float ViewPortTranslatef_x(float x) { return ( (x+1) * windowSetting.vpWidthW/2) + windowSetting.vpLeftW; }
inline float ViewPortTranslatef_y(float y) { return ( (1-y) * windowSetting.vpHeightW/2) + windowSetting.vpTopW; }
inline float ViewPortTranslatei_x(LONG x) { return x*windowSetting.fMultX; }
inline float ViewPortTranslatei_y(LONG y) { return y*windowSetting.fMultY; }
inline float ViewPortTranslatei_x(float x) { return x*windowSetting.fMultX; }
inline float ViewPortTranslatei_y(float y) { return y*windowSetting.fMultY; }

inline float GetPrimitiveDepth() { return gRDP.fPrimitiveDepth; }
inline DWORD GetPrimitiveColor() { return gRDP.primitiveColor; }
inline float* GetPrimitiveColorfv() { return gRDP.fvPrimitiveColor; }
inline DWORD GetLODFrac() { return gRDP.primLODFrac; }
inline void SetEnvColor(DWORD dwCol) 
{ 
	gRDP.envColor = dwCol; 
	gRDP.fvEnvColor[0] = ((dwCol>>16)&0xFF)/255.0f;		//r
	gRDP.fvEnvColor[1] = ((dwCol>>8)&0xFF)/255.0f;			//g
	gRDP.fvEnvColor[2] = ((dwCol)&0xFF)/255.0f;			//b
	gRDP.fvEnvColor[3] = ((dwCol>>24)&0xFF)/255.0f;		//a
}
inline DWORD GetEnvColor() { return gRDP.envColor; }
inline float* GetEnvColorfv() { return gRDP.fvEnvColor; }

inline void SetAmbientLight(DWORD color) 
{ 
	gRSP.ambientLightColor = color; 
#ifdef USING_INT_MATRIX
	gRSP.iAmbientLightR = RGBA_GETRED(color);
	gRSP.iAmbientLightG = RGBA_GETGREEN(color);
	gRSP.iAmbientLightB = RGBA_GETBLUE(color);
	gRSP.fAmbientLightR = (float)gRSP.iAmbientLightR;
	gRSP.fAmbientLightG = (float)gRSP.iAmbientLightG;
	gRSP.fAmbientLightB = (float)gRSP.iAmbientLightB;
#else
	gRSP.fAmbientLightR = (float)RGBA_GETRED(gRSP.ambientLightColor);
	gRSP.fAmbientLightG = (float)RGBA_GETGREEN(gRSP.ambientLightColor);
	gRSP.fAmbientLightB = (float)RGBA_GETBLUE(gRSP.ambientLightColor);
#endif
}

inline void SetLighting(bool bLighting) { gRSP.bLightingEnable = bLighting; }

// Generate texture coords?
inline void SetTextureGen(bool bTextureGen) { gRSP.bTextureGen = bTextureGen; }
inline void SetNumLights(DWORD dwNumLights) { gRSPnumLights = dwNumLights; }
inline DWORD GetNumLights() { return gRSPnumLights; }
inline DaedalusColor GetVertexDiffuseColor(DWORD ver) { return g_dwVecDiffuseCol[ver]; }
inline void SetScreenMult(float fMultX, float fMultY) { windowSetting.fMultX = fMultX; windowSetting.fMultY = fMultY; }
inline DaedalusColor GetLightCol(DWORD dwLight) { return gRSPlights[dwLight].col; }

#endif
