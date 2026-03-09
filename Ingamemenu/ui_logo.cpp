#include "Panel.h"
#include <xbfont.h>
#include "../config.h"
#include <xgraphics.h>

#define VERSION L"Surreal64 CE (B5.5)"

//weinerschnitzel- Skin Control
extern DWORD dwTitleColor;
extern DWORD dwIGMTitleColor;
extern void LoadSkinFile();
extern void ClearIGM();
char menuBGpath[256];
char menuBG2path[256];
extern int iIGMTitleX;
extern int iIGMTitleY;
extern int iPanelX;
extern int iPanelY;
extern int iPanelNH;
extern int iPanelNW;

// IGM HD Coords
extern int iIGMTitleX_HD;
extern int iIGMTitleY_HD;
extern int iPanelX_HD;
extern int iPanelY_HD;
extern int iPanelNH_HD;
extern int iPanelNW_HD;

bool PhysRam128();

extern char romname[256];
CPanel m_LaunchPanel;
CPanel m_BgPanel;
CPanel m_RenderPanel;
CPanel m_MenuBgPanel;
CPanel m_MenuBg2Panel;
CPanel m_ControlConfigPanel;
CPanel m_ControlsPanel;
LPDIRECT3DTEXTURE8 bgTexture = NULL;
LPDIRECT3DTEXTURE8 menuBgTexture = NULL;
LPDIRECT3DTEXTURE8 menuBg2Texture = NULL;
LPDIRECT3DTEXTURE8 ControlConfigTexture = NULL;
LPDIRECT3DTEXTURE8 controlsTexture = NULL;
extern CXBFont	m_Font;					// Font	for	text display
extern LPDIRECT3DDEVICE8 g_pd3dDevice;

LPDIRECT3DSURFACE8 surface;
D3DSURFACE_DESC    surfDesc;
LPDIRECT3DTEXTURE8 texture;
LPDIRECT3DSURFACE8 texSurface;
//extern LPDIRECT3DDEVICE8 g_pD3DDev;	<- ???
//extern bool onhd;
extern bool showdebug;

/*extern 	D3DPRESENT_PARAMETERS d3dpp;
D3DPRESENT_PARAMETERS*	g_pd3dpp		= NULL;
LPDIRECT3DSURFACE8		g_pBackBuffer	= NULL;
LPDIRECT3DSURFACE8		g_pDepthBuffer	= NULL;
LPDIRECT3DTEXTURE8 pTargetPicture;
LPDIRECT3DSURFACE8 pTargetSurface;
BOOL TargetIsTexture = FALSE;
float xoffsettarget, yoffsettarget, xscaletarget, yscaletarget;*/
extern char skinname[32];

