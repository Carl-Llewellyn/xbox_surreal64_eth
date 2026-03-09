#include "config.h"
#include <xbapp.h>
#include <xbresource.h>

#ifdef DEBUG
	bool showdebug = true;
#else
	bool showdebug = false;
#endif

#ifdef LAUNCHER
	bool bLauncherHD = true;  // Force HD in Launcher if available
#else
	bool bLauncherHD = false;
#endif

bool bEnableHDTV = false; // Set to false (640x480 default)
bool bFullScreen = false;//true;	// When Using HDTV render to whole screen
									// This setting is unused at the moment, we are talking
									// about aspect ratio here. UHLE defaults to 4:3 even in HD
									
extern "C" void loadinis();
extern "C" int loaddwPJ64PagingMem();
extern "C" int loaddwPJ64DynaMem();
extern "C" int loaddw1964PagingMem();
extern "C" int loaddw1964DynaMem();
extern "C" int loaddwMaxVideoMem(); //reinstate max video mem

//extern "C" int loadbUseLLERSP(); // not used anymore, use iAudioPlugin instead to determine which audio plugin is used
extern "C" int loadbUseRspAudio(); // control a listing
extern "C" int loadiRspPlugin();
extern "C" int loadiAudioPlugin();

// ultrahle mem settings
extern "C" int loaddwUltraCodeMem();
extern "C" int loaddwUltraGroupMem();

extern "C" int loadiPagingMethod();

extern "C" int loadbAudioBoost();
extern "C" void GetPathSaves(char *pszPathSaves);

void LoadSkinFile();
void WriteSkinFile();

enum Pak
{
	NoPak = 1,
	MemPak = 2,
	RumblePak = 3
};

enum D3D_PresentationIntervals
{
	INTERVAL_IMMEDIATE,	//screen tearing possible
	INTERVAL_ONE,	//VSync
	//INTERVAL_TWO,	//VSync
	//INTERVAL_THREE,	//VSync
	//INTERVAL_FOUR,	//not supported on XBOX
	//INTERVAL_DEFAULT,	//VSync
	INTERVAL_ONE_OR_IMMEDIATE //see SDK docs
	//INTERVAL_TWO_OR_IMMEDIATE, //see SDK docs
	//INTERVAL_THREE_OR_IMMEDIATE, //see SDK docs*/
};


int dw1964DynaMem=4;//8
int dw1964PagingMem=4;
int dwPJ64DynaMem=10;//16
int dwPJ64PagingMem=4;
int dwMaxVideoMem=4; // reinstate max free - in launcher config, this was 4 //5

// ultrahle mem settings
int dwUltraCodeMem=5;
int dwUltraGroupMem=10;

int videoplugin = _VideoPluginRice560; // 2=rice560
int iAudioPlugin = _AudioPluginJttl; // 2=JttL // use iAudioPlugin instead to determine which audio plugin is used
int iRspPlugin = _RSPPluginHLE;
bool bUseRspAudio=false; // control a listing

// leave this in case it's set in ini
bool bUseLLERSP=false;
bool bUseBasicAudio=false;


bool has128ram; // determine if the current phys ram is greater than 100MB: 128MB = true, 64MB = false

// Ez0n3 - launch screens enable / disable
bool HideLaunchScreens=0;
bool EnableVideoAudio=0;
bool EnableXMVPreview=0;
bool EnableInfoPanel=1;
//bool usePageOriginal=0; //fd - XXX default // moving to main ini
int iPagingMethod=_PagingXXX; //XXX
bool EnableBGMusic=1;
bool RandomBGMusic=false;
bool bAudioBoost=false;

//DWORD dwLastRomCRC;
char romCRC[32];
int romcounter = 0;
bool onhd;
int FlickerFilter = 1;
bool SoftDisplayFilter = 0;
int TextureMode = 3;
int AntiAliasMode = 0;
int VertexMode = 2; // default hardware
bool FrameSkip = FALSE;
int VSync = INTERVAL_IMMEDIATE; //see D3D_PresentParameters (PresentationIntervals)
//int RefreshRateInHz = 60;
float XBOX_CONTROLLER_DEAD_ZONE = 8600; // also change in SurrealMenu.cpp
float Deadzone = 26;
int Sensitivity = 10;
char skinname[32] = "Default";

char romname[256];

char szPathRoms[MAX_FILE_PATH] = "D:\\Roms\\";
char szPathMedia[MAX_FILE_PATH] = "D:\\Media\\";
char szPathSkins[MAX_FILE_PATH] = "D:\\Skins\\";
char szPathSaves[MAX_FILE_PATH] = "D:\\Saves\\";
char szPathScreenshots[MAX_FILE_PATH] = "D:\\Screenshots\\";

#define CONTROLLER_CONFIG_MAX 76 // change it here and it will auto-update the ini's
int ControllerConfig[CONTROLLER_CONFIG_MAX] 
		= {0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F,
		   0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F,
		   0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F,
		   0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F};

int ControllerReset[19]  = 
          {0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F};

//Skin.ini Defaults
char BoxartName[32] = "Cbagys3DArt";

//Font Colors
DWORD dwTitleColor = 0xFF53B77F;
DWORD dwIGMTitleColor = 0x55777777;
DWORD dwMenuItemColor = 0xCCEEEEEE;
DWORD dwMenuTitleColor = 0xFF8080FF;
DWORD dwRomListColor = 0xAAEEEEEE;
DWORD dwSelectedRomColor = 0xFFFF8000;
DWORD dwNullItemColor = 0xEE53B77F;

//Launcher XLMenu Coords
int iLaunchMenuTxtPosX = 200;
int iLaunchMenuTxtPosY = 130;
int iLaunchMenuTxtAlign = 0;
int iLaunchMenuBgPosX = 190;
int iLaunchMenuBgPosY = 120;
int iLaunchHilightPosX = 190;
int iMainMenuTxtPosX = 60;
int iMainMenuTxtPosY = 65;
int iMainMenuPosX = 45;
int iMainMenuPosY = 60;
int iMainMenuTxtAlign = 0;
int iMainHilightPosX = 45;
int iControlConfigPosX = 64;
int iControlConfigPosY = 5;
int iControlConfigWidth = 505;
int iControlConfigTxtPosX = 320;
int iControlConfigTxtPosY = 25;
int iControlConfigTxtPadLX = 5;

//Synopsis
int iSynopsisPosX = 30;
int iSynopsisPosY = 15;	// initial y position, should be a skin param???
int iSynopsisWrap = 66; // sd=66 (69 max - 3 roughly matches PosX 30 left margin), hd=??? (roughly 138 for 1280? - 3 for margin)
int iSynopsisLines = 20;
int iSynopsisAlign = 0;

//Launcher Coords
int iInfoPosX = 50;
int iInfoPosY = 310;
int iInfoTxtPosX = 60;
int iInfoTxtPosY = 316;
int iInfoTxtAlign = 0;
int iInfoTxtControlPosX = 320;
int iInfoTxtControlPosY = 430;
int iInfoTxtControlAlign = 2; // 0=left, 1=right, 2=centerX,
bool bShowRLControls = 1;
float InfoPanelTrunc = 430.0f; //float or int? float.
int iBoxPosX = 437;
int iBoxPosY = 37;
int iBoxWidth = 178;
int iBoxHeight = 178;
int iBoxPanelPosX = 430;
int iBoxPanelPosY = 30;
int iTitleX = 305;
int iTitleY = 20;
int iControlsSpacing = -3;

int iTempMessagePosX = 10;
int iTempMessagePosY = 10;
int iTempMessageAlign = 0;

// Load Window
int iLoadBoxPosX = 85;
int iLoadBoxPosY = 120;
int iLoadPanelPosX = 92;
int iLoadPanelPosY = 30;
int iLoadPanelTxtPosX = 100;
int iLoadPanelTxtPosY = 35;
int iLoadPanelTxtAlign = 0;
int iLoadPanelBarPosX = 95;
int iLoadPanelBarPosY = 67;
int iLoadPanelTrunc = 450;

int iBoxWidthAuto = 178;
int iBoxHeightAuto = 178;

//IGM Coords
int iIGMTitleX = 510;//305
int iIGMTitleY = 430;//20
int iIGMMenuPosX = 45;
int iIGMMenuPosY = 70;
int iIGMHilightPosX = 45;
int iPanelX = 0;//305
int iPanelY = 0;//65
int iPanelNW = 640;//267
int iPanelNH = 480;//200
int iIGMControlConfigBGPosX = 64;
int iIGMControlConfigBGPosY = 5;
int iControlsSpacingIGM = -3;
int iIGMControlConfigCenterX = 320;
int iIGMControlConfigWidth = 505;
int iIGMControlConfigTop = 25;
int iIGMControlConfigTxtPadLX = 5;
int iIGMConControlsPosX = 270;
int iIGMConControlsPosY = 350;
int iIGMConControlsTxtPosX = 272;
int iIGMConControlsTxtPosY = 355;
int iIGMConControlsTxtAlign = 0;

//IGM XLMenu Coords
int iIGMMenuTxtPosX = 60;
int iIGMMenuTxtPosY = 80;
int iIGMMenuTxtAlign = 0;
int iIGMStateScreenX = 60;
int iIGMStateScreenY = 270;
int iIGMStateScreenW = 256;
int iIGMStateScreenH = 191;

