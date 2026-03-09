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

#define MAIN_KEY		"Software\\RICEVIDEO\\Version612"
#define INI_FILE		"RiceVideo6.1.2.ini"
char *project_name =	"Rice's Video Plugin";


char *frameBufferSettings[] =
{
	"None (default)",
	"Hide Framebuffer Effects",
	"Basic Framebuffer",
	"Basic & Write Back",
	"Write Back & Reload",
	"Write Back Every Frame",
	"With Emulator",
	"Basic Framebuffer & With Emulator",
	"With Emulator Read Only",
	"With Emulator Write Only",
};
/*
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
};*/
//const int numberOfResolutions = sizeof(resolutions)/sizeof(int)/2;

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
	"At VI origin change",
	"At CI change",
	"At the 1st CI change",
	"At the 1st drawing",
	"Before clear the screen",
	"At VI origin update after screen is drawn (default)",
};

WindowSettingStruct windowSetting;
GlobalOptions options;
RomOptions defaultRomOptions;
RomOptions currentRomOptions;
FrameBufferOptions frameBufferOptions;
std::vector<IniSection> IniSections;
bool	bIniIsChanged = false;
char	szIniFileName[300];

//=======================================================

const RenderEngineSetting RenderEngineSettings[] =
{
	"DirectX",	DIRECTX_DEVICE,
	//"OpenGL", OGL_DEVICE,
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
	//"2x (Double the texture size)",	TEXTURE_2X_ENHANCEMENT,
	//"2xSaI", TEXTURE_2XSAI_ENHANCEMENT,
	//"hq2x", TEXTURE_HQ2X_ENHANCEMENT,
	//"lq2x", TEXTURE_LQ2X_ENHANCEMENT,
	//"hq4x", TEXTURE_HQ4X_ENHANCEMENT,
	//"Sharpen", TEXTURE_SHARPEN_ENHANCEMENT,
	//"Sharpen More", TEXTURE_SHARPEN_MORE_ENHANCEMENT,
};

const SettingInfo TextureEnhancementControlSettings[] =
{
	"Normal",	TEXTURE_ENHANCEMENT_NORMAL,
	//"Smooth",	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1,
	//"Less smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2,
	//"2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3,
	//"Less 2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4,
};

const SettingInfo colorQualitySettings[] =
{
	"16-bit",	TEXTURE_FMT_A4R4G4B4,
	//"32-bit (def)",	TEXTURE_FMT_A8R8G8B8,
};

const char*	strDXDeviceDescs[] = { "HAL", "REF" };


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


int numberOfDirectXRenderBufferSettings = sizeof(DirectXRenderBufferSettings)/sizeof(BufferSettingInfo);
int numberOfDirectXCombinerSettings = sizeof(DirectXCombinerSettings)/sizeof(BufferSettingInfo);
int numberOfDirectXDepthBufferSettings = sizeof(DirectXDepthBufferSetting)/sizeof(BufferSettingInfo);

const int numberOfRenderEngineSettings = sizeof(RenderEngineSettings)/sizeof(RenderEngineSetting);


void WriteConfiguration(void);
void GenerateCurrentRomOptions();

HWND	g_hwndTT=NULL;
HWND	g_hwndDlg=NULL;
HHOOK	g_hhk = NULL;

int DialogToStartRomIsRunning = PSH_ROM_SETTINGS;
int DialogToStartRomIsNotRunning = PSH_OPTIONS;
HWND hPropSheetHwnd = NULL;

extern "C" BOOL __stdcall EnumChildProc(HWND hwndCtrl, LPARAM lParam);
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);