void InitLogo(void)
{
  D3DFORMAT TextureFormat = D3DFMT_LIN_A8R8G8B8; // 32bit default
	
  LoadSkinFile();

  //weinersch - turn off AA for the IGM and apply 16bit hack to prevent fragging
  if(AntiAliasMode>3){
  g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
  g_pd3dDevice->SetBackBufferScale( 0.5f, 0.5f );
  if(bEnableHDTV)TextureFormat = D3DFMT_LIN_A4R4G4B4;
  }else if(AntiAliasMode>1){
  g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
  g_pd3dDevice->SetBackBufferScale( 0.5f, 1.0f );
  if(!PhysRam128() && bEnableHDTV)TextureFormat = D3DFMT_LIN_A4R4G4B4;
  }



  // thx to gamedev.net for the code - GogoAckman
  
  // get the back surface and its description
  g_pd3dDevice->GetBackBuffer(-1,0,&surface);
  surface->GetDesc(&surfDesc);
 
  // create a texture the same as the back surface
  g_pd3dDevice->CreateTexture(surfDesc.Width, surfDesc.Height, 0, D3DX_DEFAULT, TextureFormat, D3DPOOL_DEFAULT, &texture);
 
  // get the textures surface
  texture->GetSurfaceLevel(0, &texSurface);
   
  // copy the back surface to the textures surface
  D3DXLoadSurfaceFromSurface(texSurface, NULL, NULL, surface, NULL, NULL, D3DX_DEFAULT, 0);
 
  // release both surfaces
  texSurface->Release();
  surface->Release();

 
    
	
	m_RenderPanel.Create(g_pd3dDevice,	texture, true);
	
	// IGM Background
	char gamebgname[256];
	if(m_RenderPanel.GetHeight() > 512 )
		sprintf(gamebgname,"%s%s\\ingamebgHD.jpg",szPathSkins,skinname);
	else
		sprintf(gamebgname,"%s%s\\ingamebg.jpg",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, gamebgname,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, TextureFormat ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&bgTexture);		
	m_BgPanel.Create(g_pd3dDevice,	bgTexture, true);
	
	// ControlConfig Panel
	char ControlConfigpath[256];
	if(m_RenderPanel.GetHeight() > 512 )
		sprintf(ControlConfigpath,"%s%s\\IGMHD\\ControlConfigPanel.png",szPathSkins,skinname);
	else
		sprintf(ControlConfigpath,"%s%s\\IGM\\ControlConfigPanel.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, ControlConfigpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, TextureFormat ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&ControlConfigTexture);		
	m_ControlConfigPanel.Create(g_pd3dDevice,ControlConfigTexture, true);
	
	//IGM Main Menu Background
	if(m_RenderPanel.GetHeight() > 512 )
		sprintf(menuBGpath,"%s%s\\IGMHD\\MainMenuBG.png",szPathSkins,skinname);
	else
		sprintf(menuBGpath,"%s%s\\IGM\\MainMenuBG.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, menuBGpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, TextureFormat ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&menuBgTexture);		
	m_MenuBgPanel.Create(g_pd3dDevice,	menuBgTexture, true);

	//IGM Main Menu hilight bar
	if(m_RenderPanel.GetHeight() > 512 )
		sprintf(menuBG2path,"%s%s\\IGMHD\\hilight.png",szPathSkins,skinname);
	else
		sprintf(menuBG2path,"%s%s\\IGM\\hilight.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, menuBG2path,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, TextureFormat ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&menuBg2Texture);		
	m_MenuBg2Panel.Create(g_pd3dDevice,	menuBg2Texture, true);

	//IGM Control Config Menu Controls Info Background
	if(m_RenderPanel.GetHeight() > 512 )
		sprintf(menuBG2path,"%s%s\\IGMHD\\ControlsPanel.png",szPathSkins,skinname);
	else
		sprintf(menuBG2path,"%s%s\\IGM\\ControlsPanel.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, menuBG2path,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, TextureFormat ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&controlsTexture);		
	m_ControlsPanel.Create(g_pd3dDevice,	controlsTexture, true);
}

void DrawLogo()
{
	// Clear any leftovers 
	g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,0,0);

	m_BgPanel.Render(0,0);
	if(m_RenderPanel.GetHeight() > 512 )
		m_RenderPanel.Render((float)iPanelX_HD ,(float)iPanelY_HD , (float)iPanelNW_HD , (float)iPanelNH_HD , false, 0);
	else
		m_RenderPanel.Render((float)iPanelX ,(float)iPanelY , (float)iPanelNW , (float)iPanelNH , false, 0);//305,65,267,200
	m_Font.Begin();

	// Title
	if(m_RenderPanel.GetHeight() > 512 )
		m_Font.DrawText((float)iIGMTitleX_HD, (float)iIGMTitleY_HD, dwIGMTitleColor, VERSION, XBFONT_CENTER_X);//305, 20
	else
		m_Font.DrawText((float)iIGMTitleX, (float)iIGMTitleY, dwIGMTitleColor, VERSION, XBFONT_CENTER_X);//305, 20

	// Debug Info
	if (showdebug) {
	MEMORYSTATUS memStat;
	WCHAR szMemStatus[128];

	GlobalMemoryStatus(&memStat);
	swprintf(szMemStatus,L"%d MB free",(memStat.dwAvailPhys /1024 /1024));
	m_Font.DrawText(60, 35, dwTitleColor, szMemStatus, XBFONT_LEFT);
	
		/*// Ez0n3 - temp
		WCHAR szVidMemStatus[128];
		swprintf(szVidMemStatus,L"%d MB Free Vid",dwMaxVideoMem);
		m_Font.DrawText(60, 60, 0xFFFF7F7f, szVidMemStatus, XBFONT_LEFT);*/
	
	}

	m_Font.End();
}

void DestroyLogo()
{
	m_BgPanel.Destroy();
	//D3DXCreateTextureFromFileEx( g_pd3dDevice, "D:\\Media\\black.jpg",D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&bgTexture);		
	char pathmedia[256];
	sprintf(pathmedia, "%sblack.jpg", szPathMedia);
	D3DXCreateTextureFromFileEx(g_pd3dDevice, pathmedia, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_LIN_X1R5G5B5, D3DPOOL_MANAGED,D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0x00000000, NULL, NULL, &bgTexture);
	m_BgPanel.Create(g_pd3dDevice,	bgTexture, true);
	m_BgPanel.Render(0,0);

	/*m_BgPanel.Destroy();
	m_RenderPanel.Destroy();
	bgTexture->Release();
	texture->Release();*/
}

//weinerschnitzel - function to clear skin resources when returning to game
void ClearIGM(){

	// Clear skin textures
	m_RenderPanel.Destroy();
	m_ControlConfigPanel.Destroy();
	m_MenuBgPanel.Destroy();
	m_MenuBg2Panel.Destroy();
	m_BgPanel.Destroy();
	m_ControlsPanel.Destroy();
	
	// Turn on AA before leaving
	switch (AntiAliasMode)
		{
			case 2:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR );		break;
			case 3:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX );	break;
			case 4:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR );		break;
			case 5:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN );	break;
		}
	g_pd3dDevice->SetBackBufferScale( 1.0f, 1.0f );

	// Clear the viewport for any artifacts
	g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,0,0);
	
}