//IGM HD Coords
int iIGMTitleX_HD = 1100;
int iIGMTitleY_HD = 690;
int iIGMMenuPosX_HD = 45;
int iIGMMenuPosY_HD = 70;
int iIGMHilightPosX_HD = 45;
int iPanelX_HD = 0;
int iPanelY_HD = 0;
int iPanelNW_HD = 1280;
int iPanelNH_HD = 720;
int iIGMControlConfigBGPosX_HD = 64;
int iIGMControlConfigBGPosY_HD = 5;
int iControlsSpacingIGM_HD = -3;
int iIGMControlConfigCenterX_HD = 320;
int iIGMControlConfigWidth_HD = 505;
int iIGMControlConfigTop_HD = 25;
int iIGMControlConfigTxtPadLX_HD = 5;
int iIGMConControlsPosX_HD = 270;
int iIGMConControlsPosY_HD = 350;
int iIGMConControlsTxtPosX_HD = 272;
int iIGMConControlsTxtPosY_HD = 360;
int iIGMConControlsTxtAlign_HD = 0;

//IGM HD XLMenu Coords
int iIGMMenuTxtPosX_HD = 60;
int iIGMMenuTxtPosY_HD = 80;
int iIGMMenuTxtAlign_HD = 0;
int iIGMStateScreenX_HD = 60;
int iIGMStateScreenY_HD = 270;
int iIGMStateScreenW_HD = 256;
int iIGMStateScreenH_HD = 143;

//RomList Coords
int iRomListPosX = 48;
int iRomListPosY = 48;
int GAMESEL_MaxWindowList = 11;//12
int iRomListAlign = 0;
int RomListTrunc = 43;
int MenuTrunc = 28;
int iControlsPosX = 436;
int iControlsPosY = 180;
int iControlsTxtPosX = 448; //430;
int iControlsTxtPosY = 180; //180;
int iControlsTxtAlign = 0;
int iRLBorderPosX = 33;
int iRLBorderPosY = 20;

//Control Panel Coords
int iConControlsPosX = 235;
int iConControlsPosY = 340;
int iConControlsTxtPosX = 272;
int iConControlsTxtPosY = 351;
int iConControlsTxtAlign = 0;
int iSynControlsPosX = 450;
int iSynControlsPosY = 20;
int iSynControlsTxtPosX = 465;
int iSynControlsTxtPosY = 21;
int iSynControlsTxtAlign = 0;
int iCredControlsPosX = 450;
int iCredControlsPosY = 380;
int iCredControlsTxtPosX = 465;
int iCredControlsTxtPosY = 393;
int iCredControlsTxtAlign = 0;

//Logo Coords (HD)
int iLogoPosX = 770;
int iLogoPosY = 300;

//XMVPlayer
int iMovieTop = 185;
int iMovieBottom = 290;
int iMovieLeft = 435;
int iMovieRight = 600;
int iLineSpacing = -5;

int DefaultPak = RumblePak;

//freakdave - now all controllers are enabled by default, users don't seem to know how to change it in Surreal's Menu
bool EnableController1 = true;
bool EnableController2 = true;
bool EnableController3 = true; // this was false in launcher config
bool EnableController4 = true; // this was false in launcher config

extern int actualrom;
int preferedemu=0;

void ResetDefaults()
{
	preferedemu=0;
	dw1964DynaMem=4;//8
	dw1964PagingMem=4;
	dwPJ64DynaMem=10;//16
	dwPJ64PagingMem=4;
	dwMaxVideoMem=4; //reinstate max free mem //5
	bUseRspAudio=false; // control a listing

	// ultrahle mem settings
	dwUltraCodeMem=5;
	dwUltraGroupMem=10;
	
	// not used anymore - leave for ini
	bUseLLERSP=false;
	bUseBasicAudio=false;

	DefaultPak = RumblePak;
	FlickerFilter = 1;
	TextureMode = 3;
	VertexMode = 2;
	VSync = INTERVAL_IMMEDIATE;
	AntiAliasMode = 0;
	SoftDisplayFilter = 0;
	FrameSkip = FALSE;
	bEnableHDTV = false;
	bFullScreen = false;
	EnableController1 = true;
	EnableController2 = true;
	EnableController3 = true; // this was false in launcher config
	EnableController4 = true; // this was false in launcher config
	XBOX_CONTROLLER_DEAD_ZONE = 8600;
	Deadzone = 26;
	Sensitivity = 10;
	videoplugin = _VideoPluginRice560; //2=rice560

	//plugins
	iAudioPlugin = _AudioPluginJttl; //2=jttl
	iRspPlugin = _RSPPluginHLE;
	
	int iPagingMethod=_PagingXXX;

	for (int i=0;i<4;i++) {
	for (int j=0;j<19;j++){
	ControllerConfig[(i*19)+j]=ControllerReset[j];}}
}

void ResetAppDefaults()
{
	//onhd
	//skinname
	HideLaunchScreens = false;
	EnableXMVPreview = false;
	EnableVideoAudio = false;
	EnableInfoPanel = true;
	EnableBGMusic = true;
	RandomBGMusic = false;
	bAudioBoost = false;
	
	sprintf(szPathRoms,			"D:\\Roms\\");
	sprintf(szPathMedia,		"D:\\Media\\");
	sprintf(szPathSkins,		"D:\\Skins\\");
	sprintf(szPathSaves,		"D:\\Saves\\");
	sprintf(szPathScreenshots,	"D:\\Screenshots\\");
}

// Read in the config file for the whole application
int ConfigAppLoad()
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(true);
	ini.SetMultiLine(false);
	ini.SetSpaces(false); // spaces before and after =

	char szIniFilename[64];
	sprintf(szIniFilename, "T:\\surreal-ce.ini");
	OutputDebugString(szIniFilename);
	rc = ini.LoadFile(szIniFilename);
	if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Load!\n");
		return 1; // write out default?
	}
	OutputDebugStringA(" Successfully Loaded!\n");

	//romcounter = ini.GetLongValue("Settings", "romcounter", 0 );
	sprintf(skinname, "%s", ini.GetValue("Settings", "skinname", "Default" ));
	onhd = ini.GetBoolValue("Settings", "onhd", true );
	HideLaunchScreens = ini.GetBoolValue("Settings", "HideLaunchScreens", false );
	EnableXMVPreview = ini.GetBoolValue("Settings", "EnableXMVPreview", false );
	EnableVideoAudio = ini.GetBoolValue("Settings", "EnableVideoAudio", false );
	EnableInfoPanel = ini.GetBoolValue("Settings", "EnableInfoPanel", true );
	EnableBGMusic = ini.GetBoolValue("Settings", "EnableBGMusic", true);
	RandomBGMusic = ini.GetBoolValue("Settings", "RandomBGMusic", false);
	bAudioBoost = ini.GetBoolValue("Settings", "AudioBoost", false);
	
	sprintf(szPathRoms, "%s", ini.GetValue("Settings", "PathRoms", szPathRoms ));
	sprintf(szPathMedia, "%s", ini.GetValue("Settings", "PathMedia", szPathMedia ));
	sprintf(szPathSkins, "%s", ini.GetValue("Settings", "PathSkins", szPathSkins ));
	sprintf(szPathSaves, "%s", ini.GetValue("Settings", "PathSaves", szPathSaves ));
	sprintf(szPathScreenshots, "%s", ini.GetValue("Settings", "PathScreenshots", szPathScreenshots ));

	return 0;
}

// Write out the config file for the whole application

int ConfigAppSave()
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
    ini.SetMultiKey(true);
    ini.SetMultiLine(false);
	ini.SetSpaces(false); // spaces before and after =
	
	//ini.SetLongValue("Settings", "romcounter", romcounter);
	ini.SetValue("Settings", "skinname", skinname);
	ini.SetBoolValue("Settings", "onhd", onhd);
	ini.SetBoolValue("Settings", "HideLaunchScreens", HideLaunchScreens);
	ini.SetBoolValue("Settings", "EnableXMVPreview", EnableXMVPreview);
	ini.SetBoolValue("Settings", "EnableVideoAudio", EnableVideoAudio);
	ini.SetBoolValue("Settings", "EnableInfoPanel", EnableInfoPanel);
	ini.SetBoolValue("Settings", "EnableBGMusic", EnableBGMusic);
	ini.SetBoolValue("Settings", "RandomBGMusic", RandomBGMusic);
	ini.SetBoolValue("Settings", "AudioBoost", bAudioBoost);
	
	ini.SetValue("Settings", "PathRoms", szPathRoms);
	ini.SetValue("Settings", "PathMedia", szPathMedia);
	ini.SetValue("Settings", "PathSkins", szPathSkins);
	ini.SetValue("Settings", "PathSaves", szPathSaves);
	ini.SetValue("Settings", "PathScreenshots", szPathScreenshots);
	
	char szIniFilename[64];
	sprintf(szIniFilename, "T:\\surreal-ce.ini");
	OutputDebugString(szIniFilename);
	rc = ini.SaveFile(szIniFilename);
    if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Save!\n");
		return 1; //return false;
	}
	OutputDebugStringA(" Saved Successfully!\n");

	return 0;
}


