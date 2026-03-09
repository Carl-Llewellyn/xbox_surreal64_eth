#include "../config.h"
#include "../ultraxle/IoSupport.h"
#include <xbapp.h>
#include <xbresource.h>
#include <xbfont.h>
#include "../Ingamemenu/xlmenu.h"
#include "Panel.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
using namespace std;

void GetStateTimestamp(int index, char *timestamp);
extern "C" void __EMU_GetStateFilename(int index, char *timestamp, int mode);
void CreateSaveStatePreview(unsigned int index);
bool LoadSaveStatePreview(unsigned int index);

extern void _VIDEO_DisplayTemporaryMessage(const char *msg);

//weinerschnitzel - Skin Control
extern DWORD dwMenuItemColor;
extern DWORD dwMenuTitleColor;
extern char skinname[32];
extern int iIGMMenuTxtPosX;
extern int iIGMMenuTxtPosY;
extern int iIGMMenuTxtAlign;
extern int iPanelX;
extern int iPanelY;
extern int iPanelNW;
extern int iPanelNH;
extern int iIGMMenuTxtPosX_HD;
extern int iIGMMenuTxtPosY_HD;
extern int iIGMMenuTxtAlign_HD;
extern int iPanelX_HD;
extern int iPanelY_HD;
extern int iPanelNW_HD;
extern int iPanelNH_HD;
extern bool isIGM;
extern int iIGMHilightPosX;
extern int iIGMHilightPosX_HD;

DWORD GetFontAlign(int iAlign);
DWORD GetMenuFontAlign(int iAlign);

//freakdave
enum Emulators
{
	_1964,
	_Project64,
	_UltraHLE,
	_None
};

typedef struct _LaunchData
{

	// Ez0n3 - launchdata isn't used anymore, but this struct is used for exit to dash "LD_LAUNCH_DASHBOARD LaunchData" i believe
	/*
	// 1964 settings
	DWORD dw1964DynaMem;
	DWORD dw1964PagingMem;
	
	// Pj64 settings
	DWORD dwPJ64DynaMem;
	DWORD dwPJ64PagingMem;

	// common settings
	BOOL bUseLLERSP;
	
	// Ez0n3 - reinstate max video mem
	DWORD dwMaxVideoMem;
	*/
	
} LaunchData;

LPDIRECT3DTEXTURE8 pStateTexture = NULL;

/*#define SAVESTATE_XPOS 60
#define SAVESTATE_YPOS 270
#define SAVESTATE_XWIDTH 256
#define SAVESTATE_YHEIGHT 143//128 (256:143 = 16:9) (256:191 = 4:3)*/
extern int iIGMStateScreenX;
extern int iIGMStateScreenY;
extern int iIGMStateScreenW;
extern int iIGMStateScreenH;
extern int iIGMStateScreenX_HD;
extern int iIGMStateScreenY_HD;
extern int iIGMStateScreenW_HD;
extern int iIGMStateScreenH_HD;

XBGAMEPAD gamepad;

extern CPanel m_RenderPanel;
extern CPanel m_BgPanel;
extern CPanel m_MenuBgPanel;
extern CPanel m_MenuBg2Panel;
extern CPanel m_ControlConfigPanel;
extern void TakeScreenshot();

XLMenu    *m_pMainMenu;
XLMenu    *m_pSettingsMenu;
XLMenu    *m_pControllerMenu;
extern CXBFont		m_Font;					// Font	for	text display

extern void DrawLogo();
extern void DestroyLogo();
static int currentItem;

void ExitToRomList(); 
void ExitToDash();
void ShutdownXbox();
void ToggleCredits();
void ToggleFlickerFilter(bool inc);
void VideoSettingsMenu();
void ToggleTextureFilter(bool inc);
void ToggleSoftDisplayFilter();
void ToggleFrameSkip();
void ToggleSensitivity(bool inc);
void ToggleDeadzone(bool inc);
void ControllerSettingsMenu();
void TogglePak(bool inc);
void ShowDebug();
void ResetRom();

// controller config

void Control();
void Control1();
void Control2();
void Control3();
void Control4();
void ControllerMenu();
extern void ChangeControl();

