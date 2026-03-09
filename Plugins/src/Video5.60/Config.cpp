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
#include "_BldNum.h"

#include <istream>
#include <fstream>
#include "../../../config.h"

extern int TextureMode;
extern bool FrameSkip;

#define MAIN_RICE_VIDEO		"Software\\RICEVIDEO560\\WINDOW"
//#define RICE_VIDEO_INI_FILE		"D:\\RiceVideo5.6.0.ini"
#define RICE_VIDEO_INI_FILE		"RiceVideo5.6.0.ini"

char *frameBufferSettings[] =
{
	"None (default)",
	"Hide Framebuffer Effects",
	"Basic Framebuffer",
	"Basic & Write Back",
	"Write Back & Reload",
	"Write Back Every Frame",
	"With Emulator",
};

const int resolutions[][2] =
{
	{320, 240},
	{400, 300},
	{480, 360},
	{512, 384},
	{640, 480},
	{800, 600},
	{1024, 768},
	{1152, 864},
	{1280, 960},
	{1400, 1050},
	{1600, 1200},
	{1920, 1440},
	{2048, 1536},
};
const int numberOfResolutions = sizeof(resolutions)/sizeof(int)/2;

char *frameBufferWriteBackControlSettings[] =
{
	"Every Frame (default)",
	"Every 2 Frames",
	"Every 3 Frames",
	"Every 4 Frames",
	"Every 5 Frames",
	"Every 6 Frames",
	"Every 7 Frames",
	"Every 8 Frames",
};

char *renderToTextureSettings[] =
{
	"None (default)",
	"Hide Render-to-texture Effects",
	"Basic Render-to-texture",
	"Basic & Write Back",
	"Write Back & Reload",
};

const char *screenUpdateSettings[] =
{
	"At VI origin update",
	"At VI origin change (default)",
	"At CI change",
	"At the 1st CI change",
	"At the 1st drawing",
	"Before clear the screen"
};

WindowSettingStruct windowSetting;
GlobalOptions options;
RomOptions defaultRomOptions;
RomOptions currentRomOptions;
FrameBufferOptions frameBufferOptions;

//=======================================================

extern IniFile *g_pIniFile;

const RenderEngineSetting RenderEngineSettings[] =
{
	"DirectX",	DIRECTX_DEVICE,
	"OpenGL", OGL_DEVICE,
};

const SettingInfo TextureQualitySettings[] =
{
	"Default",			FORCE_DEFAULT_FILTER,
	"32-bit Texture",	FORCE_POINT_FILTER,
	"16-bit Texture",	FORCE_LINEAR_FILTER,
};



//freakdave
const SettingInfo ForceTextureFilterSettings[] =
{
	//"N64 Default Texture Filter",	FORCE_DEFAULT_FILTER,
	"Force No Filter (valid for Mip Mapping only)",	FORCE_NONE_FILTER,
	"Force Nearest Filter (faster, low quality)",	FORCE_POINT_FILTER,
	"Force Linear Filter (slower, better quality)",	FORCE_LINEAR_FILTER,
	//"Force Bilinear Filter (slower, best quality)", FORCE_BILINEAR_FILTER,
	"Force Anisotropic Filter (even slower, even better quality)", FORCE_ANISOTROPIC_FILTER,
	"Force Flatcubic Filter (cubic filtering)", FORCE_FLATCUBIC_FILTER,
	"Force Gaussiancubic Filter (different cubic kernel)", FORCE_GAUSSIANCUBIC_FILTER,
};

const SettingInfo TextureEnhancementSettings[] =
{
	"N64 original texture (No enhancement)",	TEXTURE_NO_ENHANCEMENT,
	"2x (Double the texture size)",	TEXTURE_2X_ENHANCEMENT,
	"2xSaI", TEXTURE_2XSAI_ENHANCEMENT,
	"hq2x", TEXTURE_HQ2X_ENHANCEMENT,
	"lq2x", TEXTURE_LQ2X_ENHANCEMENT,
	"hq4x", TEXTURE_HQ4X_ENHANCEMENT,
	"Sharpen", TEXTURE_SHARPEN_ENHANCEMENT,
	"Sharpen More", TEXTURE_SHARPEN_MORE_ENHANCEMENT,
};

const SettingInfo TextureEnhancementControlSettings[] =
{
	"Normal",	TEXTURE_ENHANCEMENT_NORMAL,
	"Smooth",	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1,
	"Less smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2,
	"2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3,
	"Less 2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4,
};

const SettingInfo colorQualitySettings[] =
{
	"16-bit",	TEXTURE_FMT_A4R4G4B4,
	"32-bit (def)",	TEXTURE_FMT_A8R8G8B8,
};

const char*	strDXDeviceDescs[] = { "HAL", "REF" };

const SettingInfo openGLDepthBufferSettings[] =
{
	"16-bit (def)",	16,
	"32-bit",	32,
};

const RenderEngineSetting OpenGLRenderSettings[] =
{
	"To Fit Your Video Card",	OGL_DEVICE,
	"OpenGL 1.1 (Lowest)",		OGL_1_1_DEVICE,
	"OpenGL 1.2/1.3",			OGL_1_2_DEVICE,
	"OpenGL 1.4",				OGL_1_4_DEVICE,
	//"OpenGL 1.4, the 2nd combiner",			OGL_1_4_V2_DEVICE,
	"OpenGL for Nvidia TNT or better",		OGL_TNT2_DEVICE,
	"OpenGL for Nvidia Geforce or better ",	NVIDIA_OGL_DEVICE,
};


/*
*	Constants
*/
/*
D3DFMT_D16_LOCKABLE         = 70,
D3DFMT_D32                  = 71,
D3DFMT_D15S1                = 73,
D3DFMT_D24S8                = 75,
D3DFMT_D16                  = 80,
D3DFMT_D24X8                = 77,
D3DFMT_D24X4S4              = 79,
*/

BufferSettingInfo DirectXRenderBufferSettings[] =
{
	"Copy",			1,		D3DSWAPEFFECT_COPY,
	"Flip (def)",	1,		D3DSWAPEFFECT_FLIP,
	//"Double Buffer Copy Sync",	1,		D3DSWAPEFFECT_COPY_VSYNC,
	//"Double Buffer Discard",	1,		D3DSWAPEFFECT_DISCARD,
	//"Triple Buffer Flip",		2,		D3DSWAPEFFECT_FLIP,
	//"Quadruple Buffer Flip",	3,		D3DSWAPEFFECT_FLIP,
	//"Triple Buffer Discard",	2,		D3DSWAPEFFECT_DISCARD,
};