int ConfigAppSave2()
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
    ini.SetMultiKey(true);
    ini.SetMultiLine(false);
	ini.SetSpaces(false); // spaces before and after =
	
	ini.SetLongValue("Settings", "preferedemu", preferedemu);
	ini.SetLongValue("Settings", "videoplugin", videoplugin);
	ini.SetLongValue("Settings", "iAudioPlugin", iAudioPlugin);
	ini.SetLongValue("Settings", "iRspPlugin", iRspPlugin);
	
	ini.SetLongValue("Settings", "dw1964DynaMem", dw1964DynaMem);
	ini.SetLongValue("Settings", "dw1964PagingMem", dw1964PagingMem);
	ini.SetLongValue("Settings", "dwPJ64DynaMem", dwPJ64DynaMem);
	ini.SetLongValue("Settings", "dwPJ64PagingMem", dwPJ64PagingMem);
	ini.SetLongValue("Settings", "dwMaxVideoMem", dwMaxVideoMem); // reinstate max video mem
	ini.SetBoolValue("Settings", "bUseRspAudio", bUseRspAudio); // control a listing
	
	// ultrahle mem settings
	ini.SetLongValue("Settings", "dwUltraCodeMem", dwUltraCodeMem);
	ini.SetLongValue("Settings", "dwUltraGroupMem", dwUltraGroupMem);
	
	ini.SetBoolValue("Settings", "bUseLLERSP", bUseLLERSP); // leave for ini?
	
	//weinerschnitzel - Rompaging
	//ini.SetBoolValue("Settings", "usePageOriginal", usePageOriginal); // moving to main ini
	ini.SetLongValue("Settings", "iPagingMethod", iPagingMethod);
	
	ini.SetLongValue("Settings", "Sensitivity", Sensitivity);
	ini.SetLongValue("Settings", "DefaultPak", DefaultPak);
	ini.SetLongValue("Settings", "FlickerFilter", FlickerFilter);
	ini.SetLongValue("Settings", "TextureMode", TextureMode);
	ini.SetLongValue("Settings", "VertexMode", VertexMode);
	ini.SetLongValue("Settings", "VSync", VSync);
	ini.SetLongValue("Settings", "AntiAliasMode", AntiAliasMode);
	ini.SetBoolValue("Settings", "SoftDisplayFilter", SoftDisplayFilter);
	ini.SetBoolValue("Settings", "FrameSkip", FrameSkip);
	ini.SetBoolValue("Settings", "EnableController1", EnableController1);
	ini.SetBoolValue("Settings", "EnableController2", EnableController2);
	ini.SetBoolValue("Settings", "EnableController3", EnableController3);
	ini.SetBoolValue("Settings", "EnableController4", EnableController4);
	
	
	char szFloatBuf[64];
	
	sprintf(szFloatBuf, "%.6f", XBOX_CONTROLLER_DEAD_ZONE);
	ini.SetValue("Settings", "XBOX_CONTROLLER_DEAD_ZONE", szFloatBuf);
	
	sprintf(szFloatBuf, "%.6f", Deadzone);
	ini.SetValue("Settings", "Deadzone", szFloatBuf);
	
	// controller config
	{
		for (int i = 0; i < CONTROLLER_CONFIG_MAX; i++)
		{
			char var[22];
			sprintf(var, "ControllerConfig[%i]", i);
			ini.SetLongValue("Settings", var, ControllerConfig[i]);
		}
	}
		
	ini.SetBoolValue("Settings", "EnableHDTV", bEnableHDTV);
	ini.SetBoolValue("Settings", "FullScreen", bFullScreen);

	{
		char romsavedir[64];
		sprintf(romsavedir, "%s%s", szPathSaves, romCRC);
		if(!PathFileExists(romsavedir)) {
			if(!CreateDirectory(romsavedir, NULL)) {
				OutputDebugString("Error creating directory: ");
				OutputDebugString(romsavedir);
				OutputDebugString("\n");
				return 1;
			}
		}
	}
	
	char szIniFilename[64];
	sprintf(szIniFilename, "%s%s\\%s.ini", szPathSaves, romCRC, romCRC);
	OutputDebugString(szIniFilename);
	rc = ini.SaveFile(szIniFilename);
    if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Save!\n");
		return 1; //return false;
	}
	OutputDebugStringA(" Saved Successfully!\n");

	return 0;
}


int ConfigAppLoad2()
{
	// rom specific settings (xxxxxxxx.ini)
	// if you want it backward compatible, check for old ini here 1st
	// get the vars from old file and set them in code instead of parsing it with simpleini
	// let the new save write out a new ini with those updated values the next time it's called from menu
	// just need old vars set in code and then return

	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(true);
	ini.SetMultiLine(false);
	ini.SetSpaces(false); // spaces before and after =

	char szIniFilename[64];
	sprintf(szIniFilename, "%s%s\\%s.ini", szPathSaves, romCRC, romCRC);
	OutputDebugString(szIniFilename);
	rc = ini.LoadFile(szIniFilename);
	if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Load!\n");
		return 1;
	}
	OutputDebugStringA(" Successfully Loaded!\n");

	// need to double check the defaults
	// just setting them to whatever they were for now if it's not found
	preferedemu = ini.GetLongValue("Settings", "preferedemu", preferedemu );
	videoplugin = ini.GetLongValue("Settings", "videoplugin", videoplugin );
	iAudioPlugin = ini.GetLongValue("Settings", "iAudioPlugin", iAudioPlugin );
	iRspPlugin = ini.GetLongValue("Settings", "iRspPlugin", iRspPlugin );
	
	dw1964DynaMem = ini.GetLongValue("Settings", "dw1964DynaMem", dw1964DynaMem );
	dw1964PagingMem = ini.GetLongValue("Settings", "dw1964PagingMem", dw1964PagingMem );
	dwPJ64DynaMem = ini.GetLongValue("Settings", "dwPJ64DynaMem", dwPJ64DynaMem );
	dwPJ64PagingMem = ini.GetLongValue("Settings", "dwPJ64PagingMem", dwPJ64PagingMem );
	dwMaxVideoMem = ini.GetLongValue("Settings", "dwMaxVideoMem", dwMaxVideoMem ); // reinstate max video mem
	
	bUseRspAudio = ini.GetBoolValue("Settings", "bUseRspAudio", bUseRspAudio ); // control a listing
	
	// ultrahle mem settings
	dwUltraCodeMem = ini.GetLongValue("Settings", "dwUltraCodeMem", dwUltraCodeMem );
	dwUltraGroupMem = ini.GetLongValue("Settings", "dwUltraGroupMem", dwUltraGroupMem );

	bUseLLERSP = ini.GetBoolValue("Settings", "bUseLLERSP", bUseLLERSP ); // leave for ini?
	
	//weinerschnitzel - Rompaging
	//usePageOriginal = ini.GetBoolValue("Settings", "usePageOriginal", usePageOriginal ); // moving to main ini
	iPagingMethod = ini.GetLongValue("Settings", "iPagingMethod", iPagingMethod );
	
	Sensitivity = ini.GetLongValue("Settings", "Sensitivity", Sensitivity );
	DefaultPak = ini.GetLongValue("Settings", "DefaultPak", DefaultPak );
	FlickerFilter = ini.GetLongValue("Settings", "FlickerFilter", FlickerFilter );
	TextureMode = ini.GetLongValue("Settings", "TextureMode", TextureMode );
	VertexMode = ini.GetLongValue("Settings", "VertexMode", VertexMode );
	VSync = ini.GetLongValue("Settings", "VSync", VSync );
	AntiAliasMode = ini.GetLongValue("Settings", "AntiAliasMode", AntiAliasMode );
	SoftDisplayFilter = ini.GetBoolValue("Settings", "SoftDisplayFilter", SoftDisplayFilter );
	FrameSkip = ini.GetBoolValue("Settings", "FrameSkip", FrameSkip );
	EnableController1 = ini.GetBoolValue("Settings", "EnableController1", EnableController1 );
	EnableController2 = ini.GetBoolValue("Settings", "EnableController2", EnableController2 );
	EnableController3 = ini.GetBoolValue("Settings", "EnableController3", EnableController3 );
	EnableController4 = ini.GetBoolValue("Settings", "EnableController4", EnableController4 );
	
	char szFloatBuf[64];
	
	sprintf(szFloatBuf, "%s", ini.GetValue("Settings", "XBOX_CONTROLLER_DEAD_ZONE", "" ));
	if (szFloatBuf != "")
		XBOX_CONTROLLER_DEAD_ZONE = (float) atof(szFloatBuf);
	
	sprintf(szFloatBuf, "%s", ini.GetValue("Settings", "Deadzone", "" ));
	if (szFloatBuf != "")
		Deadzone = (float) atof(szFloatBuf);

	// controller config
	{
		for (int i = 0; i < CONTROLLER_CONFIG_MAX; i++)
		{
			char var[22];
			sprintf(var, "ControllerConfig[%i]", i);
			ControllerConfig[i] = static_cast<byte>( ini.GetLongValue("Settings", var, 00 ) ); // should be byte or int? it's byte in hash
		}
	}

	//HD Emu Settings
	bEnableHDTV = ini.GetBoolValue("Settings", "EnableHDTV", bEnableHDTV);
	bFullScreen = ini.GetBoolValue("Settings", "FullScreen", bFullScreen);

	return 0;
}