// Load-Save state

void LoadStateMenu();
void LoadState1();
void LoadState2();
void LoadState3();
void LoadState4();
void LoadState5();
void SaveStateMenu();
void SaveState1();
void SaveState2();
void SaveState3();
void SaveState4();
void SaveState5();
bool bloadstate[MAX_SAVE_STATES]; //5
bool bsavestate[MAX_SAVE_STATES]; //5
bool bSatesUpdated = false;

extern void CalculateEndCredits();
extern void DrawCredits();
extern int defilement;
extern void InitLogo();

//
void incflicker(){ ToggleFlickerFilter(true); }
void decflicker(){ ToggleFlickerFilter(false); }
void incTextureFilter(){ ToggleTextureFilter(true); }
void decTextureFilter(){ ToggleTextureFilter(false); }
void incSensitivity(){ ToggleSensitivity(true); }
void decSensitivity(){ ToggleSensitivity(false); }
void incDeadzone(){ ToggleDeadzone(true); }
void decDeadzone(){ ToggleDeadzone(false); }

void incPakPlugin(){ TogglePak(true); }
void decPakPlugin(){ TogglePak(false); }
//
extern bool showdebug;
extern bool bEnableHDTV;
extern bool bFullScreen;

enum Pak
{
	NoPak = 1,
	MemPak = 2,
	RumblePak = 3
};
extern bool tookscreenshot;
void MainMenu(void)
{
	isIGM = true;
	InitLogo();

	if (pStateTexture) {
		pStateTexture->Release();
		pStateTexture = NULL;
	}

	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	int x = 0;
	int y = 0;
	int nx = 640;
	int ny = 480;

	//fd - hack to properly display HD IGM
	DWORD videoFlags = XGetVideoFlags();
	if(XGetAVPack() == XC_AV_PACK_HDTV){
		if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV)
		{
			nx = 1280;
			ny = 720;
			iPanelX = iPanelX_HD;
			iPanelY = iPanelY_HD;
			iPanelNW = iPanelNW_HD;
			iPanelNH = iPanelNH_HD;
			iIGMMenuTxtPosX = iIGMMenuTxtPosX_HD;
			iIGMMenuTxtPosY = iIGMMenuTxtPosY_HD;
			iIGMHilightPosX = iIGMHilightPosX_HD;
			iIGMMenuTxtAlign = iIGMMenuTxtAlign_HD;
			
			iIGMStateScreenX = iIGMStateScreenX_HD;
			iIGMStateScreenY = iIGMStateScreenY_HD;
			iIGMStateScreenW = iIGMStateScreenW_HD;
			iIGMStateScreenH = iIGMStateScreenH_HD;
		}
	}
	
	if (iIGMStateScreenW > 512/*256*/)
		iIGMStateScreenW = 512/*256*/;
	if (iIGMStateScreenH > 512/*256*/)
		iIGMStateScreenH = 512/*256*/;
	
	m_pMainMenu = XLMenu_Init((float)iIGMMenuTxtPosX,(float)iIGMMenuTxtPosY,12, GetMenuFontAlign(iIGMMenuTxtAlign)|MENU_WRAP,NULL);

	m_pMainMenu->itemcolor = dwMenuItemColor;
	m_pMainMenu->parent = NULL;

	while ((x != iPanelX) || (y != iPanelY) || (nx != iPanelNW) || (ny != iPanelNH))
	{
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER , D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
		m_BgPanel.Render(0,0);

		x = x + 32;
		y = y + 8;
		nx = nx - 38;
		ny = ny - 28;

		if (x >iPanelX)
			x = iPanelX;

		if (y >iPanelY)
			y = iPanelY;

		if (nx <iPanelNW)
			nx = iPanelNW;

		if (ny <iPanelNH)
			ny = iPanelNH;

		m_RenderPanel.Render((float)x ,(float)y , (float)nx , (float)ny , false, 0);
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	}

	XLMenu_SetTitle(m_pMainMenu,L"Main Menu",dwMenuTitleColor);

	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Load State",LoadStateMenu);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Save State",SaveStateMenu);
	if (showdebug) 
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Show Debug info : on",ShowDebug);
	else
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Show Debug info : off",ShowDebug);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Video Settings",VideoSettingsMenu);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Controller Settings",ControllerSettingsMenu);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Controller Config",ControllerMenu);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Take a Screenshot",TakeScreenshot);
	XLMenu_AddItem(m_pMainMenu,MITEM_DISABLED,L"------------------------",NULL);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Reset Rom",ResetRom);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Quit To RomList",ExitToRomList);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Quit Surreal",ExitToDash);
	XLMenu_AddItem(m_pMainMenu,MITEM_ROUTINE,L"Shutdown Xbox",ShutdownXbox);

	XLMenu_Activate(m_pMainMenu);

	while( XLMenu_CurMenu == m_pMainMenu)
	{
		DrawLogo();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		if (tookscreenshot) {
		tookscreenshot=false;
        XLMenu_Activate(NULL);}
	}
	
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;

	if (m_pMainMenu)
		XLMenu_Delete(m_pMainMenu);

	DestroyLogo();

}