//////////////////////////////////////////////////////////////////////////
void GenerateFrameBufferOptions(void)
{

	frameBufferOptions.bUpdateCIInfo			= false;

	frameBufferOptions.bCheckBackBufs			= false;
	frameBufferOptions.bWriteBackBufToRDRAM		= false;
	frameBufferOptions.bLoadBackBufFromRDRAM	= false;

	frameBufferOptions.bIgnore					= true;

	frameBufferOptions.bSupportRenderTextures			= false;
	frameBufferOptions.bCheckRenderTextures			= false;
	frameBufferOptions.bRenderTextureWriteBack			= false;
	frameBufferOptions.bLoadRDRAMIntoRenderTexture		= false;

	frameBufferOptions.bProcessCPUWrite			= false;
	frameBufferOptions.bProcessCPURead			= false;
	frameBufferOptions.bAtEachFrameUpdate		= false;
	frameBufferOptions.bIgnoreRenderTextureIfHeightUnknown		= false;

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
	case FRM_BUF_BASIC_AND_WITH_EMULATOR:
		// Banjo Kazooie
		frameBufferOptions.bCheckBackBufs			= true;
	case FRM_BUF_WITH_EMULATOR:
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bProcessCPUWrite			= true;
		frameBufferOptions.bProcessCPURead			= true;
		break;
	case FRM_BUF_WITH_EMULATOR_READ_ONLY:
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bProcessCPURead			= true;
		break;
	case FRM_BUF_WITH_EMULATOR_WRITE_ONLY:
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bProcessCPUWrite			= true;
		break;
	}

	switch( currentRomOptions.N64RenderToTextureEmuType )
	{
	case TXT_BUF_NONE:
		frameBufferOptions.bSupportRenderTextures			= false;
		break;
	case TXT_BUF_WRITE_BACK_AND_RELOAD:
		frameBufferOptions.bLoadRDRAMIntoRenderTexture		= true;
	case TXT_BUF_WRITE_BACK:
		frameBufferOptions.bRenderTextureWriteBack			= true;
	case TXT_BUF_NORMAL:
		frameBufferOptions.bCheckRenderTextures			= true;
		frameBufferOptions.bIgnore					= false;
	case TXT_BUF_IGNORE:
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bSupportRenderTextures			= true;
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
return;
}

uint32 ReadRegistryDwordVal(char *MainKey, char *Field)
{
	return(0);
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

		if(bEnableHDTV){
			windowSetting.uWindowDisplayWidth = windowSetting.uFullScreenDisplayWidth = 1280;
			windowSetting.uWindowDisplayHeight = windowSetting.uFullScreenDisplayHeight = 720;
		}else{
			windowSetting.uWindowDisplayWidth = windowSetting.uFullScreenDisplayWidth = 640;
			windowSetting.uWindowDisplayHeight = windowSetting.uFullScreenDisplayHeight = 480;
		}

		windowSetting.uDisplayWidth = windowSetting.uWindowDisplayWidth;
		windowSetting.uDisplayHeight = windowSetting.uWindowDisplayHeight;

		CDeviceBuilder::SelectDeviceType( DIRECTX_DEVICE );
	
		status.isMMXSupported = 1;
		status.isSSESupported = 1;
		status.isVertexShaderSupported = true;

		options.bEnableHacks = TRUE;
		options.bEnableSSE = TRUE;

		options.bEnableFog = TRUE;
		options.bWinFrameMode = FALSE;
		options.bFullTMEM = TRUE;
		options.bUseFullTMEM = TRUE;

		options.bForceSoftwareTnL = TRUE; //FALSE;
		options.bForceSoftwareClipper = TRUE;

		options.bEnableVertexShader = FALSE;
		options.RenderBufferSetting=0;
		options.forceTextureFilter = TextureMode;
		options.textureQuality = TXT_QUALITY_16BIT; //TXT_QUALITY_DEFAULT
		options.bTexRectOnly = FALSE;
		options.bSmallTextureOnly = FALSE;
		options.bLoadHiResTextures = FALSE;
		options.bDumpTexturesToFiles = FALSE;
		options.DirectXDepthBufferSetting = 0;
		options.colorQuality = TEXTURE_FMT_A8R8G8B8;
		options.textureEnhancement = 0;
		options.textureEnhancementControl = 1; //0
		options.bSkipFrame = FrameSkip;
		options.DirectXAntiAliasingValue = 0;
		options.DirectXCombiner = DX_PIXEL_SHADER;
		options.DirectXDevice = DIRECTX_DEVICE;	// HAL device
		options.DirectXAnisotropyValue = 0;
		options.DirectXMaxFSAA = 4;
		options.FPSColor = 0xFFFFFFFF;
		options.DirectXMaxAnisotropy = 2;

		defaultRomOptions.screenUpdateSetting = SCREEN_UPDATE_AT_VI_CHANGE;
		//defaultRomOptions.screenUpdateSetting = SCREEN_UPDATE_AT_VI_UPDATE_AND_DRAWN;

		defaultRomOptions.N64FrameBufferEmuType = FRM_BUF_NONE;
		defaultRomOptions.N64FrameBufferWriteBackControl = FRM_BUF_WRITEBACK_NORMAL;
		defaultRomOptions.N64RenderToTextureEmuType = TXT_BUF_NORMAL;//TXT_BUF_NONE;

		defaultRomOptions.bNormalBlender = FALSE;
		defaultRomOptions.bFastTexCRC=FALSE; //TRUE;
		defaultRomOptions.bNormalCombiner = FALSE;
		defaultRomOptions.bAccurateTextureMapping = TRUE;
		defaultRomOptions.bInN64Resolution = TRUE; //FALSE;
		defaultRomOptions.bSaveVRAM = TRUE; //FALSE;
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

	status.isVertexShaderEnabled = status.isVertexShaderSupported && options.bEnableVertexShader;
	status.bUseHW_T_L = false;
}
	