int ConfigAppSaveTemp()
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
    ini.SetMultiKey(true);
    ini.SetMultiLine(false);
	ini.SetSpaces(false); // spaces before and after =
	
	ini.SetValue("History", "romname", romname);
	ini.SetValue("History", "romCRC", romCRC);
	
	char szIniFilename[64];
	sprintf(szIniFilename, "T:\\Tmp.ini");
	OutputDebugString(szIniFilename);
	rc = ini.SaveFile(szIniFilename);
    if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Save!\n");
		return 1; //return false;
	}
	OutputDebugStringA(" Saved Successfully!\n");

	return 0;
}


int ConfigAppLoadTemp()
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(true);
	ini.SetMultiLine(false);
	ini.SetSpaces(false); // spaces before and after =

	char szIniFilename[64];
	sprintf(szIniFilename, "T:\\Tmp.ini");
	OutputDebugString(szIniFilename);
	rc = ini.LoadFile(szIniFilename);
	if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Load!\n");
		return 1;
	}
	OutputDebugStringA(" Successfully Loaded\n");

	sprintf(romname, "%s", ini.GetValue("History", "romname", "" ));
	sprintf(romCRC, "%s", ini.GetValue("History", "romCRC", "" ));

	return 0;
}

//Load Skin File
void LoadSkinFile(){

	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(true);
	ini.SetMultiLine(false);
	ini.SetSpaces(true); // spaces before and after =

	char szIniFilename[64];
	
	sprintf(szIniFilename, "%s%s\\Skin.ini", szPathSkins, skinname); // try the custom dir
	if(!PathFileExists(szIniFilename)) {
		sprintf(szIniFilename, "T:\\Skins\\%s\\Skin.ini", skinname); // then try T
		if(!PathFileExists(szIniFilename)) {
			sprintf(szIniFilename, "D:\\Skins\\%s\\Skin.ini", skinname); // then try D
		}
	}

	OutputDebugString(szIniFilename);
	rc = ini.LoadFile(szIniFilename);
	if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Load!\n");
		WriteSkinFile();
		return;
	}
	//OutputDebugStringA(" Successfully Loaded!\n");

	char szDwordBuf[16];
	char szDwordDefBuf[16];

	sprintf(szDwordDefBuf, "%08X", dwTitleColor);
	sprintf(szDwordBuf, "%s", ini.GetValue("Font Colors", "TitleColor", szDwordDefBuf ));
	dwTitleColor = strtoul(szDwordBuf, NULL, 16);
	
	sprintf(szDwordDefBuf, "%08X", dwMenuTitleColor);
	sprintf(szDwordBuf, "%s", ini.GetValue("Font Colors", "MenuTitleColor", szDwordDefBuf ));
	dwMenuTitleColor = strtoul(szDwordBuf, NULL, 16);
	
	sprintf(szDwordDefBuf, "%08X", dwIGMTitleColor);
	sprintf(szDwordBuf, "%s", ini.GetValue("Font Colors", "IGMTitleColor", szDwordDefBuf ));
	dwIGMTitleColor = strtoul(szDwordBuf, NULL, 16);
	
	sprintf(szDwordDefBuf, "%08X", dwRomListColor);
	sprintf(szDwordBuf, "%s", ini.GetValue("Font Colors", "RomListColor", szDwordDefBuf ));
	dwRomListColor = strtoul(szDwordBuf, NULL, 16);
	
	sprintf(szDwordDefBuf, "%08X", dwSelectedRomColor);
	sprintf(szDwordBuf, "%s", ini.GetValue("Font Colors", "SelectedRomColor", szDwordDefBuf ));
	dwSelectedRomColor = strtoul(szDwordBuf, NULL, 16);
	
	sprintf(szDwordDefBuf, "%08X", dwMenuItemColor);
	sprintf(szDwordBuf, "%s", ini.GetValue("Font Colors", "MenuItemColor", szDwordDefBuf ));
	dwMenuItemColor = strtoul(szDwordBuf, NULL, 16);
	
	sprintf(szDwordDefBuf, "%08X", dwNullItemColor);
	sprintf(szDwordBuf, "%s", ini.GetValue("Font Colors", "NullItemColor", szDwordDefBuf ));
	dwNullItemColor = strtoul(szDwordBuf, NULL, 16);

	
	iTitleX = ini.GetLongValue("General", "TitleX", iTitleX);
	iTitleY = ini.GetLongValue("General", "TitleY", iTitleY);
	iLineSpacing = ini.GetLongValue("General", "LineSpacing", iLineSpacing);
	// should go here?
	iTempMessagePosX = ini.GetLongValue("General", "TempMessagePosX", iTempMessagePosX);
	iTempMessagePosY = ini.GetLongValue("General", "TempMessagePosY", iTempMessagePosY);
	iTempMessageAlign = ini.GetLongValue("General", "TempMessageAlign", iTempMessageAlign);

	iMainMenuPosX = ini.GetLongValue("Main Menu", "MainMenuPosX", iMainMenuPosX);
	iMainMenuPosY = ini.GetLongValue("Main Menu", "MainMenuPosY", iMainMenuPosY);
	iMainMenuTxtPosX = ini.GetLongValue("Main Menu", "MainMenuTxtPosX", iMainMenuTxtPosX);
	iMainMenuTxtPosY = ini.GetLongValue("Main Menu", "MainMenuTxtPosY", iMainMenuTxtPosY);
	iMainMenuTxtAlign = ini.GetLongValue("Main Menu", "MainMenuTxtAlign", iMainMenuTxtAlign);
	MenuTrunc = ini.GetLongValue("Main Menu", "MenuCharacterLimit", MenuTrunc);
	iMainHilightPosX = ini.GetLongValue("Main Menu", "HilightPosX", iMainHilightPosX);

	iLaunchMenuBgPosX = ini.GetLongValue("Launch Menu", "LaunchMenuPosX", iLaunchMenuBgPosX);
	iLaunchMenuBgPosY = ini.GetLongValue("Launch Menu", "LaunchMenuPosY", iLaunchMenuBgPosY);
	iLaunchMenuTxtPosX = ini.GetLongValue("Launch Menu", "LaunchMenuTxtPosX", iLaunchMenuTxtPosX);
	iLaunchMenuTxtPosY = ini.GetLongValue("Launch Menu", "LaunchMenuTxtPosY", iLaunchMenuTxtPosY);
	iLaunchMenuTxtAlign = ini.GetLongValue("Launch Menu", "LaunchMenuTxtAlign", iLaunchMenuTxtAlign);
	iLaunchHilightPosX = ini.GetLongValue("Launch Menu", "LaunchHilightPosX", iLaunchHilightPosX);

	iInfoPosX = ini.GetLongValue("Info Panel", "InfoPosX", iInfoPosX);
	iInfoPosY = ini.GetLongValue("Info Panel", "InfoPosY", iInfoPosY);
	iInfoTxtPosX = ini.GetLongValue("Info Panel", "InfoTxtPosX", iInfoTxtPosX);
	iInfoTxtPosY = ini.GetLongValue("Info Panel", "InfoTxtPosY", iInfoTxtPosY);
	iInfoTxtAlign = ini.GetLongValue("Info Panel", "InfoTxtAlign", iInfoTxtAlign);

	iInfoTxtControlPosX = ini.GetLongValue("Rom List Controls", "InfoTxtControlPosX", iInfoTxtControlPosX);
	iInfoTxtControlPosY = ini.GetLongValue("Rom List Controls", "InfoTxtControlPosY", iInfoTxtControlPosY);
	iInfoTxtControlAlign = ini.GetLongValue("Rom List Controls", "InfoTxtControlAlign", iInfoTxtControlAlign);
	bShowRLControls = ini.GetBoolValue("Rom List Controls", "ShowControls", bShowRLControls);
	
	char szFloatBuf[64];
	sprintf(szFloatBuf, "%s", ini.GetValue("Info Panel", "InfoPanelTrunc", "" ));
	if (szFloatBuf != "")
		InfoPanelTrunc = (float) atof(szFloatBuf);

	sprintf(BoxartName, "%s", ini.GetValue("Box Art", "BoxartName", BoxartName ));
	
	iBoxPanelPosX = ini.GetLongValue("Box Art", "BoxArtPanelPosX", iBoxPanelPosX);
	iBoxPanelPosY = ini.GetLongValue("Box Art", "BoxArtPanelPosY", iBoxPanelPosY);
	iBoxPosX = ini.GetLongValue("Box Art", "BoxPosX", iBoxPosX);
	iBoxPosY = ini.GetLongValue("Box Art", "BoxPosY", iBoxPosY);
	iBoxWidth = ini.GetLongValue("Box Art", "BoxWidth", iBoxWidth);
	iBoxHeight = ini.GetLongValue("Box Art", "BoxHeight", iBoxHeight);

	iMovieLeft = ini.GetLongValue("Video Preview", "MovieLeft", iMovieLeft);
	iMovieTop = ini.GetLongValue("Video Preview", "MovieTop", iMovieTop);
	iMovieRight = ini.GetLongValue("Video Preview", "MovieRight", iMovieRight);
	iMovieBottom = ini.GetLongValue("Video Preview", "MovieBottom", iMovieBottom);
	
	iControlConfigPosX = ini.GetLongValue("Control Config", "ControlConfigPosX", iControlConfigPosX);
	iControlConfigPosY = ini.GetLongValue("Control Config", "ControlConfigPosY", iControlConfigPosY);
	iControlConfigWidth = ini.GetLongValue("Control Config", "ControlConfigWidth", iControlConfigWidth);
	iControlsSpacing= ini.GetLongValue("Control Config", "ControlConfigSpacing", iControlsSpacing);
	iControlConfigTxtPosX = ini.GetLongValue("Control Config", "ControlConfigTxtPosX", iControlConfigTxtPosX);
	iControlConfigTxtPosY = ini.GetLongValue("Control Config", "ControlConfigTxtPosY", iControlConfigTxtPosY);
	iControlConfigTxtPadLX = ini.GetLongValue("Control Config", "ControlConfigTxtPadLX", iControlConfigTxtPadLX);
	iConControlsPosX = ini.GetLongValue("Control Config", "ControlsPosX", iConControlsPosX);
	iConControlsPosY = ini.GetLongValue("Control Config", "ControlsPosY", iConControlsPosY);
	iConControlsTxtPosX = ini.GetLongValue("Control Config", "ControlsTxtPosX", iConControlsTxtPosX);
	iConControlsTxtPosY = ini.GetLongValue("Control Config", "ControlsTxtPosY", iConControlsTxtPosY);
	iConControlsTxtAlign = ini.GetLongValue("Control Config", "ControlsTxtAlign", iConControlsTxtAlign);

	iCredControlsPosX = ini.GetLongValue("Credits", "ControlsPosX", iCredControlsPosX);
	iCredControlsPosY = ini.GetLongValue("Credits", "ControlsPosY", iCredControlsPosY);
	iCredControlsTxtPosX = ini.GetLongValue("Credits", "ControlsTxtPosX", iCredControlsTxtPosX);
	iCredControlsTxtPosY = ini.GetLongValue("Credits", "ControlsTxtPosY", iCredControlsTxtPosY);
	iCredControlsTxtAlign = ini.GetLongValue("Credits", "ControlsTxtAlign", iCredControlsTxtAlign);

	iRLBorderPosX = ini.GetLongValue("Rom List", "RomListBorderPosX", iRLBorderPosX);
	iRLBorderPosY = ini.GetLongValue("Rom List", "RomListBorderPosY", iRLBorderPosY);
	iRomListPosX = ini.GetLongValue("Rom List", "RomListPosX", iRomListPosX);
	iRomListPosY = ini.GetLongValue("Rom List", "RomListPosY", iRomListPosY);
	GAMESEL_MaxWindowList = ini.GetLongValue("Rom List", "RomListSize", GAMESEL_MaxWindowList);
	iRomListAlign = ini.GetLongValue("Rom List", "RomListAlign", iRomListAlign);
	RomListTrunc = ini.GetLongValue("Rom List", "RomListCharacterLimit", RomListTrunc );

	iControlsPosX = ini.GetLongValue("Controls Panel", "ControlsPanelPosX", iControlsPosX);
	iControlsPosY = ini.GetLongValue("Controls Panel", "ControlsPanelPosY", iControlsPosY);
	iControlsTxtPosX = ini.GetLongValue("Controls Panel", "ControlsTxtPosX", iControlsTxtPosX);
	iControlsTxtPosY = ini.GetLongValue("Controls Panel", "ControlsTxtPosY", iControlsTxtPosY);
	iControlsTxtAlign = ini.GetLongValue("Controls Panel", "ControlsTxtAlign", iControlsTxtAlign);
	
	iLogoPosX = ini.GetLongValue("Logo", "LogoPosX", iLogoPosX);
	iLogoPosY = ini.GetLongValue("Logo", "LogoPosY", iLogoPosY);

	iSynopsisPosX = ini.GetLongValue("Synopsis", "SynopsisPosX", iSynopsisPosX);
	iSynopsisPosY = ini.GetLongValue("Synopsis", "SynopsisPosY", iSynopsisPosY);
	iSynopsisLines = ini.GetLongValue("Synopsis", "SynopsisLines", iSynopsisLines);
	iSynopsisWrap = ini.GetLongValue("Synopsis", "SynopsisWrap", iSynopsisWrap);
	iSynopsisAlign = ini.GetLongValue("Synopsis", "SynopsisAlign", iSynopsisAlign);
	iSynControlsPosX = ini.GetLongValue("Synopsis", "ControlsPosX", iSynControlsPosX);
	iSynControlsPosY = ini.GetLongValue("Synopsis", "ControlsPosY", iSynControlsPosY);
	iSynControlsTxtPosX = ini.GetLongValue("Synopsis", "ControlsTxtPosX", iSynControlsTxtPosX);
	iSynControlsTxtPosY = ini.GetLongValue("Synopsis", "ControlsTxtPosY", iSynControlsTxtPosY);
	iSynControlsTxtAlign = ini.GetLongValue("Synopsis", "ControlsTxtAlign", iSynControlsTxtAlign);

	//Loading Panel
	iLoadBoxPosX = ini.GetLongValue("Loading Screen", "BoxArtPosX", iLoadBoxPosX);
	iLoadBoxPosY = ini.GetLongValue("Loading Screen", "BoxArtPosY", iLoadBoxPosY);
	iLoadPanelPosX = ini.GetLongValue("Loading Screen", "LoadPanelPosX", iLoadPanelPosX);
	iLoadPanelPosY = ini.GetLongValue("Loading Screen", "LoadPanelPosY", iLoadPanelPosY);
	iLoadPanelTxtPosX = ini.GetLongValue("Loading Screen", "LoadingTxtPosX", iLoadPanelTxtPosX);
	iLoadPanelTxtPosY = ini.GetLongValue("Loading Screen", "LoadingTxtPosY", iLoadPanelTxtPosY);
	iLoadPanelTxtAlign = ini.GetLongValue("Loading Screen", "LoadingTxtAlign", iLoadPanelTxtAlign);
	iLoadPanelBarPosX = ini.GetLongValue("Loading Screen", "LoadingBarPosX", iLoadPanelBarPosX);
	iLoadPanelBarPosY = ini.GetLongValue("Loading Screen", "LoadingBarPosY", iLoadPanelBarPosY);
	iLoadPanelTrunc = ini.GetLongValue("Loading Screen", "LoadPanelTrunc", iLoadPanelTrunc);

	// In Game Menu

	iIGMTitleX = ini.GetLongValue("IGM Title", "IGMTitleX", iIGMTitleX);
	iIGMTitleY = ini.GetLongValue("IGM Title", "IGMTitleY", iIGMTitleY);

	iIGMMenuPosX = ini.GetLongValue("IGM Menu", "IGMMenuPosX", iIGMMenuPosX);
	iIGMMenuPosY = ini.GetLongValue("IGM Menu", "IGMMenuPosY", iIGMMenuPosY);
	iIGMHilightPosX = ini.GetLongValue("IGM Menu", "IGMHilightPosX", iIGMHilightPosX);
	iIGMMenuTxtPosX = ini.GetLongValue("IGM Menu", "IGMMenuTxtPosX", iIGMMenuTxtPosX);
	iIGMMenuTxtPosY = ini.GetLongValue("IGM Menu", "IGMMenuTxtPosY", iIGMMenuTxtPosY);
	iIGMMenuTxtAlign = ini.GetLongValue("IGM Menu", "IGMMenuTxtAlign", iIGMMenuTxtAlign);
	iIGMStateScreenX = ini.GetLongValue("IGM Menu", "IGMStateScreenX", iIGMStateScreenX);
	iIGMStateScreenY = ini.GetLongValue("IGM Menu", "IGMStateScreenY", iIGMStateScreenY);
	iIGMStateScreenW = ini.GetLongValue("IGM Menu", "IGMStateScreenW", iIGMStateScreenW);
	iIGMStateScreenH = ini.GetLongValue("IGM Menu", "IGMStateScreenH", iIGMStateScreenH);

	iControlsSpacingIGM = ini.GetLongValue("IGM Control Config", "ControlConfigSpacing", iControlsSpacingIGM);
	iIGMControlConfigCenterX = ini.GetLongValue("IGM Control Config", "ControlConfigCenterX", iIGMControlConfigCenterX);
	iIGMControlConfigWidth = ini.GetLongValue("IGM Control Config", "ControlConfigWidth", iIGMControlConfigWidth);
	iIGMControlConfigTop = ini.GetLongValue("IGM Control Config", "ControlConfigTop", iIGMControlConfigTop);
	iIGMControlConfigBGPosX = ini.GetLongValue("IGM Control Config", "ControlConfigBGPosX", iIGMControlConfigBGPosX);
	iIGMControlConfigBGPosY = ini.GetLongValue("IGM Control Config", "ControlConfigBGPosY", iIGMControlConfigBGPosY);