void ShowDebug()
{
	WCHAR currentname[120];
	currentItem = m_pMainMenu->curitem;
	showdebug =! showdebug;
	
	XLMenu_CurRoutine = NULL;
	
  	if (showdebug)
	swprintf(currentname,L"Show Debug info : on");
	else 
	swprintf(currentname,L"Show Debug info : off");
	XLMenu_SetItemText(&m_pMainMenu->items[currentItem], currentname);
}

void ExitToRomList() 
{
	// Ez0n3 - launchdata isn't used anymore
	//LAUNCH_DATA ld;
	//ZeroMemory(&ld, sizeof(LAUNCH_DATA));
	
	//Ez0n3 - passing the zeroed launch data crashes surreal sometimes (usually 1964/UltraHLE - unknown reason). NULL seems to be more reliable with no adverse affects.
	//XLaunchNewImage("D:\\default.xbe", &ld);
	//weinersch - 4xAA will cause a frag when using PersistDisplay(), let's only use it without AA
	if(AntiAliasMode == 0)
		D3DDevice::PersistDisplay();
	XLaunchNewImage("D:\\default.xbe", NULL);
}

void ExitToDash(void)
{
	D3DDevice::PersistDisplay();
	LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };
	XLaunchNewImage( NULL, (LAUNCH_DATA*)&LaunchData );
}

void ShutdownXbox(void)
{
	CIoSupport IOSupport;
	IOSupport.Shutdown();
}

void ResetRom(void)
{
	char szLaunchXBE[128];
	
	if (preferedemu == _UltraHLE)
	{
		sprintf(szLaunchXBE, "D:\\UltraHLE.xbe");
	}
	else
	{
		char szEmulator[16];
		char szAudioPlugin[16];
		char szVideoPlugin[16];
		//char szPagingMethod[16];
		
		switch (preferedemu) {
			case _1964: 		sprintf(szEmulator, "1964"); break;
			case _Project64: 	sprintf(szEmulator, "PJ64"); break;
			//case _UltraHLE: 	sprintf(szEmulator, "UltraHLE"); break;
			default:
				sprintf(szEmulator, "1964"); break;
		}
		
		switch (videoplugin) {
			case _VideoPluginRice510: 	sprintf(szVideoPlugin, "-510"); break;
			case _VideoPluginRice531: 	sprintf(szVideoPlugin, "-531"); break;
			case _VideoPluginRice560: 	sprintf(szVideoPlugin, "-560"); break;
			case _VideoPluginRice611: 	sprintf(szVideoPlugin, "-611"); break;
			case _VideoPluginRice612: 	sprintf(szVideoPlugin, "-612"); break;
			default:
				sprintf(szVideoPlugin, "-560"); break;
		}
		
		switch (iAudioPlugin) {
			//case _AudioPluginNone: 	sprintf(szAudioPlugin, ""); break;
			//case _AudioPluginBasic: 	sprintf(szAudioPlugin, ""); break;
			//case _AudioPluginJttl: 	sprintf(szAudioPlugin, ""); break;
			//case _AudioPluginAzimer: 	sprintf(szAudioPlugin, ""); break;
			case _AudioPluginMusyX: 	sprintf(szAudioPlugin, "M"); break;
			default:
				sprintf(szAudioPlugin, ""); break;
		}

		sprintf(szLaunchXBE, "D:\\%s%s%s.xbe", szEmulator, szVideoPlugin, szAudioPlugin);
	}

	OutputDebugString("Launching: ");
	OutputDebugString(szLaunchXBE);
	OutputDebugString("\n");
	//Sleep(100); // to see debug string
	
	// create a temporary pagefile just in case something goes wrong
	CreateDirectory("T:\\Temp", NULL);
	HANDLE hFile = CreateFile( "T:\\Temp\\codetemp.dat", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL );
	CloseHandle(hFile);

	// launch the file
	XLaunchNewImage(szLaunchXBE, NULL);
}