//---------------------------------------------------------------------------------------
BOOL InitConfiguration(void)
{
	//Initialize this DLL

	IniSections.clear();
	bIniIsChanged = false;
	strcpy(szIniFileName, INI_FILE);

	if (!ReadIniFile())
	{
		ErrorMsg("Unable to read ini file from disk");
		ReadConfiguration(); //defaults
		WriteIniFile();
		return FALSE;
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
	else if ((strstr(g_curRomInfo.szGameName, "ZELDA") != 0) && (strstr(g_curRomInfo.szGameName, "MASK") != 0))
	{
		options.enableHackForGames = HACK_FOR_ZELDA_MM;
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
	else if ((strstr(g_curRomInfo.szGameName, "Squadron") != 0))
	{
		options.enableHackForGames = HACK_FOR_ROGUE_SQUADRON;
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
	else if ((strnicmp(g_curRomInfo.szGameName, "MarioGolf64",11) == 0))
	{
		options.enableHackForGames = HACK_FOR_MARIO_GOLF;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "MLB FEATURING",13) == 0))
	{
		options.enableHackForGames = HACK_FOR_MLB;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "POLARISSNOCROSS",15) == 0))
	{
		options.enableHackForGames = HACK_FOR_POLARISSNOCROSS;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "TOP GEAR RALLY",14) == 0))
	{
		options.enableHackForGames = HACK_FOR_TOPGEARRALLY;
	}
	else if ((strnicmp(g_curRomInfo.szGameName, "DUKE NUKEM",10) == 0))
	{
		options.enableHackForGames = HACK_FOR_DUKE_NUKEM;
	}
	else if ((stricmp(g_curRomInfo.szGameName, "MARIOKART64") == 0))
	{
		options.enableHackForGames = HACK_FOR_MARIO_KART;
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

	if( options.enableHackForGames == HACK_FOR_MARIO_GOLF || options.enableHackForGames == HACK_FOR_MARIO_TENNIS )
	{
		frameBufferOptions.bIgnoreRenderTextureIfHeightUnknown = true;
	}
}

void Ini_GetRomOptions(LPGAMESETTING pGameSetting)
{
	LONG i;

	i = FindIniEntry(pGameSetting->romheader.dwCRC1,
							  pGameSetting->romheader.dwCRC2,
							  pGameSetting->romheader.nCountryID,
							  pGameSetting->szGameName);

	//lstrcpyn(pGameSetting->szGameName, IniSections[i].name, 50);

	pGameSetting->bDisableTextureCRC	= IniSections[i].bDisableTextureCRC;
	pGameSetting->bDisableCulling		= IniSections[i].bDisableCulling;
	pGameSetting->bIncTexRectEdge		= IniSections[i].bIncTexRectEdge;
	pGameSetting->bZHack				= IniSections[i].bZHack;
	pGameSetting->bTextureScaleHack		= IniSections[i].bTextureScaleHack;
	pGameSetting->bPrimaryDepthHack		= IniSections[i].bPrimaryDepthHack;
	pGameSetting->bTexture1Hack			= IniSections[i].bTexture1Hack;
	pGameSetting->bFastLoadTile			= IniSections[i].bFastLoadTile;
	pGameSetting->bUseSmallerTexture	= IniSections[i].bUseSmallerTexture;

	pGameSetting->VIWidth				= IniSections[i].VIWidth;
	pGameSetting->VIHeight				= IniSections[i].VIHeight;
	pGameSetting->UseCIWidthAndRatio	= IniSections[i].UseCIWidthAndRatio;
	pGameSetting->dwFullTMEM			= IniSections[i].dwFullTMEM;
	pGameSetting->bTxtSizeMethod2		= IniSections[i].bTxtSizeMethod2;
	pGameSetting->bEnableTxtLOD			= IniSections[i].bEnableTxtLOD;

	pGameSetting->dwFastTextureCRC		= IniSections[i].dwFastTextureCRC;
	pGameSetting->bEmulateClear			= IniSections[i].bEmulateClear;
	pGameSetting->bForceScreenClear		= IniSections[i].bForceScreenClear;
	pGameSetting->dwAccurateTextureMapping	= IniSections[i].dwAccurateTextureMapping;
	pGameSetting->dwNormalBlender		= IniSections[i].dwNormalBlender;
	pGameSetting->bDisableBlender		= IniSections[i].bDisableBlender;
	pGameSetting->dwNormalCombiner		= IniSections[i].dwNormalCombiner;
	pGameSetting->bForceDepthBuffer		= IniSections[i].bForceDepthBuffer;
	pGameSetting->bDisableObjBG			= IniSections[i].bDisableObjBG;
	pGameSetting->dwFrameBufferOption	= IniSections[i].dwFrameBufferOption;
	pGameSetting->dwRenderToTextureOption	= IniSections[i].dwRenderToTextureOption;
	pGameSetting->dwScreenUpdateSetting	= IniSections[i].dwScreenUpdateSetting;
}