//	iIGMControlConfigTxtPosX = ini.GetLongValue("IGM Control Config", "ControlConfigTxtPosX", iIGMControlConfigTxtPosX);
//	iIGMControlConfigTxtPosY = ini.GetLongValue("IGM Control Config", "ControlConfigTxtPosY", iIGMControlConfigTxtPosY);
	iIGMControlConfigTxtPadLX = ini.GetLongValue("IGM Control Config", "ControlConfigTxtPadLX", iIGMControlConfigTxtPadLX);	
	iIGMConControlsPosX = ini.GetLongValue("IGM Control Config", "ControlsPosX", iIGMConControlsPosX);
	iIGMConControlsPosY = ini.GetLongValue("IGM Control Config", "ControlsPosY", iIGMConControlsPosY);
	iIGMConControlsTxtPosX = ini.GetLongValue("IGM Control Config", "ControlsTxtPosX", iIGMConControlsTxtPosX);
	iIGMConControlsTxtPosY = ini.GetLongValue("IGM Control Config", "ControlsTxtPosY", iIGMConControlsTxtPosY);
	iIGMConControlsTxtAlign = ini.GetLongValue("IGM Control Config", "ControlsTxtAlign", iIGMConControlsTxtAlign);

	iPanelX = ini.GetLongValue("IGM Screenshot", "PanelX", iPanelX);
	iPanelY = ini.GetLongValue("IGM Screenshot", "PanelY", iPanelY);
	iPanelNW = ini.GetLongValue("IGM Screenshot", "PanelNW", iPanelNW);
	iPanelNH = ini.GetLongValue("IGM Screenshot", "PanelNH", iPanelNH);
	
	// In Game Menu HD

	iIGMTitleX_HD = ini.GetLongValue("IGM HD Title", "IGMTitleX_HD", iIGMTitleX_HD);
	iIGMTitleY_HD = ini.GetLongValue("IGM HD Title", "IGMTitleY_HD", iIGMTitleY_HD);

	iIGMMenuPosX_HD = ini.GetLongValue("IGM HD Menu", "IGMMenuPosX_HD", iIGMMenuPosX_HD);
	iIGMMenuPosY_HD = ini.GetLongValue("IGM HD Menu", "IGMMenuPosY_HD", iIGMMenuPosY_HD);
	iIGMHilightPosX_HD = ini.GetLongValue("IGM HD Menu", "IGMHilightPosX_HD", iIGMHilightPosX_HD);
	iIGMMenuTxtPosX_HD = ini.GetLongValue("IGM HD Menu", "IGMMenuTxtPosX_HD", iIGMMenuTxtPosX_HD);
	iIGMMenuTxtPosY_HD = ini.GetLongValue("IGM HD Menu", "IGMMenuTxtPosY_HD", iIGMMenuTxtPosY_HD);
	iIGMMenuTxtAlign_HD = ini.GetLongValue("IGM HD Menu", "IGMMenuTxtAlign_HD", iIGMMenuTxtAlign_HD);
	iIGMStateScreenX_HD = ini.GetLongValue("IGM HD Menu", "IGMStateScreenX_HD", iIGMStateScreenX_HD);
	iIGMStateScreenY_HD = ini.GetLongValue("IGM HD Menu", "IGMStateScreenY_HD", iIGMStateScreenY_HD);
	iIGMStateScreenW_HD = ini.GetLongValue("IGM HD Menu", "IGMStateScreenW_HD", iIGMStateScreenW_HD);
	iIGMStateScreenH_HD = ini.GetLongValue("IGM HD Menu", "IGMStateScreenH_HD", iIGMStateScreenH_HD);	

	iControlsSpacingIGM_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigSpacing_HD", iControlsSpacingIGM_HD);
	iIGMControlConfigCenterX_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigCenterX_HD", iIGMControlConfigCenterX_HD);
	iIGMControlConfigWidth_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigWidth_HD", iIGMControlConfigWidth_HD);
	iIGMControlConfigTop_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigTop_HD", iIGMControlConfigTop_HD);
	iIGMControlConfigBGPosX_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigBGPosX_HD", iIGMControlConfigBGPosX_HD);
	iIGMControlConfigBGPosY_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigBGPosY_HD", iIGMControlConfigBGPosY_HD);
