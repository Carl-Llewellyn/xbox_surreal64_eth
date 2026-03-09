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

#ifndef _RICE_RENDER_BASE_H
#define _RICE_RENDER_BASE_H

/*
 *	Global variables defined in this file were moved out of Render class
 *  to make them be accessed faster
 */

#define RICE_MATRIX_STACK		60
#define MAX_TEXTURES				8

enum
{
	RENDER_LOAD_MATRIX = 0,
	RENDER_MUL_MATRIX = 1
};

enum FillMode
{
	RICE_FILLMODE_WINFRAME,
	RICE_FILLMODE_SOLID,
};

enum { MAX_VERTS = 80 };		// F3DLP.Rej supports up to 80 verts!

void myVec3Transform(float *vecout, float *vecin, float* m);

// All these arrays are moved out of the class CRender
// to be accessed in faster speed
extern D3DXVECTOR4	g_vecProjected[MAX_VERTS];
extern float		g_vtxProjected5[1000][5];
extern VECTOR2		g_fVtxTxtCoords[MAX_VERTS];
extern DWORD		g_dwVtxDifColor[MAX_VERTS];
extern DWORD		g_dwVtxFlags[MAX_VERTS];			// Z_POS Z_NEG etc

extern RenderTexture g_textures[MAX_TEXTURES];

extern TLITVERTEX			g_vtxBuffer[1000];
extern unsigned int			g_vtxIndex[1000];

extern TLITVERTEX			g_clippedVtxBuffer[2000];
extern int					g_clippedVtxCount;

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

extern DWORD	gRSPnumLights;
extern Light	gRSPlights[16];
extern Matrix	gRSPworldProjectTransported;
extern Matrix	gRSPworldProject;
extern N64Light	gRSPn64lights[16];
extern Matrix	gRSPmodelViewTop;
extern Matrix	gRSPmodelViewTopTranspose;
extern float	gRSPfFogMin;
extern float	gRSPfFogMax;
extern float	gRSPfFogDivider;


/************************************************************************/
/*      Don't move                                                      */
/************************************************************************/
#if _MSC_VER > 1200
typedef __declspec(align(16)) struct 
#else
typedef struct 
#endif
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
#if _MSC_VER > 1200
		__m64 fAmbientLightColor64[2];
#endif
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

	//weinersch - 5.10 fog
	float	fFogMul;
	float	fFogOffset;

	DWORD	projectionMtxTop;
	DWORD	modelViewMtxTop;

	DWORD	numVertices;

	int		nVPLeftN, nVPTopN, nVPRightN, nVPBottomN, nVPWidthN, nVPHeightN, maxZ;
	int		clip_ratio_negx,	clip_ratio_negy,	clip_ratio_posx,	clip_ratio_posy;
	int		clip_ratio_left,	clip_ratio_top,	clip_ratio_right,	clip_ratio_bottom;
	int		real_clip_scissor_left,	real_clip_scissor_top,	real_clip_scissor_right,	real_clip_scissor_bottom;
	float	real_clip_ratio_negx,	real_clip_ratio_negy,	real_clip_ratio_posx,	real_clip_ratio_posy;

	Matrix	projectionMtxs[RICE_MATRIX_STACK];
	Matrix	modelviewMtxs[RICE_MATRIX_STACK];

	bool	bMatrixIsUpdated;
	bool	bCombinedMatrixIsUpdated;

	DWORD		segments[16];

	int		DKRCMatrixIndex;
	int		DKRVtxCount;
	bool	DKRVtxAddBase;
	DWORD	dwDKRVtxAddr;
	DWORD	dwDKRMatrixAddr;
	Matrix	DKRMatrixes[4];
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

#if _MSC_VER > 1200
typedef __declspec(align(16)) struct {
#else
typedef struct {
#endif
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
	DWORD	LODFrac;

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
void InitVertex(DWORD dwV, DWORD vtxIndex, bool bTexture, bool openGL = true );
void InitVertexTextureConstants();
bool AddTri(DWORD dwV0, DWORD dwV1, DWORD dwV2);
bool TestTri(DWORD dwV0, DWORD dwV1, DWORD dwV2);
extern void (*ProcessVertexData)(DWORD dwAddr, DWORD dwV0, DWORD dwNum);
void ProcessVertexDataSSE(DWORD dwAddr, DWORD dwV0, DWORD dwNum);
void ProcessVertexDataNoSSE(DWORD dwAddr, DWORD dwV0, DWORD dwNum);
void SetPrimitiveColor(DWORD dwCol, DWORD LODMin, DWORD LODFrac);
void SetPrimitiveDepth(DWORD z, DWORD dwDZ);
void SetVertexXYZ(DWORD vertex, float x, float y, float z);
void ModifyVertexInfo(DWORD where, DWORD vertex, DWORD val);
void ProcessVertexDataDKR(DWORD dwAddr, DWORD dwV0, DWORD dwNum);
void SetLightCol(DWORD dwLight, DWORD dwCol);
void SetLightDirection(DWORD dwLight, float x, float y, float z);
void ForceMainTextureIndex(int dwTile); 
void UpdateCombinedMatrix();

void ClipVertexes();

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
inline DWORD GetLODFrac() { return gRDP.LODFrac; }
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
	gRSP.fAmbientLightR = (float)RGBA_GETRED(gRSP.ambientLightColor);
	gRSP.fAmbientLightG = (float)RGBA_GETGREEN(gRSP.ambientLightColor);
	gRSP.fAmbientLightB = (float)RGBA_GETBLUE(gRSP.ambientLightColor);
}

inline void SetLighting(bool bLighting) { gRSP.bLightingEnable = bLighting; }

// Generate texture coords?
inline void SetTextureGen(bool bTextureGen) { gRSP.bTextureGen = bTextureGen; }
inline void SetNumLights(DWORD dwNumLights) { gRSPnumLights = dwNumLights; }
inline DWORD GetNumLights() { return gRSPnumLights; }
inline D3DCOLOR GetVertexDiffuseColor(DWORD ver) { return g_dwVtxDifColor[ver]; }
inline void SetScreenMult(float fMultX, float fMultY) { windowSetting.fMultX = fMultX; windowSetting.fMultY = fMultY; }
inline D3DCOLOR GetLightCol(DWORD dwLight) { return gRSPlights[dwLight].col; }

#endif