void Ini_StoreRomOptions(LPGAMESETTING pGameSetting)
{
	LONG i;

	i = FindIniEntry(pGameSetting->romheader.dwCRC1,
		pGameSetting->romheader.dwCRC2,
		pGameSetting->romheader.nCountryID,
		pGameSetting->szGameName);

	if( IniSections[i].bDisableTextureCRC	!=pGameSetting->bDisableTextureCRC )
	{
		IniSections[i].bDisableTextureCRC	=pGameSetting->bDisableTextureCRC	 ;
		bIniIsChanged=true;
	}

	if( IniSections[i].bDisableCulling	!=pGameSetting->bDisableCulling )
	{
		IniSections[i].bDisableCulling	=pGameSetting->bDisableCulling	 ;
		bIniIsChanged=true;
	}

	if( IniSections[i].dwFastTextureCRC !=pGameSetting->dwFastTextureCRC )
	{
		IniSections[i].dwFastTextureCRC	=pGameSetting->dwFastTextureCRC		 ;
		bIniIsChanged=true;
	}

	if( IniSections[i].bEmulateClear !=pGameSetting->bEmulateClear )
	{
		IniSections[i].bEmulateClear	=pGameSetting->bEmulateClear		 ;
		bIniIsChanged=true;
	}

	if( IniSections[i].dwNormalBlender		!=pGameSetting->dwNormalBlender )
	{
		IniSections[i].dwNormalBlender		=pGameSetting->dwNormalBlender		 ;
		bIniIsChanged=true;
	}

	if( IniSections[i].bDisableBlender	!=pGameSetting->bDisableBlender )
	{
		IniSections[i].bDisableBlender	=pGameSetting->bDisableBlender		 ;
		bIniIsChanged=true;
	}

	if( IniSections[i].bForceScreenClear	!=pGameSetting->bForceScreenClear )
	{
		IniSections[i].bForceScreenClear	=pGameSetting->bForceScreenClear		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].dwAccurateTextureMapping	!=pGameSetting->dwAccurateTextureMapping )
	{
		IniSections[i].dwAccurateTextureMapping	=pGameSetting->dwAccurateTextureMapping		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].dwNormalCombiner	!=pGameSetting->dwNormalCombiner )
	{
		IniSections[i].dwNormalCombiner	=pGameSetting->dwNormalCombiner		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].bForceDepthBuffer	!=pGameSetting->bForceDepthBuffer )
	{
		IniSections[i].bForceDepthBuffer	=pGameSetting->bForceDepthBuffer		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].bDisableObjBG	!=pGameSetting->bDisableObjBG )
	{
		IniSections[i].bDisableObjBG	=pGameSetting->bDisableObjBG		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].dwFrameBufferOption	!=pGameSetting->dwFrameBufferOption )
	{
		IniSections[i].dwFrameBufferOption	=pGameSetting->dwFrameBufferOption		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].dwRenderToTextureOption	!=pGameSetting->dwRenderToTextureOption )
	{
		IniSections[i].dwRenderToTextureOption	=pGameSetting->dwRenderToTextureOption		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].dwScreenUpdateSetting	!=pGameSetting->dwScreenUpdateSetting )
	{
		IniSections[i].dwScreenUpdateSetting	=pGameSetting->dwScreenUpdateSetting		 ;
		bIniIsChanged=true;
	}
	if( IniSections[i].bIncTexRectEdge	!= pGameSetting->bIncTexRectEdge )
	{
		IniSections[i].bIncTexRectEdge		=pGameSetting->bIncTexRectEdge;
		bIniIsChanged=true;
	}
	if( IniSections[i].bZHack	!= pGameSetting->bZHack )
	{
		IniSections[i].bZHack		=pGameSetting->bZHack;
		bIniIsChanged=true;
	}
	if( IniSections[i].bTextureScaleHack	!= pGameSetting->bTextureScaleHack )
	{
		IniSections[i].bTextureScaleHack		=pGameSetting->bTextureScaleHack;
		bIniIsChanged=true;
	}
	if( IniSections[i].bPrimaryDepthHack	!= pGameSetting->bPrimaryDepthHack )
	{
		IniSections[i].bPrimaryDepthHack		=pGameSetting->bPrimaryDepthHack;
		bIniIsChanged=true;
	}
	if( IniSections[i].bTexture1Hack	!= pGameSetting->bTexture1Hack )
	{
		IniSections[i].bTexture1Hack		=pGameSetting->bTexture1Hack;
		bIniIsChanged=true;
	}
	if( IniSections[i].bFastLoadTile	!= pGameSetting->bFastLoadTile )
	{
		IniSections[i].bFastLoadTile	=pGameSetting->bFastLoadTile;
		bIniIsChanged=true;
	}
	if( IniSections[i].bUseSmallerTexture	!= pGameSetting->bUseSmallerTexture )
	{
		IniSections[i].bUseSmallerTexture	=pGameSetting->bUseSmallerTexture;
		bIniIsChanged=true;
	}
	if( IniSections[i].VIWidth	!= pGameSetting->VIWidth )
	{
		IniSections[i].VIWidth	=pGameSetting->VIWidth;
		bIniIsChanged=true;
	}
	if( IniSections[i].VIHeight	!= pGameSetting->VIHeight )
	{
		IniSections[i].VIHeight	=pGameSetting->VIHeight;
		bIniIsChanged=true;
	}
	if( IniSections[i].UseCIWidthAndRatio	!= pGameSetting->UseCIWidthAndRatio )
	{
		IniSections[i].UseCIWidthAndRatio	=pGameSetting->UseCIWidthAndRatio;
		bIniIsChanged=true;
	}
	if( IniSections[i].dwFullTMEM	!= pGameSetting->dwFullTMEM )
	{
		IniSections[i].dwFullTMEM	=pGameSetting->dwFullTMEM;
		bIniIsChanged=true;
	}
	if( IniSections[i].bTxtSizeMethod2	!= pGameSetting->bTxtSizeMethod2 )
	{
		IniSections[i].bTxtSizeMethod2	=pGameSetting->bTxtSizeMethod2;
		bIniIsChanged=true;
	}
	if( IniSections[i].bEnableTxtLOD	!= pGameSetting->bEnableTxtLOD )
	{
		IniSections[i].bEnableTxtLOD	=pGameSetting->bEnableTxtLOD;
		bIniIsChanged=true;
	}

	if( bIniIsChanged )
	{
		WriteIniFile();
		TRACE0("Rom option is changed and saved");
	}
}