bool tookscreenshot=false;
void TakeScreenshot()
{
	//This is broken - freakdave
	
	//Ez0n3 - remove invalid chars from romname for screenshot path - fixes problem with ss dir using ftp (exists, but not accessible)
	char romnamess[42];
	for (int i=0;i<=42;i++){
		if (romname[i]=='<' || romname[i]=='>' || romname[i]=='=' || romname[i]=='?' || romname[i]==':' || romname[i]==';' || romname[i]=='*' || romname[i]=='+' || romname[i]==',' || romname[i]=='/' || romname[i]=='|') romnamess[i] = '_';
		else if (romname[i]=='\0' || romname[i]=='\t' || romname[i]=='\n' || i>=42) { romnamess[i]='\0'; break; }
 		else romnamess[i] = romname[i];
	}
	for (int i=42;i>0;i--){
		if (romnamess[i] == ' ') romnamess[i] = '/0';
		else break;
	}

	char screen[256];
	int actualscreen;

	sprintf(screen,"%s%s",szPathScreenshots,romnamess);

	if (!PathFileExists(szPathScreenshots)) {
		if(!CreateDirectory(szPathScreenshots, NULL)) {
			OutputDebugString(szPathScreenshots);
			OutputDebugStringA(" Could Not Be Created!\n");
			return;
		}
	}
	if (!PathFileExists(screen)) {
		if(!CreateDirectory(screen, NULL)) {
			OutputDebugString(screen);
			OutputDebugStringA(" Could Not Be Created!\n");
			return;
		}
	}
		
	for (int i=0;i<100;i++) {

		sprintf(screen,"%s%s\\screenshot%d.bmp",szPathScreenshots,romnamess,i);

		if (!PathFileExists(screen))
		{
          actualscreen = i;
		  break;
		}
	}
	
	sprintf(screen,"%s%s\\screenshot%d.bmp",szPathScreenshots,romnamess,actualscreen);

	D3DSurface *surface = NULL;

	//Clear Skin Images
	m_ControlConfigPanel.Destroy();
	m_MenuBgPanel.Destroy();
	m_MenuBg2Panel.Destroy();
	m_BgPanel.Destroy();

	//Render the taken ScreenShot
	m_RenderPanel.Render(0,0);

	//Clear the Artifacts
	g_pd3dDevice->Present(0, 0, 0, 0);

	//Copy the Front Buffer
	g_pd3dDevice->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &surface);
	//g_pd3dDevice->GetRenderTarget(&surface);

	//Write the ScreenShot
	XGWriteSurfaceToFile(surface, screen);

	//Release Used parts
	surface->Release();
	m_RenderPanel.Destroy();

	//DrawLogo();
	tookscreenshot=true;
	