BufferSettingInfo DirectXDepthBufferSetting[] =
{
	"16-bit (def)",				D3DFMT_D16,				D3DFMT_D16,
		//"16-bit signed",			D3DFMT_D15S1,			D3DFMT_D15S1,
		//"16-bit lockable",			D3DFMT_D16_LOCKABLE,	D3DFMT_D16_LOCKABLE,
		//"32-bit",					D3DFMT_D32,				D3DFMT_D32,
		"32-bit signed",			D3DFMT_D24S8,			D3DFMT_D24S8,
		//"32-bit D24X8",				D3DFMT_D24X4S4,			D3DFMT_D24X4S4,
		//"32-bit D24X4S4",			D3DFMT_D24X8,			D3DFMT_D24X8,
};

BufferSettingInfo DirectXCombinerSettings[] =
{
	"To Fit Your Video Card",			DX_BEST_FIT,			DX_BEST_FIT,
	"For Low End Video Cards",			DX_LOW_END,				DX_LOW_END,
	"For High End Video Card",			DX_HIGH_END,			DX_HIGH_END,
	"For NVidia TNT/TNT2/Geforce/GF2",	DX_NVIDIA_TNT,			DX_NVIDIA_TNT,
	"Limited 2 stage combiner",			DX_2_STAGES,			DX_2_STAGES,
	"Limited 3 stage combiner",			DX_3_STAGES,			DX_3_STAGES,
	"Limited 4 stage combiner",			DX_4_STAGES,			DX_4_STAGES,
	"Pixel Shader",						DX_PIXEL_SHADER,		DX_PIXEL_SHADER,
	"Semi-Pixel Shader",				DX_SEMI_PIXEL_SHADER,	DX_SEMI_PIXEL_SHADER,
};

const SettingInfo OnScreenDisplaySettings[] =
{
	"Display Nothing",							ONSCREEN_DISPLAY_NOTHING,
	"Display DList Per Second",					ONSCREEN_DISPLAY_DLIST_PER_SECOND,
	"Display Frame Per Second",					ONSCREEN_DISPLAY_FRAME_PER_SECOND,
	"Display Debug Information Only",			ONSCREEN_DISPLAY_DEBUG_INFORMATION_ONLY,
	"Display Messages From CPU Core Only",		ONSCREEN_DISPLAY_TEXT_FROM_CORE_ONLY,
	"Display DList Per Second With Core Msgs",	ONSCREEN_DISPLAY_DLIST_PER_SECOND_WITH_CORE_MSG,
	"Display Frame Per Second With Core Msgs",	ONSCREEN_DISPLAY_FRAME_PER_SECOND_WITH_CORE_MSG,
	"Display Debug Information With Core Msgs",	ONSCREEN_DISPLAY_DEBUG_INFORMATION_WITH_CORE_MSG,
};

int numberOfDirectXRenderBufferSettings = sizeof(DirectXRenderBufferSettings)/sizeof(BufferSettingInfo);
int numberOfDirectXCombinerSettings = sizeof(DirectXCombinerSettings)/sizeof(BufferSettingInfo);
int numberOfDirectXDepthBufferSettings = sizeof(DirectXDepthBufferSetting)/sizeof(BufferSettingInfo);

const int numberOfRenderEngineSettings = sizeof(RenderEngineSettings)/sizeof(RenderEngineSetting);
const int numberOfOpenGLRenderEngineSettings = sizeof(OpenGLRenderSettings)/sizeof(RenderEngineSetting);

void WriteConfiguration(void);
void GenerateCurrentRomOptions();

HWND	g_hwndTT=NULL;
HWND	g_hwndDlg=NULL;
HHOOK	g_hhk = NULL;

extern "C" BOOL __stdcall EnumChildProc(HWND hwndCtrl, LPARAM lParam);
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
VOID OnWMNotify(LPARAM lParam);
BOOL CreateDialogTooltip(void);
BOOL EnumChildWndTooltip(void);
 
//////////////////////////////////////////////////////////////////////////
void GenerateFrameBufferOptions(void)
{
	if( CDeviceBuilder::GetGeneralDeviceType() == OGL_DEVICE )
	{
		// OpenGL does not support much yet
		if( currentRomOptions.N64FrameBufferEmuType != FRM_BUF_NONE )
			currentRomOptions.N64FrameBufferEmuType = FRM_BUF_IGNORE;
		if( currentRomOptions.N64RenderToTextureEmuType != TXT_BUF_NONE )
			currentRomOptions.N64RenderToTextureEmuType = TXT_BUF_IGNORE;
	}

	frameBufferOptions.bUpdateCIInfo			= false;

	frameBufferOptions.bCheckBackBufs			= false;
	frameBufferOptions.bWriteBackBufToRDRAM		= false;
	frameBufferOptions.bLoadBackBufFromRDRAM	= false;

	frameBufferOptions.bIgnore					= true;

	frameBufferOptions.bSupportTxtBufs			= false;
	frameBufferOptions.bCheckTxtBufs			= false;
	frameBufferOptions.bTxtBufWriteBack			= false;
	frameBufferOptions.bLoadRDRAMIntoTxtBuf		= false;

	frameBufferOptions.bProcessCPUWrite			= false;
	frameBufferOptions.bProcessCPURead			= false;
	frameBufferOptions.bAtEachFrameUpdate		= false;

	switch( currentRomOptions.N64FrameBufferEmuType )
	{
	case FRM_BUF_NONE:
		break;
	case FRM_BUF_COMPLETE:
		frameBufferOptions.bAtEachFrameUpdate		= true;
		frameBufferOptions.bProcessCPUWrite			= true;
		frameBufferOptions.bProcessCPURead			= true;
		frameBufferOptions.bUpdateCIInfo			= true;
		break;
	case FRM_BUF_WRITEBACK_AND_RELOAD:
		frameBufferOptions.bLoadBackBufFromRDRAM	= true;
	case FRM_BUF_BASIC_AND_WRITEBACK:
		frameBufferOptions.bWriteBackBufToRDRAM		= true;
	case FRM_BUF_BASIC:
		frameBufferOptions.bCheckBackBufs			= true;
	case FRM_BUF_IGNORE:
		frameBufferOptions.bUpdateCIInfo			= true;
		break;
	case FRM_BUF_WITH_EMULATOR:
		// For Dr. Mario
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bProcessCPUWrite			= true;
		frameBufferOptions.bProcessCPURead			= true;
		break;
	}

	switch( currentRomOptions.N64RenderToTextureEmuType )
	{
	case TXT_BUF_NONE:
		frameBufferOptions.bSupportTxtBufs			= false;
		break;
	case TXT_BUF_WRITE_BACK_AND_RELOAD:
		frameBufferOptions.bLoadRDRAMIntoTxtBuf		= true;
	case TXT_BUF_WRITE_BACK:
		frameBufferOptions.bTxtBufWriteBack			= true;
	case TXT_BUF_NORMAL:
		frameBufferOptions.bCheckTxtBufs			= true;
		frameBufferOptions.bIgnore					= false;
	case TXT_BUF_IGNORE:
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bSupportTxtBufs			= true;
		break;
	}

	if( currentRomOptions.screenUpdateSetting >= SCREEN_UPDATE_AT_CI_CHANGE )
	{
		frameBufferOptions.bUpdateCIInfo = true;
	}
}
//////////////////////////////////////////////////////////////////////////

