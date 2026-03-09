/*
Copyright (C) 2002 Rice1964

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

#ifndef _RICE_CONFIG_H_
#define _RICE_CONFIG_H_

#include "typedefs.h"

typedef enum
{
	OGL_DEVICE,
	OGL_1_1_DEVICE,
	OGL_1_2_DEVICE,
	OGL_1_3_DEVICE,
	OGL_1_4_DEVICE,
	OGL_1_4_V2_DEVICE,
	OGL_TNT2_DEVICE,
	NVIDIA_OGL_DEVICE,

	DIRECTX_DEVICE,
	DIRECTX_9_DEVICE,
	DIRECTX_8_DEVICE,
	DIRECTX_7_DEVICE,
	DIRECTX_6_DEVICE,
	DIRECTX_5_DEVICE,
	XBOX_DIRECTX_DEVICE,
} SupportedDeviceType;

enum DirectXCombinerType
{
	DX_DISABLE_COMBINER,
	DX_BEST_FIT,
	DX_LOW_END,
	DX_HIGH_END,
	DX_NVIDIA_TNT,
	DX_2_STAGES,
	DX_3_STAGES,
	DX_4_STAGES,
	DX_PIXEL_SHADER,
	DX_SEMI_PIXEL_SHADER,
};


typedef struct
{
	char *	name;
	SupportedDeviceType		type;
} RenderEngineSetting;

enum {
	FRM_BUF_NONE,
	FRM_BUF_IGNORE,
	FRM_BUF_BASIC,
	FRM_BUF_BASIC_AND_WRITEBACK,
	FRM_BUF_WRITEBACK_AND_RELOAD,
	FRM_BUF_COMPLETE,
	FRM_BUF_WITH_EMULATOR, //6
};

enum {
	FRM_BUF_WRITEBACK_NORMAL,
	FRM_BUF_WRITEBACK_1_2,
	FRM_BUF_WRITEBACK_1_3,
	FRM_BUF_WRITEBACK_1_4,
	FRM_BUF_WRITEBACK_1_5,
	FRM_BUF_WRITEBACK_1_6,
	FRM_BUF_WRITEBACK_1_7,
	FRM_BUF_WRITEBACK_1_8,
};

enum {
	TXT_BUF_NONE,
	TXT_BUF_IGNORE,
	TXT_BUF_NORMAL,
	TXT_BUF_WRITE_BACK,
	TXT_BUF_WRITE_BACK_AND_RELOAD	,
};

enum {
	TXT_QUALITY_DEFAULT,
	TXT_QUALITY_32BIT,
	TXT_QUALITY_16BIT,
};

//freakdave
enum {
	//
	FORCE_NONE_FILTER,
	FORCE_POINT_FILTER,
	FORCE_LINEAR_FILTER,
	//FORCE_BILINEAR_FILTER,
	FORCE_ANISOTROPIC_FILTER,
	FORCE_FLATCUBIC_FILTER,
	FORCE_GAUSSIANCUBIC_FILTER,
	FORCE_DEFAULT_FILTER,
};

enum {
	TEXTURE_ENHANCEMENT_NORMAL,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4,
};

enum {
	TEXTURE_NO_ENHANCEMENT,
	TEXTURE_2X_ENHANCEMENT,
	TEXTURE_2XSAI_ENHANCEMENT,
	TEXTURE_HQ2X_ENHANCEMENT,
	TEXTURE_LQ2X_ENHANCEMENT,
	TEXTURE_HQ4X_ENHANCEMENT,
	TEXTURE_SHARPEN_ENHANCEMENT,
	TEXTURE_SHARPEN_MORE_ENHANCEMENT,
};

enum {
	SCREEN_UPDATE_DEFAULT = 0,
	SCREEN_UPDATE_AT_VI_UPDATE = 1,
	SCREEN_UPDATE_AT_VI_CHANGE = 2,
	SCREEN_UPDATE_AT_CI_CHANGE = 3,
	SCREEN_UPDATE_AT_1ST_CI_CHANGE = 4,
	SCREEN_UPDATE_AT_1st_PRIMITIVE = 5,
	SCREEN_UPDATE_BEFORE_SCREEN_CLEAR = 6,
};

enum {
	ONSCREEN_DISPLAY_NOTHING = 0,
	ONSCREEN_DISPLAY_DLIST_PER_SECOND,
	ONSCREEN_DISPLAY_FRAME_PER_SECOND,
	ONSCREEN_DISPLAY_DEBUG_INFORMATION_ONLY,
	ONSCREEN_DISPLAY_TEXT_FROM_CORE_ONLY,
	ONSCREEN_DISPLAY_DLIST_PER_SECOND_WITH_CORE_MSG,
	ONSCREEN_DISPLAY_FRAME_PER_SECOND_WITH_CORE_MSG,
	ONSCREEN_DISPLAY_DEBUG_INFORMATION_WITH_CORE_MSG,
};

enum HACK_FOR_GAMES
{
	NO_HACK_FOR_GAME,
	HACK_FOR_BANJO_TOOIE,
	HACK_FOR_DR_MARIO,
	HACK_FOR_ZELDA,
	HACK_FOR_MARIO_TENNIS,
	HACK_FOR_BANJO,
	HACK_FOR_PD,
	HACK_FOR_GE,
	HACK_FOR_PILOT_WINGS,
	HACK_FOR_YOSHI,
	HACK_FOR_NITRO,
	HACK_FOR_TONYHAWK,
	HACK_FOR_NASCAR,
	HACK_FOR_SUPER_BOWLING,
	HACK_FOR_CONKER,
	HACK_FOR_ALL_STAR_BASEBALL,
	HACK_FOR_TIGER_HONEY_HUNT,
	HACK_REVERSE_XY_COOR,
	HACK_REVERSE_Y_COOR,
	HACK_FOR_GOLDEN_EYE,
	HACK_FOR_FZERO,
	HACK_FOR_COMMANDCONQUER,
	HACK_FOR_RUMBLE,
	HACK_FOR_SOUTH_PARK_RALLY,
	HACK_FOR_BUST_A_MOVE,
	HACK_FOR_OGRE_BATTLE,
	HACK_FOR_TWINE,
	HACK_FOR_EXTREME_G2,
};

enum {
	NOT_USE_CI_WIDTH_AND_RATIO,
	USE_CI_WIDTH_AND_RATIO_FOR_NTSC,
	USE_CI_WIDTH_AND_RATIO_FOR_PAL,
};

typedef struct {
	BOOL	bEnableHacks;
	BOOL	bEnableFog;
	BOOL	bWinFrameMode;
	BOOL	bForceSoftwareTnL;
	BOOL	bForceSoftwareClipper;
	BOOL	bEnableSSE;
	BOOL	bSkipFrame;
	BOOL	bDisplayTooltip;
	BOOL	bHideAdvancedOptions;
	BOOL	bFullTMEM;
	BOOL	bUseFullTMEM;

	DWORD	bDisplayOnscreenFPS;
	DWORD	FPSColor;

	DWORD	forceTextureFilter;
	DWORD	textureEnhancement;
	DWORD	textureEnhancementControl;
	DWORD	textureQuality;
	BOOL	bTexRectOnly;
	BOOL	bSmallTextureOnly;

	int		RenderBufferSetting;

	int		DirectXDepthBufferSetting;
	DWORD	DirectXAntiAliasingValue;
	DWORD	DirectXAnisotropyValue;

	DWORD	DirectXMaxFSAA;
	DWORD	DirectXMaxAnisotropy;
	int		DirectXCombiner;
	int		DirectXDevice;

	int		OpenglDepthBufferSetting;
	int		OpenglRenderSetting;
	DWORD	colorQuality;

	HACK_FOR_GAMES	enableHackForGames;
} GlobalOptions;

extern GlobalOptions options;

typedef struct {
	bool	bUpdateCIInfo;

	bool	bCheckBackBufs;			// Check texture again against the recent backbuffer addresses
	bool	bWriteBackBufToRDRAM;	// If a recent backbuffer is used, write its content back to RDRAM
	bool	bLoadBackBufFromRDRAM;	// Load content from RDRAM and draw into backbuffer
	bool	bIgnore;				// Ignore all rendering into texture buffers

	bool	bSupportTxtBufs;		// Support render-to-texture
	bool	bCheckTxtBufs;			// Check texture again against the the last texture buffer addresses
	bool	bTxtBufWriteBack;		// Write back texture buffer into RDRAM
	bool	bLoadRDRAMIntoTxtBuf;	// Load RDRAM content and render into texture buffer

	bool	bAtEachFrameUpdate;		// Reload and write back at each frame buffer and CI update

	bool	bProcessCPUWrite;
	bool	bProcessCPURead;

	bool	bFillRectNextTextureBuffer;
	//bool	bFillColor;
} FrameBufferOptions;

extern FrameBufferOptions frameBufferOptions;

BOOL InitConfiguration(void);

typedef struct {
	DWORD	N64FrameBufferEmuType;
	DWORD	N64FrameBufferWriteBackControl;
	DWORD	N64RenderToTextureEmuType;
	DWORD	screenUpdateSetting;
	BOOL 	bNormalCombiner;
	BOOL 	bNormalBlender;
	BOOL 	bFastTexCRC;
	BOOL 	bAccurateTextureMapping;
	BOOL 	bInN64Resolution;
	BOOL 	bDoubleSizeForSmallTxtrBuf;
	BOOL 	bSaveVRAM;
	BOOL 	bOverlapAutoWriteBack;
} RomOptions;

extern RomOptions defaultRomOptions;
extern RomOptions currentRomOptions;

typedef struct IniSection
{
	bool    bOutput;
	char	crccheck[50];
	char	name[50];

	// Options with changeable default values
	DWORD	dwNormalCombiner;
	DWORD	dwNormalBlender;
	DWORD	dwFastTextureCRC;
	DWORD	dwAccurateTextureMapping;
	DWORD	dwFrameBufferOption;
	DWORD	dwRenderToTextureOption;
	DWORD	dwScreenUpdateSetting;

	// Options with FALSE as default values
	BOOL	bDisableBlender;
	BOOL	bForceScreenClear;
	BOOL	bEmulateClear;
	BOOL	bForceDepthBuffer;

	// Less useful options
	BOOL	bDisableObjBG;
	BOOL 	bDisableTextureCRC;
	BOOL 	bIncTexRectEdge;
	BOOL 	bTextureScaleHack;
	BOOL 	bFastLoadTile;
	BOOL 	bPrimaryDepthHack;
	BOOL 	bTexture1Hack;
	BOOL	bDisableCulling;

	int		VIWidth;
	int		VIHeight;
	DWORD	UseCIWidthAndRatio;

	DWORD	dwFullTMEM;
	BOOL	bTxtSizeMethod2;
	BOOL	bEnableTxtLOD;
} section;

class IniFile 
{
public: //functions

	IniFile(char *szFileName);
	~IniFile();
		
	BOOL ReadIniFile();
	void WriteIniFile();
	int FindEntry(DWORD dwCRC1, DWORD dwCRC2, BYTE nCountryID, LPCTSTR szName); 


public:
	bool	bChanged;


	std::vector<IniSection> sections;
	//section sections[1000];
	//int sectionstotal;

private: //functions

	void OutputSectionDetails(DWORD i, FILE * fh);

private: //variables

	char m_szFileName[300];
	char m_szSectionName[300];

};


struct ROMHeader
{
	BYTE  x1, x2, x3, x4;
	DWORD dwClockRate;
	DWORD dwBootAddressOffset;
	DWORD dwRelease;
	DWORD dwCRC1;
	DWORD dwCRC2;
	uint64   qwUnknown1;
	char  szName[20];
	DWORD dwUnknown2;
	WORD  wUnknown3;
	BYTE  nUnknown4;
	BYTE  nManufacturer;
	WORD  wCartID;
	s8    nCountryID;
	BYTE  nUnknown5;
};
#pragma pack()

typedef struct 
{
	// Other info from the rom. This is for convenience
	TCHAR	szGameName[50+1];
	s8	nCountryID;

	// Copy of the ROM header
	ROMHeader	romheader;

	// With changeable default values
	DWORD	dwNormalCombiner;
	DWORD	dwNormalBlender;
	DWORD	dwAccurateTextureMapping;
	DWORD	dwFastTextureCRC;
	DWORD	dwFrameBufferOption;
	DWORD	dwRenderToTextureOption;
	DWORD	dwScreenUpdateSetting;

	// With FALSE as its default values
	BOOL	bForceScreenClear;
	BOOL	bEmulateClear;
	BOOL	bForceDepthBuffer;
	BOOL	bDisableBlender;

	// Less useful options
	BOOL	bDisableObjBG;
	BOOL	bDisableTextureCRC;
	BOOL	bIncTexRectEdge;
	BOOL	bTextureScaleHack;
	BOOL	bFastLoadTile;
	BOOL	bPrimaryDepthHack;
	BOOL	bTexture1Hack;
	BOOL 	bDisableCulling;
	int		VIWidth;
	int		VIHeight;
	DWORD	UseCIWidthAndRatio;

	DWORD	dwFullTMEM;
	BOOL	bTxtSizeMethod2;
	BOOL	bEnableTxtLOD;
} RomInfo, *LPROMINFO;

typedef struct
{
	s8	nCountryID;
	LPCTSTR szName;
	DWORD nTvType;
} CountryIDInfo;


extern const CountryIDInfo g_CountryCodeInfo[];

extern RomInfo g_curRomInfo;

void ROM_GetRomNameFromHeader(TCHAR * szName, ROMHeader * pHdr);
void ROM_ByteSwap_3210(void *v, DWORD dwLen);

#define TV_SYSTEM_NTSC		1
#define TV_SYSTEM_PAL		0
DWORD CountryCodeToTVSystem(DWORD countryCode);


//#define COMPLETE_N64_TMEM


enum {
    PSH_OPTIONS,
    PSH_DIRECTX,
    PSH_OPENGL,
    PSH_TEXTURE,
    PSH_DEFAULTS,
    PSH_ROM_SETTINGS,
};

void CreateOptionsDialogs(HWND hParent);

#endif