#ifdef DEBUG
	OutputDebugString("Writing Screenshot: ");
	OutputDebugString(screen);
	OutputDebugString("\n");
#endif
}
bool PhysRam128(){
  MEMORYSTATUS memStatus;
  GlobalMemoryStatus( &memStatus );
  if( memStatus.dwTotalPhys < (100 * 1024 * 1024) )
	  return false;
  else
	  return true;
}
/*
void CreateRenderTarget()
{
	g_pd3dDevice->CreateTexture(640, 480, 1, 0, D3DFMT_LIN_X8R8G8B8, D3DPOOL_DEFAULT, &pTargetPicture);
	m_TargetPanel.Create(g_pd3dDevice, pTargetPicture, true);
	pTargetPicture->GetSurfaceLevel(0, &pTargetSurface);
//	g_pd3dDevice->GetDepthStencilSurface(&g_pDepthBuffer);
//	g_pd3dDevice->SetRenderTargetFast(pTargetSurface, g_pDepthBuffer, 0);

//	g_pd3dDevice->GetRenderTarget(&g_pBackBuffer);
}

void SetAsRenderTarget()
{
	//if (!TargetIsTexture) {
		g_pd3dDevice->GetDepthStencilSurface(&g_pDepthBuffer);
	    //g_pd3dDevice->GetBackBuffer(0, 0, &g_pBackBuffer);
		//g_pd3dDevice->GetRenderTarget(&g_pBackBuffer);
		//g_pd3dDevice->SetRenderTargetFast(pTargetSurface, NULL, 0);
		//D3DDevice::SetRenderTarget(pTargetSurface,0);
		//D3DDevice::GetDepthStencilSurface(&g_pDepthBuffer);
		//TargetIsTexture = TRUE;
	//}

		g_pd3dDevice->GetRenderTarget(&g_pBackBuffer);
		//g_pd3dDevice->GetBackBuffer(-1, 0, &pTargetSurface);
		g_pd3dDevice->SetRenderTargetFast(pTargetSurface, g_pDepthBuffer, 0);
		//pTargetPicture->GetSurfaceLevel(0, &texSurface);
 
        // copy the back surface to the textures surface
        //D3DXLoadSurfaceFromSurface(texSurface, NULL, NULL, pTargetSurface, NULL, NULL, D3DX_DEFAULT, 0);
		//m_TargetPanel.SetRenderVariables(180,180,180,180);
}

void RestoreRenderTarget()
{
//	if (TargetIsTexture) {
		g_pd3dDevice->SetRenderTargetFast(g_pBackBuffer, g_pDepthBuffer, 0);
		//D3DDevice::SetRenderTarget(g_pBackBuffer,g_pDepthBuffer);
//		TargetIsTexture = FALSE;
//	}
}

void RenderScreen()
{
	RestoreRenderTarget();
//	m_TargetPanel.SetTextureState(1);
	m_TargetPanel.Render(115,115);
}

void SetRenderVariables(float x, float y, float nw, float nh)
{
//	m_TargetPanel.SetRenderVariables(x, y, nw, nh, false, false);
}*/