//========================================================================

BOOL TestRegistry(void)
{
	return FALSE;	 
}

void WriteConfiguration(void)
{
 
}

bool isMMXSupported() 
{ 
	int IsMMXSupported; 
	__asm 
	{ 
		mov eax,1	// CPUID level 1 
		cpuid		// EDX = feature flag 
		and edx,0x800000		// test bit 23 of feature flag 
		mov IsMMXSupported,edx	// != 0 if MMX is supported 
	} 
	if (IsMMXSupported != 0) 
		return true; 
	else 
		return false; 
} 

bool isSSESupported() 
{
#if _MSC_VER > 1200
	int SSESupport;

	// And finally, check the CPUID for Streaming SIMD Extensions support.
	_asm{
		mov		eax, 1			// Put a "1" in eax to tell CPUID to get the feature bits
		cpuid					// Perform CPUID (puts processor feature info into EDX)
		and		edx, 02000000h	// Test bit 25, for Streaming SIMD Extensions existence.
		mov		SSESupport, edx	// SIMD Extensions).  Set return value to 1 to indicate,
	}
	
	if (SSESupport != 0) 
		return true; 
	else 
		return false; 
#else
	return false;
#endif
} 

void ReadConfiguration(void)
{
	options.bEnableHacks = TRUE;
	options.bEnableSSE = TRUE;

	defaultRomOptions.screenUpdateSetting = SCREEN_UPDATE_AT_VI_CHANGE;

	status.isMMXSupported = isMMXSupported();
	status.isSSESupported = isSSESupported();
	defaultRomOptions.N64FrameBufferEmuType = FRM_BUF_NONE;
	defaultRomOptions.N64FrameBufferWriteBackControl = FRM_BUF_WRITEBACK_NORMAL;
	defaultRomOptions.N64RenderToTextureEmuType = TXT_BUF_NONE;
		
	
		options.bEnableFog = TRUE;
		options.bWinFrameMode = FALSE;
		options.bFullTMEM = TRUE;
		options.bUseFullTMEM = TRUE;
		options.bForceSoftwareTnL = TRUE;
		options.bForceSoftwareClipper = FALSE;
		options.bEnableSSE = TRUE;
		options.RenderBufferSetting = 0;
		//options.forceTextureFilter = 0;
		//freakdave - override default texture filter setting
		options.forceTextureFilter = TextureMode;
		options.textureQuality = TXT_QUALITY_32BIT;
		options.bTexRectOnly = FALSE;
		options.bSmallTextureOnly = FALSE;
		options.DirectXDepthBufferSetting = 0;
		options.OpenglDepthBufferSetting = 16;
		options.colorQuality = TEXTURE_FMT_A8R8G8B8;
		options.textureEnhancement = 0;
		options.textureEnhancementControl = 1;
		options.bSkipFrame = FrameSkip;
		options.bDisplayTooltip = FALSE;
		options.bHideAdvancedOptions = TRUE;
		options.bDisplayOnscreenFPS = FALSE;
		options.DirectXAntiAliasingValue = 0;
		options.DirectXCombiner = DX_PIXEL_SHADER;
		options.DirectXDevice = XBOX_DIRECTX_DEVICE;	// HAL device
		options.DirectXAnisotropyValue = 0;
		options.DirectXMaxFSAA = 16;
		options.FPSColor = 0xFFFFFFFF;
		options.DirectXMaxAnisotropy = 16;

		defaultRomOptions.N64FrameBufferEmuType = FRM_BUF_NONE;
		defaultRomOptions.N64FrameBufferWriteBackControl = FRM_BUF_WRITEBACK_NORMAL;
		defaultRomOptions.N64RenderToTextureEmuType = TXT_BUF_NONE;
		
		defaultRomOptions.bNormalBlender = FALSE;
		defaultRomOptions.bFastTexCRC=TRUE;
		defaultRomOptions.bNormalCombiner = FALSE;
		defaultRomOptions.bAccurateTextureMapping = TRUE;
		defaultRomOptions.bInN64Resolution = TRUE;//FALSE
		defaultRomOptions.bSaveVRAM = TRUE;//FALSE
		defaultRomOptions.bOverlapAutoWriteBack = FALSE;
		defaultRomOptions.bDoubleSizeForSmallTxtrBuf = FALSE;
		windowSetting.uFullScreenRefreshRate = 0;	// 0 is the default value, means to use Window default frequency
		

	//freakdave
	//0 = None (valid for Mips only)
	//1 = Point
	//2 = Linear
	//3 = Anisotropic
	//4 = Flatcubic
	//5 = Gaussiancubic
	//get TextureFilter information from ini file
	options.forceTextureFilter = TextureMode;
	 
	if (status.isMMXSupported)
		MsgInfo("MMX Enabled");
	else
		MsgInfo("MMX Disabled");
	 
	status.isSSEEnabled = status.isSSESupported && options.bEnableSSE;
	if( status.isSSEEnabled )
	{
		ProcessVertexData = ProcessVertexDataSSE;
		MsgInfo("SSE Enabled");
	}
	else
	{
		ProcessVertexData = ProcessVertexDataNoSSE;
		MsgInfo("SSE Disabled");
	}
}
	
//---------------------------------------------------------------------------------------
BOOL InitConfiguration(void)
{
	//Initialize this DLL

	if( g_pIniFile == NULL )	//Test if we have done initialization
	{
		g_pIniFile = new IniFile(RICE_VIDEO_INI_FILE);
		if (g_pIniFile == NULL)
		{
			ReadConfiguration(); //defaults
			return FALSE;
		}

		if (!g_pIniFile->ReadIniFile())
		{
			ErrorMsg("Unable to read ini file from disk");
			ReadConfiguration(); //defaults
			g_pIniFile->WriteIniFile();
			return FALSE;
		}
	}

	ReadConfiguration();
	return TRUE;
}