//	iIGMControlConfigTxtPosX_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigTxtPosX_HD", iIGMControlConfigTxtPosX_HD);
//	iIGMControlConfigTxtPosY_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigTxtPosY_HD", iIGMControlConfigTxtPosY_HD);
	iIGMControlConfigTxtPadLX_HD = ini.GetLongValue("IGM HD Control Config", "ControlConfigTxtPadLX_HD", iIGMControlConfigTxtPadLX_HD);	
	iIGMConControlsPosX_HD = ini.GetLongValue("IGM HD Control Config", "ControlsPosX_HD", iIGMConControlsPosX_HD);
	iIGMConControlsPosY_HD = ini.GetLongValue("IGM HD Control Config", "ControlsPosY_HD", iIGMConControlsPosY_HD);
	iIGMConControlsTxtPosX_HD = ini.GetLongValue("IGM HD Control Config", "ControlsTxtPosX_HD", iIGMConControlsTxtPosX_HD);
	iIGMConControlsTxtPosY_HD = ini.GetLongValue("IGM HD Control Config", "ControlsTxtPosY_HD", iIGMConControlsTxtPosY_HD);
	iIGMConControlsTxtAlign_HD = ini.GetLongValue("IGM HD Control Config", "ControlsTxtAlign_HD", iIGMConControlsTxtAlign_HD);

	iPanelX_HD = ini.GetLongValue("IGM HD Screenshot", "PanelX_HD", iPanelX_HD);
	iPanelY_HD = ini.GetLongValue("IGM HD Screenshot", "PanelY_HD", iPanelY_HD);
	iPanelNW_HD = ini.GetLongValue("IGM HD Screenshot", "PanelNW_HD", iPanelNW_HD);
	iPanelNH_HD = ini.GetLongValue("IGM HD Screenshot", "PanelNH_HD", iPanelNH_HD);
	
	
	// fixed-ratio - force proper ratio
	if (iBoxWidth > iBoxHeight) { // && (iBoxHeight < (iBoxWidth * 0.75f)) // to keep it from overflowing if it's almost square
		iBoxWidthAuto = iBoxWidth;
		iBoxHeightAuto = iBoxWidth;
	} else {
		iBoxWidthAuto = iBoxHeight;
		iBoxHeightAuto = iBoxHeight;
	}
	
	// auto-ratio - allow improper ratio
	/*float fRatio = (640.0f / 480.0f);
	if (iBoxWidth > iBoxHeight) {
		iBoxWidthAuto = iBoxWidth;
		iBoxHeightAuto = (iBoxHeight * fRatio);
	} else {
		iBoxWidthAuto = (iBoxWidth * fRatio);
		iBoxHeightAuto = iBoxHeight;
	}*/
	
	OutputDebugStringA(" Successfully Loaded!\n");
	
	return;
}