// ** TOOLTIP CODE FROM MSDN LIBRARY SAMPLE WITH SEVERAL MODIFICATIONS **

// DoCreateDialogTooltip - creates a tooltip control for a dialog box, 
//     enumerates the child control windows, and installs a hook 
//     procedure to monitor the message stream for mouse messages posted 
//     to the control windows. 
// Returns TRUE if successful, or FALSE otherwise. 
// 
// Global variables 
// g_hinst - handle to the application instance. 
// g_hwndTT - handle to the tooltip control. 
// g_hwndDlg - handle to the dialog box. 
// g_hhk - handle to the hook procedure. 

BOOL CreateDialogTooltip(void) 
{
    return TRUE;
} 

BOOL EnumChildWndTooltip(void)
{
    return TRUE;
}

// EmumChildProc - registers control windows with a tooltip control by
//     using the TTM_ADDTOOL message to pass the address of a 
//     TOOLINFO structure. 
// Returns TRUE if successful, or FALSE otherwise. 
// hwndCtrl - handle of a control window. 
// lParam - application-defined value (not used). 
extern "C"  BOOL __stdcall EnumChildProc(HWND hwndCtrl, LPARAM lParam) 
{ 
    return TRUE; 
} 


std::ifstream& getline( std::ifstream &is, char *str );


///////////////////////////////////////////////
//// Constructors / Deconstructors
///////////////////////////////////////////////