void GenerateCurrentRomOptions()
{
	currentRomOptions.N64FrameBufferEmuType		=g_curRomInfo.dwFrameBufferOption;	
	currentRomOptions.N64FrameBufferWriteBackControl		=defaultRomOptions.N64FrameBufferWriteBackControl;	
	currentRomOptions.N64RenderToTextureEmuType	=g_curRomInfo.dwRenderToTextureOption;	
	currentRomOptions.screenUpdateSetting		=g_curRomInfo.dwScreenUpdateSetting;
	currentRomOptions.bNormalCombiner			=g_curRomInfo.dwNormalCombiner;
	currentRomOptions.bNormalBlender			=g_curRomInfo.dwNormalBlender;
	currentRomOptions.bFastTexCRC				=g_curRomInfo.dwFastTextureCRC;
	currentRomOptions.bAccurateTextureMapping	=g_curRomInfo.dwAccurateTextureMapping;

	options.enableHackForGames = NO_HACK_FOR_GAME;
	if ((strncmp(g_curRomInfo.szGameName, "BANJO TOOIE", 11) == 0))
	{
		options.enableHackForGames = HACK_FOR_BANJO_TOOIE;
	}
	else if ((strncmp(g_curRomInfo.szGameName, "DR.MARIO", 8) == 0))
	{
		options.enableHackForGames = HACK_FOR_DR_MARIO;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "Pilot", 5) == 0))
	{
		options.enableHackForGames = HACK_FOR_PILOT_WINGS;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "YOSHI", 5) == 0))
	{
		options.enableHackForGames = HACK_FOR_YOSHI;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "NITRO", 5) == 0))
	{
		options.enableHackForGames = HACK_FOR_NITRO;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "TONY HAWK", 9) == 0))
	{
		options.enableHackForGames = HACK_FOR_TONYHAWK;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "THPS", 4) == 0))
	{
		options.enableHackForGames = HACK_FOR_TONYHAWK;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "SPIDERMAN", 9) == 0))
	{
		options.enableHackForGames = HACK_FOR_TONYHAWK;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "NASCAR", 6) == 0))
	{
		options.enableHackForGames = HACK_FOR_NASCAR;
	}
	else if ((strstr(g_curRomInfo.szGameName, "ZELDA") != 0))
	{
		options.enableHackForGames = HACK_FOR_ZELDA;
	}
	else if ((strstr(g_curRomInfo.szGameName, "Ogre") != 0))
	{
		options.enableHackForGames = HACK_FOR_OGRE_BATTLE;
	}
	else if ((strstr(g_curRomInfo.szGameName, "TWINE") != 0))
	{
		options.enableHackForGames = HACK_FOR_TWINE;
	}
	else if ((strstr(g_curRomInfo.szGameName, "Baseball") != 0) && (strstr(g_curRomInfo.szGameName, "Star") != 0))
	{
		options.enableHackForGames = HACK_FOR_ALL_STAR_BASEBALL;
	}
	else if ((strstr(g_curRomInfo.szGameName, "Tigger") != 0) && (strstr(g_curRomInfo.szGameName, "Honey") != 0))
	{
		options.enableHackForGames = HACK_FOR_TIGER_HONEY_HUNT;
	}
	else if ((strstr(g_curRomInfo.szGameName, "Bust") != 0) && (strstr(g_curRomInfo.szGameName, "Move") != 0))
	{
		options.enableHackForGames = HACK_FOR_BUST_A_MOVE;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "MarioTennis",11) == 0))
	{
		options.enableHackForGames = HACK_FOR_MARIO_TENNIS;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "SUPER BOWLING",13) == 0))
	{
		options.enableHackForGames = HACK_FOR_SUPER_BOWLING;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "CONKER",6) == 0))
	{
		options.enableHackForGames = HACK_FOR_CONKER;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "MK_MYTHOLOGIES",14) == 0))
	{
		options.enableHackForGames = HACK_REVERSE_Y_COOR;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "Fighting Force",14) == 0))
	{
		options.enableHackForGames = HACK_REVERSE_XY_COOR;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "GOLDENEYE",9) == 0))
	{
		options.enableHackForGames = HACK_FOR_GOLDEN_EYE;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "F-ZERO",6) == 0))
	{
		options.enableHackForGames = HACK_FOR_FZERO;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "Command&Conquer",15) == 0))
	{
		options.enableHackForGames = HACK_FOR_COMMANDCONQUER;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "READY 2 RUMBLE",14) == 0))
	{
		options.enableHackForGames = HACK_FOR_RUMBLE;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "READY to RUMBLE",15) == 0))
	{
		options.enableHackForGames = HACK_FOR_RUMBLE;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "South Park Rally",16) == 0))
	{
		options.enableHackForGames = HACK_FOR_SOUTH_PARK_RALLY;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "Extreme G 2",11) == 0))
	{
		options.enableHackForGames = HACK_FOR_EXTREME_G2;
	}

	if( currentRomOptions.N64FrameBufferEmuType == 0 )		currentRomOptions.N64FrameBufferEmuType = defaultRomOptions.N64FrameBufferEmuType;
	else currentRomOptions.N64FrameBufferEmuType--;
	if( currentRomOptions.N64RenderToTextureEmuType == 0 )		currentRomOptions.N64RenderToTextureEmuType = defaultRomOptions.N64RenderToTextureEmuType;
	else currentRomOptions.N64RenderToTextureEmuType--;
	if( currentRomOptions.screenUpdateSetting == 0 )		currentRomOptions.screenUpdateSetting = defaultRomOptions.screenUpdateSetting;
	if( currentRomOptions.bNormalCombiner == 0 )			currentRomOptions.bNormalCombiner = defaultRomOptions.bNormalCombiner;
	else currentRomOptions.bNormalCombiner--;
	if( currentRomOptions.bNormalBlender == 0 )			currentRomOptions.bNormalBlender = defaultRomOptions.bNormalBlender;
	else currentRomOptions.bNormalBlender--;
	if( currentRomOptions.bFastTexCRC == 0 )				currentRomOptions.bFastTexCRC = defaultRomOptions.bFastTexCRC;
	else currentRomOptions.bFastTexCRC--;
	if( currentRomOptions.bAccurateTextureMapping == 0 )		currentRomOptions.bAccurateTextureMapping = defaultRomOptions.bAccurateTextureMapping;
	else currentRomOptions.bAccurateTextureMapping--;

	options.bUseFullTMEM = ((options.bFullTMEM && (g_curRomInfo.dwFullTMEM == 0)) || g_curRomInfo.dwFullTMEM == 2);

	GenerateFrameBufferOptions();
}

extern IniFile * g_pIniFile;