void WriteSkinFile(){

	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
    ini.SetMultiKey(true);
    ini.SetMultiLine(false);
	ini.SetSpaces(true); // spaces before and after =
	

	char szDwordBuf[16];
	
	sprintf(szDwordBuf, "%08X", dwTitleColor);
	ini.SetValue("Font Colors", "TitleColor", szDwordBuf); //0xFF53B77F

	sprintf(szDwordBuf, "%08X", dwIGMTitleColor);
	ini.SetValue("Font Colors", "IGMTitleColor", szDwordBuf); //0x33777777
	
	sprintf(szDwordBuf, "%08X", dwMenuTitleColor);
	ini.SetValue("Font Colors", "MenuTitleColor", szDwordBuf); //0xFF8080FF
	
	sprintf(szDwordBuf, "%08X", dwRomListColor);
	ini.SetValue("Font Colors", "RomListColor", szDwordBuf); //0xAAEEEEEE
	
	sprintf(szDwordBuf, "%08X", dwSelectedRomColor);
	ini.SetValue("Font Colors", "SelectedRomColor", szDwordBuf); //0xFFFF8000
	
	sprintf(szDwordBuf, "%08X", dwMenuItemColor);
	ini.SetValue("Font Colors", "MenuItemColor", szDwordBuf); //0xCCEEEEEE
	
	sprintf(szDwordBuf, "%08X", dwNullItemColor);
	ini.SetValue("Font Colors", "NullItemColor", szDwordBuf); //0xEE53B77F

	ini.SetLongValue("General", "TitleX", iTitleX);
	ini.SetLongValue("General", "TitleY", iTitleY);
	ini.SetLongValue("General", "LineSpacing", iLineSpacing);
	// should go here?
	ini.SetLongValue("General", "TempMessagePosX", iTempMessagePosX);
	ini.SetLongValue("General", "TempMessagePosY", iTempMessagePosY);
	ini.SetLongValue("General", "TempMessageAlign", iTempMessageAlign);

	ini.SetLongValue("Main Menu", "MainMenuPosX", iMainMenuPosX);
	ini.SetLongValue("Main Menu", "MainMenuPosY", iMainMenuPosY);
	ini.SetLongValue("Main Menu", "MainMenuTxtPosX", iMainMenuTxtPosX);
	ini.SetLongValue("Main Menu", "MainMenuTxtPosY", iMainMenuTxtPosY);
	ini.SetLongValue("Main Menu", "MainMenuTxtAlign", iMainMenuTxtAlign);
	ini.SetLongValue("Main Menu", "MenuCharacterLimit", MenuTrunc);
	ini.SetLongValue("Main Menu", "HilightPosX", iMainHilightPosX);

	ini.SetLongValue("Launch Menu", "LaunchMenuPosX", iLaunchMenuBgPosX);
	ini.SetLongValue("Launch Menu", "LaunchMenuPosY", iLaunchMenuBgPosY);
	ini.SetLongValue("Launch Menu", "LaunchMenuTxtPosX", iLaunchMenuTxtPosX);
	ini.SetLongValue("Launch Menu", "LaunchMenuTxtPosY", iLaunchMenuTxtPosY);
	ini.SetLongValue("Launch Menu", "LaunchMenuTxtAlign", iLaunchMenuTxtAlign);	
	ini.SetLongValue("Launch Menu", "LaunchHilightPosX", iLaunchHilightPosX);
	
	ini.SetLongValue("Info Panel", "InfoPosX", iInfoPosX);
	ini.SetLongValue("Info Panel", "InfoPosY", iInfoPosY);
	ini.SetLongValue("Info Panel", "InfoTxtPosX", iInfoTxtPosX);
	ini.SetLongValue("Info Panel", "InfoTxtPosY", iInfoTxtPosY);
	ini.SetLongValue("Info Panel", "InfoTxtAlign", iInfoTxtAlign);

	ini.SetLongValue("Rom List Controls", "InfoTxtControlPosX", iInfoTxtControlPosX);
	ini.SetLongValue("Rom List Controls", "InfoTxtControlPosY", iInfoTxtControlPosY);
	ini.SetLongValue("Rom List Controls", "InfoTxtControlAlign", iInfoTxtControlAlign);
	ini.SetBoolValue("Rom List Controls", "ShowControls", bShowRLControls);
	
	char szFloatBuf[64];
	sprintf(szFloatBuf, "%.0f", InfoPanelTrunc);
	ini.SetValue("Info Panel", "InfoPanelTrunc", szFloatBuf);

	ini.SetValue("Box Art", "BoxartName", BoxartName);
	ini.SetLongValue("Box Art", "BoxArtPanelPosX", iBoxPanelPosX);
	ini.SetLongValue("Box Art", "BoxArtPanelPosY", iBoxPanelPosY);
	ini.SetLongValue("Box Art", "BoxPosX", iBoxPosX);
	ini.SetLongValue("Box Art", "BoxPosY", iBoxPosY);
	ini.SetLongValue("Box Art", "BoxWidth", iBoxWidth);
	ini.SetLongValue("Box Art", "BoxHeight", iBoxHeight);

	ini.SetLongValue("Video Preview", "MovieLeft", iMovieLeft);
	ini.SetLongValue("Video Preview", "MovieTop", iMovieTop);
	ini.SetLongValue("Video Preview", "MovieRight", iMovieRight);
	ini.SetLongValue("Video Preview", "MovieBottom", iMovieBottom);
	
	ini.SetLongValue("Control Config", "ControlConfigPosX", iControlConfigPosX);
	ini.SetLongValue("Control Config", "ControlConfigPosY", iControlConfigPosY);
	ini.SetLongValue("Control Config", "ControlConfigWidth", iControlConfigWidth);
	ini.SetLongValue("Control Config", "ControlConfigSpacing", iControlsSpacing);
	ini.SetLongValue("Control Config", "ControlConfigTxtPosX", iControlConfigTxtPosX);
	ini.SetLongValue("Control Config", "ControlConfigTxtPosY", iControlConfigTxtPosY);
	ini.SetLongValue("Control Config", "ControlConfigTxtPadLX", iControlConfigTxtPadLX);
	ini.SetLongValue("Control Config", "ControlsPosX", iConControlsPosX);
	ini.SetLongValue("Control Config", "ControlsPosY", iConControlsPosY);
	ini.SetLongValue("Control Config", "ControlsTxtPosX", iConControlsTxtPosX);
	ini.SetLongValue("Control Config", "ControlsTxtPosY", iConControlsTxtPosY);
	ini.SetLongValue("Control Config", "ControlsTxtAlign", iConControlsTxtAlign);
	
	ini.SetLongValue("Credits", "ControlsPosX", iCredControlsPosX);
	ini.SetLongValue("Credits", "ControlsPosY", iCredControlsPosY);
	ini.SetLongValue("Credits", "ControlsTxtPosX", iCredControlsTxtPosX);
	ini.SetLongValue("Credits", "ControlsTxtPosY", iCredControlsTxtPosY);
	ini.SetLongValue("Credits", "ControlsTxtAlign", iCredControlsTxtAlign);

	ini.SetLongValue("Rom List", "RomListBorderPosX", iRLBorderPosX);
	ini.SetLongValue("Rom List", "RomListBorderPosY", iRLBorderPosY);
	ini.SetLongValue("Rom List", "RomListPosX", iRomListPosX);
	ini.SetLongValue("Rom List", "RomListPosY", iRomListPosY);
	ini.SetLongValue("Rom List", "RomListSize", GAMESEL_MaxWindowList);
	ini.SetLongValue("Rom List", "RomListAlign", iRomListAlign);
	ini.SetLongValue("Rom List", "RomListCharacterLimit", RomListTrunc);

	ini.SetLongValue("Controls Panel", "ControlsPanelPosX", iControlsPosX);
	ini.SetLongValue("Controls Panel", "ControlsPanelPosY", iControlsPosY);
	ini.SetLongValue("Controls Panel", "ControlsTxtPosX", iControlsTxtPosX);
	ini.SetLongValue("Controls Panel", "ControlsTxtPosY", iControlsTxtPosY);
	ini.SetLongValue("Controls Panel", "ControlsTxtAlign", iControlsTxtAlign);
	
	ini.SetLongValue("Logo", "LogoPosX", iLogoPosX);
	ini.SetLongValue("Logo", "LogoPosY", iLogoPosY);

	ini.SetLongValue("Synopsis", "SynopsisPosX", iSynopsisPosX);
	ini.SetLongValue("Synopsis", "SynopsisPosY", iSynopsisPosY);
	ini.SetLongValue("Synopsis", "SynopsisLines", iSynopsisLines);
	ini.SetLongValue("Synopsis", "SynopsisWrap", iSynopsisWrap);
	ini.SetLongValue("Synopsis", "SynopsisAlign", iSynopsisAlign);
	ini.SetLongValue("Synopsis", "ControlsPosX", iSynControlsPosX);
	ini.SetLongValue("Synopsis", "ControlsPosY", iSynControlsPosY);
	ini.SetLongValue("Synopsis", "ControlsTxtPosX", iSynControlsTxtPosX);
	ini.SetLongValue("Synopsis", "ControlsTxtPosY", iSynControlsTxtPosY);
	ini.SetLongValue("Synopsis", "ControlsTxtAlign", iSynControlsTxtAlign);


	ini.SetLongValue("IGM Title", "IGMTitleX", iIGMTitleX);
	ini.SetLongValue("IGM Title", "IGMTitleY", iIGMTitleY);

	ini.SetLongValue("IGM Menu", "IGMMenuPosX", iIGMMenuPosX);
	ini.SetLongValue("IGM Menu", "IGMMenuPosY", iIGMMenuPosY);
	ini.SetLongValue("IGM Menu", "IGMHilightPosX", iIGMHilightPosX);
	ini.SetLongValue("IGM Menu", "IGMMenuTxtPosX", iIGMMenuTxtPosX);
	ini.SetLongValue("IGM Menu", "IGMMenuTxtPosY", iIGMMenuTxtPosY);
	ini.SetLongValue("IGM Menu", "IGMMenuTxtAlign", iIGMMenuTxtAlign);
	ini.SetLongValue("IGM Menu", "IGMStateScreenX", iIGMStateScreenX);
	ini.SetLongValue("IGM Menu", "IGMStateScreenY", iIGMStateScreenY);
	ini.SetLongValue("IGM Menu", "IGMStateScreenW", iIGMStateScreenW);
	ini.SetLongValue("IGM Menu", "IGMStateScreenH", iIGMStateScreenH);

	ini.SetLongValue("IGM Control Config", "ControlConfigSpacing", iControlsSpacingIGM);
	ini.SetLongValue("IGM Control Config", "ControlConfigCenterX", iIGMControlConfigCenterX);
	ini.SetLongValue("IGM Control Config", "ControlConfigWidth", iIGMControlConfigWidth);
	ini.SetLongValue("IGM Control Config", "ControlConfigTop", iIGMControlConfigTop);
	ini.SetLongValue("IGM Control Config", "ControlConfigBGPosX", iIGMControlConfigBGPosX);
	ini.SetLongValue("IGM Control Config", "ControlConfigBGPosY", iIGMControlConfigBGPosY);
//	ini.SetLongValue("IGM Control Config", "ControlConfigTxtPosX", iIGMControlConfigTxtPosX);
//	ini.SetLongValue("IGM Control Config", "ControlConfigTxtPosY", iIGMControlConfigTxtPosY);
	ini.SetLongValue("IGM Control Config", "ControlConfigTxtPadLX", iIGMControlConfigTxtPadLX);
	ini.SetLongValue("IGM Control Config", "ControlsPosX", iIGMConControlsPosX);
	ini.SetLongValue("IGM Control Config", "ControlsPosY", iIGMConControlsPosY);
	ini.SetLongValue("IGM Control Config", "ControlsTxtPosX", iIGMConControlsTxtPosX);
	ini.SetLongValue("IGM Control Config", "ControlsTxtPosY", iIGMConControlsTxtPosY);
	ini.SetLongValue("IGM Control Config", "ControlsTxtAlign", iIGMConControlsTxtAlign);

	ini.SetLongValue("IGM Screenshot", "PanelX", iPanelX);
	ini.SetLongValue("IGM Screenshot", "PanelY", iPanelY);
	ini.SetLongValue("IGM Screenshot", "PanelNW", iPanelNW);
	ini.SetLongValue("IGM Screenshot", "PanelNH", iPanelNH);

	//Loading Panel
	ini.SetLongValue("Loading Screen", "BoxArtPosX", iLoadBoxPosX);
	ini.SetLongValue("Loading Screen", "BoxArtPosY", iLoadBoxPosY);
	ini.SetLongValue("Loading Screen", "LoadPanelPosX", iLoadPanelPosX);
	ini.SetLongValue("Loading Screen", "LoadPanelPosY", iLoadPanelPosY);
	ini.SetLongValue("Loading Screen", "LoadingTxtPosX", iLoadPanelTxtPosX);
	ini.SetLongValue("Loading Screen", "LoadingTxtPosY", iLoadPanelTxtPosY);
	ini.SetLongValue("Loading Screen", "LoadingTxtAlign", iLoadPanelTxtAlign);	
	ini.SetLongValue("Loading Screen", "LoadingBarPosX", iLoadPanelBarPosX);
	ini.SetLongValue("Loading Screen", "LoadingBarPosY", iLoadPanelBarPosY);
	ini.SetLongValue("Loading Screen", "LoadPanelTrunc", iLoadPanelTrunc);
	

	// In Game Menu HD

	ini.SetLongValue("IGM HD Title", "IGMTitleX_HD", iIGMTitleX_HD);
	ini.SetLongValue("IGM HD Title", "IGMTitleY_HD", iIGMTitleY_HD);

	ini.SetLongValue("IGM HD Menu", "IGMMenuPosX_HD", iIGMMenuPosX_HD);
	ini.SetLongValue("IGM HD Menu", "IGMMenuPosY_HD", iIGMMenuPosY_HD);
	ini.SetLongValue("IGM HD Menu", "IGMHilightPosX_HD", iIGMHilightPosX_HD);
	ini.SetLongValue("IGM HD Menu", "IGMMenuTxtPosX_HD", iIGMMenuTxtPosX_HD);
	ini.SetLongValue("IGM HD Menu", "IGMMenuTxtPosY_HD", iIGMMenuTxtPosY_HD);
	ini.SetLongValue("IGM HD Menu", "IGMMenuTxtAlign_HD", iIGMMenuTxtAlign_HD);
	ini.SetLongValue("IGM HD Menu", "IGMStateScreenX_HD", iIGMStateScreenX_HD);
	ini.SetLongValue("IGM HD Menu", "IGMStateScreenY_HD", iIGMStateScreenY_HD);
	ini.SetLongValue("IGM HD Menu", "IGMStateScreenW_HD", iIGMStateScreenW_HD);
	ini.SetLongValue("IGM HD Menu", "IGMStateScreenH_HD", iIGMStateScreenH_HD);

	ini.SetLongValue("IGM HD Control Config", "ControlConfigSpacing_HD", iControlsSpacingIGM_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlConfigCenterX_HD", iIGMControlConfigCenterX_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlConfigWidth_HD", iIGMControlConfigWidth_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlConfigTop_HD", iIGMControlConfigTop_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlConfigBGPosX_HD", iIGMControlConfigBGPosX_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlConfigBGPosY_HD", iIGMControlConfigBGPosY_HD);
//	ini.SetLongValue("IGM HD Control Config", "ControlConfigTxtPosX_HD", iIGMControlConfigTxtPosX_HD);
//	ini.SetLongValue("IGM HD Control Config", "ControlConfigTxtPosY_HD", iIGMControlConfigTxtPosY_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlConfigTxtPadLX_HD", iIGMControlConfigTxtPadLX_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlsPosX_HD", iIGMConControlsPosX_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlsPosY_HD", iIGMConControlsPosY_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlsTxtPosX_HD", iIGMConControlsTxtPosX_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlsTxtPosY_HD", iIGMConControlsTxtPosY_HD);
	ini.SetLongValue("IGM HD Control Config", "ControlsTxtAlign_HD", iIGMConControlsTxtAlign_HD);

	ini.SetLongValue("IGM HD Screenshot", "PanelX_HD", iPanelX_HD);
	ini.SetLongValue("IGM HD Screenshot", "PanelY_HD", iPanelY_HD);
	ini.SetLongValue("IGM HD Screenshot", "PanelNW_HD", iPanelNW_HD);
	ini.SetLongValue("IGM HD Screenshot", "PanelNH_HD", iPanelNH_HD);


	char szIniFilename[64];
	if(onhd || strcmp(szPathSkins,"D:\\Skins\\") != 0) {
		// if it's on the hd or a custom path is specified, we'll save it there
		sprintf(szIniFilename, "%s%s\\Skin.ini", szPathSkins, skinname);
	} else {
		// otherwise, we'll save it to T
		sprintf(szIniFilename, "T:\\Skins\\%s\\Skin.ini", skinname);

		if (!PathFileExists("T:\\Skins"))
			CreateDirectory("T:\\Skins", NULL);
		
		char pathskinini[MAX_FILE_PATH];
		sprintf(pathskinini, "T:\\Skins\\%s", skinname);
		if (!PathFileExists(pathskinini))
			CreateDirectory(pathskinini, NULL);
	}


	OutputDebugString(szIniFilename);
	rc = ini.SaveFile(szIniFilename);
    if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Save!\n");
		return; //return false;
	}
	OutputDebugStringA(" Saved Successfully!\n");

	return;
}