char * left(char * src, int nchars)
{
	static char dst[300];			// BUGFIX (STRMNNRM)
	strncpy(dst,src,nchars);
	dst[nchars]=0;
	return dst;
}

char * right(char *src, int nchars)
{/*
	static char dst[300];			// BUGFIX (STRMNNRM)
	strncpy(dst, src + strlen(src) - nchars, nchars);
	dst[nchars]=0;
	return dst;*/
	static char dst[300];

	int srclen = strlen(src);
	if (nchars >= srclen)
	{
		strcpy(dst, src);
	}
	else
	{
		strncpy(dst, src + srclen - nchars, nchars);
		dst[nchars]=0;
	}
	return dst;
}

char * tidy(char * s)
{
	char * p = s + lstrlen(s);

	p--;
	while (p >= s && (*p == ' ' || *p == 0xa || *p == '\n') )
	{
		*p = 0;
		p--;
	}
	return s;

}

extern void GetPluginDir( char * Directory );

BOOL ReadIniFile()
{
	std::ifstream inifile;
	char readinfo[100];
	char trim[]="{}"; //remove first and last character

	char filename[256];
	GetPluginDir(filename);
	strcat(filename,szIniFileName);
	//strcpy(filename,szIniFileName);
	
	// try D if it's not on T
	if (!PathFileExists(filename)) {
		sprintf(filename, "D:\\%s", szIniFileName);
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

				//StrTrim(readinfo,trim);		// Fix me
				readinfo[strlen(readinfo)-1]='\0';
				strcpy(newsection.crccheck, readinfo+1);

				newsection.bDisableTextureCRC = FALSE;
				newsection.bDisableCulling = FALSE;
				newsection.bIncTexRectEdge = FALSE;
				newsection.bZHack = FALSE;
				newsection.bTextureScaleHack = FALSE;
				newsection.bFastLoadTile = FALSE;
				newsection.bUseSmallerTexture = FALSE;
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

				IniSections.push_back(newsection);

			}
			else
			{		
				int sectionno = IniSections.size() - 1;

				if (lstrcmpi(left(readinfo,4), "Name")==0)
					strcpy(IniSections[sectionno].name,right(readinfo,strlen(readinfo)-5));

				if (lstrcmpi(left(readinfo,17), "DisableTextureCRC")==0)
					IniSections[sectionno].bDisableTextureCRC=true;

				if (lstrcmpi(left(readinfo,14), "DisableCulling")==0)
					IniSections[sectionno].bDisableCulling=true;

				if (lstrcmpi(left(readinfo,16), "PrimaryDepthHack")==0)
					IniSections[sectionno].bPrimaryDepthHack=true;

				if (lstrcmpi(left(readinfo,12), "Texture1Hack")==0)
					IniSections[sectionno].bTexture1Hack=true;

				if (lstrcmpi(left(readinfo,12), "FastLoadTile")==0)
					IniSections[sectionno].bFastLoadTile=true;

				if (lstrcmpi(left(readinfo,17), "UseSmallerTexture")==0)
					IniSections[sectionno].bUseSmallerTexture=true;

				if (lstrcmpi(left(readinfo,14), "IncTexRectEdge")==0)
					IniSections[sectionno].bIncTexRectEdge=true;

				if (lstrcmpi(left(readinfo,5), "ZHack")==0)
					IniSections[sectionno].bZHack=true;

				if (lstrcmpi(left(readinfo,16), "TexRectScaleHack")==0)
					IniSections[sectionno].bTextureScaleHack=true;

				if (lstrcmpi(left(readinfo,7), "VIWidth")==0)
					IniSections[sectionno].VIWidth = strtol(right(readinfo,3),NULL,10);

				if (lstrcmpi(left(readinfo,8), "VIHeight")==0)
					IniSections[sectionno].VIHeight = strtol(right(readinfo,3),NULL,10);

				if (lstrcmpi(left(readinfo,18), "UseCIWidthAndRatio")==0)
					IniSections[sectionno].UseCIWidthAndRatio = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,8), "FullTMEM")==0)
					IniSections[sectionno].dwFullTMEM = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,24), "AlternativeTxtSizeMethod")==0)
					IniSections[sectionno].bTxtSizeMethod2 = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,12), "EnableTxtLOD")==0)
					IniSections[sectionno].bEnableTxtLOD = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,12), "DisableObjBG")==0)
					IniSections[sectionno].bDisableObjBG = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,16), "ForceScreenClear")==0)
					IniSections[sectionno].bForceScreenClear = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,22), "AccurateTextureMapping")==0)
					IniSections[sectionno].dwAccurateTextureMapping = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,14), "FastTextureCRC")==0)
					IniSections[sectionno].dwFastTextureCRC = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,12), "EmulateClear")==0)
					IniSections[sectionno].bEmulateClear = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,18), "NormalAlphaBlender")==0)
					IniSections[sectionno].dwNormalBlender = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,19), "DisableAlphaBlender")==0)
					IniSections[sectionno].bDisableBlender = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,19), "NormalColorCombiner")==0)
					IniSections[sectionno].dwNormalCombiner = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,16), "ForceDepthBuffer")==0)
					IniSections[sectionno].bForceDepthBuffer = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,20), "FrameBufferEmulation")==0)
					//IniSections[sectionno].dwFrameBufferOption = strtol(right(readinfo,1),NULL,10);
					IniSections[sectionno].dwFrameBufferOption = strtol(readinfo+21,NULL,10);

				if (lstrcmpi(left(readinfo,15), "RenderToTexture")==0)
					IniSections[sectionno].dwRenderToTextureOption = strtol(right(readinfo,1),NULL,10);

				if (lstrcmpi(left(readinfo,19), "ScreenUpdateSetting")==0)
					IniSections[sectionno].dwScreenUpdateSetting = strtol(right(readinfo,1),NULL,10);
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

