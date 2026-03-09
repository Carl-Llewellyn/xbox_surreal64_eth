#include "Launcher.h"
#include "IniFile.h"
#include "BoxArtTable.h"
#include "IoSupport.h"
#include "RomList.h"
#include "Panel.h"
#include "config.h"
#include <xbapp.h>
#include <xbresource.h>
#include <xbfont.h>
#include "zlib/zlib.h"
#include "zlib/unzip.h"
#include "../Ingamemenu/xlmenu.h"
#include "musicmanager.h"
#include "Util.h"
#include <vector>

#include "VideoTable.h"
#include "PathExplorer.h"

//weinerschnitzel - Skin Control
extern DWORD dwMenuItemColor;
extern DWORD dwMenuTitleColor;
extern char skinname[32];
extern int iMainMenuTxtPosX;
extern int iMainMenuTxtPosY;
extern int iMainMenuTxtAlign;
extern int iLaunchMenuTxtPosX;
extern int iLaunchMenuTxtPosY;
extern int iLaunchMenuTxtAlign;
extern int iControlsTxtPosX;
extern int iControlsTxtPosY;
extern int iControlsTxtAlign;
extern int itemh;
extern int isLaunchMenu;
extern int fontcenter;
extern int BoxartType;
extern bool EnableVideoAudio;
extern bool EnableXMVPreview;
extern bool EnableInfoPanel;
//extern bool usePageOriginal;
extern int iPagingMethod;
extern bool EnableBGMusic;
extern bool RandomBGMusic;
extern bool bAudioBoost;

DWORD GetFontAlign(int iAlign);
DWORD GetMenuFontAlign(int iAlign);

extern CMusicManager  music;
XBGAMEPAD gamepad;
extern int preferedemu;
XLMenu    *m_pMainMenu;
XLMenu    *m_pSettingsMenu;
//XLMenu    *m_pSkinMenu;
extern CXBFont		m_Font;					// Font	for	text display
extern void DrawLogo(bool Menu);
static int currentItem;
//extern bool onhd;
extern int videoplugin;
extern char skinname[32];
extern void ReloadSkin();
extern void WriteSkinFile();

//extern bool PhysRam128();
extern int ConfigAppLoad3(); // Ez0n3 - used for fixing user preferences in main menu
extern bool has128ram;

string GetEmulatorName(int p_iEmulator);
string GetAudioPluginName(int p_iAudioPlugin);
string GetVideoPluginName(int p_iVideoPlugin);
string GetRspPluginName(int p_iRspPlugin);
string GetPagingMethodName(int p_iPagingMethod);

extern int iRspPlugin;
extern int iAudioPlugin;
extern bool bUseRspAudio; // control a listing - NEEDS A MENU OPTION -

void ToggleEmulator(bool inc);
void ToggleRSPPlugin(bool inc); // give RSP its own
void ToggleAudioPlugin(bool inc); // replace usellersp bool with iAudioPlugin
void ToggleVideoPlugin(bool inc);

//Launcher Settings
void LauncherSettingsMenu();
void ToggleHideLaunchScreens();
void ToggleEnableVideoAudio();
void ToggleEnableXMVPreview();
void ToggleEnableInfoPanel();
void ToggleEnableBGMusic();
void ToggleRandomBGMusic();
void ToggleAudioBoost();
void ToggleSkin(bool inc);
char szSkinSelect[256];

//Default Settings - WIP
void DefaultSettingsMenu();

//Video Settings
void VideoSettingsMenu();
void ToggleFlickerFilter(bool inc);
void ToggleVertexMode(bool inc);
void ToggleTextureFilter(bool inc);
void ToggleAAMode(bool inc);
void ToggleSoftDisplayFilter();
void ToggleFrameSkip();
void ToggleVSync(bool inc);
void ToggleHDTV();
void ToggleFullScreen();

//Emulator Settings
void SettingsMenu();
void ToggleRSPAudio();
//void TogglePagingMethod();
void TogglePagingMethod(bool inc);
void ToggleMaxVideoMem(bool inc);
void ToggleVideoplugin(bool inc);
void Toggle1964DynaMem(bool inc);
void Toggle1964PagingMem(bool inc);
void TogglePJ64DynaMem(bool inc);
void TogglePJ64PagingMem(bool inc);

//Controller Settings
void ControllerSettingsMenu();
void ToggleSensitivity(bool inc);
void ToggleDeadzone(bool inc);
//void TogglePak();
void TogglePak(bool inc);
void ToggleController1();
void ToggleController2();
void ToggleController3();
void ToggleController4();

//Controller Config
void ControllerMenu();
extern void ChangeControl();
void Control();
void Control1();
void Control2();
void Control3();
void Control4();

//Skin...
static int ChangeSkin(void);
//static int SkinMenu(void);

//Synopsis
void ToggleSynopsis();
extern void DrawSynopsis();
extern void LoadSynopsis(DWORD crc);
extern void ClearSynopsis();
extern int actualrom;

//Credits 
extern void CalculateEndCredits();
extern void DrawCredits();
extern float FSTxtPos;
extern int iSynopTxtPos;

void ToggleCredits();
void ExitToDash();
void ShutdownXbox();
void LaunchHideScreens(void);

//Video Settings Menu Functions
void incTextureFilter(){ ToggleTextureFilter(true); }
void decTextureFilter(){ ToggleTextureFilter(false); }

void incflicker(){ ToggleFlickerFilter(true); }
void decflicker(){ ToggleFlickerFilter(false); }

void incVertexMode(){ ToggleVertexMode(true); }
void decVertexMode(){ ToggleVertexMode(false); }

void incVSync(){ ToggleVSync(true); }
void decVSync(){ ToggleVSync(false); }

void incAAMode(){ ToggleAAMode(true); }
void decAAMode(){ ToggleAAMode(false); }

//Emulator Settings Menu Functions
void incEmulator(){ ToggleEmulator(true); }
void decEmulator(){ ToggleEmulator(false); }

void incAudioPlugin(){ ToggleAudioPlugin(true); }
void decAudioPlugin(){ ToggleAudioPlugin(false); }

void incVideoPlugin(){ ToggleVideoPlugin(true); }
void decVideoPlugin(){ ToggleVideoPlugin(false); }

void incRSPPlugin(){ ToggleRSPPlugin(true); }
void decRSPPlugin(){ ToggleRSPPlugin(false); }

void incPagingMethod(){ TogglePagingMethod(true); }
void decPagingMethod(){ TogglePagingMethod(false); }

void incMaxVideoMem(){ ToggleMaxVideoMem(true); }
void decMaxVideoMem(){ ToggleMaxVideoMem(false); }

void inc1964DynaMem(){ Toggle1964DynaMem(true); }
void dec1964DynaMem(){ Toggle1964DynaMem(false); }

void inc1964PagingMem(){ Toggle1964PagingMem(true); }
void dec1964PagingMem(){ Toggle1964PagingMem(false); }

void incPJ64DynaMem(){ TogglePJ64DynaMem(true); }
void decPJ64DynaMem(){ TogglePJ64DynaMem(false); }

void incPJ64PagingMem(){ TogglePJ64PagingMem(true); }
void decPJ64PagingMem(){ TogglePJ64PagingMem(false); }

//Controller Settings Menu Functions
void incSensitivity(){ ToggleSensitivity(true); }
void decSensitivity(){ ToggleSensitivity(false); }

void incDeadzone(){ ToggleDeadzone(true); }
void decDeadzone(){ ToggleDeadzone(false); }

void incPakPlugin(){ TogglePak(true); }
void decPakPlugin(){ TogglePak(false); }

void incSkin(){ ToggleSkin(true); }
void decSkin(){ ToggleSkin(false); }


//
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

// Ez0n3 - it's 256 everywhere else. why buck the sys
extern char romname[256]; //extern char romname[32];

// get the audio plugins name from int
string GetAudioPluginName(int p_iAudioPlugin)
{
	string szAudioPlugin;

	switch (p_iAudioPlugin) {
		case _AudioPluginNone : szAudioPlugin = "None"; break;
		case _AudioPluginLleRsp : szAudioPlugin = "LLE RSP"; break;//leave this here to preserve ini settings
		case _AudioPluginBasic : szAudioPlugin = "Basic"; break;
		case _AudioPluginJttl : szAudioPlugin = "JttL"; break;
		case _AudioPluginAzimer : szAudioPlugin = "Azimer"; break;
		case _AudioPluginMusyX : szAudioPlugin = "MusyX"; break;
		//case _AudioPluginMissing : 
		default : szAudioPlugin = "JttL"; break;//freakdave - set JttL plugin as default
	}	

	return szAudioPlugin;
}

// get the video plugins name from int
string GetVideoPluginName(int p_iVideoPlugin)
{
	string szVideoPlugin;

	switch (p_iVideoPlugin) {
		case _VideoPluginRice510 : szVideoPlugin = "Rice 5.1.0"; break;
		case _VideoPluginRice531 : szVideoPlugin = "Rice 5.3.1"; break;
		case _VideoPluginRice560 : szVideoPlugin = "Rice 5.6.0"; break;
		case _VideoPluginRice611 : szVideoPlugin = "Rice 6.1.1"; break;
		case _VideoPluginRice612 : szVideoPlugin = "Rice 6.1.2"; break;
		case _VideoPluginMissing : 
		default : szVideoPlugin = "Unknown"; break;
	}	

	return szVideoPlugin;
}

// get the video plugins name from int
string GetEmulatorName(int p_iEmulator)
{
	string szEmulator;

	switch (p_iEmulator) {
		case _1964 : szEmulator = "1964"; break;
		case _Project64 : szEmulator = "Project64"; break;
		case _UltraHLE : szEmulator = "UltraHLE"; break;
		case _None : 
		default : szEmulator = "Unknown"; break;
	}	
	return szEmulator;
}

// get the rsp plugins name from int
string GetRspPluginName(int p_iRspPlugin)
{
	string szRSPPlugin;

	switch (p_iRspPlugin) {
		case _RSPPluginNone :
		case _RSPPluginMissing : szRSPPlugin = "None"; break;
		case _RSPPluginLLE : szRSPPlugin = "Low Level"; break;
		case _RSPPluginM64p : szRSPPlugin = "M64Plus HLE"; break;
		case _RSPPluginHLE : 
		default : szRSPPlugin = "High Level"; break; // set hle as default
	}	

	return szRSPPlugin;
}

string GetPagingMethodName(int p_iPagingMethod)
{
	string szPagingMethod;

	switch (p_iPagingMethod) {
		//case _PagingNone :
		case _PagingMissing : szPagingMethod = "None"; break;
		//case _PagingVMM : szPagingMethod = "VMM"; break;
		case _PagingS10 : szPagingMethod = "1.0"; break;
		case _PagingXXX : 
		default : szPagingMethod = "XXX"; break; // default XXX
	}	

	return szPagingMethod;
}