void GetStateTimestamp(int index, char *timestamp)
{
	char filename[255];
	__EMU_GetStateFilename(index, filename, 0);

	struct _stat buf;
	if(_stat(filename, &buf ) != 0) {
		sprintf(timestamp, "%i: Empty", index);
	} else {
		char time[100] = "";
		strftime(time, 100, "%Y-%m-%d %H:%M:%S", localtime(&buf.st_mtime));
		sprintf(timestamp, "%i: %s", index, time);
	}
}

void CreateSaveStatePreview(unsigned int index)
{
	if (pStateTexture) {
		pStateTexture->Release();
		pStateTexture = NULL;
	}

	//we need a temporary surface
	D3DSurface *surf = NULL;
	//D3DSURFACE_DESC surfdsc;

	//D3DTexture *tex = NULL;
	D3DSurface *texsurf = NULL;

	//clear from possible artifacts
	//FIXME: destroy panels here?
	//Clear Skin Images
	m_ControlConfigPanel.Destroy();
	m_MenuBgPanel.Destroy();
	m_MenuBg2Panel.Destroy();
	m_BgPanel.Destroy();

	//Render the taken ScreenShot
	m_RenderPanel.Render(0,0);

	//Clear the Artifacts
	g_pd3dDevice->Present(0, 0, 0, 0);

	//get the backbuffer surface and its description
	g_pd3dDevice->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &surf);
	//surf->GetDesc(&surfdsc);

	//create the preview texture
	//g_pd3dDevice->CreateTexture(/*iIGMStateScreenW*/128, /*iIGMStateScreenH*/128, 0, D3DX_DEFAULT, D3DFMT_LIN_A8R8G8B8/*surfdsc.Format*/, D3DPOOL_DEFAULT, &tex);

	//go down to surface level
	//tex->GetSurfaceLevel(0, &texsurf);
	g_pd3dDevice->CreateImageSurface(256, 256, D3DFMT_LIN_A8R8G8B8, &texsurf);

	//copy the backbuffer surface to the texture surface
	D3DXLoadSurfaceFromSurface(texsurf, NULL, NULL, surf, NULL, NULL, D3DX_DEFAULT, 0);

	char filename[255];
	__EMU_GetStateFilename((index+1), filename, 1);

	//write out the preview image
	XGWriteSurfaceToFile(texsurf, filename);

	//release both surfaces
	texsurf->Release();
	texsurf = NULL;
	surf->Release();
	surf = NULL;

	//release the texture
	//tex->Release();
	//tex = NULL;
	
	LoadSaveStatePreview(index); // show new one
}