void WriteIniFile()
{
	// noting to write atm - needs work
	return;
	
	TCHAR szFileNameOut[MAX_PATH+1];
	TCHAR szFileNameDelete[MAX_PATH+1];
	TCHAR filename[MAX_PATH+1];
	uint32 i;
	FILE * fhIn;
	FILE * fhOut;
	TCHAR szBuf[1024+1];
	char trim[]="{}\n\r"; //remove first and last character

	GetPluginDir(szFileNameOut);
	GetPluginDir(szFileNameDelete);
	wsprintf(filename, "%s.tmp", szIniFileName);
	strcat(szFileNameOut, filename);
	wsprintf(filename, "%s.del", szIniFileName);
	strcat(szFileNameDelete, filename);

	GetPluginDir(filename);
	strcat(filename,szIniFileName);
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
	for (i = 0; i < IniSections.size(); i++)
	{
		IniSections[i].bOutput = false;
	}


	while (fgets(szBuf, 1024, fhIn))
	{
		if (szBuf[0] == '{')
		{
			BOOL bFound = FALSE;
			// Start of section
			//StrTrim(szBuf,trim);
			tidy(szBuf);
			szBuf[strlen(szBuf)-1]='\0';

			for (i = 0; i < IniSections.size(); i++)
			{
				if (IniSections[i].bOutput)
					continue;

				if (lstrcmpi(szBuf+1, IniSections[i].crccheck) == 0)
				{
					// Output this CRC
					OutputSectionDetails(i, fhOut);
					IniSections[i].bOutput = true;
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
	for (i = 0; i < IniSections.size(); i++)
	{
		// Skip any that have not been done.
		if (IniSections[i].bOutput)
			continue;
		// Output this CRC
		// Removed at request of Genueix :)
		//fprintf(fhOut, "// Automatically generated entry - may need editing\n");
		OutputSectionDetails(i, fhOut);
		IniSections[i].bOutput = true;
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

	bIniIsChanged = false;
}


void OutputSectionDetails(uint32 i, FILE * fh)
{
	fprintf(fh, "{%s}\n", IniSections[i].crccheck);

	fprintf(fh, "Name=%s\n", IniSections[i].name);
	//fprintf(fh, "UCode=%d\n", IniSections[i].ucode);

	// Tri-state variables
	if (IniSections[i].dwAccurateTextureMapping != 0)
		fprintf(fh, "AccurateTextureMapping=%d\n", IniSections[i].dwAccurateTextureMapping);

	if (IniSections[i].dwFastTextureCRC != 0)
		fprintf(fh, "FastTextureCRC=%d\n", IniSections[i].dwFastTextureCRC);

	if (IniSections[i].dwNormalBlender != 0)
		fprintf(fh, "NormalAlphaBlender=%d\n", IniSections[i].dwNormalBlender);

	if (IniSections[i].dwNormalCombiner != 0)
		fprintf(fh, "NormalColorCombiner=%d\n", IniSections[i].dwNormalCombiner);


	// Normal bi-state variables
	if (IniSections[i].bDisableTextureCRC)
		fprintf(fh, "DisableTextureCRC\n");

	if (IniSections[i].bDisableCulling)
		fprintf(fh, "DisableCulling\n");

	if (IniSections[i].bPrimaryDepthHack)
		fprintf(fh, "PrimaryDepthHack\n");

	if (IniSections[i].bTexture1Hack)
		fprintf(fh, "Texture1Hack\n");

	if (IniSections[i].bFastLoadTile)
		fprintf(fh, "FastLoadTile\n");

	if (IniSections[i].bUseSmallerTexture)
		fprintf(fh, "UseSmallerTexture\n");

	if (IniSections[i].bIncTexRectEdge)
		fprintf(fh, "IncTexRectEdge\n");

	if (IniSections[i].bZHack)
		fprintf(fh, "ZHack\n");

	if (IniSections[i].bTextureScaleHack)
		fprintf(fh, "TexRectScaleHack\n");

	if (IniSections[i].VIWidth > 0)
		fprintf(fh, "VIWidth=%d\n", IniSections[i].VIWidth);

	if (IniSections[i].VIHeight > 0)
		fprintf(fh, "VIHeight=%d\n", IniSections[i].VIHeight);

	if (IniSections[i].UseCIWidthAndRatio > 0)
		fprintf(fh, "UseCIWidthAndRatio=%d\n", IniSections[i].UseCIWidthAndRatio);

	if (IniSections[i].dwFullTMEM > 0)
		fprintf(fh, "FullTMEM=%d\n", IniSections[i].dwFullTMEM);

	if (IniSections[i].bTxtSizeMethod2 != FALSE )
		fprintf(fh, "AlternativeTxtSizeMethod=%d\n", IniSections[i].bTxtSizeMethod2);

	if (IniSections[i].bEnableTxtLOD != FALSE )
		fprintf(fh, "EnableTxtLOD=%d\n", IniSections[i].bEnableTxtLOD);

	if (IniSections[i].bDisableObjBG != 0 )
		fprintf(fh, "DisableObjBG=%d\n", IniSections[i].bDisableObjBG);

	if (IniSections[i].bForceScreenClear != 0)
		fprintf(fh, "ForceScreenClear=%d\n", IniSections[i].bForceScreenClear);

	if (IniSections[i].bEmulateClear != 0)
		fprintf(fh, "EmulateClear=%d\n", IniSections[i].bEmulateClear);

	if (IniSections[i].bDisableBlender != 0)
		fprintf(fh, "DisableAlphaBlender=%d\n", IniSections[i].bDisableBlender);

	if (IniSections[i].bForceDepthBuffer != 0)
		fprintf(fh, "ForceDepthBuffer=%d\n", IniSections[i].bForceDepthBuffer);

	if (IniSections[i].dwFrameBufferOption != 0)
		fprintf(fh, "FrameBufferEmulation=%d\n", IniSections[i].dwFrameBufferOption);

	if (IniSections[i].dwRenderToTextureOption != 0)
		fprintf(fh, "RenderToTexture=%d\n", IniSections[i].dwRenderToTextureOption);

	if (IniSections[i].dwScreenUpdateSetting != 0)
		fprintf(fh, "ScreenUpdateSetting=%d\n", IniSections[i].dwScreenUpdateSetting);

	fprintf(fh, "\n");			// Spacer
}


// Find the entry corresponding to the specified rom. 
// If the rom is not found, a new entry is created
// The resulting value is returned
//void __cdecl DebuggerAppendMsg (const char * Message, ...);
int FindIniEntry(uint32 dwCRC1, uint32 dwCRC2, uint8 nCountryID, LPCTSTR szName)
{
	uint32 i;
	CHAR szCRC[50+1];

	// Generate the CRC-ID for this rom:
	wsprintf(szCRC, "%08x%08x-%02x", dwCRC1, dwCRC2, nCountryID);

	for (i = 0; i < IniSections.size(); i++)
	{
		if (lstrcmpi(szCRC, IniSections[i].crccheck) == 0)
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
	newsection.bZHack = FALSE;
	newsection.bTextureScaleHack = FALSE;
	newsection.bFastLoadTile = FALSE;
	newsection.bUseSmallerTexture = FALSE;
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

	IniSections.push_back(newsection);

	bIniIsChanged = true;				// Flag to indicate we should be updated
	return IniSections.size()-1;			// -1 takes into account increment
}


GameSetting g_curRomInfo;

// Swap bytes from 80 37 12 40
// to              40 12 37 80
void ROM_ByteSwap_3210(void *v, uint32 dwLen)
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


uint32 CountryCodeToTVSystem(uint32 countryCode)
{
	uint32 system;
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