void MainMenu(void)
{
	isLaunchMenu = 0;
	// Ez0n3 - get user pref instead of always loading defaults
	//ConfigAppLoad2();	
	ConfigAppLoad3();
	//sprintf(menuBGpath,"D:\\Skins\\%s\\Launcher\\MainMenuBG.png",skinname);
	

	DWORD dwMenuCommand = 0;
	WCHAR currentname[256];

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pMainMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,13, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	
	m_pMainMenu->itemcolor = dwMenuItemColor;
	m_pMainMenu->parent = NULL;

	XLMenu_SetTitle(m_pMainMenu,L"Main Menu",dwMenuTitleColor);

	//XLMenu_AddItem(m_pMainMenu,MITEM_DISABLED,L"General Settings",NULL);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Launcher Settings",LauncherSettingsMenu);

	// temp location for a "Restore Defaults" menu
	// not sure where it should go since it will change settings that may affect the default menu's parent
	// have to test to see if parent menu setttings change if the default menu is nested in it
	// IE: if default menu is child of "Launcher Settings", will the values in Launcher Settings update
	// when returning from defaults menu after the launcher settings get set to their defaults
	// works ok in main menu because no menus with settings it might change are shown/returned to/from
	// if that's not a problem, it should be able to be nested anywhere
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Default Settings",DefaultSettingsMenu); // WIP

	//XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Choose Skin",SkinMenu);

	swprintf(currentname,L"%S",romname);
	XLMenu_AddItem(m_pMainMenu,MITEM_DISABLED,currentname,NULL);
	XLMenu_AddItem(m_pMainMenu,MITEM_DISABLED,L"Specific Settings",NULL);
	
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Video Settings",VideoSettingsMenu);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Emulator Settings",SettingsMenu);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Controller Settings",ControllerSettingsMenu);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Controller Config",ControllerMenu);
	
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"View Synopsis",ToggleSynopsis); // trying on bottom, below "settings"
	
	XLMenu_AddItem(m_pMainMenu,MITEM_DISABLED,L"--------------------",NULL); // little bit shorter for both sd and hd
	
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Credits",ToggleCredits);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Quit Surreal",ExitToDash);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Shutdown Xbox",ShutdownXbox);


	XLMenu_Activate(m_pMainMenu);
	
	//XLMenu_Routine(MENU_NEXTITEM); // commented while "General Settings" is commented

	while( XLMenu_CurMenu == m_pMainMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);

		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}
	
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;

	if (m_pMainMenu)
		XLMenu_Delete(m_pMainMenu);

}

void ToggleCredits()
{
	//sprintf(menuBGpath,"D:\\Skins\\%s\\Launcher\\CreditsBG.png",skinname);
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pSettingsMenu = XLMenu_Init(0, 0, 1, MENU_WRAP, NULL);


	m_pSettingsMenu->parent = m_pMainMenu;
    
	XLMenu_SetTitle(m_pSettingsMenu,L" ",0x00FFFFFF);


	XLMenu_Activate(m_pSettingsMenu);

	CalculateEndCredits();
    bool bquit=false;

	while(!bquit)
	{
		DrawCredits();
		if(g_Gamepads->bLastAnalogButtons[XINPUT_GAMEPAD_A]) FSTxtPos = FSTxtPos - 1.5;
		else FSTxtPos = FSTxtPos - 0.5;
		if(g_Gamepads->bPressedAnalogButtons[XINPUT_GAMEPAD_B]) bquit = true;
		XBInput_GetInput();
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}
	XLMenu_Routine(MENU_BACK);
	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);
}

void ToggleSynopsis()
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pSettingsMenu = XLMenu_Init(0, 0, 1, MENU_WRAP, NULL);
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L" ",0x00FFFFFF);
	XLMenu_Activate(m_pSettingsMenu);

	iSynopTxtPos = 0; // reset
	
	bool bDpadUp = false;
	bool bDpadDn = false;

	Rom *rom = g_romList.GetRomAt(actualrom);
	LoadSynopsis(rom->m_dwCrc1);
	bool bquit=false;
	while(!bquit)
	{
		DrawSynopsis();
		XBInput_GetInput();

		float fWindowVelocity =	(float)((g_Gamepads->bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER])/256.0f);
		fWindowVelocity -= (float)((g_Gamepads->bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER])/256.0f);
		fWindowVelocity -= (float)((-g_Gamepads->sThumbLY)/32256.0f); // adds .01 at rest
		if ((fWindowVelocity <= -0.1f || fWindowVelocity >= 0.1f) && XBUtil_Timer(TIMER_GETAPPTIME) > 0.1f) {
			XBUtil_Timer(TIMER_RESET);
			iSynopTxtPos -= ((int)(fWindowVelocity * 100.0f) / 10); // 1 to 10
		}
		
		if (g_Gamepads->bPressedAnalogButtons[XINPUT_GAMEPAD_X]) {
			//if (XBUtil_Timer(TIMER_GETAPPTIME) > 0.1f) {
				XBUtil_Timer(TIMER_RESET);
				iSynopTxtPos -= 2;
				bDpadUp = true;
				bDpadDn = false;
			//}
		}
		else if (bDpadUp && g_Gamepads->bAnalogButtons[XINPUT_GAMEPAD_X]) {
			if (XBUtil_Timer(TIMER_GETAPPTIME) > 0.1f) {
				XBUtil_Timer(TIMER_RESET);
				iSynopTxtPos -= 2;
			}
		}
		else if (g_Gamepads->bPressedAnalogButtons[XINPUT_GAMEPAD_A]) {
			//if (XBUtil_Timer(TIMER_GETAPPTIME) > 0.1f) {
				XBUtil_Timer(TIMER_RESET);
				iSynopTxtPos += 2;
				bDpadUp = false;
				bDpadDn = true;
			//}
		}
		else if (bDpadDn && g_Gamepads->bAnalogButtons[XINPUT_GAMEPAD_A]) {
			if (XBUtil_Timer(TIMER_GETAPPTIME) > 0.1f) {
				XBUtil_Timer(TIMER_RESET);
				iSynopTxtPos += 2;
			}
		}
		else if (g_Gamepads->wButtons & XINPUT_GAMEPAD_DPAD_UP) {
			if (XBUtil_Timer(TIMER_GETAPPTIME) > 0.1f) {
				XBUtil_Timer(TIMER_RESET);
				iSynopTxtPos -= 1;
			}
		}
		else if (g_Gamepads->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
			if (XBUtil_Timer(TIMER_GETAPPTIME) > 0.1f) {
				XBUtil_Timer(TIMER_RESET);
				iSynopTxtPos += 1;
			}
		}
		else {
			XBUtil_Timer(TIMER_STOP);
			bDpadUp = false;
			bDpadDn = false;
		}

		if(g_Gamepads->bPressedAnalogButtons[XINPUT_GAMEPAD_B]) bquit=true;
		
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}
	ClearSynopsis();
	
	XLMenu_Routine(MENU_BACK);
	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);
}

// Skin
/*static int SkinMenu(void)
{
	//sprintf(menuBGpath,"D:\\Skins\\%s\\Launcher\\SkinMenuBG.png",skinname);
	XBGAMEPAD gamepad;
	//char skins_path[] = "d:\\skins\\*";
	HANDLE				hFind;	
	WIN32_FIND_DATAA	oFindData;
	WCHAR				wstr[MITEM_STRINGLEN];
	int					nSkinCount = 0;
	
	char skins_path[256];
	sprintf(skins_path, "%s*", szPathSkins);

	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine	= NULL;
	XLMenu_CurMenu		= NULL;
	XLMenu_SetFont(&m_Font);

	// On débute la recherche
	hFind = FindFirstFile(skins_path, &oFindData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// Un repertoire donc on regarde si il y a un config.ini dedans
			if (oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					nSkinCount++;
			}
		} while (FindNextFile(hFind, &oFindData));
	}
	// Fin de la recherche
	FindClose(hFind);

	if (nSkinCount < 2) return MROUTINE_RETURN;

	m_pSkinMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY, nSkinCount + 2, MENU_LEFT|MENU_WRAP, NULL);
	m_pSkinMenu->itemcolor = dwMenuItemColor;
	m_pSkinMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSkinMenu, L"Change Skin  ",dwMenuTitleColor);
	XLMenu_SetMaxShow(m_pSkinMenu, nSkinCount + 2);

	swprintf(wstr, L"Current: %S", skinname);
	XLMenu_AddItem(m_pSkinMenu, MITEM_DISABLED, wstr, NULL);
	XLMenu_AddItem(m_pSkinMenu, MITEM_DISABLED, L"----------------------------", NULL);

	// On débute la recherche
	hFind = FindFirstFile(skins_path, &oFindData);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// Un repertoire donc on regarde si il y a un config.ini dedans
			if (oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					swprintf(wstr, L"%S", oFindData.cFileName);
					XLMenu_AddItem(m_pSkinMenu, MITEM_ROUTINE, wstr, ChangeSkin);
			}
		} while (FindNextFile(hFind, &oFindData));
	}
	// Fin de la recherche
	FindClose(hFind);

	m_pSkinMenu->curitem = 2;
	XLMenu_Activate(m_pSkinMenu);

	while(XLMenu_CurMenu == m_pSkinMenu) {
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pSkinMenu) {
		XLMenu_Delete(m_pSkinMenu);
		m_pSkinMenu = NULL;
	}

	return MROUTINE_RETURN;
}*/

static char *szSkinElem[] = {

	"IGM\\ControlConfigPanel.png",
	"IGM\\Font.xpr",
	"IGM\\hilight.png",
	"IGM\\MainMenuBG.png",
	"IGM\\ControlsPanel.png",

	"IGMHD\\ControlConfigPanel.png",
	"IGMHD\\Font.xpr",
	"IGMHD\\hilight.png",
	"IGMHD\\MainMenuBG.png",
	"IGMHD\\ControlsPanel.png",

	"Launcher\\BoxartBG.png",
	"Launcher\\ControlConfigPanel.png",
	"Launcher\\ControlsPanel.png",
	"Launcher\\Font.xpr",
	"Launcher\\hilight.png",
	"Launcher\\InfoPanel.png",
	"Launcher\\LaunchPanel.png",
	"Launcher\\LoadingBG.png",
	"Launcher\\Logo.png",
	"Launcher\\MainMenuBG.png",
	"Launcher\\RomListBG.png",

	"bg.png",
	"ingamebg.jpg",
	"ingamebgHD.jpg",
	//"main.wma",
	"MSFont.xpr",
	"progress.png",
	"skin.ini",

NULL};

bool CheckSkin(const char *szSkin)
{
	bool bCheckPass = true;

	for (int i=0; szSkinElem[i]; i++) {
		char testname[256];
		sprintf(testname,"%s%s\\%s", szPathSkins, szSkin, szSkinElem[i]);
		if (!PathFileExists(testname)) {
			
			// if it's the default skin, let it load without skin.ini
			if (strcmp("Default", szSkin) == 0 && strcmp("skin.ini", szSkinElem[i]) == 0)
				continue;
		
			Log("Failed To Load Skin File: %s", testname);
			bCheckPass = false;
			//break; // let it gather all missing files
		}
	}
	
	return bCheckPass;
}

