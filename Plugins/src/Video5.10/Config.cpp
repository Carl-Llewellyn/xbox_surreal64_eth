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

WindowSettingStruct windowSetting;
GlobalOptions options;
RomOptions defaultRomOptions;
RomOptions currentRomOptions;

extern int TextureMode;


char *frameBufferSettings[] =
{
	"Disable (default)",
	"Ignore",
	"Copy",
	"Faster",
	"Faster+Recheck",
	"Complete",
	"Copy & Back",
	"With Emulator",
};

const char *screenUpdateSettings[] =
{
	"At VI origin update (default)",
	"At VI origin change",
	"At CI change",
	"At the 1st CI change",
	"At the 1st drawing"
};

const char *gammaCorrectionSettings[] =
{
	"Disable",
	"1 - brighter",
	"2 - brighter more",
	"3 - brighter more",
	"4 - brighter more",
	"Darker",
	"Darker more"
};

const char *openGLDepthBufferSettings[] =
{
	"Default",
	"16 bits",
	"32 bits"
};

//=======================================================

//BYTE g_gammaValues[256];
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
	"1280 x 1024",
	"1400 x 1050",
	"1600 x 1200",
	"1920 x 1440",
	"2048 x 1536"
};

const int numberOfResolutions = sizeof(resolutions)/sizeof(char*); */

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
	"16 bits",	TEXTURE_FMT_A4R4G4B4,
	"32 bits",	TEXTURE_FMT_A8R8G8B8,
};

const RenderEngineSetting OpenGLRenderSettings[] =
{
	"OpenGL best fit to your video card ",	OGL_DEVICE,
	"OpenGL 1.1 (Lowest)",		OGL_1_1_DEVICE,
	"OpenGL 1.2/1.3/1.4 for ATI, Nvidia and others",		OGL_1_2_DEVICE,
	"OpenGL for Nvidia TNT or better",		OGL_TNT2_DEVICE,
	"OpenGL for Nvidia Geforce or better ",	NVIDIA_OGL_DEVICE,
};
*/
const int numberOfRenderEngineSettings = sizeof(RenderEngineSettings)/sizeof(RenderEngineSetting);
//const int numberOfOpenGLRenderEngineSettings = sizeof(OpenGLRenderSettings)/sizeof(RenderEngineSetting);

#define MAIN_RICE_DAEDALUS_4		"Software\\RICEDAEDALUS510\\WINDOW"
//#define RICE_DAEDALUS_INI_FILE		"D:\\RiceDaedalus5.1.0.ini"
#define RICE_DAEDALUS_INI_FILE		"RiceDaedalus5.1.0.ini"

void WriteConfiguration(void);
void GenerateCurrentRomOptions();
 


//========================================================================

