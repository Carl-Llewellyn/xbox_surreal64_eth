/*
Copyright (C) 2002 Rice

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

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#define uint8  unsigned __int8
#define uint16 unsigned __int16
#define uint32 unsigned __int32
#define uint64 unsigned __int64

#define sint8  __int8
#define sint16 __int16
#define sint32 __int32


//Fix me, these macro should not be used anymore in DirectX 8
#define RGBA_GETALPHA(rgb)      ((rgb) >> 24)
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)

typedef D3DXMATRIX DaedalusMatrix;
typedef D3DCOLOR DaedalusColor;
typedef D3DVECTOR Vector3;
typedef void* LPDAEDALUSTEXTURE ;


#define DAEDALUS_COLOR_RGBA D3DCOLOR_RGBA

inline DWORD argb2rgba(DWORD c) { return (c<<8)|(c>>24); }
#define ARGB2RGBA	argb2rgba		// For OpenGL
//#define ARGB2RGBA					// For DirectX, DirectX is using ARGB

struct N64Light
{
	DWORD dwRGBA, dwRGBACopy;
	char x,y,z;			// Direction
	BYTE pad;
};


typedef struct
{
	unsigned int	dwFormat:3;
	unsigned int	dwSize:2;
	unsigned int	dwWidth:10;
	uint32			dwAddr;
} SetImgInfo;

struct Tile 
{
	// Set by RDP_SetTile
	unsigned int dwFormat	:3;		// e.g. RGBA, YUV etc
	unsigned int dwSize		:2;		// e.g 4/8/16/32bpp
	unsigned int dwLine		:9;		// Ummm...
	unsigned int dwPalette	:4;		// 0..15 - a palette index?
	DWORD dwTMem;					// Texture memory location

	unsigned int bClampS	:1;
	unsigned int bClampT	:1;
	unsigned int bMirrorS	:1;
	unsigned int bMirrorT	:1;

	unsigned int dwMaskS	:4;
	unsigned int dwMaskT	:4;
	unsigned int dwShiftS	:4;
	unsigned int dwShiftT	:4;

	// Set by RDP_SetTileSize
	DWORD sl;		// Upper left S		- 8:3
	DWORD tl;		// Upper Left T		- 8:3
	DWORD sh;		// Lower Right S
	DWORD th;		// Lower Right T

	int   hilite_sl;
	int	  hilite_tl;
	int   hilite_sh;
	int	  hilite_th;

	DWORD dwDXT;

	DWORD dwPitch;

	DWORD dwWidth;
	DWORD dwHeight;

	float fShiftScaleS;
	float fShiftScaleT;

	u32   lastTileCmd;
};


typedef struct
{
	float u;
	float v;
} TexCord;

struct VECTOR2_struct
{
	float x;
	float y;
	VECTOR2_struct( float newx, float newy )	{x=newx; y=newy;}
	VECTOR2_struct()	{x=0; y=0;}
};

typedef VECTOR2_struct VECTOR2;

typedef struct
{
	short x;
	short y;
} IVector2;

typedef struct 
{
	short x;
	short y;
	short z;
} IVector3;

#define DAEDALUSFVF_TLITVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2 ) 
typedef struct _TLITVERTEX {
	float x,y,z;
	float rhw;
	union {
		DaedalusColor  dcDiffuse;
		struct {
			BYTE b;
			BYTE g;
			BYTE r;
			BYTE a;
		};
	};
	DaedalusColor  dcSpecular;
	TexCord tcord[2];
} TLITVERTEX, *LPTLITVERTEX;


#define DAEDALUSFVF_LITVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR ) 
typedef struct _LITVERTEX {
	float x,y,z;
	float rhw;
	union {
		DaedalusColor  dcDiffuse;
		struct {
			BYTE b;
			BYTE g;
			BYTE r;
			BYTE a;
		};
	};
	DaedalusColor  dcSpecular;
} LITVERTEX, *LPLITVERTEX;

typedef struct _FILLRECTVERTEX {
	float	x,y,z;
	float	rhw;
	DaedalusColor dcDiffuse;
} FILLRECTVERTEX, *LPFILLRECTVERTEX;


typedef struct
{
	union {
		struct {
			float x;
			float y;
			float z;
			float dummy;
		};
		//__m64 dir64[2];
	};

	union {
		struct {
			BYTE r;
			BYTE g;
			BYTE b;
			BYTE a;
		};
		DWORD col;
	};

	union {
		struct {
			float fr;
			float fg;
			float fb;
			float fa;
		};
		float fcolors[4];
		//__m64 fcolor64[2];
	};
} DaedalusLight;



typedef struct tagTSSInfo
{
	DWORD dwColorOp;
	DWORD dwColorArg0;
	DWORD dwColorArg1;
	DWORD dwColorArg2;
	DWORD dwAlphaOp;
	DWORD dwAlphaArg0;
	DWORD dwAlphaArg1;
	DWORD dwAlphaArg2;
	DWORD dwResultArg;
	
	DWORD dwMinFilter;
	DWORD dwMagFilter;

	DWORD dwAddressUMode;
	DWORD dwAddressVMode;
	DWORD dwAddressW;

	DWORD dwTexCoordIndex;
	LPDIRECT3DBASETEXTURE8 pTexture; 
} TSSInfo;

typedef struct
{
	char na;
	char nz;	// b
	char ny;	//g
	char nx;	//r
}NormalStruct;

typedef struct _FiddledVtx
{
	short y;
	short x;
	
	short flag;
	short z;
	
	short tv;
	short tu;
	
	union {
		struct _rgba {
			BYTE a;
			BYTE b;
			BYTE g;
			BYTE r;
		} rgba;
		NormalStruct norma;
	};
} FiddledVtx;

typedef struct _FiddledVtxDKR
{
	s16 y;
	s16 x;
	
	u8 a;
	u8 b;
	s16 z;
	
	u8 g;
	u8 r;
	
} FiddledVtxDKR;

typedef struct 
{
	s16 y;
	s16	x;
	u16	cidx;
	s16 z;
	s16 t;
	s16 s;
} N64VtxPD;

class CTexture;
class COGLTexture;
class CDirectXTexture;
typedef struct {
	LPDAEDALUSTEXTURE m_lpsTexturePtr;
	union {
		CTexture *			m_pCTexture;
		CDirectXTexture *	m_pCDirectXTexture;
		COGLTexture *		m_pCOGLTexture;
	};
	
	DWORD m_dwTileWidth;
	DWORD m_dwTileHeight;
	float m_fTexWidth;
	float m_fTexHeight;		// Float to avoid converts when processing verts
} DaedalusRenderTexture;


typedef struct {
	LPDIRECT3DTEXTURE8 *psurf;
	SetImgInfo	CI_Info;
	u32	width;
	u32 height;
} FakeFrameBufferInfo;



typedef struct
{
	unsigned __int32	dwFormat;
	unsigned __int32	dwSize;
	unsigned __int32	dwWidth;
	unsigned __int32	dwAddr;

	unsigned __int32	dwHeight;
	unsigned __int32	dwMemSize;

	bool				bCopied;

	unsigned __int32	dwCRC;
	unsigned __int32	lastUsedFrame;
	unsigned __int32	bUsedByVIAtFrame;
} RecentCIInfo;

typedef struct
{
	u32		addr;
	u32		FrameCount;
} RecentViOriginInfo;

typedef enum {
	SHADE_DISABLED,
	SHADE_FLAT,
	SHADE_SMOOTH,
} RenderShadeMode;

typedef enum {
	TEXTURE_UV_FLAG_WRAP,
	TEXTURE_UV_FLAG_MIRROR,
	TEXTURE_UV_FLAG_CLAMP,
} TextureUVFlag;

typedef struct
{
	TextureUVFlag	N64flag;
	DWORD			realFlag;
} UVFlagMap;

//freakdave
typedef enum {
	FILTER_NONE,
	FILTER_POINT,
	FILTER_LINEAR,
	FILTER_ANISOTROPIC,
	FILTER_FLATCUBIC,
	FILTER_GAUSSIANCUBIC,
} DaedalusTextureFilter;

typedef struct 
{
	DaedalusTextureFilter	N64filter;
	DWORD					realFilter;
} TextureFilterMap;

typedef  struct
{
	union {
		DWORD dwords[8];
		struct {
			int tl:14;
			unsigned int :2;

			unsigned int tile:3;
			unsigned int level:3;
			unsigned int :1;
			unsigned int lft:1;

			unsigned int cmd:6;
			unsigned int :2;

			int th:14;
			unsigned int :2;

			int ym:14;
			unsigned int :2;

			unsigned int dxldyFrac:16;
			int dxldy:16;

			unsigned int xlFrac:16;
			int sl:16;

			unsigned int dxhdyFrac:16;
			int dxhdy:16;

			unsigned int xhFrac:16;
			int sh:16;

			unsigned int dxmdyFrac:16;
			int dxmdy:16;

			unsigned int xmFrac:16;
			int xm:16;
		};
	};
}RDPEdgeCoeff;


typedef  struct
{
	union {
		DWORD dwords[16];
		struct {
			int a:16;
			int b:16;
			int g:16;
			int r:16;

			int	dadx:16;
			int	dbdx:16;
			int dgdx:16;
			int drdx:16;

			unsigned int aFrac:16;
			unsigned int bFrac:16;
			unsigned int gFrac:16;
			unsigned int rFrac:16;

			unsigned int dadxFrac:16;
			unsigned int dbdxFrac:16;
			unsigned int dgdxFrac:16;
			unsigned int drdxFrac:16;

			int	dade:16;
			int	dbde:16;
			int dgde:16;
			int drde:16;

			int	dady:16;
			int	dbdy:16;
			int dgdy:16;
			int drdy:16;

			unsigned int dadeFrac:16;
			unsigned int dbdeFrac:16;
			unsigned int dgdeFrac:16;
			unsigned int drdeFrac:16;

			unsigned int dadyFrac:16;
			unsigned int dbdyFrac:16;
			unsigned int dgdyFrac:16;
			unsigned int drdyFrac:16;
		};
	};
}RDPShadeCoeff;


typedef  struct
{
	union {
		DWORD dwords[16];
		struct {
			int	:16;
			int	w:16;
			int t:16;
			int s:16;

			int :16;
			int dwdx:16;
			int	dtdx:16;
			int dsdx:16;

			int :16;
			unsigned int wFrac:16;
			unsigned int tFrac:16;
			unsigned int sFrac:16;

			int :16;
			int dwde:16;
			int	dtde:16;
			int dsde:16;

			int :16;
			int dwdy:16;
			int	dtdy:16;
			int dsdy:16;

			int :16;
			unsigned int dwdeFrac:16;
			unsigned int dtdeFrac:16;
			unsigned int dsdeFrac:16;

			int :16;
			unsigned int dwdyFrac:16;
			unsigned int dtdyFrac:16;
			unsigned int dsdyFrac:16;
		};
	};
}RDPTextureCoeff;


typedef  struct
{
	union {
		DWORD dwords[4];
		struct {
			unsigned int dzdxFrac:16;
			int	dzdx:16;
			unsigned int zFrac:16;
			int z:16;

			unsigned int dzdyFrac:16;
			int	dzdy:16;
			unsigned int dzdeFrac:16;
			int dzde:16;
		};
	};
}RDPZCoeff;

typedef struct {
	char	*description;
	int		number;
	DWORD	setting;
} BufferSettingInfo;

typedef struct {
	char	*description;
	DWORD	setting;
} SettingInfo;

#endif