static int ChangeSkin(void)
{
	if (strcmp(skinname, szSkinSelect) == 0) {
		char msg[100];
		sprintf(msg, "%s Skin Already Applied", szSkinSelect);
		ShowTempMessage(msg);
		
		XLMenu_CurRoutine = NULL;
		return 0;
	}

	if (CheckSkin(szSkinSelect)) { //checkpass
		OutputDebugString(szSkinSelect);
		OutputDebugStringA(" Skin is Loading\n");
	
		// On change la skin
		strcpy( skinname, szSkinSelect );
		
		// On charge la nouvelle skin
		ReloadSkin();

		// On sauvegarde la configuration
		ConfigAppSave();
	}
	else {
		OutputDebugString(szSkinSelect);
		OutputDebugString(" Skin Failed to Load!\n");
		
		char msg[100];
		sprintf(msg, "%s Skin Failed to Load", szSkinSelect);
		ShowTempMessage(msg);
		
		XLMenu_CurRoutine = NULL;
		return 0;
	}
	
#ifdef DEBUG
	Sleep(300); // to see debug string
#endif
	
	// Some Resources need to reboot to refresh properly
	D3DDevice::PersistDisplay();
	XLaunchNewImage("D:\\default.xbe", NULL);

	return 1;
}

void ExitToDash(void)
{
	LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };
	XLaunchNewImage( NULL, (LAUNCH_DATA*)&LaunchData );
}

void ShutdownXbox(void)
{
	CIoSupport IOSupport;
	IOSupport.Shutdown();
}

void SettingsMenu(void)
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];

	m_pSettingsMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,13, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL); //11
		m_pSettingsMenu->itemcolor = dwMenuItemColor;
		m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Settings",dwMenuTitleColor);

	// Emulator Selector
	swprintf(currentname, L"Emulator : %S", GetEmulatorName(preferedemu).c_str());
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incEmulator,decEmulator);
	
	// Video Plugin Selector
	swprintf(currentname, L"Video Plugin : %S", GetVideoPluginName(videoplugin).c_str());
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incVideoPlugin,decVideoPlugin);

	// Audio Plugin Selector
	swprintf(currentname, L"Audio Plugin : %S", GetAudioPluginName(iAudioPlugin).c_str());
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incAudioPlugin,decAudioPlugin);
	
	// RSP Plugin Selector
	swprintf(currentname, L"RSP Plugin : %S", GetRspPluginName(iRspPlugin).c_str());
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incRSPPlugin,decRSPPlugin);
	
	// RSP Audio Enable
	if (bUseRspAudio)
		swprintf(currentname,L"Enable RSP Audio : Yes");
	else
		swprintf(currentname,L"Enable RSP Audio : No");	
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleRSPAudio);

	// Rom Paging Method
	/*if (usePageOriginal)
		swprintf(currentname,L"Rom Paging Method : 1.0");
	else
		swprintf(currentname,L"Rom Paging Method : XXX");	
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,TogglePagingMethod);*/
	swprintf(currentname, L"Rom Paging Method : %S", GetPagingMethodName(iPagingMethod).c_str());
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incPagingMethod,decPagingMethod);
	
	// Max Video Memory - reinstated
	if (dwMaxVideoMem == 0)
		swprintf(currentname,L"Max Video Memory : Auto");
	else
		swprintf(currentname,L"Max Video Memory : %d MB",dwMaxVideoMem);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incMaxVideoMem,decMaxVideoMem);

	
		
	// 1964 Settings
	XLMenu_AddItem(m_pSettingsMenu,MITEM_DISABLED,L"1964",NULL);

	swprintf(currentname,L"Dynarec Memory : %d MB",dw1964DynaMem);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,inc1964DynaMem,dec1964DynaMem);

	swprintf(currentname,L"Paging Memory : %d MB",dw1964PagingMem);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,inc1964PagingMem,dec1964PagingMem);

	// PJ64 Settings
	XLMenu_AddItem(m_pSettingsMenu,MITEM_DISABLED,L"Project 64",NULL);

	swprintf(currentname,L"Dynarec Memory : %d MB",dwPJ64DynaMem);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incPJ64DynaMem,decPJ64DynaMem);

	swprintf(currentname,L"Paging Memory : %d MB",dwPJ64PagingMem);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incPJ64PagingMem,decPJ64PagingMem);
	
	
	XLMenu_Activate(m_pSettingsMenu);

	bool bLaunchFromMenu = false;
	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo(true);
		// Add Y - Launch button text after ui_logo draws
		m_Font.Begin();
		m_Font.DrawText((float)iControlsTxtPosX, (float)(iControlsTxtPosY+(3*itemh)), dwMenuItemColor, L"\403 Launch", GetFontAlign(iControlsTxtAlign));//480,350
		m_Font.End();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		else if (gamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_Y]){ // launch the rom from the config screen with Y
			bLaunchFromMenu = true;
			XLMenu_CurRoutine = NULL;
			XLMenu_CurMenu = NULL;
			break;
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);

	ConfigAppSave2();

	if (bLaunchFromMenu) {
		LaunchHideScreens();
	}
}

// Audio plugin toggle - use iAudioPlugin instead of the usellersp bool
void ToggleAudioPlugin(bool inc)
{	
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
		iAudioPlugin++;
		if(iAudioPlugin == _AudioPluginLleRsp)//Skip LLE RSP Audio
			iAudioPlugin++;
		if (iAudioPlugin > (_AudioPluginMissing - 1)) iAudioPlugin=0;
	}
	else
	{
		iAudioPlugin--;
		if(iAudioPlugin == _AudioPluginLleRsp)//Skip LLE RSP Audio
			iAudioPlugin--;
		if (iAudioPlugin < 0) iAudioPlugin=(_AudioPluginMissing - 1);
	}

	XLMenu_CurRoutine = NULL;
	
	// audio plugins
	swprintf(currentname, L"Audio Plugin : %S", GetAudioPluginName(iAudioPlugin).c_str());
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
}

// RSP plugin toggle
void ToggleRSPPlugin(bool inc)
{
	WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
		iRspPlugin++;
		if (!EmuDoesNoRsp(preferedemu) && iRspPlugin == _RSPPluginNone) iRspPlugin++;
		if (iRspPlugin > (_RSPPluginMissing - 1)) iRspPlugin=0;
	}
	else
	{
		iRspPlugin--;
		if (!EmuDoesNoRsp(preferedemu) && iRspPlugin == _RSPPluginNone) iRspPlugin--;
		if (iRspPlugin < 0) iRspPlugin=(_RSPPluginMissing - 1);
	}
	
	if (!EmuDoesNoRsp(preferedemu) && iRspPlugin == _RSPPluginNone) iRspPlugin++;
	
	XLMenu_CurRoutine = NULL;
	
	// rsp plugins
	swprintf(currentname, L"RSP Plugin : %S", GetRspPluginName(iRspPlugin).c_str());
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
	
	// Disable RSP Audio if we aren't using an RSP plugin that supports it
	if (!RspDoesAlist(iRspPlugin)) {
		bUseRspAudio  = false;
		swprintf(currentname, L"Enable RSP Audio : No");
		XLMenu_SetItemText(&m_pSettingsMenu->items[(currentItem + 1)], currentname);
	}
}

void ToggleRSPAudio()
{
	currentItem = m_pSettingsMenu->curitem;
	
	// Only change RSP Audio if we are using an RSP plugin that supports it
	if(RspDoesAlist(iRspPlugin))
	bUseRspAudio = !bUseRspAudio;
	
	XLMenu_CurRoutine = NULL;

	if (bUseRspAudio)
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable RSP Audio : Yes");
	else
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable RSP Audio : No");

		
	ConfigAppSave2();	
}

/*void TogglePagingMethod()
{
	currentItem = m_pSettingsMenu->curitem;

	if(!has128ram)
	usePageOriginal = !usePageOriginal;
	XLMenu_CurRoutine = NULL;
	
	if (usePageOriginal)
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Rom Paging Method : 1.0");
	else
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Rom Paging Method : XXX");
		
	ConfigAppSave2();	
}*/
void TogglePagingMethod(bool inc)
{
	WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
		iPagingMethod++;
		if (iPagingMethod > (_PagingMissing - 1)) iPagingMethod=0;
	}
	else
	{
		iPagingMethod--;
		if (iPagingMethod < 0) iPagingMethod=(_PagingMissing - 1);
	}
	
	XLMenu_CurRoutine = NULL;
	
	// paging method
	swprintf(currentname, L"Rom Paging Method : %S", GetPagingMethodName(iPagingMethod).c_str());
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
}

// Video plugin toggle
void ToggleVideoPlugin(bool inc)
{	
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
		videoplugin++;
		if (videoplugin > (_VideoPluginMissing - 1)) videoplugin=0;
	}
	else
	{
		videoplugin--;
		if (videoplugin < 0) videoplugin=(_VideoPluginMissing - 1);
	}

	XLMenu_CurRoutine = NULL;
	
	// video plugins
	swprintf(currentname, L"Video Plugin : %S", GetVideoPluginName(videoplugin).c_str());
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
}

// Emulator toggle
void ToggleEmulator(bool inc)
{	
	bool bUpdateRsp = false;
	
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
		preferedemu++;
		
		if (!EmuDoesNoRsp(preferedemu) && iRspPlugin==_RSPPluginNone) //PJ64 needs RSP 
			bUpdateRsp = true;

		if (preferedemu > (_None - 1)) preferedemu=0;
	}
	else
	{
		preferedemu--;
		
		if(!EmuDoesNoRsp(preferedemu) && iRspPlugin==_RSPPluginNone) //PJ64 needs RSP
			bUpdateRsp = true;
			
		if (preferedemu < 0) preferedemu=(_None - 1);
	}

	XLMenu_CurRoutine = NULL;
	
	// emulators
	swprintf(currentname, L"Emulator : %S", GetEmulatorName(preferedemu).c_str());
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
	
	// needs work
	if (bUpdateRsp && !EmuDoesNoRsp(preferedemu)) {
		iRspPlugin++; // lle
		// rsp plugins
		swprintf(currentname, L"RSP Plugin : %S", GetRspPluginName(iRspPlugin).c_str());
		XLMenu_SetItemText(&m_pSettingsMenu->items[(currentItem + 3)], currentname); // too dependant on position but can't get it to update any other way - bandaid
	}
}

void ToggleMaxVideoMem(bool inc)
{
	// Ez0n3 - we'll use the old one until freakdave finishes this
	int maxVideoMem = 10;
	
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
    dwMaxVideoMem++;
	if (dwMaxVideoMem > maxVideoMem) dwMaxVideoMem=0; // allow "0" for auto
	}
	else
	{
    dwMaxVideoMem--;
	if (dwMaxVideoMem < 0) dwMaxVideoMem=maxVideoMem; // allow "0" for auto
	}

	XLMenu_CurRoutine = NULL;
	
	if (dwMaxVideoMem == 0)
		swprintf(currentname,L"Max Video Memory : Auto");
	else
		swprintf(currentname,L"Max Video Memory : %d MB",dwMaxVideoMem);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
}