void Ini_GetRomOptions(LPROMINFO pRomInfo)
{
	LONG i;

	i = g_pIniFile->FindEntry(pRomInfo->romheader.dwCRC1,
							  pRomInfo->romheader.dwCRC2,
							  pRomInfo->romheader.nCountryID,
							  pRomInfo->szGameName);

	lstrcpyn(pRomInfo->szGameName, g_pIniFile->sections[i].name, 50);

	pRomInfo->bDisableTextureCRC	= g_pIniFile->sections[i].bDisableTextureCRC;
	pRomInfo->bDisableCulling		= g_pIniFile->sections[i].bDisableCulling;
	pRomInfo->bIncTexRectEdge		= g_pIniFile->sections[i].bIncTexRectEdge;
	pRomInfo->bTextureScaleHack		= g_pIniFile->sections[i].bTextureScaleHack;
	pRomInfo->bPrimaryDepthHack		= g_pIniFile->sections[i].bPrimaryDepthHack;
	pRomInfo->bTexture1Hack			= g_pIniFile->sections[i].bTexture1Hack;
	pRomInfo->bFastLoadTile			= g_pIniFile->sections[i].bFastLoadTile;
	pRomInfo->VIWidth				= g_pIniFile->sections[i].VIWidth;
	pRomInfo->VIHeight				= g_pIniFile->sections[i].VIHeight;
	pRomInfo->UseCIWidthAndRatio	= g_pIniFile->sections[i].UseCIWidthAndRatio;
	pRomInfo->dwFullTMEM			= g_pIniFile->sections[i].dwFullTMEM;
	pRomInfo->bTxtSizeMethod2		= g_pIniFile->sections[i].bTxtSizeMethod2;
	pRomInfo->bEnableTxtLOD			= g_pIniFile->sections[i].bEnableTxtLOD;

	pRomInfo->dwFastTextureCRC		= g_pIniFile->sections[i].dwFastTextureCRC;
	pRomInfo->bEmulateClear		= g_pIniFile->sections[i].bEmulateClear;
	pRomInfo->bForceScreenClear	= g_pIniFile->sections[i].bForceScreenClear;
	pRomInfo->dwAccurateTextureMapping	= g_pIniFile->sections[i].dwAccurateTextureMapping;
	pRomInfo->dwNormalBlender		= g_pIniFile->sections[i].dwNormalBlender;
	pRomInfo->bDisableBlender		= g_pIniFile->sections[i].bDisableBlender;
	pRomInfo->dwNormalCombiner		= g_pIniFile->sections[i].dwNormalCombiner;
	pRomInfo->bForceDepthBuffer	= g_pIniFile->sections[i].bForceDepthBuffer;
	pRomInfo->bDisableObjBG			= g_pIniFile->sections[i].bDisableObjBG;
	pRomInfo->dwFrameBufferOption	= g_pIniFile->sections[i].dwFrameBufferOption;
	pRomInfo->dwRenderToTextureOption	= g_pIniFile->sections[i].dwRenderToTextureOption;
	pRomInfo->dwScreenUpdateSetting	= g_pIniFile->sections[i].dwScreenUpdateSetting;
}

void Ini_StoreRomOptions(LPROMINFO pRomInfo)
{
	LONG i;

	i = g_pIniFile->FindEntry(pRomInfo->romheader.dwCRC1,
		pRomInfo->romheader.dwCRC2,
		pRomInfo->romheader.nCountryID,
		pRomInfo->szGameName);

	if( g_pIniFile->sections[i].bDisableTextureCRC	!=pRomInfo->bDisableTextureCRC )
	{
		g_pIniFile->sections[i].bDisableTextureCRC	=pRomInfo->bDisableTextureCRC	 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].bDisableCulling	!=pRomInfo->bDisableCulling )
	{
		g_pIniFile->sections[i].bDisableCulling	=pRomInfo->bDisableCulling	 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].dwFastTextureCRC !=pRomInfo->dwFastTextureCRC )
	{
		g_pIniFile->sections[i].dwFastTextureCRC	=pRomInfo->dwFastTextureCRC		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].bEmulateClear !=pRomInfo->bEmulateClear )
	{
		g_pIniFile->sections[i].bEmulateClear	=pRomInfo->bEmulateClear		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].dwNormalBlender		!=pRomInfo->dwNormalBlender )
	{
		g_pIniFile->sections[i].dwNormalBlender		=pRomInfo->dwNormalBlender		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].bDisableBlender	!=pRomInfo->bDisableBlender )
	{
		g_pIniFile->sections[i].bDisableBlender	=pRomInfo->bDisableBlender		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].bForceScreenClear	!=pRomInfo->bForceScreenClear )
	{
		g_pIniFile->sections[i].bForceScreenClear	=pRomInfo->bForceScreenClear		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwAccurateTextureMapping	!=pRomInfo->dwAccurateTextureMapping )
	{
		g_pIniFile->sections[i].dwAccurateTextureMapping	=pRomInfo->dwAccurateTextureMapping		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwNormalCombiner	!=pRomInfo->dwNormalCombiner )
	{
		g_pIniFile->sections[i].dwNormalCombiner	=pRomInfo->dwNormalCombiner		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bForceDepthBuffer	!=pRomInfo->bForceDepthBuffer )
	{
		g_pIniFile->sections[i].bForceDepthBuffer	=pRomInfo->bForceDepthBuffer		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bDisableObjBG	!=pRomInfo->bDisableObjBG )
	{
		g_pIniFile->sections[i].bDisableObjBG	=pRomInfo->bDisableObjBG		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwFrameBufferOption	!=pRomInfo->dwFrameBufferOption )
	{
		g_pIniFile->sections[i].dwFrameBufferOption	=pRomInfo->dwFrameBufferOption		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwRenderToTextureOption	!=pRomInfo->dwRenderToTextureOption )
	{
		g_pIniFile->sections[i].dwRenderToTextureOption	=pRomInfo->dwRenderToTextureOption		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwScreenUpdateSetting	!=pRomInfo->dwScreenUpdateSetting )
	{
		g_pIniFile->sections[i].dwScreenUpdateSetting	=pRomInfo->dwScreenUpdateSetting		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bIncTexRectEdge	!= pRomInfo->bIncTexRectEdge )
	{
		g_pIniFile->sections[i].bIncTexRectEdge		=pRomInfo->bIncTexRectEdge;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bTextureScaleHack	!= pRomInfo->bTextureScaleHack )
	{
		g_pIniFile->sections[i].bTextureScaleHack		=pRomInfo->bTextureScaleHack;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bPrimaryDepthHack	!= pRomInfo->bPrimaryDepthHack )
	{
		g_pIniFile->sections[i].bPrimaryDepthHack		=pRomInfo->bPrimaryDepthHack;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bTexture1Hack	!= pRomInfo->bTexture1Hack )
	{
		g_pIniFile->sections[i].bTexture1Hack		=pRomInfo->bTexture1Hack;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bFastLoadTile	!= pRomInfo->bFastLoadTile )
	{
		g_pIniFile->sections[i].bFastLoadTile	=pRomInfo->bFastLoadTile;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].VIWidth	!= pRomInfo->VIWidth )
	{
		g_pIniFile->sections[i].VIWidth	=pRomInfo->VIWidth;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].VIHeight	!= pRomInfo->VIHeight )
	{
		g_pIniFile->sections[i].VIHeight	=pRomInfo->VIHeight;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].UseCIWidthAndRatio	!= pRomInfo->UseCIWidthAndRatio )
	{
		g_pIniFile->sections[i].UseCIWidthAndRatio	=pRomInfo->UseCIWidthAndRatio;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwFullTMEM	!= pRomInfo->dwFullTMEM )
	{
		g_pIniFile->sections[i].dwFullTMEM	=pRomInfo->dwFullTMEM;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bTxtSizeMethod2	!= pRomInfo->bTxtSizeMethod2 )
	{
		g_pIniFile->sections[i].bTxtSizeMethod2	=pRomInfo->bTxtSizeMethod2;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].bEnableTxtLOD	!= pRomInfo->bEnableTxtLOD )
	{
		g_pIniFile->sections[i].bEnableTxtLOD	=pRomInfo->bEnableTxtLOD;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->bChanged )
	{
		TRACE0("Rom option is changed");
	}
	else
	{
		TRACE0("Rom option isn't changed");
	}
}
 
