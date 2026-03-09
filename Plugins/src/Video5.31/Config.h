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
	OGL_TNT2_DEVICE,
	NVIDIA_OGL_DEVICE,

	DIRECTX_DEVICE,
	DIRECTX_9_DEVICE,
	DIRECTX_8_DEVICE,
	DIRECTX_7_DEVICE,
	DIRECTX_6_DEVICE,
	DIRECTX_5_DEVICE,
	XBOX_DIRECTX_DEVICE,
} DaedalusSupportedDeviceType;

typedef struct
{
	char *	name;
	DaedalusSupportedDeviceType		type;
} RenderEngineSetting;

extern char *frameBufferSettings[10];


enum {
	FRM_NONE,
	FRM_IGNORE,
	FRM_RENDER_TO_TEXTURE_FASTER,
	FRM_RENDER_TO_TEXTURE,
	FRM_RENDER_TO_TEXTURE_PLUS,
	FRM_BASIC,
	FRM_BASIC_AND_WRITEBACK,
	FRM_WRITEBACK_AND_RELOAD,
	FRM_COMPLETE,
	FRM_WITH_EMULATOR, //9
};

//freakdave
enum {
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
	TEXTURE_ENHANCEMENT_SMALL_TEXTURE_ONLY,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3,
	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4,
};

enum {
	TEXTURE_NO_ENHANCEMENT,
	TEXTURE_2X_ENHANCEMENT,
	TEXTURE_2X_TEXRECT_ENHANCEMENT,
	TEXTURE_2XSAI_ENHANCEMENT,
	TEXTURE_2XSAI_TEXRECT_ENHANCEMENT,
	TEXTURE_SHARPEN_ENHANCEMENT=TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4+1,
	TEXTURE_SHARPEN_MORE_ENHANCEMENT=TEXTURE_SHARPEN_ENHANCEMENT+1,
};

enum {
	SCREEN_UPDATE_DEFAULT = 0,
	SCREEN_UPDATE_AT_VI_UPDATE = 1,
	SCREEN_UPDATE_AT_VI_CHANGE = 2,
	SCREEN_UPDATE_AT_CI_CHANGE = 3,
	SCREEN_UPDATE_AT_1ST_CI_CHANGE = 4,
	SCREEN_UPDATE_AT_1st_PRIMITIVE = 5,
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
};


typedef struct {
	BOOL	bWinFrameMode;
	BOOL	enableHacks;
	BOOL	enableFog;
	BOOL	bForceSoftwareTnL;
	BOOL	enableSSE;
	BOOL	skipFrame;
	BOOL	bDisplayTooltip;

	DWORD	forceTextureFilter;
	DWORD	textureEnhancement;
	DWORD	textureEnhancementControl;

	int		RenderBufferSetting;

	int		DirectXDepthBufferSetting;
	int		OpenglDepthBufferSetting;
	int		OpenglRenderSetting;
	DWORD	OpenglColorBufferSetting;

	HACK_FOR_GAMES	enableHackForGames;
} GlobalOptions;

extern GlobalOptions options;

typedef struct {
	DWORD	N64FrameBufferEmuType;
	DWORD	screenUpdateSetting;
	BOOL 	normalColorCombiner;
	BOOL 	normalAlphaBlender;
	BOOL 	bFastTexCRC;
	BOOL 	forceBufferClear;
	BOOL 	dwEnableObjBG;
	BOOL 	accurateTextureMapping;
	BOOL	bEmulateClear;
	BOOL	bSupportSelfRenderTexture;
} RomOptions;

typedef struct {
	bool	bUpdateCIInfo;

	bool	bSupportBackBufs;			// Check texture again against the recent backbuffer addresses
	bool	bCheckBackBufs;			// Check texture again against the recent backbuffer addresses
	bool	bWriteBackBufToRDRAM;	// If a recent backbuffer is used, write its content back to RDRAM
	bool	bLoadBackBufFromRDRAM;	// Load content from RDRAM and draw into backbuffer
	bool	bIgnore;				// Ignore all rendering into texture buffers

	bool	bSupportTxtBufs;		// Support render-to-texture
	bool	bCheckTxtBufs;			// Check texture again against the the last texture buffer addresses
	bool	bTxtBufWriteBack;		// Write back texture buffer into RDRAM
	bool	bLoadRDRAMIntoTxtBuf;	// Load RDRAM content and render into texture buffer

	bool	bAtEachFrameUpdate;		// Reload and write back at each frame buffer and CI update

	bool	bIngoreIfCIIsZI;		// If CImg is pointed to ZImg, then ignore all drawing other than FillRect
	bool	bProcessZIDrawing;		// Treat ZI as a texture buffer if drawing onto it

	bool	bProcessCPUWrite;
	bool	bProcessCPURead;
} FrameBufferOptions;

extern FrameBufferOptions frameBufferOptions;

extern RomOptions defaultRomOptions;
extern RomOptions currentRomOptions;

LRESULT APIENTRY ConfigDialog(HWND hDlg, unsigned message, LONG wParam, LONG lParam) ;
BOOL InitConfiguration(void);

#endif