void Toggle1964DynaMem(bool inc)
{
	// Ez0n3 - bump up the memory settings
	int maxDynaMem = 20; //16
	if (has128ram) maxDynaMem = 32;

    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc) 
	{
    dw1964DynaMem++;
	if (dw1964DynaMem > maxDynaMem) dw1964DynaMem=0;
	}
	else
	{
    dw1964DynaMem--;
	if (dw1964DynaMem < 0) dw1964DynaMem=maxDynaMem;
	}
	
	XLMenu_CurRoutine = NULL;
	
	swprintf(currentname,L"Dynarec Memory : %d MB",dw1964DynaMem);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);


}

void Toggle1964PagingMem(bool inc)
{
	// Ez0n3 - bump up the memory settings
	int maxPagingMem = 20; //16
	if (has128ram) maxPagingMem = 64;
	
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
    dw1964PagingMem++;
	if (dw1964PagingMem > maxPagingMem) dw1964PagingMem=0;
	}
	else
	{
    dw1964PagingMem--;
	if (dw1964PagingMem < 0) dw1964PagingMem=maxPagingMem;
	}

	XLMenu_CurRoutine = NULL;
	
	swprintf(currentname,L"Paging Memory : %d MB",dw1964PagingMem);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

}

void TogglePJ64DynaMem(bool inc)
{
	// Ez0n3 - bump up the memory settings
	int maxDynaMem = 20; //16
	if (has128ram) maxDynaMem = 32;

    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
    dwPJ64DynaMem++;
	if (dwPJ64DynaMem > maxDynaMem) dwPJ64DynaMem=0;
	}
	else
	{
    dwPJ64DynaMem--;
	if (dwPJ64DynaMem < 0) dwPJ64DynaMem=maxDynaMem;
	}
	
	XLMenu_CurRoutine = NULL;
	
	swprintf(currentname,L"Dynarec Memory : %d MB",dwPJ64DynaMem);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

}

void TogglePJ64PagingMem(bool inc)
{
	// Ez0n3 - bump up the memory settings
	int maxPagingMem = 20; //16
	if (has128ram) maxPagingMem = 64;

    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc) 
	{
    dwPJ64PagingMem++;
	if (dwPJ64PagingMem > maxPagingMem) dwPJ64PagingMem=0;
	}
	else
	{
    dwPJ64PagingMem--;
	if (dwPJ64PagingMem < 0) dwPJ64PagingMem=maxPagingMem;
	}
	
	XLMenu_CurRoutine = NULL;
	
	swprintf(currentname,L"Paging Memory : %d MB",dwPJ64PagingMem);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

}

extern void Launch();
//int m_emulator; // Ez0n3 - why was this used at all?
void Launch1964();
void PJ64Launch();
void UltraHLELaunch();
void launch510();
void launch531();
void launch560();
void launch611();
void launch612();
extern int actualrom;
extern void display_compatible();

void selectvideomode(void)
{
	isLaunchMenu = 1;
	
	// Ez0n3 - get user pref instead of always just defaults
	//ConfigAppLoad2();	
	//int tmp_preferedemu = preferedemu; // get the just selected pref emu
	ConfigAppLoad3();
	//preferedemu = tmp_preferedemu; // re-set it to retain value - doesn't get saved in between emu/plug selections

	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pSettingsMenu = XLMenu_Init((float)iLaunchMenuTxtPosX, (float)iLaunchMenuTxtPosY,5, GetMenuFontAlign(iLaunchMenuTxtAlign)|MENU_WRAP, NULL);//210, 160

	m_pSettingsMenu->itemcolor = dwMenuItemColor;
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Select Video Plugin",dwMenuTitleColor);

	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Rice 5.10",launch510);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Rice 5.31",launch531);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Rice 5.60",launch560);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Rice 6.11",launch611);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Rice 6.12",launch612);

	XLMenu_Activate(m_pSettingsMenu);

	
	/*for (int i=0;i< videoplugin;i++){
		XLMenu_Routine(MENU_NEXTITEM);
		//Ez0n3 - had to increase this a tad to give the ini time to load
		Sleep(300); //too fast for ini load
	}*/
	m_pSettingsMenu->curitem = videoplugin; // skip the loop/sleep

	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}
	
	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);

}

void LaunchMenu(void)
{
	isLaunchMenu = 1;
	// Ez0n3 - get user pref instead of always just defaults
	// ConfigAppLoad2();	
	ConfigAppLoad3();
	
	

	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pMainMenu = XLMenu_Init((float)iLaunchMenuTxtPosX, (float)iLaunchMenuTxtPosY,3, GetMenuFontAlign(iLaunchMenuTxtAlign)|MENU_WRAP, NULL);//210, 160

	
	m_pMainMenu->itemcolor = dwMenuItemColor;
	m_pMainMenu->parent = NULL;

	XLMenu_SetTitle(m_pMainMenu,L"Launch Menu",dwMenuTitleColor);

	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Launch with 1964",Launch1964);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Launch with PJ64",PJ64Launch);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Launch with UltraHLE",UltraHLELaunch);


	XLMenu_Activate(m_pMainMenu);

	
	/*for (int i=0;i< preferedemu;i++){
		XLMenu_Routine(MENU_NEXTITEM);
		//Ez0n3 - had to increase this a tad to give the ini time to load
		Sleep(300); //too fast for ini load
	}*/
	m_pMainMenu->curitem = preferedemu; // skip the loop/sleep

	while( XLMenu_CurMenu == m_pMainMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}
	
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;

	if (m_pMainMenu)
		XLMenu_Delete(m_pMainMenu);

}

void Launch1964(void)
{
	//m_emulator = _1964;
	preferedemu = _1964;
	ConfigAppSave2();
	selectvideomode();
}

void PJ64Launch(void)
{
	//m_emulator = _Project64;
	preferedemu = _Project64;
	ConfigAppSave2();
	selectvideomode();
}

void UltraHLELaunch(void)
{
	XLMenu_Activate(NULL);            // kill the current menu
    XLMenu_CurRoutine = NULL; 
	//m_emulator = _UltraHLE;
	preferedemu = _UltraHLE;
	ConfigAppSave2();
	Launch();
}

void launch510()
{
videoplugin = 0;
ConfigAppSave2();
Launch();
}

void launch531()
{
videoplugin = 1;
ConfigAppSave2();
Launch();
}

void launch560()
{
videoplugin = 2;
ConfigAppSave2();
Launch();
}

void launch611()
{
videoplugin = 3;
ConfigAppSave2();
Launch();
}

void launch612()
{
videoplugin = 4;
ConfigAppSave2();
Launch();
}


// Ez0n3 - launch rom without emulator and video plugin selectors
void LaunchHideScreens(void)
{
	//ConfigAppLoad2();
	ConfigAppLoad3();
	
	/* // configappload3 should handle this
	// just to be safe
	switch (videoplugin) {
		case _VideoPluginRice510 : videoplugin = _VideoPluginRice510; //5.1.0
			break;
		case _VideoPluginRice531 : videoplugin = _VideoPluginRice531; //5.3.1
			break;
		case _VideoPluginRice560 : videoplugin = _VideoPluginRice560; //5.6.0
			break;
		case _VideoPluginRice611 : videoplugin = _VideoPluginRice611; //6.1.1
			break;
		case _VideoPluginRice612 : videoplugin = _VideoPluginRice612; //6.1.2
			break;
		default : videoplugin = _VideoPluginRice560; // launch with 560 if no plugin is set or unrecognized
			break;
	}

	// don't really need to re-set preferedemu - just to be safe
	switch (preferedemu) {
		case _UltraHLE : preferedemu = _UltraHLE; //m_emulator = _UltraHLE; 
			break;
		case _Project64 : preferedemu = _Project64; //m_emulator = _Project64; 
			break;
		case _1964 : //preferedemu = _1964; //m_emulator = _1964; 
			//break;
		default : preferedemu = _1964; // launch with 1964 if no preferred emulator is set //m_emulator = _1964; 
			break;
	}*/
	
	ConfigAppSave2();
	Launch();
}


void VideoSettingsMenu(void)
{
	ConfigAppLoad3();

	//sprintf(menuBGpath,"D:\\Skins\\%s\\Launcher\\VidSetMenuBG.png",skinname);
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];

	// Ez0n3 - more items
	//m_pSettingsMenu = XLMenu_Init(60,80,4, MENU_LEFT|MENU_WRAP, NULL);
	m_pSettingsMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,8, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	
	m_pSettingsMenu->itemcolor = dwMenuItemColor;
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Video Settings",dwMenuTitleColor);

    if (!FlickerFilter)
	swprintf(currentname,L"Flicker Filter : Disabled");
	else 
	swprintf(currentname,L"Flicker Filter : %d",FlickerFilter);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incflicker,decflicker);

	if (!SoftDisplayFilter)
	swprintf(currentname,L"Soft Display Filter : No");
	else 
	swprintf(currentname,L"Soft Display Filter : Yes");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleSoftDisplayFilter);

	switch (TextureMode){
		case 1 : 	swprintf(currentname,L"Texture Filter : Point");
			break;
		case 2 : 	swprintf(currentname,L"Texture Filter : Linear");
			break;
		case 3 : 	swprintf(currentname,L"Texture Filter : Anisotropic");
			break;
		case 4 : 	swprintf(currentname,L"Texture Filter : Quincunx");
			break;
		case 5 : 	swprintf(currentname,L"Texture Filter : Gaussian");
			break;	}
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incTextureFilter,decTextureFilter);

	switch (VertexMode)	{
	 case 0:
			swprintf(currentname,L"Vertex : Pure Device");
	    break;
	 case 1:
			swprintf(currentname,L"Vertex : Soft Vertex");
	    break;	
	 case 2:
			swprintf(currentname,L"Vertex : Hard Vertex");
	    break;
	 case 3:
			swprintf(currentname,L"Vertex : Mixed Vertex");
		break;	}
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incVertexMode,decVertexMode);


	//FrameSkip
	if (!FrameSkip)
	swprintf(currentname,L"Skip Frames : No");
	else 
	swprintf(currentname,L"Skip Frames : Yes");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleFrameSkip);

	//VSync (Fullscreen Presentation Intervals)
	switch (VSync){
		case 0 : 	swprintf(currentname,L"VSync : No");//L"VSync : INTERVAL_IMMEDIATE");
			break;
		case 1 : 	swprintf(currentname,L"VSync : Yes");//L"VSync : INTERVAL_ONE");
			break;
		case 2 : 	swprintf(currentname,L"VSync : Auto");
			break;
		/*case 2 : 	swprintf(currentname,L"VSync : INTERVAL_TWO");
			break;
		case 3 : 	swprintf(currentname,L"VSync : INTERVAL_THREE");
			break;
		case 4 : 	swprintf(currentname,L"VSync : INTERVAL_DEFAULT");
			break;
		case 5 : 	swprintf(currentname,L"VSync : INTERVAL_ONE_OR_IMMEDIATE");
			break;
		case 6 : 	swprintf(currentname,L"VSync : INTERVAL_TWO_OR_IMMEDIATE");
			break;
		case 7 : 	swprintf(currentname,L"VSync : INTERVAL_THREE_OR_IMMEDIATE");
			break;*/
	}
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incVSync,decVSync);

	if (!bEnableHDTV){
		swprintf(currentname,L"Enable 720p : No");
	}
	else
	{
		if(XGetAVPack() == XC_AV_PACK_HDTV)
        {
			//fd: Enable 720p only if there's actually an AV PACK attached to the XBOX
			swprintf(currentname,L"Enable 720p : Yes");
		}else{
			swprintf(currentname,L"Enable 720p : No");
		}
	}
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleHDTV);
	//fd - only UHLE uses this setting, defaults to 4:3