BOOL CreateDialogTooltip(void) 
{
     
	
    return TRUE;
} 

BOOL EnumChildWndTooltip(void)
{
	return false;
}
 

// GetMsgProc - monitors the message stream for mouse messages intended 
//     for a control window in the dialog box. 
// Returns a message-dependent value. 
// nCode - hook code. 
// wParam - message flag (not used). 
// lParam - address of an MSG structure. 
 

std::ifstream& getline( std::ifstream &is, char *str );


///////////////////////////////////////////////
//// Constructors / Deconstructors
///////////////////////////////////////////////

IniFile::IniFile(char *szFileName)
{
	sections.clear();
	bChanged = false;

	if (szFileName == NULL)
		strcpy(m_szFileName, "");
	else 
		strcpy(m_szFileName, szFileName);

}

IniFile::~IniFile()
{
	if (bChanged)
	{
		WriteIniFile();
		TRACE0("Write back INI file");
	}
}

char * left(char * src, int nchars)
{
	static char dst[300];			// BUGFIX (STRMNNRM)
	strncpy(dst,src,nchars);
	dst[nchars]=0;
	return dst;
}

char * right(char *src, int nchars)
{
	static char dst[300];			// BUGFIX (STRMNNRM)
	strncpy(dst, src + strlen(src) - nchars, nchars);
	dst[nchars]=0;
	return dst;
}

char * tidy(char * s)
{
	char * p = s + lstrlen(s);

	p--;
	while (p >= s && *p == ' ')
	{
		*p = 0;
		p--;
	}
	return s;

}

extern void GetPluginDir( char * Directory );

BOOL IniFile::ReadIniFile()
{
	std::ifstream inifile;
	char readinfo[100];
	char tempreadinfo1[100];
	char tempreadinfo2[100];
	char trim[]="{}"; //remove first and last character

	char filename[256];
	GetPluginDir(filename);
	strcat(filename,m_szFileName);
	//strcpy(filename,m_szFileName);

	// try D if it's not on T
	if (!PathFileExists(filename)) {
		sprintf(filename, "D:\\%s", m_szFileName);
		if (!PathFileExists(filename)) {
			ErrorMsg("%s Failed to Load!", filename);
			return FALSE;
		} else {
			MsgInfo("%s Loaded Successfully!", filename);
		}
	} else {
		MsgInfo("%s Loaded Successfully!", filename);
	}
	
	inifile.open(filename);

	if (inifile.fail())
	{
		return FALSE;
	}

	while (getline(inifile,readinfo)/*&&sectionno<999*/)
	{
		tidy(readinfo);

		if (readinfo[0] == '/')
			continue;

		if (!lstrcmpi(readinfo,"")==0)
		{
			if (readinfo[0] == '{') //if a section heading
			{
				section newsection;

				memset(tempreadinfo2, 0x00, sizeof(tempreadinfo2));

				strcpy(tempreadinfo1,&readinfo[1]);
				strncpy(tempreadinfo2, tempreadinfo1,strlen(tempreadinfo1) - 1);
				strcpy(readinfo, tempreadinfo2);

				strcpy(newsection.crccheck, readinfo);

				newsection.bDisableTextureCRC = FALSE;
				newsection.bDisableCulling = FALSE;
				newsection.bIncTexRectEdge = FALSE;
				newsection.bTextureScaleHack = FALSE;
				newsection.bFastLoadTile = FALSE;
				newsection.bPrimaryDepthHack = FALSE;
				newsection.bTexture1Hack = FALSE;
				newsection.bDisableObjBG = FALSE;
				newsection.VIWidth = -1;
				newsection.VIHeight = -1;
				newsection.UseCIWidthAndRatio = NOT_USE_CI_WIDTH_AND_RATIO;
				newsection.dwFullTMEM = 0;
				newsection.bTxtSizeMethod2 = FALSE;
				newsection.bEnableTxtLOD = FALSE;

				newsection.bEmulateClear = FALSE;
				newsection.bForceScreenClear = FALSE;
				newsection.bDisableBlender = FALSE;
				newsection.bForceDepthBuffer = FALSE;
				newsection.dwFastTextureCRC = 0;
				newsection.dwAccurateTextureMapping = 0;
				newsection.dwNormalBlender = 0;
				newsection.dwNormalCombiner = 0;
				newsection.dwFrameBufferOption = 0;
				newsection.dwRenderToTextureOption = 0;
				newsection.dwScreenUpdateSetting = 0;

				sections.push_back(newsection);

			}
			else
			{		
				int sectionno = sections.size() - 1;

				if (lstrcmpi(left(readinfo,4), "Name")==0)
					strcpy(sections[sectionno].name,right(readinfo,strlen(readinfo)-5));

				if (lstrcmpi(left(readinfo,20), "DisableTextureCRC")==0)
					sections[sectionno].bDisableTextureCRC=true;

				if (lstrcmpi(left(readinfo,20), "DisableCulling")==0)
					sections[sectionno].bDisableCulling=true;

				if (lstrcmpi(left(readinfo,16), "PrimaryDepthHack")==0)
					sections[sectionno].bPrimaryDepthHack=true;

				if (lstrcmpi(left(readinfo,16), "Texture1Hack")==0)
					sections[sectionno].bTexture1Hack=true;

				if (lstrcmpi(left(readinfo,16), "FastLoadTile")==0)
					sections[sectionno].bFastLoadTile=true;

				if (lstrcmpi(left(readinfo,14), "IncTexRectEdge")==0)
					sections[sectionno].bIncTexRectEdge=true;

				if (lstrcmpi(left(readinfo,16), "TexRectScaleHack")==0)
					sections[sectionno].bTextureScaleHack=true;

				if (lstrcmpi(left(readinfo,7), "VIWidth")==0)
					sections[sectionno].VIWidth = strtol(right(readinfo,3),NULL,10);

				if (lstrcmpi(left(readinfo,8), "VIHeight")==0)
					sections[sectionno].VIHeight = strtol(right(readinfo,3),NULL,10);

				if (lstrcmpi(left(readinfo,18), "UseCIWidthAndRatio")==0)
					sections[sectionno].UseCIWidthAndRatio = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,8), "FullTMEM")==0)
					sections[sectionno].dwFullTMEM = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,24), "AlternativeTxtSizeMethod")==0)
					sections[sectionno].bTxtSizeMethod2 = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,12), "EnableTxtLOD")==0)
					sections[sectionno].bEnableTxtLOD = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,12), "DisableObjBG")==0)
					sections[sectionno].bDisableObjBG = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,16), "ForceScreenClear")==0)
					sections[sectionno].bForceScreenClear = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,22), "AccurateTextureMapping")==0)
					sections[sectionno].dwAccurateTextureMapping = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,14), "FastTextureCRC")==0)
					sections[sectionno].dwFastTextureCRC = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,12), "EmulateClear")==0)
					sections[sectionno].bEmulateClear = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,18), "NormalAlphaBlender")==0)
					sections[sectionno].dwNormalBlender = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,19), "DisableAlphaBlender")==0)
					sections[sectionno].bDisableBlender = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,19), "NormalColorCombiner")==0)
					sections[sectionno].dwNormalCombiner = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,16), "ForceDepthBuffer")==0)
					sections[sectionno].bForceDepthBuffer = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,20), "FrameBufferEmulation")==0)
					sections[sectionno].dwFrameBufferOption = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,15), "RenderToTexture")==0)
					sections[sectionno].dwRenderToTextureOption = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,19), "ScreenUpdateSetting")==0)
					sections[sectionno].dwScreenUpdateSetting = strtol(right(readinfo,1),NULL,10);
			}
		}
	}
	inifile.close();

	return TRUE;
}