bool LoadSaveStatePreview(unsigned int index)
{
	if (pStateTexture) {
		pStateTexture->Release();
		pStateTexture = NULL;
	}
	
	//load our texture from file
	char filename[255];
	__EMU_GetStateFilename((index+1), filename, 1);
	
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	
	if (iIGMStateScreenW > iIGMStateScreenH) {
		iScreenWidth = iIGMStateScreenW;
		iScreenHeight = iIGMStateScreenW;
	} else {
		iScreenWidth = iIGMStateScreenH;
		iScreenHeight = iIGMStateScreenH;
	}
	
	if(PathFileExists(filename)){
		D3DXCreateTextureFromFileEx(g_pd3dDevice, filename, iScreenWidth, iScreenHeight, 0, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0x00000000, NULL, NULL, &pStateTexture);
	}else{
		//D3DXCreateTextureFromFileEx(g_pd3dDevice, "D:\\Media\\ui\\defsave.png", 256, 128, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0x00000000,NULL, NULL, &saveGamePreview[index].texture);
		char pathmedia[256];
		
		if (bEnableHDTV)
			sprintf(pathmedia, "%sui\\defsave_hd.png", szPathMedia);
		else
			sprintf(pathmedia, "%sui\\defsave.png", szPathMedia);
		
		if(PathFileExists(pathmedia)){
			D3DXCreateTextureFromFileEx(g_pd3dDevice, pathmedia, iScreenWidth, iScreenHeight, 0, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0x00000000, NULL, NULL, &pStateTexture);
		}else{
			return false;
		}
	}
	
	return true;
}

void LoadStateMenu(void)
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pSettingsMenu = XLMenu_Init((float)iIGMMenuTxtPosX,(float)iIGMMenuTxtPosY,MAX_SAVE_STATES, GetMenuFontAlign(iIGMMenuTxtAlign)|MENU_WRAP, NULL);

	m_pSettingsMenu->itemcolor = dwMenuItemColor;
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Load State",dwMenuTitleColor);

  	/*XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Load State 1",LoadState1);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Load State 2",LoadState2);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Load State 3",LoadState3);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Load State 4",LoadState4);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Load State 5",LoadState5);*/
	
	char timestamp[256];
	WCHAR currentname[256];
	
	GetStateTimestamp(1, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,LoadState1);
	//LoadSaveStatePreview(0);
	
	GetStateTimestamp(2, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,LoadState2);
	//LoadSaveStatePreview(1);
	
	GetStateTimestamp(3, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,LoadState3);
	//LoadSaveStatePreview(2);
	
	GetStateTimestamp(4, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,LoadState4);
	//LoadSaveStatePreview(3);

	GetStateTimestamp(5, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,LoadState5);
	//LoadSaveStatePreview(4);

	XLMenu_Activate(m_pSettingsMenu);

	bool bLoadPreview = false;
	
	//int selected = 0;
	int selected = m_pSettingsMenu->curitem;
	bLoadPreview = LoadSaveStatePreview(selected); // just the 1st one
/*
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	
	if (iIGMStateScreenW > iIGMStateScreenH) {
		iScreenWidth = iIGMStateScreenW;
		iScreenHeight = iIGMStateScreenW;
	} else {
		iScreenWidth = iIGMStateScreenH;
		iScreenHeight = iIGMStateScreenH;
	}
*/
	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		
		// update preview on change
		if (selected != m_pSettingsMenu->curitem) {
			selected = m_pSettingsMenu->curitem;
			
			bLoadPreview = LoadSaveStatePreview(selected);
		}
		
		//@weinersch do your skinning magic here
		if (bLoadPreview) {
		g_pd3dDevice->SetTexture(0, pStateTexture);

		g_pd3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);
		g_pd3dDevice->Begin(D3DPT_QUADLIST);
		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 0.0f, 0.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iIGMStateScreenX,  (float)iIGMStateScreenY, 0.0f, 1.0f );

		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 1.0f, 0.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)(iIGMStateScreenX + iIGMStateScreenW/*iScreenWidth*/),  (float)iIGMStateScreenY,    0.0f, 1.0f );

		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 1.0f, 1.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)(iIGMStateScreenX + iIGMStateScreenW/*iScreenWidth*/), (float)(iIGMStateScreenY + iIGMStateScreenH/*iScreenHeight*/),    0.0f, 1.0f );

		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 0.0f, 1.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iIGMStateScreenX, (float)(iIGMStateScreenY + iIGMStateScreenH/*iScreenHeight*/), 0.0f, 1.0f );
		g_pd3dDevice->End();

		g_pd3dDevice->SetTexture(0, NULL);
		}

		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (pStateTexture) {
		pStateTexture->Release();
		pStateTexture = NULL;
	}

	if (m_pSettingsMenu){
		XLMenu_Delete(m_pSettingsMenu);
	}
}

void LoadState1()
{
	bloadstate[0]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Loaded State 1");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
}