/*
	if (!bFullScreen)
		swprintf(currentname,L"UHLE Aspect Ratio 4:3");
	else 
		swprintf(currentname,L"UHLE Aspect Ratio 16:9");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleFullScreen);
*/
	switch (AntiAliasMode){
		case 0	: 	swprintf(currentname,L"AntiAliasing : None");
			break;
		case 1	: 	swprintf(currentname,L"AntiAliasing : Edge");
			break;
		case 2	: 	swprintf(currentname,L"AntiAliasing : 2x Linear");
			break;
		case 3 : 	swprintf(currentname,L"AntiAliasing : 2x Quincunx");
			break;
		case 4	: 	swprintf(currentname,L"AntiAliasing : 4x Linear");
			break;	
		case 5	: 	swprintf(currentname,L"AntiAliasing : 4x Gaussian");
			break;	
	}

	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incAAMode,decAAMode);

	XLMenu_Activate(m_pSettingsMenu);

	bool bLaunchFromMenu = false;
	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo(true);
		// Add Y - Launch button text after ui_logo draws
		m_Font.Begin();
		m_Font.DrawText((float)iControlsTxtPosX, (float)(iControlsTxtPosY+(3*itemh)), dwMenuItemColor, L"\403 Launch", GetFontAlign(iControlsTxtAlign));//480,350
		m_Font.End();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		else if (gamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_Y]){ // launch the rom from the config screen with Y
			bLaunchFromMenu = true;
			XLMenu_CurRoutine = NULL;
			XLMenu_CurMenu = NULL;
			break;
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);

	// if you want the launch screens, need to set up a parent before switching calling Launch
	// IE: just "Launch();" will show the emulator selection window, but it will lock if you try to back out to the rom list
	// Rom List		<->		Launch (1964, PJ64, UltraHLE)	<->		videoplugin
	// Main Menu	<->		Video Settings (^^Y^^)
	// jumping from Video Settings to Launch menu is ok, but pressing "B" (back) to Rom List while in Launch will crash
	if (bLaunchFromMenu) {
		LaunchHideScreens();
	}
}

void LauncherSettingsMenu(void)
{
	
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];

	m_pSettingsMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,10, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	m_pSettingsMenu->itemcolor = dwMenuItemColor;
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Launcher Settings",dwMenuTitleColor);

	// Ez0n3 - launch screens enable / disable
	if (HideLaunchScreens)
		swprintf(currentname,L"Hide Launch Screens : Yes");
	else
		swprintf(currentname,L"Hide Launch Screens : No");
		
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleHideLaunchScreens);
	
	if (EnableXMVPreview)
		swprintf(currentname,L"Enable XMV Preview : Yes");
	else
		swprintf(currentname,L"Enable XMV Preview : No");
		
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleEnableXMVPreview);
	
	if (EnableVideoAudio)
		swprintf(currentname,L"Enable XMV Audio : Yes");
	else
		swprintf(currentname,L"Enable XMV Audio : No");
		
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleEnableVideoAudio);
	
	if (EnableInfoPanel)
		swprintf(currentname,L"Enable Info Panel : Yes");
	else
		swprintf(currentname,L"Enable Info Panel : No");

	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleEnableInfoPanel);

	if (EnableBGMusic)
		swprintf(currentname,L"Enable BG Music : Yes");
	else
		swprintf(currentname,L"Enable BG Music : No");
		
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleEnableBGMusic);
	
	if (RandomBGMusic)
		swprintf(currentname,L"Random BG Music : Yes");
	else
		swprintf(currentname,L"Random BG Music : No");
		
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleRandomBGMusic);
	
	if (bAudioBoost)
		swprintf(currentname,L"Boost Audio dB : Yes");
	else
		swprintf(currentname,L"Boost Audio dB : No");
		
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleAudioBoost);

	XLMenu_AddItem(m_pSettingsMenu,MITEM_DISABLED,L"--------------------",NULL); // same size as the other one
	
	sprintf(szSkinSelect, "%s", skinname);
	swprintf(currentname, L"Skin : %S", skinname);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incSkin,decSkin);
	
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Apply Skin",ChangeSkin);
	
		
	XLMenu_Activate(m_pSettingsMenu);

	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);
}


void ToggleSkin(bool inc)
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	
	HANDLE 				hFind;
	WIN32_FIND_DATA		oFindData;
	vector<string>		vFileList;
	int					iSkinCount = 0;
	int					iSkinCurrent = 0;

	char pathskins[256];
	sprintf(pathskins, "%s*.*", szPathSkins);
	
	// get all of the skin dirs to a vector
	hFind = FindFirstFile(pathskins, &oFindData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if(oFindData.cFileName[0] == '.' ) {
				continue;
			}
			else if (oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				string szFilename (oFindData.cFileName);
			
				// uppercase first letter - optional
				if ((szFilename[0]>=97) && (szFilename[0]<=122))
					szFilename[0] = szFilename[0]-32;
			
				vFileList.push_back(szFilename);
				iSkinCount++;
			}
		} while (FindNextFile(hFind, &oFindData));
	}
	FindClose(hFind);
	
	// nothing to do
	if (iSkinCount < 2) {
		vFileList.clear();
		XLMenu_CurRoutine = NULL;
		return;
	}

	// sort the dir list
	sort(vFileList.begin(), vFileList.end(), StringRLTL);
	
	// loop through it to get the current skin
	int iVecCnt = 0;
	for (vector<string>::iterator i = vFileList.begin(); i != vFileList.end(); ++i) {
		char dir[256];
		sprintf(dir, "%s", (*i).c_str());

		if (strcmp(szSkinSelect, dir) == 0) {
			iSkinCurrent = iVecCnt;
			break;
		}
		iVecCnt++;
	}
  
	if (inc)
	{
		iSkinCurrent++;
		if (iSkinCurrent > (iSkinCount - 1)) iSkinCurrent = 0;
	}
	else
	{
		iSkinCurrent--;
		if (iSkinCurrent < 0) iSkinCurrent = (iSkinCount - 1);
	}
	
	// update the current selected skin
	sprintf(szSkinSelect, "%s", vFileList[iSkinCurrent].c_str());
	
	// clean up the vector
	//for (vector<string>::iterator i = vFileList.begin(); i != vFileList.end(); ++i) {
	//}
	vFileList.clear();

	XLMenu_CurRoutine = NULL;

	swprintf(currentname,L"Skin : %S",szSkinSelect);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
}


// Ez0n3 - enable / disable launch screens
void ToggleHideLaunchScreens()
{
	currentItem = m_pSettingsMenu->curitem;

	HideLaunchScreens = !HideLaunchScreens;
	XLMenu_CurRoutine = NULL;
	
	if (HideLaunchScreens)
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Hide Launch Screens : Yes");
	else
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Hide Launch Screens : No");
		
	ConfigAppSave();	
}

void ToggleEnableInfoPanel()
{
	currentItem = m_pSettingsMenu->curitem;

	EnableInfoPanel = !EnableInfoPanel;
	XLMenu_CurRoutine = NULL;
	
	if (EnableInfoPanel)
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable Info Panel : Yes");
	else
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable Info Panel : No");
		
	ConfigAppSave();	
}

void ToggleEnableBGMusic()
{

	currentItem = m_pSettingsMenu->curitem;

	EnableBGMusic = !EnableBGMusic;
	XLMenu_CurRoutine = NULL;
	
	if (EnableBGMusic){
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable BG Music : Yes");
		music.Play();
	}else{
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable BG Music : No");
		music.Pause();
	}
	ConfigAppSave();
}

void ToggleRandomBGMusic()
{
	currentItem = m_pSettingsMenu->curitem;

	RandomBGMusic = !RandomBGMusic;
	XLMenu_CurRoutine = NULL;
	
	if (RandomBGMusic){
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Random BG Music : Yes");
		music.SetRandom(true);
	}else{
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Random BG Music : No");
		music.SetRandom(false);
	}
	ConfigAppSave();
}

void ToggleAudioBoost()
{
	currentItem = m_pSettingsMenu->curitem;

	bAudioBoost = !bAudioBoost;
	XLMenu_CurRoutine = NULL;
	
	if (bAudioBoost){
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Boost Audio dB : Yes");
	}else{
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Boost Audio dB : No");
	}
	// re-init music if already init
	music.Stop();
	music.Initialize();
	if (EnableBGMusic)
		music.Play();

	ConfigAppSave();
}

void ToggleEnableXMVPreview()
{
	currentItem = m_pSettingsMenu->curitem;

	EnableXMVPreview = !EnableXMVPreview;
	XLMenu_CurRoutine = NULL;
	
	if (EnableXMVPreview)
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable XMV Preview : Yes");
	else
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable XMV Preview : No");
		
	ConfigAppSave();	
}

void ToggleEnableVideoAudio()
{
	extern void resetMovie();
	currentItem = m_pSettingsMenu->curitem;

	EnableVideoAudio = !EnableVideoAudio;
	resetMovie();

	XLMenu_CurRoutine = NULL;
	
	if (EnableVideoAudio){
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable XMV Audio : Yes");
		if(EnableBGMusic)
		music.Pause();
	}else{
		XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], L"Enable XMV Audio : No");
		if(EnableBGMusic)
		music.Play();
	}
		
	ConfigAppSave();	
}

void ToggleFlickerFilter(bool inc)
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
	FlickerFilter++;
    if (FlickerFilter > 5) FlickerFilter = 0;
	}
	else
	{
	FlickerFilter--;
    if (FlickerFilter < 0) FlickerFilter = 5;
	}
	//D3DDevice :: SetFlickerFilter(FlickerFilter);
	
	XLMenu_CurRoutine = NULL;
	
    if (!FlickerFilter)
	swprintf(currentname,L"Flicker Filter : Disabled");
	else 
	swprintf(currentname,L"Flicker Filter : %d",FlickerFilter);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleSoftDisplayFilter()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	SoftDisplayFilter =! SoftDisplayFilter;
	//D3DDevice::SetSoftDisplayFilter(SoftDisplayFilter);
	
	XLMenu_CurRoutine = NULL;
	
  	if (!SoftDisplayFilter)
	swprintf(currentname,L"Soft Display Filter : No");
	else 
	swprintf(currentname,L"Soft Display Filter : Yes");
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleFrameSkip()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	FrameSkip =! FrameSkip;
	
	XLMenu_CurRoutine = NULL;
	
  	if (!FrameSkip)
	swprintf(currentname,L"Skip Frames : No");
	else 
	swprintf(currentname,L"Skip Frames : Yes");
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}