//read a line from the ini file
std::ifstream & getline(std::ifstream & is, char *str)
{
	char buf[100];

	is.getline(buf,100);
	strcpy( str,buf);
	return is;
}

void IniFile::WriteIniFile()
{
	// noting to write atm - needs work
	return;

	TCHAR szFileNameOut[MAX_PATH+1];
	TCHAR szFileNameDelete[MAX_PATH+1];
	TCHAR filename[MAX_PATH+1];
	DWORD i;
	FILE * fhIn;
	FILE * fhOut;
	TCHAR szBuf[1024+1];
	char trim[]="{}\n\r"; //remove first and last character

	GetPluginDir(szFileNameOut);
	GetPluginDir(szFileNameDelete);
	wsprintf(filename, "%s.tmp", m_szFileName);
	strcat(szFileNameOut, filename);
	wsprintf(filename, "%s.del", m_szFileName);
	strcat(szFileNameDelete, filename);

	GetPluginDir(filename);
	strcat(filename,m_szFileName);
	fhIn = fopen(filename, "r");
	if (fhIn == NULL)
	{
		// Create a new file
		fhOut = fopen(filename,"w");
		fclose(fhOut);
		return;
	}

	fhOut = fopen(szFileNameOut, "w");
	if (fhOut == NULL)
	{
		fclose(fhIn);
		return;
	}

	// Mark all sections and needing to be written
	for (i = 0; i < sections.size(); i++)
	{
		sections[i].bOutput = false;
	}


	while (fgets(szBuf, 1024, fhIn))
	{
		if (szBuf[0] == '{')
		{
			BOOL bFound = FALSE;
			// Start of section
		 

			for (i = 0; i < sections.size(); i++)
			{
				if (sections[i].bOutput)
					continue;

				if (lstrcmpi(szBuf, sections[i].crccheck) == 0)
				{
					// Output this CRC
					OutputSectionDetails(i, fhOut);
					sections[i].bOutput = true;
					bFound = TRUE;
					break;
				}
			}
			if (!bFound)
			{
				// Do what? This should never happen, unless the user
				// replaces the inifile while game is running!
			}
		}
		else if (szBuf[0] == '/')
		{
			// Comment
			fputs(szBuf, fhOut);
			continue;
		}

	}

	// Input buffer done-  process any new entries!
	for (i = 0; i < sections.size(); i++)
	{
		// Skip any that have not been done.
		if (sections[i].bOutput)
			continue;
		// Output this CRC
		// Removed at request of Genueix :)
		//fprintf(fhOut, "// Automatically generated entry - may need editing\n");
		OutputSectionDetails(i, fhOut);
		sections[i].bOutput = true;
	}

	fclose(fhOut);
	fclose(fhIn);

	// Create the new file
	/*
	MoveFile(filename, szFileNameDelete);
	MoveFile(szFileNameOut, filename);
	DeleteFile(szFileNameDelete);
	*/
	DeleteFile(filename);
	MoveFile(szFileNameOut, filename);

	bChanged = false;
}


void IniFile::OutputSectionDetails(DWORD i, FILE * fh)
{
	fprintf(fh, "{%s}\n", sections[i].crccheck);

	fprintf(fh, "Name=%s\n", sections[i].name);
	//fprintf(fh, "UCode=%d\n", sections[i].ucode);

	// Tri-state variables
	if (sections[i].dwAccurateTextureMapping != 0)
		fprintf(fh, "AccurateTextureMapping=%d\n", sections[i].dwAccurateTextureMapping);

	if (sections[i].dwFastTextureCRC != 0)
		fprintf(fh, "FastTextureCRC=%d\n", sections[i].dwFastTextureCRC);

	if (sections[i].dwNormalBlender != 0)
		fprintf(fh, "NormalAlphaBlender=%d\n", sections[i].dwNormalBlender);

	if (sections[i].dwNormalCombiner != 0)
		fprintf(fh, "NormalColorCombiner=%d\n", sections[i].dwNormalCombiner);


	// Normal bi-state variables
	if (sections[i].bDisableTextureCRC)
		fprintf(fh, "DisableTextureCRC\n");

	if (sections[i].bDisableCulling)
		fprintf(fh, "DisableCulling\n");

	if (sections[i].bPrimaryDepthHack)
		fprintf(fh, "PrimaryDepthHack\n");

	if (sections[i].bTexture1Hack)
		fprintf(fh, "Texture1Hack\n");

	if (sections[i].bFastLoadTile)
		fprintf(fh, "FastLoadTile\n");

	if (sections[i].bIncTexRectEdge)
		fprintf(fh, "IncTexRectEdge\n");

	if (sections[i].bTextureScaleHack)
		fprintf(fh, "TexRectScaleHack\n");

	if (sections[i].VIWidth > 0)
		fprintf(fh, "VIWidth=%d\n", sections[i].VIWidth);

	if (sections[i].VIHeight > 0)
		fprintf(fh, "VIHeight=%d\n", sections[i].VIHeight);

	if (sections[i].UseCIWidthAndRatio > 0)
		fprintf(fh, "UseCIWidthAndRatio=%d\n", sections[i].UseCIWidthAndRatio);

	if (sections[i].dwFullTMEM > 0)
		fprintf(fh, "FullTMEM=%d\n", sections[i].dwFullTMEM);

	if (sections[i].bTxtSizeMethod2 != FALSE )
		fprintf(fh, "AlternativeTxtSizeMethod=%d\n", sections[i].bTxtSizeMethod2);

	if (sections[i].bEnableTxtLOD != FALSE )
		fprintf(fh, "EnableTxtLOD=%d\n", sections[i].bEnableTxtLOD);

	if (sections[i].bDisableObjBG != 0 )
		fprintf(fh, "DisableObjBG=%d\n", sections[i].bDisableObjBG);

	if (sections[i].bForceScreenClear != 0)
		fprintf(fh, "ForceScreenClear=%d\n", sections[i].bForceScreenClear);

	if (sections[i].bEmulateClear != 0)
		fprintf(fh, "EmulateClear=%d\n", sections[i].bEmulateClear);

	if (sections[i].bDisableBlender != 0)
		fprintf(fh, "DisableAlphaBlender=%d\n", sections[i].bDisableBlender);

	if (sections[i].bForceDepthBuffer != 0)
		fprintf(fh, "ForceDepthBuffer=%d\n", sections[i].bForceDepthBuffer);

	if (sections[i].dwFrameBufferOption != 0)
		fprintf(fh, "FrameBufferEmulation=%d\n", sections[i].dwFrameBufferOption);

	if (sections[i].dwRenderToTextureOption != 0)
		fprintf(fh, "RenderToTexture=%d\n", sections[i].dwRenderToTextureOption);

	if (sections[i].dwScreenUpdateSetting != 0)
		fprintf(fh, "ScreenUpdateSetting=%d\n", sections[i].dwScreenUpdateSetting);

	fprintf(fh, "\n");			// Spacer
}