void LoadState2()
{
	bloadstate[1]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Loaded State 2");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
}

void LoadState3()
{
	bloadstate[2]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Loaded State 3");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
}

void LoadState4()
{
	bloadstate[3]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Loaded State 4");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
}

void LoadState5()
{
	bloadstate[4]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Loaded State 5");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
}

void SaveStateMenu(void)
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);


	m_pSettingsMenu = XLMenu_Init((float)iIGMMenuTxtPosX,(float)iIGMMenuTxtPosY,MAX_SAVE_STATES, GetMenuFontAlign(iIGMMenuTxtAlign)|MENU_WRAP, NULL);

	m_pSettingsMenu->itemcolor = dwMenuItemColor;
	m_pSettingsMenu->parent = m_pMainMenu;

	XLMenu_SetTitle(m_pSettingsMenu,L"Save State",dwMenuTitleColor);

  	/*XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Save State 1",SaveState1);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Save State 2",SaveState2);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Save State 3",SaveState3);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Save State 4",SaveState4);
  	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,L"Save State 5",SaveState5);*/

	char timestamp[256];
	WCHAR currentname[256];
	
	GetStateTimestamp(1, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,SaveState1);
	//LoadSaveStatePreview(0);
	
	GetStateTimestamp(2, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,SaveState2);
	//LoadSaveStatePreview(1);

	GetStateTimestamp(3, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,SaveState3);
	//LoadSaveStatePreview(2);

	GetStateTimestamp(4, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,SaveState4);
	//LoadSaveStatePreview(3);

	GetStateTimestamp(5, timestamp);
	swprintf(currentname,L"%S",timestamp);
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,SaveState5);
	//LoadSaveStatePreview(4);

	XLMenu_Activate(m_pSettingsMenu);

	bool bLoadPreview = false;
	
	//int selected = 0;
	int selected = m_pSettingsMenu->curitem;
	bLoadPreview = LoadSaveStatePreview(selected); // just the 1st one
/*
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	
	if (iIGMStateScreenW > iIGMStateScreenH) {
		iScreenWidth = iIGMStateScreenW;
		iScreenHeight = iIGMStateScreenW;
	} else {
		iScreenWidth = iIGMStateScreenH;
		iScreenHeight = iIGMStateScreenH;
	}
	*/
	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);

		// update preview on change
		if (selected != m_pSettingsMenu->curitem) {
			selected = m_pSettingsMenu->curitem;
			
			bLoadPreview = LoadSaveStatePreview(selected);
		}

		//fd: FIXME!
		//@weinersch do your skinning magic here
		if (bLoadPreview) {
		g_pd3dDevice->SetTexture(0, pStateTexture);

		g_pd3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);
		g_pd3dDevice->Begin(D3DPT_QUADLIST);
		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 0.0f, 0.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iIGMStateScreenX,  (float)iIGMStateScreenY, 0.0f, 1.0f );

		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 1.0f, 0.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)(iIGMStateScreenX + iIGMStateScreenW/*iScreenWidth*/),  (float)iIGMStateScreenY,    0.0f, 1.0f );

		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 1.0f, 1.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)(iIGMStateScreenX + iIGMStateScreenW/*iScreenWidth*/), (float)(iIGMStateScreenY + iIGMStateScreenH/*iScreenHeight*/),    0.0f, 1.0f );

		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 0.0f, 1.0f );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iIGMStateScreenX, (float)(iIGMStateScreenY + iIGMStateScreenH/*iScreenHeight*/), 0.0f, 1.0f );
		g_pd3dDevice->End();

		g_pd3dDevice->SetTexture(0, NULL);
		}
		
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (pStateTexture) {
		pStateTexture->Release();
		pStateTexture = NULL;
	}

	if (m_pSettingsMenu){
		XLMenu_Delete(m_pSettingsMenu);
	}
}


void SaveState1()
{
	bsavestate[0]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Saved State 1");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	CreateSaveStatePreview(0);
}

void SaveState2()
{
	bsavestate[1]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Saved State 2");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	CreateSaveStatePreview(1);
}

void SaveState3()
{
	bsavestate[2]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Saved State 3");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	CreateSaveStatePreview(2);
}

