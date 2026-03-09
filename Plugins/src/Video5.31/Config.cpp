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
#include "../../../config.h"

char *frameBufferSettings[] =
{
	"None (default)",
	"Hide framebuffer effects",
	"Render-to-texture faster",
	"Render-to-texture",
	"Render-to-texture plus",
	"Basic framebuffer",
	"Basic & Write back",
	"Write back & Reload",
	"Complete",
	"With Emulator",
};

const char *screenUpdateSettings[] =
{
	"At VI origin update",
	"At VI origin change (default)",
	"At CI change",
	"At the 1st CI change",
	"At the 1st drawing"
};

WindowSettingStruct windowSetting;
GlobalOptions options;
RomOptions defaultRomOptions;
RomOptions currentRomOptions;
FrameBufferOptions frameBufferOptions;

//=======================================================

extern IniFile *g_pIniFile;
/*
const char *resolutions[] =
{
	"320 x 240",
	"400 x 300",
	"480 x 360",
	"512 x 384",
	"640 x 480",
	"800 x 600",
	"1024 x 768",
	"1152 x 864",
	"1280 x 960",
	"1400 x 1050",
	"1600 x 1200",
	"1920 x 1440",
	"2048 x 1536"
};

const int numberOfResolutions = sizeof(resolutions)/sizeof(char*);

const char *fullScreenFrequencies[] = {
	"Default Hz",
	"50 Hz",
	"55 Hz",
	"60 Hz",
	"65 Hz",
	"70 Hz",
	"72 Hz",
	"75 Hz",
	"80 Hz",
	"85 Hz",
	"90 Hz",
	"95 Hz",
	"100 Hz",
	"110 Hz",
	"120 Hz",
};
const int numberOffullScreenFrequencies = sizeof(fullScreenFrequencies)/sizeof(char*);
*/

const RenderEngineSetting RenderEngineSettings[] =
{
	"DirectX",	DIRECTX_DEVICE,
	"OpenGL", OGL_DEVICE,
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
	"2x - texture rectangle only",	TEXTURE_2X_TEXRECT_ENHANCEMENT,
	"2xSaI", TEXTURE_2XSAI_ENHANCEMENT,
	"2xSaI - texture rectangle only", TEXTURE_2XSAI_TEXRECT_ENHANCEMENT,
	"Sharpen", TEXTURE_SHARPEN_ENHANCEMENT,
	"Sharpen More", TEXTURE_SHARPEN_MORE_ENHANCEMENT,
};

const SettingInfo TextureEnhancementControlSettings[] =
{
	"Normal",	TEXTURE_ENHANCEMENT_NORMAL,
	"Enhance small textures only",	TEXTURE_ENHANCEMENT_SMALL_TEXTURE_ONLY,
	"Smooth",	TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1,
	"Less smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2,
	"2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3,
	"Less 2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4,
};
/*
const SettingInfo openGLColorBufferSettings[] =
{
	"16-bit",	TEXTURE_FMT_A4R4G4B4,
	"32-bit (def)",	TEXTURE_FMT_A8R8G8B8,
};
*/

const SettingInfo openGLDepthBufferSettings[] =
{
	"16-bit (def)",	16,
	"32-bit",	32,
};
/*
const RenderEngineSetting OpenGLRenderSettings[] =
{
	"To Fit Your Video Card",	OGL_DEVICE,
	"OpenGL 1.1 (Lowest)",		OGL_1_1_DEVICE,
	"OpenGL 1.2/1.3",			OGL_1_2_DEVICE,
	"OpenGL 1.4",				OGL_1_4_DEVICE,
	"OpenGL for Nvidia TNT or better",		OGL_TNT2_DEVICE,
	"OpenGL for Nvidia Geforce or better ",	NVIDIA_OGL_DEVICE,
};*/

const int numberOfRenderEngineSettings = sizeof(RenderEngineSettings)/sizeof(RenderEngineSetting);
//const int numberOfOpenGLRenderEngineSettings = sizeof(OpenGLRenderSettings)/sizeof(RenderEngineSetting);

#define MAIN_RICE_DAEDALUS_4		"Software\\RICEDAEDALUS520\\WINDOW"
//#define RICE_DAEDALUS_INI_FILE		"D:\\RiceDaedalus5.3.1.ini"
#define RICE_DAEDALUS_INI_FILE		"RiceDaedalus5.3.1.ini"