void loadinis() {
	//Check for CD/DVD
	if(XGetDiskSectorSize("D:\\") == 2048){
		onhd = FALSE;
	}else{
		onhd = TRUE;
	}
	ConfigAppLoad();
	ConfigAppSave();
	ConfigAppLoadTemp();
	ConfigAppLoad2();
}

int loaddwPJ64PagingMem(){ return dwPJ64PagingMem;}
int loaddwPJ64DynaMem(){ return dwPJ64DynaMem;}
int loaddw1964PagingMem(){ return dw1964PagingMem;}
int loaddw1964DynaMem(){ return dw1964DynaMem;}
int loaddwMaxVideoMem(){ return dwMaxVideoMem;} //reinstate max video mem

//int loadbUseLLERSP(){ return bUseLLERsp;}
//int loadbUseLLERSP() { if (bUseLLERSP) return 1; return 0;} // not used anymore, use iAudioPlugin instead to determine if basic audio is used
int loadbUseRspAudio() { if (bUseRspAudio) return 1; return 0;} // control a listing
int loadiRspPlugin(){ return iRspPlugin;}
int loadiAudioPlugin(){ return iAudioPlugin;}

// ultrahle mem settings
int loaddwUltraCodeMem(){ return dwUltraCodeMem;}
int loaddwUltraGroupMem(){ return dwUltraGroupMem;}

int loadiPagingMethod(){ return iPagingMethod;}
int loadbAudioBoost() { if (bAudioBoost) return 1; return 0; };

/* // not used?
char* LabelCheck(char *s,char *szLabel)
{
	int nLen;
	if (s == NULL) {
		return NULL;
	}
	if (szLabel == NULL) {
		return NULL;
	}
	nLen = strlen(szLabel);
	
	SKIP_WS(s);													// Skip whitespace

	if (strncmp(s, szLabel, nLen)){								// Doesn't match
		return NULL;
	}
	return s + nLen;
}

// Skin

char* LabelCheckNew( char *s, char *szLabel )
{
	char	*returnvalue = NULL;
	int		nLen;

	if ( s == NULL )
		return NULL;

	if ( szLabel == NULL )
		return NULL;

	// Longueur de ce que l'on recherche
	nLen = strlen( szLabel );
	
	// On saute les blancs
	SKIP_WS( s );

	// On ne tiens pas compte de la casse
	if ( _strnicmp( s, szLabel, nLen ) )
		return NULL;

	// On passe à la suite
	s += nLen;

	// On saute les blancs
	SKIP_WS( s );

	// On cherche le "="
	if ( *s != '=' )
		return NULL;

	// On passe à la suite
	s++;

	// On saute les blancs
	SKIP_WS( s );

	// On lis ce qu'il y a entre les guillemets
	if ( QuoteRead( &returnvalue, NULL, s ) ) return NULL;
	// On retourne la valeur
	return returnvalue;
}


int QuoteRead(char** pszQuote, char** pszEnd, char* szSrc)		// Read a quoted string from szSrc and point to the end
{
	static char szQuote[QUOTE_MAX];
	char *s = szSrc;
	int i;
	
	SKIP_WS(s);													// Skip whitespace

	if (*s != '\"')	{											// Didn't start with a quote
		return 1;
	}
	s++;

	// Copy the characters into szQuote
	for (i = 0; i < QUOTE_MAX - 1; i++, s++) {
		if (*s == '\"') {
			s++;
			break;
		}
		if (*s == 0) {											// Stop copying if end of line or end of string
			break;
		}
		szQuote[i] = *s;										// Copy character to szQuote
	}
	szQuote[i] = 0;												// Zero-terminate szQuote

	if (pszQuote) {
		*pszQuote = szQuote;
	}
	if (pszEnd)	{
		*pszEnd = s;
	}
	
	return 0;
}
*/

bool PathFileExists(const char *pszPath)
{   
    return GetFileAttributes(pszPath) != INVALID_FILE_ATTRIBUTES;   
}

void GetPathSaves(char *pszPathSaves)
{
	sprintf(pszPathSaves, "%s", szPathSaves);
}