void SaveState4()
{
	bsavestate[3]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Saved State 4");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	CreateSaveStatePreview(3);
}

void SaveState5()
{
	bsavestate[4]=true;
	bSatesUpdated=true;
	_VIDEO_DisplayTemporaryMessage("Saved State 5");
	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	CreateSaveStatePreview(4);
}

void VideoSettingsMenu(void)
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];

	m_pSettingsMenu = XLMenu_Init((float)iIGMMenuTxtPosX,(float)iIGMMenuTxtPosY,5, GetMenuFontAlign(iIGMMenuTxtAlign)|MENU_WRAP, NULL);

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

	//FrameSkip
	if (!FrameSkip)
	swprintf(currentname,L"Skip Frames : No");
	else 
	swprintf(currentname,L"Skip Frames : Yes");
	XLMenu_AddItem(m_pSettingsMenu,MITEM_ROUTINE,currentname,ToggleFrameSkip);

	XLMenu_Activate(m_pSettingsMenu);


	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	}

	if (m_pSettingsMenu)
		XLMenu_Delete(m_pSettingsMenu);

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
	D3DDevice :: SetFlickerFilter(FlickerFilter);

	
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
	D3DDevice::SetSoftDisplayFilter(SoftDisplayFilter);

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
	D3DDevice::SetTextureStageState(0, D3DTSS_MINFILTER, TextureMode);
	D3DDevice::SetTextureStageState(0, D3DTSS_MAGFILTER, TextureMode);

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

void ControllerSettingsMenu()
{
	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

    WCHAR currentname[120];

	m_pSettingsMenu = XLMenu_Init((float)iIGMMenuTxtPosX,(float)iIGMMenuTxtPosY,3, GetMenuFontAlign(iIGMMenuTxtAlign)|MENU_WRAP, NULL);

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
			swprintf(currentname,L"Sensitivity : %d%%",Sensitivity * 10);
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

	XLMenu_AddItem2(m_pSettingsMenu,MITEM_ROUTINE,currentname,incPakPlugin,decPakPlugin);

	XLMenu_Activate(m_pSettingsMenu);

	while( XLMenu_CurMenu == m_pSettingsMenu)
	{
		DrawLogo();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
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
	//freakdave
	//Don't forget to alter the floats in the brackets accordingly if you change XBOX_CONTROLLER_DEAD_ZONE
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


// controller config

extern void Drawcontrol();
extern void DrawcontrolHD();
extern int selectedelement;
extern bool changebutton;
int controller;
extern void ControllerReset();
extern "C" void _INPUT_LoadButtonMap(int *cfgData);
extern int ControllerConfig[76];

void ControllerMenu(void)
{

	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pSettingsMenu = XLMenu_Init((float)iIGMMenuTxtPosX,(float)iIGMMenuTxtPosY,4, GetMenuFontAlign(iIGMMenuTxtAlign)|MENU_WRAP, NULL);

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
		DrawLogo();
		dwMenuCommand = getAllGamepadsCommand(&gamepad);
		XLMenu_Routine(dwMenuCommand);
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

	DWORD dwMenuCommand = 0;

	XLMenu_CurRoutine = NULL;
	XLMenu_CurMenu = NULL;
	XLMenu_SetFont(&m_Font);

	m_pControllerMenu = XLMenu_Init((float)iIGMMenuTxtPosX,(float)iIGMMenuTxtPosY,1, GetMenuFontAlign(iIGMMenuTxtAlign)|MENU_WRAP, NULL);

	
	m_pControllerMenu->parent = m_pSettingsMenu;

	XLMenu_SetTitle(m_pControllerMenu,L" ",dwMenuTitleColor);


	XLMenu_Activate(m_pControllerMenu);

    bool bquit=false;
    selectedelement=0;

	while(!bquit)
	{
	if(bEnableHDTV)
		DrawcontrolHD();
	else
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
		if(bEnableHDTV)
			DrawcontrolHD();
		else
			Drawcontrol();
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	ChangeControl();}
	}
	ConfigAppSave2();
	_INPUT_LoadButtonMap(ControllerConfig);
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