extern int TextureMode;
void WriteConfiguration(void);
void GenerateCurrentRomOptions();


//////////////////////////////////////////////////////////////////////////
void GenerateFrameBufferOptions(void)
{
	/*if( CDeviceBuilder::GetGeneralDeviceType() == OGL_DEVICE && currentRomOptions.N64FrameBufferEmuType != FRM_NONE )
	{
		// OpenGL does not support much yet
		currentRomOptions.N64FrameBufferEmuType = FRM_IGNORE;
	}
	*/
	//currentRomOptions.N64FrameBufferEmuType = FRM_NONE;

	frameBufferOptions.bUpdateCIInfo			= false;

	frameBufferOptions.bSupportBackBufs			= false;
	frameBufferOptions.bCheckBackBufs			= false;
	frameBufferOptions.bWriteBackBufToRDRAM		= false;
	frameBufferOptions.bLoadBackBufFromRDRAM	= false;

	frameBufferOptions.bIgnore					= true;

	frameBufferOptions.bSupportTxtBufs			= false;
	frameBufferOptions.bCheckTxtBufs			= false;
	frameBufferOptions.bTxtBufWriteBack			= false;
	frameBufferOptions.bLoadRDRAMIntoTxtBuf		= false;

	frameBufferOptions.bIngoreIfCIIsZI			= true;
	frameBufferOptions.bProcessZIDrawing		= false;

	frameBufferOptions.bProcessCPUWrite			= false;
	frameBufferOptions.bProcessCPURead			= false;

	switch( currentRomOptions.N64FrameBufferEmuType )
	{
	case FRM_NONE:
		break;
	case FRM_COMPLETE:
		frameBufferOptions.bAtEachFrameUpdate		= true;
		frameBufferOptions.bProcessCPUWrite			= true;
		frameBufferOptions.bProcessCPURead			= true;
	case FRM_WRITEBACK_AND_RELOAD:
		frameBufferOptions.bLoadBackBufFromRDRAM	= true;
	case FRM_BASIC_AND_WRITEBACK:
		frameBufferOptions.bWriteBackBufToRDRAM		= true;
		frameBufferOptions.bTxtBufWriteBack			= true;
	case FRM_BASIC:
		frameBufferOptions.bCheckBackBufs			= true;
		frameBufferOptions.bSupportBackBufs			= true;
	case FRM_RENDER_TO_TEXTURE_PLUS:
		frameBufferOptions.bProcessZIDrawing		= true;
		frameBufferOptions.bIngoreIfCIIsZI			= false;
	case FRM_RENDER_TO_TEXTURE:
		frameBufferOptions.bLoadRDRAMIntoTxtBuf		= true;
		frameBufferOptions.bTxtBufWriteBack			= true;
	case FRM_RENDER_TO_TEXTURE_FASTER:
		frameBufferOptions.bCheckTxtBufs			= true;
		frameBufferOptions.bIgnore					= false;
	case FRM_IGNORE:
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bSupportTxtBufs			= true;
		break;
	case FRM_WITH_EMULATOR:
		// For Dr. Mario
		frameBufferOptions.bSupportBackBufs			= true;
		frameBufferOptions.bSupportTxtBufs			= true;
		frameBufferOptions.bUpdateCIInfo			= true;
		frameBufferOptions.bProcessCPUWrite			= true;
		frameBufferOptions.bProcessCPURead			= true;
		break;
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
	options.enableHacks = TRUE;
	options.enableSSE = TRUE;

	defaultRomOptions.screenUpdateSetting = SCREEN_UPDATE_AT_VI_CHANGE;//SCREEN_UPDATE_AT_VI_UPDATE;//
	defaultRomOptions.dwEnableObjBG = 1;
	status.isMMXSupported = 1;//isMMXSupported();
	status.isSSESupported = 0;//isSSESupported();
	defaultRomOptions.N64FrameBufferEmuType = FRM_NONE;


	DWORD videoFlags = XGetVideoFlags();

		windowSetting.uWindowDisplayWidth = ReadRegistryDwordVal(MAIN_RICE_DAEDALUS_4, "WinModeWidth");
	if( windowSetting.uWindowDisplayWidth == 0 )
	{
		windowSetting.uWindowDisplayWidth = 640;

		if(XGetAVPack() == XC_AV_PACK_HDTV) {
			if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV) {
				windowSetting.uWindowDisplayWidth = 1280;
			}
		}
			
	}

		windowSetting.uWindowDisplayHeight = ReadRegistryDwordVal(MAIN_RICE_DAEDALUS_4, "WinModeHeight");
	if( windowSetting.uWindowDisplayHeight == 0 )
	{
		windowSetting.uWindowDisplayHeight = 480;

		if(XGetAVPack() == XC_AV_PACK_HDTV) {
			if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV) {
				windowSetting.uWindowDisplayHeight = 720;
			}
		}
	}
		
		windowSetting.uDisplayWidth = windowSetting.uWindowDisplayWidth;
		windowSetting.uDisplayHeight = windowSetting.uWindowDisplayHeight;


		windowSetting.uFullScreenDisplayWidth = ReadRegistryDwordVal(MAIN_RICE_DAEDALUS_4, "FulScreenWidth");
	if( windowSetting.uFullScreenDisplayWidth == 0 )
	{
		windowSetting.uWindowDisplayWidth = 640;

		if(XGetAVPack() == XC_AV_PACK_HDTV) {
			if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV) {
				windowSetting.uWindowDisplayWidth = 1280;
			}
		}
	}
		windowSetting.uFullScreenDisplayHeight = ReadRegistryDwordVal(MAIN_RICE_DAEDALUS_4, "FulScreenHeight");
	if( windowSetting.uFullScreenDisplayHeight == 0 )
	{
		windowSetting.uWindowDisplayHeight = 480;

		if(XGetAVPack() == XC_AV_PACK_HDTV) {
			if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV) {
				windowSetting.uWindowDisplayHeight = 720;
			}
		}
	}

	
		options.bWinFrameMode = 0;

		defaultRomOptions.normalAlphaBlender = 0;
		defaultRomOptions.normalColorCombiner = 0;

		options.enableFog = TRUE;
		options.enableSSE = TRUE;
		options.skipFrame = FrameSkip;

		options.RenderBufferSetting = 0;//1;

		options.textureEnhancement = 0;
		options.textureEnhancementControl = 0;

		options.bForceSoftwareTnL = 0;
		
		//fd - We only need ConfigAppLoad2 here (seems to work without this call)
		//ConfigAppLoad2();
		//fd - override default texture filter setting
		options.forceTextureFilter = TextureMode;
		
		CDeviceBuilder::SelectDeviceType(XBOX_DIRECTX_DEVICE);
		g_dwDirectXCombinerType = DX_2_STAGES;
		options.DirectXDepthBufferSetting = 0;
		defaultRomOptions.bFastTexCRC = TRUE;

		options.bDisplayTooltip = 0;
		defaultRomOptions.forceBufferClear = 0;
		defaultRomOptions.accurateTextureMapping = 1;

		defaultRomOptions.bSupportSelfRenderTexture = 1;
		
		windowSetting.uFullScreenFrequency = 0;	// 0 is the default value, means to use Window default frequency
		
	if (status.isMMXSupported)
		MsgInfo("MMX Enabled");
	else
		MsgInfo("MMX Disabled");
		
	status.isSSEEnabled = status.isSSESupported && options.enableSSE;
	if( status.isSSEEnabled )
	{
		SetNewVertexInfo = SetNewVertexInfoSSE;
		MsgInfo("SSE Enabled");
	}
	else
	{
		SetNewVertexInfo = SetNewVertexInfoNoSSE;
		MsgInfo("SSE Disabled");
	}
}
	