void ToggleVSync(bool inc)
{
WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
	VSync++;
    if (VSync > INTERVAL_ONE_OR_IMMEDIATE/*INTERVAL_THREE_OR_IMMEDIATE*/) VSync = INTERVAL_IMMEDIATE;
	}
	else
	{
	VSync--;
    if (VSync < INTERVAL_IMMEDIATE) VSync = INTERVAL_ONE_OR_IMMEDIATE/*INTERVAL_THREE_OR_IMMEDIATE*/;
	}
	
	XLMenu_CurRoutine = NULL;
	
	switch (VSync){
		case 0 : 	swprintf(currentname,L"VSync : No");//L"VSync : INTERVAL_IMMEDIATE");
			break;
		case 1 : 	swprintf(currentname,L"VSync : Yes");//L"VSync : INTERVAL_ONE");
			break;
		case 2 : 	swprintf(currentname,L"VSync : Auto");
			break;
		/*case 2 : 	swprintf(currentname,L"VSync : INTERVAL_TWO");
			break;
		case 3 : 	swprintf(currentname,L"VSync : INTERVAL_THREE");
			break;
		case 4 : 	swprintf(currentname,L"VSync : INTERVAL_DEFAULT");
			break;
		case 5 : 	swprintf(currentname,L"VSync : INTERVAL_ONE_OR_IMMEDIATE");
			break;
		case 6 : 	swprintf(currentname,L"VSync : INTERVAL_TWO_OR_IMMEDIATE");
			break;
		case 7 : 	swprintf(currentname,L"VSync : INTERVAL_THREE_OR_IMMEDIATE");
			break;*/	
	}
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
	ConfigAppSave2();
}

void ToggleAAMode(bool inc)
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
	AntiAliasMode++;
    if (AntiAliasMode > 5) AntiAliasMode = 0;
	}
	else
	{
	AntiAliasMode--;
    if (AntiAliasMode < 0) AntiAliasMode = 5;
	}
	
	XLMenu_CurRoutine = NULL;
	
	switch (AntiAliasMode){
		case 0		: 	swprintf(currentname,L"AntiAliasing : None");
			break;
		case 1		: 	swprintf(currentname,L"AntiAliasing : Edge");
			break;
		case 2	: 	swprintf(currentname,L"AntiAliasing : 2x Linear");
			break;
		case 3 : 	swprintf(currentname,L"AntiAliasing : 2x Quincunx");
			break;
		case 4	: 	swprintf(currentname,L"AntiAliasing : 4x Linear");
			break;	
		case 5	: 	swprintf(currentname,L"AntiAliasing : 4x Gaussian");
			break;	
	}
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleTextureFilter(bool inc)
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
	TextureMode++;
    if (TextureMode > 5) TextureMode = 1;
	}
	else
	{
	TextureMode--;
    if (TextureMode < 1) TextureMode = 5;
	}
	//D3DDevice::SetTextureStageState(0, D3DTSS_MINFILTER, TextureMode);
	//D3DDevice::SetTextureStageState(0, D3DTSS_MAGFILTER, TextureMode);
	
	XLMenu_CurRoutine = NULL;
	
	switch (TextureMode){
		case 1 : 	swprintf(currentname,L"Texture Filter : Point");
			break;
		case 2 : 	swprintf(currentname,L"Texture Filter : Linear");
			break;
		case 3 : 	swprintf(currentname,L"Texture Filter : Anisotropic");
			break;
		case 4 : 	swprintf(currentname,L"Texture Filter : Quincunx");
			break;
		case 5 : 	swprintf(currentname,L"Texture Filter : Gaussian");
			break;	}
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleVertexMode(bool inc)
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
	VertexMode++;
    if (VertexMode > 3) VertexMode = 0;
	}
	else
	{
	VertexMode--;
    if (VertexMode < 0) VertexMode = 3;
	}
	
	XLMenu_CurRoutine = NULL;
	
	switch (VertexMode)	{
	 case 0: swprintf(currentname,L"Vertex : Pure Device");
	    break;
	 case 1 : swprintf(currentname,L"Vertex : Soft Vertex");
	    break;	
	 case 2 : swprintf(currentname,L"Vertex : Hard Vertex");
	    break;
	 case 3 : swprintf(currentname,L"Vertex : Mixed Vertex");
		break;	}
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleHDTV()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	bEnableHDTV =! bEnableHDTV;
	
	XLMenu_CurRoutine = NULL;
	
	if (!bEnableHDTV){
		swprintf(currentname,L"Enable 720p : No");
	}
	else 
	{
		if(XGetAVPack() == XC_AV_PACK_HDTV)
        {
			swprintf(currentname,L"Enable 720p : Yes");
		}else{
			swprintf(currentname,L"Enable 720p : No");
		}
	}
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleFullScreen()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	bFullScreen =! bFullScreen;
	
	XLMenu_CurRoutine = NULL;
	
  	if (!bFullScreen)
		swprintf(currentname,L"UHLE Aspect Ratio 4:3");
	else 
		swprintf(currentname,L"UHLE Aspect Ratio 16:9");
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ControllerSettingsMenu()
{
	//sprintf(menuBGpath,"D:\\Skins\\%s\\Launcher\\ContSetMenuBG.png",skinname);
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];

	m_pSettingsMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,7, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	m_pSettingsMenu->itemcolor = dwMenuItemColor;
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Controller Settings",dwMenuTitleColor);
	
	//Sensitivity
	switch(Sensitivity){
		case 0:
			swprintf(currentname,L"Sensitivity : Increasing");
			break;
		case 11:
			swprintf(currentname,L"Sensitivity : Decreasing");
			break;
		default:
            swprintf(currentname,L"Sensitivity : %d%%", Sensitivity * 10);
			break;
	}
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incSensitivity,decSensitivity);
	
	//Deadzone
	swprintf(currentname,L"Analog Deadzone : %2.0f%%", Deadzone);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incDeadzone,decDeadzone);

	//Mempak/RumblePak/NoPak
	if (DefaultPak == NoPak)
	{
		swprintf(currentname,L"Default Pak : None");
	}
	else if (DefaultPak == MemPak)
	{
		swprintf(currentname,L"Default Pak : MemPak");
	}
	else if (DefaultPak == RumblePak)
	{
		swprintf(currentname,L"Default Pak : RumblePak");
	}
	
	//XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,TogglePak);
	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incPakPlugin,decPakPlugin);

	//Controller States
	if (!EnableController1)
	swprintf(currentname,L"Controller 1 : Disabled");
	else 
	swprintf(currentname,L"Controller 1 : Enabled");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleController1);

	if (!EnableController2)
	swprintf(currentname,L"Controller 2 : Disabled");
	else 
	swprintf(currentname,L"Controller 2 : Enabled");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleController2);

	if (!EnableController3)
	swprintf(currentname,L"Controller 3 : Disabled");
	else 
	swprintf(currentname,L"Controller 3 : Enabled");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleController3);

	if (!EnableController4)
	swprintf(currentname,L"Controller 4 : Disabled");
	else 
	swprintf(currentname,L"Controller 4 : Enabled");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleController4);




	XLMenu_Activate(m_pSettingsMenu);

	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);

}

void ToggleSensitivity(bool inc)
{
	WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	if (inc)
	{
	Sensitivity += 1;
	if(Sensitivity > 11) Sensitivity = 0;
	}
	else
	{
    Sensitivity -= 1;
	if(Sensitivity < 0) Sensitivity = 11;
	}
	
	XLMenu_CurRoutine = NULL;
	
	switch(Sensitivity){
		case 0:
			swprintf(currentname,L"Sensitivity : Increasing");
			break;
		case 11:
			swprintf(currentname,L"Sensitivity : Decreasing");
			break;
		default:
			swprintf(currentname,L"Sensitivity : %d%%",Sensitivity * 10);
			break;
	}
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleDeadzone(bool inc)
{
	WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	
	if (inc)
	{
	Deadzone += 1;
	if(Deadzone > 100) Deadzone = 0;
	}
	else
	{
    Deadzone -= 1;
	if(Deadzone < 0) Deadzone = 100;
	}

	XBOX_CONTROLLER_DEAD_ZONE = float(32768) * (float(Deadzone)/float(100));
	
	XLMenu_CurRoutine = NULL;
	
	swprintf(currentname,L"Analog Deadzone: %2.0f%%",Deadzone);
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void TogglePak(bool inc)
{
	WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	
	if (inc)
	{
		DefaultPak++;
		if(DefaultPak > RumblePak) DefaultPak = NoPak;
	}
	else
	{
		DefaultPak--;
		if(DefaultPak < NoPak) DefaultPak = RumblePak;
	}

	XLMenu_CurRoutine = NULL;

	//Mempak/RumblePak/NoPak
	if (DefaultPak == NoPak)
	{
		swprintf(currentname,L"Default Pak : None");
	}
	else if (DefaultPak == MemPak)
	{
		swprintf(currentname,L"Default Pak : MemPak");
	}
	else if (DefaultPak == RumblePak)
	{
		swprintf(currentname,L"Default Pak : RumblePak");
	}

	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);
	ConfigAppSave2();
}

void ToggleController1()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	EnableController1 =! EnableController1;
	
	XLMenu_CurRoutine = NULL;
	
  	if (!EnableController1)
	swprintf(currentname,L"Controller 1 : Disabled");
	else 
	swprintf(currentname,L"Controller 1 : Enabled");
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleController2()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	EnableController2 =! EnableController2;
	
	XLMenu_CurRoutine = NULL;
	
  	if (!EnableController2)
	swprintf(currentname,L"Controller 2 : Disabled");
	else 
	swprintf(currentname,L"Controller 2 : Enabled");
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleController3()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	//3rd Controller bug fixed
	EnableController3 =! EnableController3;
	
	XLMenu_CurRoutine = NULL;
	
  	if (!EnableController3)
	swprintf(currentname,L"Controller 3 : Disabled");
	else 
	swprintf(currentname,L"Controller 3 : Enabled");
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

void ToggleController4()
{
    WCHAR currentname[120];
	currentItem = m_pSettingsMenu->curitem;
	EnableController4 =! EnableController4;
	
	XLMenu_CurRoutine = NULL;
	
  	if (!EnableController4)
	swprintf(currentname,L"Controller 4 : Disabled");
	else 
	swprintf(currentname,L"Controller 4 : Enabled");
	XLMenu_SetItemText(&m_pSettingsMenu->items[currentItem], currentname);

	ConfigAppSave2();
}

// controller config

extern void Drawcontrol();
extern int selectedelement;
extern bool changebutton;
int controller;
extern void ControllerReset();
XLMenu *m_pControllerMenu;

void ControllerMenu(void)
{
	//sprintf(menuBGpath,"D:\\Skins\\%s\\Launcher\\ContSetMenuBG.png",skinname);
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pSettingsMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,4, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	m_pSettingsMenu->itemcolor = dwMenuItemColor;
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Select Controller",dwMenuTitleColor);

	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Controller 1",Control1);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Controller 2",Control2);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Controller 3",Control3);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Controller 4",Control4);


	XLMenu_Activate(m_pSettingsMenu);

	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);

	ConfigAppSave2();

}