LRESULT APIENTRY ConfigDialog(HWND hDlg, unsigned message, LONG wParam, LONG lParam) 
{ 
	 
	return FALSE;
}


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

	//fd - We only need ConfigAppLoad2 here
	//ConfigAppLoadTemp();
	//ConfigAppLoad();
	ConfigAppLoad2(); // load this at the top

	options.enableHacks = TRUE;
	options.enableSSE = TRUE;

	defaultRomOptions.screenUpdateSetting = SCREEN_UPDATE_AT_VI_CHANGE;//SCREEN_UPDATE_AT_VI_UPDATE;
	defaultRomOptions.dwEnableObjBG = TRUE;
	status.isMMXSupported = 1;
	status.isSSESupported = 0;
	defaultRomOptions.N64FrameBufferEmuType = FRM_DISABLE;

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
	//options.enableHacks = ReadRegistryDwordVal(MAIN_RICE_DAEDALUS_4, "EnableHacks");
	
	defaultRomOptions.normalAlphaBlender = 0;
	defaultRomOptions.normalColorCombiner = 0;

	//defaultRomOptions.dwEnableObjBG = ReadRegistryDwordVal(MAIN_RICE_DAEDALUS_4, "EnableObjBG");
	options.enableFog = 1;
	options.enableSSE = 1;
	options.skipFrame = FrameSkip;
	options.RenderBufferSetting = 0;
	options.gamma_correction = 0;
	options.textureEnhancement = 0;
	options.textureEnhancementControl = 0;

	
	
	options.forceTextureFilter = TextureMode;
	
	CDeviceBuilder::SelectDeviceType(XBOX_DIRECTX_DEVICE);
	g_dwDirectXCombinerType = 1;
	options.DirectXDepthBufferSetting = 0;
	defaultRomOptions.bFastTexCRC = 1;
	windowSetting.bUseEmulatorToolbar = 0;
	defaultRomOptions.forceBufferClear = 0;
	defaultRomOptions.accurateTextureMapping = 1;

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
	currentRomOptions.forceBufferClear			=g_curRomInfo.dwForceScreenClear;
	currentRomOptions.dwEnableObjBG				=g_curRomInfo.dwEnableObjBG;
	currentRomOptions.accurateTextureMapping	=g_curRomInfo.dwAccurateTextureMapping;

	if( currentRomOptions.N64FrameBufferEmuType == 0 )		currentRomOptions.N64FrameBufferEmuType = defaultRomOptions.N64FrameBufferEmuType;
	else currentRomOptions.N64FrameBufferEmuType--;
	if( currentRomOptions.screenUpdateSetting == 0 )		currentRomOptions.screenUpdateSetting = defaultRomOptions.screenUpdateSetting;
	else currentRomOptions.screenUpdateSetting--;
	if( currentRomOptions.normalColorCombiner == 0 )		currentRomOptions.normalColorCombiner = defaultRomOptions.normalColorCombiner;
	else currentRomOptions.normalColorCombiner--;
	if( currentRomOptions.normalAlphaBlender == 0 )			currentRomOptions.normalAlphaBlender = defaultRomOptions.normalAlphaBlender;
	else currentRomOptions.normalAlphaBlender--;
	if( currentRomOptions.bFastTexCRC == 0 )				currentRomOptions.bFastTexCRC = defaultRomOptions.bFastTexCRC;
	else currentRomOptions.bFastTexCRC--;
	if( currentRomOptions.forceBufferClear == 0 )			currentRomOptions.forceBufferClear = defaultRomOptions.forceBufferClear;
	else currentRomOptions.forceBufferClear--;
	if( currentRomOptions.dwEnableObjBG == 0 )				currentRomOptions.dwEnableObjBG = defaultRomOptions.dwEnableObjBG;
	else currentRomOptions.dwEnableObjBG--;
	if( currentRomOptions.accurateTextureMapping == 0 )		currentRomOptions.accurateTextureMapping = defaultRomOptions.accurateTextureMapping;
	else currentRomOptions.accurateTextureMapping--;
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
	lstrcpyn(pRomInfo->szComment, g_pIniFile->sections[i].comment, 100);
	lstrcpyn(pRomInfo->szInfo, g_pIniFile->sections[i].info, 100);

	pRomInfo->bDisablePatches		= g_pIniFile->sections[i].bDisablePatches;
	pRomInfo->bDisableTextureCRC	= g_pIniFile->sections[i].bDisableTextureCRC;
	pRomInfo->bDisableEeprom		= g_pIniFile->sections[i].bDisableEeprom;
	pRomInfo->bIncTexRectEdge		= g_pIniFile->sections[i].bIncTexRectEdge;
	pRomInfo->bTextureScaleHack		= g_pIniFile->sections[i].bTextureScaleHack;
	pRomInfo->bDisableSpeedSync		= g_pIniFile->sections[i].bDisableSpeedSync;
	pRomInfo->bDisableDynarec		= g_pIniFile->sections[i].bDisableDynarec;
	pRomInfo->bExpansionPak			= g_pIniFile->sections[i].bExpansionPak;
	pRomInfo->bPrimaryDepthHack		= g_pIniFile->sections[i].bPrimaryDepthHack;
	pRomInfo->bTexture1Hack			= g_pIniFile->sections[i].bTexture1Hack;
	pRomInfo->bFastLoadTile			= g_pIniFile->sections[i].bFastLoadTile;
	pRomInfo->VIWidth				= g_pIniFile->sections[i].VIWidth;
	pRomInfo->VIHeight				= g_pIniFile->sections[i].VIHeight;

	pRomInfo->dwEepromSize			= g_pIniFile->sections[i].dwEepromSize;
	pRomInfo->dwRescanCount			= g_pIniFile->sections[i].dwRescanCount;

	pRomInfo->dwFastTextureCRC		= g_pIniFile->sections[i].dwFastTextureCRC;
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

	if( g_pIniFile->sections[i].bDisablePatches	!=pRomInfo->bDisablePatches )
	{
		g_pIniFile->sections[i].bDisablePatches		=pRomInfo->bDisablePatches		 ;
		g_pIniFile->bChanged=true;
	}

	if( g_pIniFile->sections[i].bDisableTextureCRC	!=pRomInfo->bDisableTextureCRC )
	{
		g_pIniFile->sections[i].bDisableTextureCRC	=pRomInfo->bDisableTextureCRC	 ;
		g_pIniFile->bChanged=true;
	}

	g_pIniFile->sections[i].bDisableEeprom		=pRomInfo->bDisableEeprom		 ;
	g_pIniFile->sections[i].bIncTexRectEdge		=pRomInfo->bIncTexRectEdge		 ;
	g_pIniFile->sections[i].bTextureScaleHack	=pRomInfo->bTextureScaleHack		 ;
	g_pIniFile->sections[i].bDisableSpeedSync	=pRomInfo->bDisableSpeedSync	 ;
	g_pIniFile->sections[i].bDisableDynarec		=pRomInfo->bDisableDynarec		 ;
	g_pIniFile->sections[i].bExpansionPak		=pRomInfo->bExpansionPak		 ;
	g_pIniFile->sections[i].bPrimaryDepthHack	=pRomInfo->bPrimaryDepthHack		 ;
	g_pIniFile->sections[i].bTexture1Hack		=pRomInfo->bTexture1Hack		 ;
	g_pIniFile->sections[i].bFastLoadTile		=pRomInfo->bFastLoadTile		 ;
	g_pIniFile->sections[i].VIWidth				=pRomInfo->VIWidth		 ;
	g_pIniFile->sections[i].VIHeight			=pRomInfo->VIHeight		 ;

	g_pIniFile->sections[i].dwEepromSize		=pRomInfo->dwEepromSize			 ;
	g_pIniFile->sections[i].dwRescanCount		=pRomInfo->dwRescanCount		 ;
																				 
	if( g_pIniFile->sections[i].dwFastTextureCRC !=pRomInfo->dwFastTextureCRC )
	{
		g_pIniFile->sections[i].dwFastTextureCRC	=pRomInfo->dwFastTextureCRC		 ;
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


 
