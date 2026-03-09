/*
Copyright (C) 2001 StrmnNrmn

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


#ifndef __DAEDALUS_RDP_GFX_H__
#define __DAEDALUS_RDP_GFX_H__

#include "ultra_sptask.h"
//#include "Debugger.h"

typedef struct {
	bool	used;
	DWORD	crc_size;
	DWORD	crc_800;
	DWORD	ucode;
	DWORD	minor_ver;
	DWORD	variant;
	char	rspstr[200];
	
	DWORD	ucStart;
	DWORD	ucSize;
	DWORD	ucDStart;
	DWORD	ucDSize;
	DWORD	ucCRC;
	DWORD	ucDWORD1;
	DWORD	ucDWORD2;
	DWORD	ucDWORD3;
	DWORD	ucDWORD4;
} UcodeInfo;


typedef struct
{
	u32		ucode;
	u32		crc_size;
	u32		crc_800;
	const CHAR * ucode_name;
	bool	non_nearclip;
	bool	reject;
} MicrocodeData;

struct TileDescriptor
{
	// Set by SetTile
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

	// Set by SetTileSize
	unsigned int SL		:10;	// Upper left S		- 8:3
	unsigned int TL		:10;	// Upper Left T		- 8:3
	unsigned int SH		:10;	// Lower Right S
	unsigned int TH		:10;	// Lower Right T
};

enum LoadType
{
	BY_NEVER_SET,
	BY_LOAD_BLOCK,
	BY_LOAD_TILE,
	BY_LOAD_TLUT,
};

struct LoadCmdInfo
{
	LoadType	loadtype;
	unsigned int SL		:10;	// Upper left S		- 8:3
	unsigned int TL		:10;	// Upper Left T		- 8:3
	unsigned int SH		:10;	// Lower Right S
	unsigned int TH		:10;	// Lower Right T
	unsigned int dxt	:12;
};

struct LoadInfoSaveType
{
	LoadCmdInfo		loadCmdInfo;
	TileDescriptor	tileInfo;
	SetImgInfo		timgInfo;

	LoadInfoSaveType & operator = (const LoadCmdInfo &lcmdinfo)
	{
		memcpy(&loadCmdInfo, &lcmdinfo, sizeof(LoadCmdInfo));
		return *this;
	}

	LoadInfoSaveType & operator = (const TileDescriptor &tinfo)
	{
		memcpy(&tileInfo, &tinfo, sizeof(TileDescriptor));
		return *this;
	}

	LoadInfoSaveType & operator = (const SetImgInfo &imginfo)
	{
		memcpy(&timgInfo, &imginfo, sizeof(SetImgInfo));
		return *this;
	}

	LoadInfoSaveType & operator = (const LoadInfoSaveType &info)
	{
		memcpy(this, &info, sizeof(LoadInfoSaveType));
		return *this;
	}
};

typedef struct {
	DWORD dwFormat;
	DWORD dwSize;
	BOOL  bSetBy;

	DWORD dwLoadAddress;
	DWORD dwTotalWords;
	DWORD dwDXT;
	BOOL  bSwapped;

	DWORD dwWidth;
	DWORD dwLine;

	DWORD SL;
	DWORD SH;
	DWORD TL;
	DWORD TH;

	DWORD ucodeCount;

	DWORD dwTmem;
} TMEMLoadMapInfo;

typedef struct {	// This is in Intel format
  uint32 SourceImagePointer;
  uint32 TlutPointer;

  short SubImageWidth;
  short Stride;

  char  SourceImageBitSize;
  char  SourceImageType;
  short SubImageHeight;

  short SourceImageOffsetT;
  short SourceImageOffsetS;

  char	dummy[4]; 
} SpriteStruct;			//Converted Sprint struct in Intel format

typedef struct{
	s16 px;
	s16 py;
	float scaleX;
	float scaleY;
	BYTE  flipX; 
	BYTE  flipY;
	SpriteStruct *spritePtr;
} Sprite2DInfo;


typedef struct
{
	//#define	GBL_c1(m1a, m1b, m2a, m2b)	\
	//	(m1a) << 30 | (m1b) << 26 | (m2a) << 22 | (m2b) << 18
	//#define	GBL_c2(m1a, m1b, m2a, m2b)	\
	//	(m1a) << 28 | (m1b) << 24 | (m2a) << 20 | (m2b) << 16
	unsigned int	c2_m2b:2;
	unsigned int	c1_m2b:2;
	unsigned int	c2_m2a:2;
	unsigned int	c1_m2a:2;
	unsigned int	c2_m1b:2;
	unsigned int	c1_m1b:2;
	unsigned int	c2_m1a:2;
	unsigned int	c1_m1a:2;
} RDP_BlenderSetting;

typedef struct
{
	union
	{
		struct
		{
			// Low bits
			unsigned int		alpha_compare : 2;			// 0..1
			unsigned int		depth_source : 1;			// 2..2

		//	unsigned int		render_mode : 13;			// 3..15
			unsigned int		aa_en : 1;					// 3
			unsigned int		z_cmp : 1;					// 4
			unsigned int		z_upd : 1;					// 5
			unsigned int		im_rd : 1;					// 6
			unsigned int		clr_on_cvg : 1;				// 7

			unsigned int		cvg_dst : 2;				// 8..9
			unsigned int		zmode : 2;					// 10..11

			unsigned int		cvg_x_alpha : 1;			// 12
			unsigned int		alpha_cvg_sel : 1;			// 13
			unsigned int		force_bl : 1;				// 14
			unsigned int		tex_edge : 1;				// 15 - Not used

			unsigned int		blender : 16;				// 16..31

			// High bits
			unsigned int		blend_mask : 4;				// 0..3 - not supported
			unsigned int		alpha_dither : 2;			// 4..5
			unsigned int		rgb_dither : 2;				// 6..7
			
			unsigned int		key_en : 1;				// 8..8
			unsigned int		text_conv : 3;				// 9..11
			unsigned int		text_filt : 2;				// 12..13
			unsigned int		text_tlut : 2;				// 14..15

			unsigned int		text_lod : 1;				// 16..16
			unsigned int		text_sharpen : 1;			// 17..18
			unsigned int		text_detail : 1;			// 17..18
			unsigned int		text_persp : 1;				// 19..19
			unsigned int		cycle_type : 2;				// 20..21
			unsigned int		reserved : 1;				// 22..22 - not supported
			unsigned int		atomic_prim : 1;				// 23..23

			unsigned int		pad : 8;					// 24..31 - padding

		};
		u64			_u64;
		u32			_u32[2];
	};
} RDP_OtherMode;


typedef enum 
{ 
	CMD_SETTILE, 
	CMD_SETTILE_SIZE, 
	CMD_LOADBLOCK, 
	CMD_LOADTILE, 
	CMD_LOADTLUT, 
	CMD_SET_TEXTURE,
	CMD_LOAD_OBJ_TXTR,
} SetTileCmdType;


// The display list PC stack. Before this was an array of 10
// items, but this way we can nest as deeply as necessary. 

typedef struct 
{
	DWORD addr;
	DWORD limit;
	// Push/pop?
} DList;

typedef struct
{
	int x0, y0, x1, y1, mode;
	int left, top, right, bottom;
} ScissorType;

#define ISDRAWINGTRI	(status.primitiveType<PRIM_LINE3D)
#define ISDRAWTEXTRECT	(status.primitiveType==PRIM_TEXTRECT||status.primitiveType==PRIM_TEXTRECTFLIP)
#define ISDRAWFILLRECT	(status.primitiveType==PRIM_FILLRECT)

// Mask down to 0x003FFFFF?
#define RDPSegAddr(seg) ( gRSP.segments[((seg)>>24)&0x0F] + ((seg)&0x00FFFFFF) )
#define RDRAM_UWORD(addr)	(*(DWORD *)((addr)+g_pu8RamBase))
#define RDRAM_SWORD(addr)	(*(s32 *)((addr)+g_pu8RamBase))
#define RDRAM_UHALF(addr)	(*(u16 *)(((addr)^2)+g_pu8RamBase))
#define RDRAM_SHALF(addr)	(*(s16 *)(((addr)^2)+g_pu8RamBase))
#define RDRAM_UBYTE(addr)	(*(BYTE *)(((addr)^3)+g_pu8RamBase))
#define RDRAM_SBYTE(addr)	(*(s8 *)(((addr)^3)+g_pu8RamBase))
#define pRDRAM_UWORD(addr)	((DWORD *)((addr)+g_pu8RamBase))
#define pRDRAM_SWORD(addr)	((s32 *)((addr)+g_pu8RamBase))
#define pRDRAM_UHALF(addr)	((u16 *)(((addr)^2)+g_pu8RamBase))
#define pRDRAM_SHALF(addr)	((s16 *)(((addr)^2)+g_pu8RamBase))
#define pRDRAM_UBYTE(addr)	((BYTE *)(((addr)^3)+g_pu8RamBase))
#define pRDRAM_SBYTE(addr)	((s8 *)(((addr)^3)+g_pu8RamBase))

extern WORD g_wRDPPal[];
extern const char *textluttype[4];

extern char *pszImgFormat[8];
extern char *pszImgSize[4];
extern BYTE pnImgSize[4];
extern char *textlutname[4];

HRESULT DLParser_Init();
HRESULT RDP_GFX_Reset();
void RDP_Cleanup();
void DLParser_Process(OSTask * pTask);
void RDP_DLParser_Process(void);


// Various debugger commands:
void RDP_DumpNextDisplayList();
void RDP_DumpNextScreen();
void RDP_DropTextures();
void RDP_MakeTexturesBlue();
void RDP_MakeTexturesNormal();
void RDP_DumpTextures();
void RDP_NoDumpTextures();

void SetupTextures();
void RDP_InitRenderState();
void RDP_DumpVtxInfo(DWORD dwAddress, DWORD dwV0, DWORD dwN);
void RDP_DumpScreenshot();
void RDP_MoveMemLight(DWORD dwLight, DWORD dwAddress);
void RDP_MoveMemViewport(DWORD dwAddress);
void RDP_NOIMPL_WARN(LPCTSTR op);
void RDP_GFX_Force_Matrix(DWORD dwAddress);
void RDP_GFX_InitGeometryMode();
void RDP_SetUcode(int ucode, u32 ucStart=0, u32 ucDStart=0, u32 cdSize=0);
DWORD CalcalateCRC(DWORD* srcPtr, DWORD srcSize);
void RDP_GFX_PopDL();

void TriggerDPInterrupt();
uint32 DLParser_CheckUcode(u32 ucStart, u32 ucDStart, u32 ucSize, u32 ucDSize);

void SaveFakeFrameBuffer(void);
void CopyBackToFrameBuffer(DWORD addr, DWORD fmt, DWORD siz, DWORD width, DWORD height, DWORD bufWidth, DWORD bufHeight, DWORD startaddr=0xFFFFFFFF, DWORD memsize=0xFFFFFFFF, DWORD pitch=0, D3DFORMAT surf_fmt=D3DFMT_A8R8G8B8, IDirect3DSurface8 *surf=NULL);



// Lkb - I changed this to DAEDALUS_RELEASE_BUILD (from _DEBUG) because
// I tend to dump display lists a lot when developing. This #define 
// can be used for other features that don't need to be included in
// a public release 
#ifndef DAEDALUS_RELEASE_BUILD
void __cdecl DL_PF(LPCTSTR szFormat, ...);
#else
// VC7
#if _MSC_VER >= 1300
#define DL_PF __noop
#else
inline void DL_PF_Noop() {}
// avoid warning C4390: ';' : empty controlled statement found; is this the intent?
#define DL_PF(x) DL_PF_Noop()
#pragma warning(disable : 4002) 
#endif
#endif

#endif	// __DAEDALUS_RDP_GFX_H__