void Control1()
{
controller=0;
Control();
}
void Control2()
{
controller=1;
Control();
}
void Control3()
{
controller=2;
Control();
}
void Control4()
{
controller=3;
Control();
}

void Control()
{
	//sprintf(menuBGpath,"D:\\Skins\\%s\\Launcher\\ContMenuBG.png",skinname);
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pControllerMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,1, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	
	m_pControllerMenu->parent = m_pSettingsMenu;

	XLMenu_SetTitle(m_pControllerMenu,L" ",dwMenuTitleColor);


	XLMenu_Activate(m_pControllerMenu);

    bool bquit=false;
    selectedelement=0;

	while(!bquit)
	{
	DirectSoundDoWork();
	music.Process();
    Drawcontrol();
	XBInput_GetInput();

	if(g_Gamepads[controller].wPressedButtons	& XINPUT_GAMEPAD_DPAD_DOWN)
	{
	selectedelement++;
	if (selectedelement>18) selectedelement=0; }

	if(g_Gamepads[controller].wPressedButtons	& XINPUT_GAMEPAD_DPAD_UP)
	{
	selectedelement--;
	if (selectedelement<0) selectedelement=18; }

    if(g_Gamepads[controller].bPressedAnalogButtons[XINPUT_GAMEPAD_A]) changebutton=true;

    if(g_Gamepads[controller].bPressedAnalogButtons[XINPUT_GAMEPAD_X]) ControllerReset();

	for (int i=0;i<4;i++){
		if(g_Gamepads[i].bPressedAnalogButtons[XINPUT_GAMEPAD_B])	bquit=true;}

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	if (changebutton) {
    Drawcontrol();
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	ChangeControl();}
	}
	ConfigAppSave2();
	XLMenu_Routine(MENU_BACK);
	if (m_pControllerMenu)
		XLMenu_Delete(m_pControllerMenu);
}

// non menu align
DWORD GetFontAlign(int iAlign)
{
	switch (iAlign) {
		case 3: return XBFONT_CENTER_Y; // include?
		case 2: return XBFONT_CENTER_X;
		case 1: return XBFONT_RIGHT;
		case 0:
		default: return XBFONT_LEFT;
	}
}

// menu align
DWORD GetMenuFontAlign(int iAlign)
{
	switch (GetFontAlign(iAlign)) {
		//case XBFONT_CENTER_Y: return MENU_CENTER_Y; // include?
		case XBFONT_CENTER_X: return MENU_CENTER_X;
		case XBFONT_RIGHT: return MENU_RIGHT;
		case XBFONT_LEFT:
		default: return MENU_LEFT;
	}
}


// ---------------------------------------
// DEFAULT SETTINGS MENU - WIP
// ---------------------------------------

XLMenu *m_pConfirmMenu;
XLMenu *m_pDefaultsMenu;
extern void ResetDefaults();
extern void ResetAppDefaults();
bool bConfirmMenuChoice = false; // used by ConfirmMenu
bool ConfirmMenu(char *szConfimMsg, XLMenu *mParent);
void RestoreLauncherSettings();
void RestoreAllRomSettings();
void RestoreCurrentRomSettings();

//#define DEBUG_RESTORE_DEFAULTS 1 // for debugging

XLMenu *m_pDirectoryMenu;
void DirectoryMenu(int iDirType);
int iDirectoryType = 0;

// has to match up with menu items
enum DirectoryIndex
{
	_DirNone = 0,
	_DirRoms = 4,
	_DirMedia = 6,
	_DirSkins = 8,
	_DirSaves = 10,
	_DirScreenshots = 12
};

//extern void MoveCursorToRom(int iPos);
extern void BuildDirectories();

bool bForceRomBrowser = false;
void OpenRomBrowser()
{
	bForceRomBrowser = true;
	iDirectoryType = _DirRoms;
	DirectoryMenu(iDirectoryType);
}

void DirectorySelect()
{
	XLMenu_CurRoutine = NULL;
	WCHAR currentName[512];
	bool bReboot = false;
	
	char szDirSelect[256];
	sprintf(szDirSelect, "%s", g_PathList.GetCurrent().c_str());
	
	if (strlen(szDirSelect) < 3)
		return;
	
	char szRestoreMsg[256];
	switch (iDirectoryType) {
		case _DirRoms:			sprintf(szRestoreMsg, "Update Roms Path? (reboot)");	break;
		case _DirMedia:			sprintf(szRestoreMsg, "Update Media Path? (reboot)");	break;
		case _DirSkins:			sprintf(szRestoreMsg, "Update Skins Path? (reboot)");	break;
		case _DirSaves:			sprintf(szRestoreMsg, "Update Saves Path?");			break;
		case _DirScreenshots:	sprintf(szRestoreMsg, "Update Screenshots Path?");		break;
	}

	if (ConfirmMenu(szRestoreMsg, (bForceRomBrowser ? m_pDirectoryMenu : m_pDefaultsMenu)))
	{
		// might have to destroy/build some things
		if (iDirectoryType == _DirRoms)
		{
			sprintf(szPathRoms, "%s", szDirSelect);
			bReboot = true;
			
			// just rebooting for now until there's a way to destroy and rebuild all path dependencies
			//g_iniFile.Load("Surreal.ini");
			/*MoveCursorToRom(0);
			g_boxArtTable.Refresh();
			g_VideoTable.Refresh();
			g_romList.Refresh();*/
		}
		else if (iDirectoryType == _DirMedia)
		{
			sprintf(szPathMedia, "%s", szDirSelect);
			bReboot = true;
		}
		else if (iDirectoryType == _DirSkins)
		{
			sprintf(szPathSkins, "%s", szDirSelect);
			bReboot = true;
		}
		else if (iDirectoryType == _DirSaves)
		{
			sprintf(szPathSaves, "%s", szDirSelect);
		}
		else if (iDirectoryType == _DirScreenshots)
		{
			sprintf(szPathScreenshots, "%s", szDirSelect);
		}

		ConfigAppSave();
		BuildDirectories();
		
		if (bReboot) {
			XLaunchNewImage("D:\\default.xbe", NULL);
		}
		
		if (!bForceRomBrowser) {
			// update the disabled path items text
			swprintf(currentName,L"%S", szDirSelect);
			XLMenu_SetItemText(&m_pDefaultsMenu->items[iDirectoryType], currentName); // has to match with enum
		}
	}
}

void DirectoryMenuUpdate()
{
    WCHAR currentName[512];
	currentItem = m_pDirectoryMenu->curitem;
	XLMenu_CurRoutine = NULL;
	
	g_PathList.Update(currentItem);

	for (int i=0; i<g_PathList.GetMax(); i++)
	{
		// update the menu items text
		swprintf(currentName,L"%S", g_PathList.GetName(i).c_str());
		XLMenu_SetItemText(&m_pDirectoryMenu->items[i], currentName);

		if (g_PathList.GetName(i).length() == 0) {
			m_pDirectoryMenu->items[i].flags = MITEM_DISABLED; // disable
		}
		else {
			m_pDirectoryMenu->items[i].flags = MITEM_ROUTINE; // enable
		}

	}
	
	// update the disabled path items text
	swprintf(currentName,L"Path: %S", g_PathList.GetCurrent().c_str());
	XLMenu_SetItemText(&m_pDirectoryMenu->items[g_PathList.GetMax() + 1], currentName); // line after dashes
	
	//m_pDirectoryMenu->curitem = 0;
	m_pDirectoryMenu->curitem = g_PathList.GetSelected();
}

// browse local files
void DirectoryMenu(int iDirType)
{
	// if this menus items change, need to update DirectoryIndex enum

	//XLMenu_CurRoutine = NULL;
	WCHAR currentName[512];

	iDirectoryType = iDirType; // update the global
	
	// max num items before scroll buttons appear
	g_PathList.SetMax(10);
	int iNumMenuItems = (g_PathList.GetMax() + 3);
	
	// make room for a quit button
	if (bForceRomBrowser) {
		g_PathList.SetMax(g_PathList.GetMax() - 1);
		iNumMenuItems += 1;
	}

	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pDirectoryMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,iNumMenuItems, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);
	
	if (bForceRomBrowser)
		m_pDirectoryMenu->parent = NULL;
	else
		m_pDirectoryMenu->parent = m_pDefaultsMenu;
	
	m_pDirectoryMenu->curitem = 0;

	char szDirType[256];
	switch (iDirectoryType) {
		case _DirRoms:			sprintf(szDirType, "Roms");			break;
		case _DirMedia:			sprintf(szDirType, "Media");		break;
		case _DirSkins:			sprintf(szDirType, "Skins");		break;
		case _DirSaves:			sprintf(szDirType, "Saves");		break;
		case _DirScreenshots:	sprintf(szDirType, "Screenshots");	break;
	}
	
	if (bForceRomBrowser) {
		XLMenu_SetTitle(m_pDirectoryMenu,L"Select The Rom Directory",dwMenuTitleColor);
	}
	else {
		swprintf(currentName,L"Change Directory: %S", szDirType);
		XLMenu_SetTitle(m_pDirectoryMenu,currentName,dwMenuTitleColor);
	}

	// fill with blanks
	for (int i=0; i<g_PathList.GetMax(); i++) {
		XLMenu_AddItem(m_pDirectoryMenu,MITEM_ROUTINE,L"",DirectoryMenuUpdate);
	}
	
	// the order of these is important to other funcs
	XLMenu_AddItem(m_pDirectoryMenu,MITEM_DISABLED,L"--------------------",NULL); // = g_PathList.GetMax() + 0
	XLMenu_AddItem(m_pDirectoryMenu,MITEM_DISABLED,L"Path:",NULL); // = g_PathList.GetMax() + 1
	XLMenu_AddItem(m_pDirectoryMenu,MITEM_ROUTINE,L"Select",DirectorySelect);
	
	if (bForceRomBrowser)
		XLMenu_AddItem(m_pDirectoryMenu,MITEM_ROUTINE,L"Quit Surreal",ExitToDash);
	
	XLMenu_Activate(m_pDirectoryMenu);
	
	g_PathList.Build();
	
	DirectoryMenuUpdate();

	while(XLMenu_CurMenu == m_pDirectoryMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		
		// don't allow back if the browser is being forced
		if (bForceRomBrowser) {
			if (gamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_B] || gamepad.wPressedButtons&XINPUT_GAMEPAD_BACK)
				continue;
		}
		
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	//ConfigAppSave();
	
	g_PathList.Destroy();
	
	//XLMenu_Routine(MENU_BACK);
	if (m_pDirectoryMenu)
		XLMenu_Delete(m_pDirectoryMenu);
}