// Find the entry corresponding to the specified rom. 
// If the rom is not found, a new entry is created
// The resulting value is returned
void __cdecl DebuggerAppendMsg (const char * Message, ...);
int IniFile::FindEntry(DWORD dwCRC1, DWORD dwCRC2, BYTE nCountryID, LPCTSTR szName)
{
	DWORD i;
	CHAR szCRC[50+1];

	// Generate the CRC-ID for this rom:
	wsprintf(szCRC, "%08x%08x-%02x", dwCRC1, dwCRC2, nCountryID);

	for (i = 0; i < sections.size(); i++)
	{
		if (lstrcmpi(szCRC, sections[i].crccheck) == 0)
		{
			return i;
		}
	}

	// Add new entry!!!
	section newsection;

	strcpy(newsection.crccheck, szCRC);

	lstrcpyn(newsection.name, szName, 50);
	newsection.bDisableTextureCRC = FALSE;
	newsection.bDisableCulling = FALSE;
	newsection.bIncTexRectEdge = FALSE;
	newsection.bTextureScaleHack = FALSE;
	newsection.bFastLoadTile = FALSE;
	newsection.bPrimaryDepthHack = FALSE;
	newsection.bTexture1Hack = FALSE;
	newsection.bDisableObjBG = FALSE;
	newsection.VIWidth = -1;
	newsection.VIHeight = -1;
	newsection.UseCIWidthAndRatio = NOT_USE_CI_WIDTH_AND_RATIO;
	newsection.dwFullTMEM = 0;
	newsection.bTxtSizeMethod2 = FALSE;
	newsection.bEnableTxtLOD = FALSE;

	newsection.bEmulateClear = FALSE;
	newsection.bForceScreenClear = FALSE;
	newsection.bDisableBlender = FALSE;
	newsection.bForceDepthBuffer = FALSE;
	newsection.dwFastTextureCRC = 0;
	newsection.dwAccurateTextureMapping = 0;
	newsection.dwNormalBlender = 0;
	newsection.dwNormalCombiner = 0;
	newsection.dwFrameBufferOption = 0;
	newsection.dwRenderToTextureOption = 0;
	newsection.dwScreenUpdateSetting = 0;

	sections.push_back(newsection);

	bChanged = true;				// Flag to indicate we should be updated
	return sections.size()-1;			// -1 takes into account increment
}


RomInfo g_curRomInfo;

// Swap bytes from 80 37 12 40
// to              40 12 37 80
void ROM_ByteSwap_3210(void *v, DWORD dwLen)
{
	__asm
	{
		mov		esi, v
			mov		edi, v
			mov		ecx, dwLen

			add		edi, ecx

top:
		mov		al, byte ptr [esi + 0]
		mov		bl, byte ptr [esi + 1]
		mov		cl, byte ptr [esi + 2]
		mov		dl, byte ptr [esi + 3]

		mov		byte ptr [esi + 0], dl		//3
			mov		byte ptr [esi + 1], cl		//2
			mov		byte ptr [esi + 2], bl		//1
			mov		byte ptr [esi + 3], al		//0

			add		esi, 4
			cmp		esi, edi
			jne		top

	}
}


void ROM_GetRomNameFromHeader(TCHAR * szName, ROMHeader * pHdr)
{
	TCHAR * p;

	memcpy(szName, pHdr->szName, 20);
	szName[20] = '\0';

	p = szName + (lstrlen(szName) -1);		// -1 to skip null
	while (p >= szName && *p == ' ')
	{
		*p = 0;
		p--;
	}
}


DWORD CountryCodeToTVSystem(DWORD countryCode)
{
	DWORD system;
	switch(countryCode)
	{
		/* Demo */
	case 0:
		system = TV_SYSTEM_NTSC;
		break;

	case '7':
		system = TV_SYSTEM_NTSC;
		break;

	case 0x41:
		system = TV_SYSTEM_NTSC;
		break;

		/* Germany */
	case 0x44:
		system = TV_SYSTEM_PAL;
		break;

		/* USA */
	case 0x45:
		system = TV_SYSTEM_NTSC;
		break;

		/* France */
	case 0x46:
		system = TV_SYSTEM_PAL;
		break;

		/* Italy */
	case 'I':
		system = TV_SYSTEM_PAL;
		break;

		/* Japan */
	case 0x4A:
		system = TV_SYSTEM_NTSC;
		break;

		/* Europe - PAL */
	case 0x50:
		system = TV_SYSTEM_PAL;
		break;

	case 'S':	/* Spain */
		system = TV_SYSTEM_PAL;
		break;

		/* Australia */
	case 0x55:
		system = TV_SYSTEM_PAL;
		break;

	case 0x58:
		system = TV_SYSTEM_PAL;
		break;

		/* Australia */
	case 0x59:
		system = TV_SYSTEM_PAL;
		break;

	case 0x20:
	case 0x21:
	case 0x38:
	case 0x70:
		system = TV_SYSTEM_PAL;
		break;

		/* ??? */
	default:
		system = TV_SYSTEM_PAL;
		break;
	}

	return system;
}

 
 