//---------------------------------------------------------------------------------------
BOOL InitConfiguration(void)
{
	//Initialize this DLL

	if( g_pIniFile == NULL )	//Test if we have done initialization
	{
		g_pIniFile = new IniFile(RICE_DAEDALUS_INI_FILE);
		if (g_pIniFile == NULL)
		{
			ReadConfiguration(); //defaults
			return FALSE;
		}

		if (!g_pIniFile->ReadIniFile())
		{
			ErrorMsg("Unable to read Daedalus.ini file from disk");
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
	currentRomOptions.screenUpdateSetting		=g_curRomInfo.dwScreenUpdateSetting;
	currentRomOptions.normalColorCombiner		=g_curRomInfo.dwNormalCombiner;
	currentRomOptions.normalAlphaBlender		=g_curRomInfo.dwNormalBlender;
	currentRomOptions.bFastTexCRC				=g_curRomInfo.dwFastTextureCRC;
	currentRomOptions.bEmulateClear				=g_curRomInfo.dwEmulateClear;
	currentRomOptions.bSupportSelfRenderTexture	=g_curRomInfo.dwSupportSelfRenderTexture;
	currentRomOptions.forceBufferClear			=g_curRomInfo.dwForceScreenClear;
	currentRomOptions.dwEnableObjBG				=g_curRomInfo.dwEnableObjBG;
	currentRomOptions.accurateTextureMapping	=g_curRomInfo.dwAccurateTextureMapping;

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
	else if ((strstr(g_curRomInfo.szGameName, "Baseball") != 0) && (strstr(g_curRomInfo.szGameName, "Star") != 0))
	{
		options.enableHackForGames = HACK_FOR_ALL_STAR_BASEBALL;
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
	if( currentRomOptions.N64FrameBufferEmuType == 0 )		currentRomOptions.N64FrameBufferEmuType = defaultRomOptions.N64FrameBufferEmuType;
	else currentRomOptions.N64FrameBufferEmuType--;
	if( currentRomOptions.screenUpdateSetting == 0 )		currentRomOptions.screenUpdateSetting = defaultRomOptions.screenUpdateSetting;
	if( currentRomOptions.normalColorCombiner == 0 )		currentRomOptions.normalColorCombiner = defaultRomOptions.normalColorCombiner;
	else currentRomOptions.normalColorCombiner--;
	if( currentRomOptions.normalAlphaBlender == 0 )			currentRomOptions.normalAlphaBlender = defaultRomOptions.normalAlphaBlender;
	else currentRomOptions.normalAlphaBlender--;
	if( currentRomOptions.bFastTexCRC == 0 )				currentRomOptions.bFastTexCRC = defaultRomOptions.bFastTexCRC;
	else currentRomOptions.bFastTexCRC--;
	if( currentRomOptions.bEmulateClear == 0 )				currentRomOptions.bEmulateClear = defaultRomOptions.bEmulateClear;
	else currentRomOptions.bEmulateClear--;
	if( currentRomOptions.bSupportSelfRenderTexture == 0 )	currentRomOptions.bSupportSelfRenderTexture = defaultRomOptions.bSupportSelfRenderTexture;
	else currentRomOptions.bSupportSelfRenderTexture--;
	if( currentRomOptions.forceBufferClear == 0 )			currentRomOptions.forceBufferClear = defaultRomOptions.forceBufferClear;
	else currentRomOptions.forceBufferClear--;
	if( currentRomOptions.dwEnableObjBG == 0 )				currentRomOptions.dwEnableObjBG = defaultRomOptions.dwEnableObjBG;
	else currentRomOptions.dwEnableObjBG--;
	if( currentRomOptions.accurateTextureMapping == 0 )		currentRomOptions.accurateTextureMapping = defaultRomOptions.accurateTextureMapping;
	else currentRomOptions.accurateTextureMapping--;

	GenerateFrameBufferOptions();
}

extern IniFile * g_pIniFile;

void Ini_GetRomOptions(LPROMINFO pRomInfo)
{
	LONG i;

	i = g_pIniFile->FindEntry(pRomInfo->rh.dwCRC1,
							  pRomInfo->rh.dwCRC2,
							  pRomInfo->rh.nCountryID,
							  pRomInfo->szGameName);

	pRomInfo->ucode = g_pIniFile->sections[i].ucode;

	lstrcpyn(pRomInfo->szGameName, g_pIniFile->sections[i].name, 50);

	pRomInfo->bDisableTextureCRC	= g_pIniFile->sections[i].bDisableTextureCRC;
	pRomInfo->bIncTexRectEdge		= g_pIniFile->sections[i].bIncTexRectEdge;
	pRomInfo->bTextureScaleHack		= g_pIniFile->sections[i].bTextureScaleHack;
	pRomInfo->bPrimaryDepthHack		= g_pIniFile->sections[i].bPrimaryDepthHack;
	pRomInfo->bTexture1Hack			= g_pIniFile->sections[i].bTexture1Hack;
	pRomInfo->bFastLoadTile			= g_pIniFile->sections[i].bFastLoadTile;
	pRomInfo->VIWidth				= g_pIniFile->sections[i].VIWidth;
	pRomInfo->VIHeight				= g_pIniFile->sections[i].VIHeight;

	pRomInfo->dwFastTextureCRC		= g_pIniFile->sections[i].dwFastTextureCRC;
	pRomInfo->dwEmulateClear		= g_pIniFile->sections[i].dwEmulateClear;
	pRomInfo->dwSupportSelfRenderTexture		= g_pIniFile->sections[i].dwSupportSelfRenderTexture;
	pRomInfo->dwForceScreenClear	= g_pIniFile->sections[i].dwForceScreenClear;
	pRomInfo->dwAccurateTextureMapping	= g_pIniFile->sections[i].dwAccurateTextureMapping;
	pRomInfo->dwNormalBlender		= g_pIniFile->sections[i].dwNormalBlender;
	pRomInfo->dwNormalCombiner		= g_pIniFile->sections[i].dwNormalCombiner;
	pRomInfo->dwEnableObjBG			= g_pIniFile->sections[i].dwEnableObjBG;
	pRomInfo->dwFrameBufferOption	= g_pIniFile->sections[i].dwFrameBufferOption;
	pRomInfo->dwScreenUpdateSetting	= g_pIniFile->sections[i].dwScreenUpdateSetting;
}
/*
void Ini_StoreRomOptions(LPROMINFO pRomInfo)
{
	LONG i;

	i = g_pIniFile->FindEntry(pRomInfo->rh.dwCRC1,
		pRomInfo->rh.dwCRC2,
		pRomInfo->rh.nCountryID,
		pRomInfo->szGameName);

	if( g_pIniFile->sections[i].bDisableTextureCRC	!=pRomInfo->bDisableTextureCRC )
	{
		g_pIniFile->sections[i].bDisableTextureCRC	=pRomInfo->bDisableTextureCRC	 ;
		g_pIniFile->bChanged=true;
	}

	g_pIniFile->sections[i].bIncTexRectEdge		=pRomInfo->bIncTexRectEdge		 ;
	g_pIniFile->sections[i].bTextureScaleHack	=pRomInfo->bTextureScaleHack		 ;
	g_pIniFile->sections[i].bPrimaryDepthHack	=pRomInfo->bPrimaryDepthHack		 ;
	g_pIniFile->sections[i].bTexture1Hack		=pRomInfo->bTexture1Hack		 ;
	g_pIniFile->sections[i].bFastLoadTile		=pRomInfo->bFastLoadTile		 ;
	g_pIniFile->sections[i].VIWidth				=pRomInfo->VIWidth		 ;
	g_pIniFile->sections[i].VIHeight			=pRomInfo->VIHeight		 ;

	if( g_pIniFile->sections[i].dwFastTextureCRC !=pRomInfo->dwFastTextureCRC )
	{
		g_pIniFile->sections[i].dwFastTextureCRC	=pRomInfo->dwFastTextureCRC		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].dwEmulateClear !=pRomInfo->dwEmulateClear )
	{
		g_pIniFile->sections[i].dwEmulateClear	=pRomInfo->dwEmulateClear		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].dwSupportSelfRenderTexture !=pRomInfo->dwSupportSelfRenderTexture )
	{
		g_pIniFile->sections[i].dwSupportSelfRenderTexture	=pRomInfo->dwSupportSelfRenderTexture		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].dwNormalBlender		!=pRomInfo->dwNormalBlender )
	{
		g_pIniFile->sections[i].dwNormalBlender		=pRomInfo->dwNormalBlender		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwForceScreenClear	!=pRomInfo->dwForceScreenClear )
	{
		g_pIniFile->sections[i].dwForceScreenClear	=pRomInfo->dwForceScreenClear		 ;
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
	if( g_pIniFile->sections[i].dwEnableObjBG	!=pRomInfo->dwEnableObjBG )
	{
		g_pIniFile->sections[i].dwEnableObjBG	=pRomInfo->dwEnableObjBG		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwFrameBufferOption	!=pRomInfo->dwFrameBufferOption )
	{
		g_pIniFile->sections[i].dwFrameBufferOption	=pRomInfo->dwFrameBufferOption		 ;
		g_pIniFile->bChanged=true;
	}
	if( g_pIniFile->sections[i].dwScreenUpdateSetting	!=pRomInfo->dwScreenUpdateSetting )
	{
		g_pIniFile->sections[i].dwScreenUpdateSetting	=pRomInfo->dwScreenUpdateSetting		 ;
		g_pIniFile->bChanged=true;
	}
}

 */