void DirectoryRomsMenu()
{
	DirectoryMenu(_DirRoms);
}
void DirectoryMediaMenu()
{
	DirectoryMenu(_DirMedia);
}
void DirectorySkinsMenu()
{
	DirectoryMenu(_DirSkins);
}
void DirectorySavesMenu()
{
	DirectoryMenu(_DirSaves);
}
void DirectoryScreenshotsMenu()
{
	DirectoryMenu(_DirScreenshots);
}

void DefaultMenuUpdatePaths()
{
	// update the path items - has to match with enum!
    WCHAR currentName[512];

	swprintf(currentName,L"%S", szPathRoms);
	XLMenu_SetItemText(&m_pDefaultsMenu->items[_DirRoms], currentName);
	
	swprintf(currentName,L"%S", szPathMedia);
	XLMenu_SetItemText(&m_pDefaultsMenu->items[_DirMedia], currentName);
	
	swprintf(currentName,L"%S", szPathSkins);
	XLMenu_SetItemText(&m_pDefaultsMenu->items[_DirSkins], currentName);
	
	swprintf(currentName,L"%S", szPathSaves);
	XLMenu_SetItemText(&m_pDefaultsMenu->items[_DirSaves], currentName);
	
	swprintf(currentName,L"%S", szPathScreenshots);
	XLMenu_SetItemText(&m_pDefaultsMenu->items[_DirScreenshots], currentName);
}

void DefaultSettingsMenu(void)
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];

	m_pDefaultsMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,13, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	m_pDefaultsMenu->itemcolor = dwMenuItemColor;
	m_pDefaultsMenu->parent = m_pMainMenu;
	m_pDefaultsMenu->curitem = 0;

	XLMenu_SetTitle(m_pDefaultsMenu,L"Default Settings",dwMenuTitleColor);

	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,L"Restore Launcher",RestoreLauncherSettings);
	
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,L"Restore All Roms",RestoreAllRomSettings);
	
	swprintf(currentname,L"Restore %S",romname);
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,currentname,RestoreCurrentRomSettings);

	XLMenu_AddItem(m_pDefaultsMenu,MITEM_DISABLED,L"Rom Path :",NULL);
	swprintf(currentname,L"%S", szPathRoms);
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,currentname,DirectoryRomsMenu);
	
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_DISABLED,L"Media Path :",NULL);
	swprintf(currentname,L"%S", szPathMedia);
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,currentname,DirectoryMediaMenu);
	
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_DISABLED,L"Skin Path :",NULL);
	swprintf(currentname,L"%S", szPathSkins);
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,currentname,DirectorySkinsMenu);
	
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_DISABLED,L"Save Path :",NULL);
	swprintf(currentname,L"%S", szPathSaves);
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,currentname,DirectorySavesMenu);
	
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_DISABLED,L"Screenshot Path :",NULL);
	swprintf(currentname,L"%S", szPathScreenshots);
	XLMenu_AddItem(m_pDefaultsMenu,MITEM_ROUTINE,currentname,DirectoryScreenshotsMenu);
	
		
	XLMenu_Activate(m_pDefaultsMenu);

	while( XLMenu_CurMenu == m_pDefaultsMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pDefaultsMenu)
		XLMenu_Delete(m_pDefaultsMenu);
}

void RestoreLauncherSettings()
{
	XLMenu_CurRoutine = NULL;
	
	bool bReboot = false;
	
	// need to reboot if it's going to change one of these for now
	if (g_iniFile.IsLoaded()) {
		if (strcmp(szPathRoms, g_iniFile.GetRomPath().c_str()) != 0 || strcmp(szPathMedia, g_iniFile.GetMediaPath().c_str()) != 0 || strcmp(szPathSkins, g_iniFile.GetSkinPath().c_str()) != 0)
				bReboot = true;
	}
	else {
		if (strcmp(szPathRoms, "D:\\Roms\\") != 0 || strcmp(szPathMedia, "D:\\Media\\") != 0 || strcmp(szPathSkins, "D:\\Skins\\") != 0)
				bReboot = true;
	}

	char szRestoreMsg[256];
	
	if (bReboot)
		sprintf(szRestoreMsg, "Restore Launcher Defaults? (reboot)");
	else
		sprintf(szRestoreMsg, "Restore Launcher Defaults?");
	
	if (ConfirmMenu(szRestoreMsg, m_pDefaultsMenu))
	{
		char szRemIniFile[256] = "T:\\surreal-ce.ini";
		
#ifndef DEBUG_RESTORE_DEFAULTS
		if (PathFileExists(szRemIniFile)) {
			if (!DeleteFile(szRemIniFile)) {
				OutputDebugString(szRemIniFile);
				OutputDebugStringA(" Could Not Be Deleted!\n");
			}
		}
#else
		if (PathFileExists(szRemIniFile)) {
			OutputDebugString(szRemIniFile);
			OutputDebugStringA(" Is The File To Be Deleted!\n");
		} else {
			OutputDebugString(szRemIniFile);
			OutputDebugStringA(" Does Not Exist!\n");
		}
#endif

		if (bReboot) {
			XLaunchNewImage("D:\\default.xbe", NULL);
		}

		ResetAppDefaults();
		ConfigAppLoadPaths();
		ConfigAppLoad();
		
		DefaultMenuUpdatePaths();
		
		music.Stop();
		music.Initialize();
		if (EnableBGMusic)
			music.Play();
			
		char msg[100] = "Launcher Defaults Restored";
		ShowTempMessage(msg);
	}
}

void RestoreAllRomSettings()
{
	XLMenu_CurRoutine = NULL;
	
	char szRestoreMsg[256] = "Restore All Rom Defaults?";

	if (ConfirmMenu(szRestoreMsg, m_pDefaultsMenu))
	{
		HANDLE 				hFind;
		WIN32_FIND_DATA		oFindData;
		
		// remove the all the ini files from the Saves dir
		char szRemSettings[256];
		sprintf(szRemSettings, "%s*.*", szPathSaves);
		
		hFind = FindFirstFile(szRemSettings, &oFindData);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if(oFindData.cFileName[0] == '.' || strlen(oFindData.cFileName) != 8){
					continue;
				}
				else if (oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char szRemIniFile[256];
					sprintf(szRemIniFile, "%s%s\\%s.ini", szPathSaves, oFindData.cFileName, oFindData.cFileName);
					
#ifndef DEBUG_RESTORE_DEFAULTS
					if (PathFileExists(szRemIniFile)) {
						if (!DeleteFile(szRemIniFile)) {
							OutputDebugString(szRemIniFile);
							OutputDebugStringA(" Could Not Be Deleted!\n");
						}
					}
#else
					if (PathFileExists(szRemIniFile)) {
						OutputDebugString(szRemIniFile);
						OutputDebugStringA(" Is The File To Be Deleted!\n");
					} else {
						OutputDebugString(szRemIniFile);
						OutputDebugStringA(" Does Not Exist!\n");
					}
#endif
				}
			} while (FindNextFile(hFind, &oFindData));
		}
		FindClose(hFind);
			
		ResetDefaults();
		ConfigAppLoad3();
		
		char msg[100] = "All Rom Defaults Restored";
		ShowTempMessage(msg);
	}
}

void RestoreCurrentRomSettings()
{
	XLMenu_CurRoutine = NULL;
	//char romnametrunc[256];
	char szRestoreMsg[256];
	extern int MenuTrunc;
	
	/*for(int i=0; i<=(MenuTrunc-18); i++){ // Truncate the Rom Name when it creates a long menu title
		romnametrunc[i] = romname[i];
		if(i == MenuTrunc)
			romnametrunc[i+1]= '/0';
	}*/
	// maybe set up a func that can be re-used? Truncate "true" will draw ellipses
	sprintf(szRestoreMsg, "Restore %s?", Truncate(string(romname), MenuTrunc, false).c_str()); //romnametrunc //(MenuTrunc-18) // Defaults
	
	if (ConfirmMenu(szRestoreMsg, m_pDefaultsMenu))
	{
		Rom *rom = g_romList.GetRomAt(actualrom);

		char szRemIniFile[256];
		sprintf(szRemIniFile, "%s%08x\\%08x.ini", szPathSaves, rom->m_dwCrc1, rom->m_dwCrc1);
		
#ifndef DEBUG_RESTORE_DEFAULTS
		if (PathFileExists(szRemIniFile)) {
			if (!DeleteFile(szRemIniFile)) {
				OutputDebugString(szRemIniFile);
				OutputDebugStringA(" Could Not Be Deleted!\n");
			}
		}
#else
		if (PathFileExists(szRemIniFile)) {
			OutputDebugString(szRemIniFile);
			OutputDebugStringA(" Is The File To Be Deleted!\n");
		} else {
			OutputDebugString(szRemIniFile);
			OutputDebugStringA(" Does Not Exist!\n");
		}
#endif
		ResetDefaults();
		ConfigAppLoad3();
		
		char msg[100];
		sprintf(msg, "%s Defaults Restored", romname);
		ShowTempMessage(msg);
	}
}

void ConfirmMenuNo()
{
	bConfirmMenuChoice = false;
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
}

void ConfirmMenuYes()
{
	bConfirmMenuChoice = true;
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
}

bool ConfirmMenu(char *szConfimMsg, XLMenu *mParent)
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];
	
	bConfirmMenuChoice = false;

	m_pConfirmMenu = XLMenu_Init((float)iMainMenuTxtPosX, (float)iMainMenuTxtPosY,2, GetMenuFontAlign(iMainMenuTxtAlign)|MENU_WRAP, NULL);

	m_pConfirmMenu->itemcolor = dwMenuItemColor;
	m_pConfirmMenu->parent = mParent;
	m_pConfirmMenu->curitem = 0;

	swprintf(currentname,L"%S", szConfimMsg);
	XLMenu_SetTitle(m_pConfirmMenu,currentname,dwMenuTitleColor);

	XLMenu_AddItem(m_pConfirmMenu,MITEM_ROUTINE,L"No",ConfirmMenuNo);
	XLMenu_AddItem(m_pConfirmMenu,MITEM_ROUTINE,L"Yes",ConfirmMenuYes);
	
	XLMenu_Activate(m_pConfirmMenu);

	while( XLMenu_CurMenu == m_pConfirmMenu)
	{
		DrawLogo(true);
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		//freakdave - check for music playing
		if(gamepad.wPressedButtons&XINPUT_GAMEPAD_RIGHT_THUMB){
			if(EnableBGMusic){
				if(music.IsPlaying()){
					music.Pause();
				}else{
					music.Play();
				}
			}
		}
		else if(gamepad.wPressedButtons&XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pConfirmMenu)
		XLMenu_Delete(m_pConfirmMenu);
		
	// return to previous menu
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = mParent;
	
	return bConfirmMenuChoice;
}
