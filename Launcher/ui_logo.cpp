#include "Panel.h"
//#include "../include/xmvhelper.h"
//#include "RomList.h"
#include "config.h"
#include <xbfont.h>
#include "musicmanager.h"
#include "Network.h"

extern vector<string> vSynopsis;

extern bool initMovie(char*);
extern void drawMovie();
extern bool getPlaying();
extern void stopMovie();
extern DWORD dwMenuItemColor;
extern DWORD dwTitleColor;
char menuBG2path[256]; 
char menuBGpath[256];
char BoxartBGpath[256];
extern int iInfoPosX;
extern int iInfoPosY;
extern int iInfoTxtPosX;
extern int iInfoTxtPosY;
extern int iInfoTxtAlign;
extern int iInfoTxtControlPosX;
extern int iInfoTxtControlPosY;
extern int iInfoTxtControlAlign;
extern char BoxArtPath[32];
extern int iBoxPosX;
extern int iBoxPosY;

//extern int iBoxWidth;
//extern int iBoxHeight;
extern int iBoxWidthAuto;
extern int iBoxHeightAuto;

extern int iBoxPanelPosX;
extern int iBoxPanelPosY;
extern int iControlsPosX;
extern int iControlsPosY;
extern int iConControlsPosX; // not used?
extern int iConControlsPosY; // not used?
extern int iSynControlsPosX;
extern int iSynControlsPosY;
extern int iSynControlsTxtPosX;
extern int iSynControlsTxtPosY;
extern int iSynControlsTxtAlign;
extern int iCredControlsPosX;
extern int iCredControlsPosY;
extern int iCredControlsTxtPosX;
extern int iCredControlsTxtPosY;
extern int iCredControlsTxtAlign;
extern int iControlsTxtPosX;
extern int iControlsTxtPosY;
extern int iControlsTxtAlign;
extern int iLogoPosX;
extern int iLogoPosY;
extern int iSynopsisPosX;
extern int iSynopsisPosY;
extern int iSynopsisLines;
extern int iSynopsisAlign;
extern float InfoPanelTrunc;
extern void LoadSkinFile();
extern int fontcenter;
extern bool isScrolling;
extern bool EnableXMVPreview;
extern bool EnableInfoPanel;
extern bool bShowRLControls;
int itemh = 0;
extern int iLineSpacing;

int menucenter = 256;
extern CMusicManager  music;
extern int actualrom;
extern char nameofrom[120];
CPanel m_BgPanel;
CPanel m_BoxPanel;
CPanel m_BoxBG;
CPanel m_PgPanel;
CPanel m_LoadPanel;
CPanel m_RLPanel;
CPanel m_InfoPanel;
CPanel m_ControlsPanel;
CPanel m_ControlConfigPanel;
CPanel m_LogoPanel;
CPanel m_SynopsisPanel;
CPanel m_MenuBgPanel;
CPanel m_MenuBg2Panel;
CPanel m_LaunchPanel;
LPDIRECT3DTEXTURE8 infoTexture;
LPDIRECT3DTEXTURE8 bgTexture;
LPDIRECT3DTEXTURE8 RLTexture;
LPDIRECT3DTEXTURE8 LoadBGTexture;
LPDIRECT3DTEXTURE8 PgTexture;
LPDIRECT3DTEXTURE8 BoxTexture;
LPDIRECT3DTEXTURE8 BoxBGTexture;
LPDIRECT3DTEXTURE8 ControlsTexture;
LPDIRECT3DTEXTURE8 ControlConfigTexture;
LPDIRECT3DTEXTURE8 LogoTexture;
LPDIRECT3DTEXTURE8 SynTexture;
LPDIRECT3DTEXTURE8 menuBgTexture;
LPDIRECT3DTEXTURE8 menuBg2Texture;
LPDIRECT3DTEXTURE8 launchTexture;


//CXMVSample m_MoviePlayer;
LPDIRECT3DTEXTURE8 VideoTexture;
CPanel m_VideoPanel;

//extern int romcounter; // use romlist func instead
extern CXBFont	m_Font;					// Font	for	text display
extern LPDIRECT3DDEVICE8 g_pd3dDevice;
int nombre=0;
float FSTxtPos=540;
float FSTxtEnd=0;
int iSynopTxtPos=0; // so it doesn't mess with credits
//extern bool onhd;
extern char skinname[32];
extern int ConfigAppLoad3();

char usedimage[99];
char usedvideo[99];

extern DWORD GetFontAlign(int iAlign);

void InitRomList(void)
{
	
	char RLpath[256];
	sprintf(RLpath,"%s%s\\Launcher\\RomListBG.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, RLpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&RLTexture);		
	m_RLPanel.Create(g_pd3dDevice,	RLTexture, true);
	
	char Infopath[256];
	sprintf(Infopath,"%s%s\\Launcher\\InfoPanel.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, Infopath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&infoTexture);		
	m_InfoPanel.Create(g_pd3dDevice,infoTexture, true);
	
	char Launchpath[256];
	sprintf(Launchpath,"%s%s\\Launcher\\LaunchPanel.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, Launchpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&launchTexture);		
	m_LaunchPanel.Create(g_pd3dDevice,launchTexture, true);

	char Logopath[256];
	sprintf(Logopath,"%s%s\\Launcher\\Logo.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, Logopath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&LogoTexture);		
	m_LogoPanel.Create(g_pd3dDevice,LogoTexture, true);

	char SynopsisBGpath[256];
	sprintf(SynopsisBGpath,"%s%s\\Launcher\\Synopsis.png",szPathSkins,skinname);
	if(PathFileExists (SynopsisBGpath)){
	D3DXCreateTextureFromFileEx( g_pd3dDevice, SynopsisBGpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&SynTexture);		
	m_SynopsisPanel.Create(g_pd3dDevice,SynTexture, true);
	}
}

void InitLogo(void)
{
	char bgpath[256];
	sprintf(bgpath,"%s%s\\bg.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, bgpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&bgTexture);		
	m_BgPanel.Create(g_pd3dDevice,	bgTexture, true);
	
	char Controlspath[256];
	sprintf(Controlspath,"%s%s\\Launcher\\ControlsPanel.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, Controlspath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&ControlsTexture);		
	m_ControlsPanel.Create(g_pd3dDevice,ControlsTexture, true);
	
	char ControlConfigpath[256];
	D3DSURFACE_DESC    ControlConfigSurfDesc;
	sprintf(ControlConfigpath,"%s%s\\Launcher\\ControlConfigPanel.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, ControlConfigpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&ControlConfigTexture);		
	m_ControlConfigPanel.Create(g_pd3dDevice,ControlConfigTexture, true);
	ControlConfigTexture->GetLevelDesc(0, &ControlConfigSurfDesc);
	menucenter = ControlConfigSurfDesc.Width;
	
	InitRomList();


	sprintf(menuBGpath,"%s%s\\Launcher\\MainMenuBG.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, menuBGpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&menuBgTexture);		
	m_MenuBgPanel.Create(g_pd3dDevice,	menuBgTexture, true);
	
	sprintf(menuBG2path,"%s%s\\Launcher\\hilight.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, menuBG2path,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&menuBg2Texture);		
	m_MenuBg2Panel.Create(g_pd3dDevice,	menuBg2Texture, true);
	
	itemh = (int)m_Font.GetFontHeight();
	
	
}

void DrawLogo(bool Menu)
{
	DirectSoundDoWork();
	music.Process();
	sprintf(BoxartBGpath,"%s%s\\Launcher\\BoxartBG.png",szPathSkins,skinname);
	


	m_BgPanel.Render(0,0);
	m_LogoPanel.Render((float)iLogoPosX, (float)iLogoPosY);//770,300


	// if the main "all" list is empty, it will crash just after this when it tries to load a rom that isn't there
	if (g_romList.IsListEmpty(All)) return;
	

	//weinerschnitzel - Draw Boxart everywhere		
	char imagename[99];
	char videoname[99];
	
	Rom *rom = g_romList.GetRomAt(actualrom);
	sprintf(imagename,"%s%s",szPathMedia,rom->m_szBoxArtFilename.c_str());
	sprintf(videoname,"%s%s",szPathMedia,rom->m_szVideoFilename.c_str());
	
	
	
	// Preview BG
		m_BoxBG.Destroy();
		D3DXCreateTextureFromFileEx( g_pd3dDevice, BoxartBGpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&BoxBGTexture);		
		m_BoxBG.Create(g_pd3dDevice, BoxBGTexture, true);
		m_BoxBG.Render((float)iBoxPanelPosX, (float)iBoxPanelPosY);//430,30
	
	
	
	// Preview Art	
	// Change boxart if the selected rom changes, it extists, and as long as the menu isnt scrolling
		if(stricmp(imagename, usedimage) && PathFileExists( imagename ) && !isScrolling) {
				m_BoxPanel.Destroy();
				D3DXCreateTextureFromFileEx( g_pd3dDevice, imagename,D3DX_DEFAULT, D3DX_DEFAULT,1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&BoxTexture);		
				m_BoxPanel.Create(g_pd3dDevice,	BoxTexture, true);
				strcpy(usedimage, imagename);
		}
	// Check, then Render
		if(PathFileExists( imagename ) && !isScrolling)
		m_BoxPanel.Render((float)iBoxPosX, (float)iBoxPosY, (float)iBoxWidthAuto, (float)iBoxHeightAuto,1,0);//435,35
	
	
	if(EnableXMVPreview){
	
		if(stricmp(videoname, usedvideo)!=0){
			if ( PathFileExists( videoname ) && !isScrolling) {
				initMovie(videoname);
				strcpy(usedvideo, videoname);
			}
		}
		
		if(isScrolling == false)
			drawMovie();
		else
			stopMovie();
	}

	
	
#ifdef DEBUG
	MEMORYSTATUS memStat;
	WCHAR szMemStatus[128];

	GlobalMemoryStatus(&memStat);
	swprintf(szMemStatus,L"%d Mb Free",(memStat.dwAvailPhys /1024 /1024));

	m_Font.Begin();

	m_Font.DrawText(60.0f, 20.0f, dwTitleColor, szMemStatus, XBFONT_LEFT);

	m_Font.End();
#endif

	if (Menu){
	
	m_ControlsPanel.Render((float)iControlsPosX, (float)iControlsPosY);

	m_Font.Begin();
	m_Font.DrawText((float)iControlsTxtPosX, (float)(iControlsTxtPosY), dwMenuItemColor, L"\400 Select / Next", GetFontAlign(iControlsTxtAlign));//480,300
	m_Font.DrawText((float)iControlsTxtPosX, (float)(iControlsTxtPosY+itemh), dwMenuItemColor, L"\402 Select / Prev", GetFontAlign(iControlsTxtAlign));//480,325
	m_Font.DrawText((float)iControlsTxtPosX, (float)(iControlsTxtPosY+(2*itemh)), dwMenuItemColor, L"\401 Back", GetFontAlign(iControlsTxtAlign));//480,350
	m_Font.End();
	}
	

	if (!Menu && (g_romList.GetRomListSize() !=0) && EnableInfoPanel){ //romcounter
	int y = 40;
	WCHAR	   m_currentname[120];	// Use this to store InfoPanel text

		
	m_InfoPanel.Render((float)iInfoPosX, (float)iInfoPosY);
    m_Font.Begin();

// rom size
		int romsize = (rom->m_romSize / 0x100000 * 8); 
		swprintf( m_currentname, L"Rom Size : %d Mbits", romsize);
		m_Font.DrawText((float)iInfoTxtPosX, (float)iInfoTxtPosY, dwMenuItemColor, m_currentname, XBFONT_TRUNCATED|GetFontAlign(iInfoTxtAlign),	InfoPanelTrunc);//50, 310

// country
		int country = rom->m_byCountry;
		char country2[10];
		
		// Ez0n3 - update the countries

		switch (country) {
			//case  : sprintf( country2, "Demo" ); break;
			//case  : sprintf( country2, "Beta" ); break;
			case 74 : sprintf( country2, "Japan" ); break; //0x4A
			case 65 : sprintf( country2, "USA/Japan" ); break; //0x41
			case 68 : sprintf( country2, "Germany" ); break; //0x44
			case 69 : sprintf( country2, "America" ); break; //0x45
			case 70 : sprintf( country2, "France" ); break; //0x46
			case 73 : sprintf( country2, "Italy" ); break; //0x49
			case 83 : sprintf( country2, "Spain" ); break; //0x53
			case 85 : //0x55
			case 89 : sprintf( country2, "Australia" ); break; //0x59
			case 32 : //0x20
			case 33 : //0x21 
			case 56 : //0x38 
			case 80 : //0x50 
			case 112 : //0x70 
			case 88 : sprintf( country2, "Europe" ); break; //0x58
			default : sprintf( country2, "Unknown" ); break;
		}

		
		swprintf( m_currentname, L"Country : %S", country2);
		m_Font.DrawText((float)iInfoTxtPosX, (float)(iInfoTxtPosY+(itemh+iLineSpacing)), dwMenuItemColor, m_currentname, XBFONT_TRUNCATED|GetFontAlign(iInfoTxtAlign), InfoPanelTrunc);
		// zip name
		char zipname[120];

		sprintf(zipname,rom->GetFileName().c_str());

		/* remove rom path */
		int counterbackslash = 0;

		// find last backslash
		for (int i=0;i<120;i++){
			if (zipname[i]=='\\')  counterbackslash=i;
			if (zipname[i] == '\0') break;
		}
		// remove it
		for (int i=0;i<120;i++){
			for (int j= counterbackslash+1;j<120;j++){
				zipname[i]=zipname[j];
				i++;
			if (zipname[j] == '\0') break;
			}
			zipname[i+1] = '\0';
			break;
		}
		swprintf( m_currentname, L"Rom Name : %S", zipname );
		m_Font.DrawText((float)iInfoTxtPosX, (float)(iInfoTxtPosY+(2*(itemh+iLineSpacing))), dwMenuItemColor, m_currentname, XBFONT_TRUNCATED|GetFontAlign(iInfoTxtAlign), InfoPanelTrunc);

//comments
		Rom *sRom = g_romList.GetRomAt(actualrom);
		swprintf( m_currentname, L"Comments: %S", sRom->GetIniEntry()->szComments );
		m_Font.DrawText((float)iInfoTxtPosX, (float)(iInfoTxtPosY+(3*(itemh+iLineSpacing))), dwMenuItemColor, m_currentname, XBFONT_TRUNCATED|GetFontAlign(iInfoTxtAlign), InfoPanelTrunc);
		
// rom counter
		swprintf( m_currentname, L"[%d Roms]", g_romList.GetRomListSize() );
		m_Font.DrawText((float)iInfoTxtPosX, (float)(iInfoTxtPosY+(4*(itemh+iLineSpacing))), dwMenuItemColor, m_currentname, XBFONT_TRUNCATED|GetFontAlign(iInfoTxtAlign), InfoPanelTrunc);

//Controls
		if(bShowRLControls){
			swprintf( m_currentname, L"\403 Refresh   \402 Surreal Setup   \406 - \407 Fast Scroll" );
			m_Font.DrawText((float)iInfoTxtControlPosX, (float)iInfoTxtControlPosY, dwMenuItemColor, m_currentname, GetFontAlign(iInfoTxtControlAlign));
			
			swprintf( m_currentname, L"\404 Add/Remove Favorites   \405 Favorites List" );
			m_Font.DrawText((float)iInfoTxtControlPosX, (float)(iInfoTxtControlPosY+(itemh+iLineSpacing)), dwMenuItemColor, m_currentname, GetFontAlign(iInfoTxtControlAlign));
		}
		
		m_Font.End();
	}
		
	// Temporary Message
	if (Menu) // when a menu is open
		DrawTempMessage(); // on top of menu items
}
void ClearTextures(){
	m_ControlConfigPanel.Destroy();
	//m_BoxBGPanel.Destroy();
	m_BoxPanel.Destroy();
	m_MenuBgPanel.Destroy();
	m_MenuBg2Panel.Destroy();

}

void DrawSynopsis()
{
	DirectSoundDoWork();
	music.Process();//BGMProgress();
	
	WCHAR m_currentname[4096];
	//int CtrlHeight = 0;

	m_BgPanel.Render(0,0);

	/*if(fontcenter == 320){
		//CtrlHeight = 480 - (itemh*3 + 15);
	}
	else if(fontcenter == 640){
		//CtrlHeight = 720 - (itemh*3 + 15);
	}*/

	int iSynopTxtBotPos = ((int)vSynopsis.size() - (iSynopsisLines + 1));
	
	// lock it at the top and bottom
	if (iSynopTxtPos < 0) iSynopTxtPos = 0;
	else if (iSynopTxtPos > iSynopTxtBotPos) iSynopTxtPos = iSynopTxtBotPos;

	/*int iPosY = 0;
	int iLineCount = 0;
	for (vector<string>::iterator i = vSynopsis.begin(); i != vSynopsis.end(); ++i)
	{
		// only draw visible lines
		if (iLineCount < iSynopTxtPos) {
			iLineCount++;
			continue;
		}
		else if (iLineCount > (iSynopTxtPos + iSynopsisLines)) {
			break;
		}
		
		string szLine (*i);
		swprintf (m_currentname,L"%S",szLine.c_str());

		// needs its own begin/end or it will throw vertex buffer warnings
		m_Font.Begin();
		m_Font.DrawText((float)iSynopsisPosX, (float)(iPosY + iSynopsisPosY), dwMenuItemColor, m_currentname, GetFontAlign(iSynopsisAlign));
		m_Font.End();
		
		iPosY += (itemh + iLineSpacing);
		iLineCount++;
	}*/
	
	int iPosY = 0;
	for (int i = iSynopTxtPos; i <= (iSynopTxtPos + iSynopsisLines); i++)
	{
		string szLine (vSynopsis[i]);
		swprintf (m_currentname,L"%S",szLine.c_str());

		// needs its own begin/end or it will throw vertex buffer warnings
		m_Font.Begin();
		m_Font.DrawText((float)iSynopsisPosX, (float)(iPosY + iSynopsisPosY), dwMenuItemColor, m_currentname, GetFontAlign(iSynopsisAlign));
		m_Font.End();
		
		iPosY += (itemh + iLineSpacing);
	}

	
	float CtrlExtnt = m_Font.GetTextWidth(L"\400 Scroll Down");
	m_ControlsPanel.Render((float)iSynControlsPosX, (float)iSynControlsPosY);
		m_Font.Begin();
		/*
		m_Font.DrawText((float)(fontcenter*2-CtrlExtnt-15), (float)CtrlHeight, dwMenuItemColor,L"\400 Scroll Down" , XBFONT_LEFT); // A
		m_Font.DrawText((float)(fontcenter*2-CtrlExtnt-15), (float)(CtrlHeight + itemh), dwMenuItemColor,L"\402 Scroll Up" , XBFONT_LEFT); // X
		m_Font.DrawText((float)(fontcenter*2-CtrlExtnt-15), (float)(CtrlHeight + (itemh*2)), dwMenuItemColor,L"\401 Back" , XBFONT_LEFT); // B
		*/
		m_Font.DrawText((float)(iSynControlsTxtPosX), (float)(iSynControlsTxtPosY), dwMenuItemColor,L"\400 Scroll Down" , GetFontAlign(iSynControlsTxtAlign)); // A
		m_Font.DrawText((float)(iSynControlsTxtPosX), (float)(iSynControlsTxtPosY + itemh), dwMenuItemColor,L"\402 Scroll Up" , GetFontAlign(iSynControlsTxtAlign)); // X
		m_Font.DrawText((float)(iSynControlsTxtPosX), (float)(iSynControlsTxtPosY + (itemh*2)), dwMenuItemColor,L"\401 Back" , GetFontAlign(iSynControlsTxtAlign)); // B

		m_Font.End();
}

void DrawCredits()
{
	DirectSoundDoWork();
	music.Process();//BGMProgress();


	m_BgPanel.Render(0,0);

	WCHAR	   m_currentname[99];	// Game	name written to	a string
	char name[99];
	//int CtrlHeight;
	FILE *fp;

	if(fontcenter == 320){
		if (FSTxtPos <= FSTxtEnd-580) FSTxtPos = 580;
		//CtrlHeight = 480 - (itemh*2 + 15);
	}
	else if(fontcenter == 640){
		if (FSTxtPos <= FSTxtEnd-820) FSTxtPos = 820;
		//CtrlHeight = 720 - (itemh*2 + 15);
	}

	float y=FSTxtPos;

	if (PathFileExists("D:\\Credits.txt"))
		fp=fopen("D:\\Credits.txt","r");
	else if (PathFileExists("T:\\Credits.txt"))
		fp=fopen("T:\\Credits.txt","r");
	else
		return;
	
	m_Font.Begin();

	while(!feof(fp)){
		fseek(fp,nombre,SEEK_SET);
		if ((fgets(name,99,fp))!= NULL)
			nombre=nombre+(strlen(name));
		else
		{
			sprintf(name," ");
			nombre=0;
		}
		swprintf (m_currentname,L"%S",name);
		m_Font.DrawText((float)fontcenter, y, dwMenuItemColor, m_currentname, XBFONT_CENTER_X);
		y+=itemh;
	}

	m_Font.End();

	m_ControlsPanel.Render((float)iCredControlsPosX, (float)iCredControlsPosY);
	m_Font.Begin();
	float CtrlExtnt = m_Font.GetTextWidth(L"\400 Fast Scroll");
	m_Font.DrawText((float)(iCredControlsTxtPosX), (float)(iCredControlsTxtPosY), dwMenuItemColor,L"\400 Fast Scroll" , GetFontAlign(iCredControlsTxtAlign));
	m_Font.DrawText((float)(iCredControlsTxtPosX), (float)(iCredControlsTxtPosY + itemh), dwMenuItemColor,L"\401 Back" , GetFontAlign(iCredControlsTxtAlign));
	m_Font.End();
	fclose(fp);
}

#ifdef DEBUG
	#define USE_XNET
#endif
void CalculateEndCredits()
{
	char name[99];
	FILE *fp;

#ifdef USE_XNET
	if(g_xNet.IsXboxConnected())
	{
		OutputDebugString("XBOX is connected!\n");
		g_xNet.InitNetwork();

		//#ifdef FTP
		//ftplib g_ftp;
		//g_ftp.Connect("64.22.111.74:21");
		//g_ftp.Login("user", "pass");
		//g_ftp.Get("d:\\test.jpg", "/img/controller.jpg", ftplib::transfermode::image);
		//g_ftp.Quit();
		//#endif

		//#ifdef HTTP
		//g_xNet.InitWinsock();
		//g_xNet.DownloadFile("64.22.111.74", "/fr34kd4v3.jpg", "D:\\test.jpg");
		//#endif

		//Cleanup Winsock after both HTTP AND FTP
		//g_xNet.CleanupWinsock();
		g_xNet.CleanupNetwork();
	}else{
		OutputDebugString("XBOX is NOT connected!\n");
	}
#endif
	
	if (PathFileExists("D:\\Credits.txt"))
		fp=fopen("D:\\Credits.txt","r");
	else if (PathFileExists("T:\\Credits.txt"))
		fp=fopen("T:\\Credits.txt","r");
	else
		return;
	
	float y=0;

	while(!feof(fp))
	{
		fseek(fp,nombre,SEEK_SET);
	if ((fgets(name,99,fp))!= NULL)
	{
		nombre=nombre+(strlen(name));
	}
		y+=itemh;
	}

	FSTxtEnd = -1*(y); //-x?
	fclose(fp);
}

void CreateProgress()
{
	char progressname[256];
	sprintf(progressname,"%s%s\\progress.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, progressname,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&PgTexture);		
	m_PgPanel.Create(g_pd3dDevice,	PgTexture, true);
	char loadingBGpath[256];
	sprintf(loadingBGpath,"%s%s\\Launcher\\LoadingBG.png",szPathSkins,skinname);
	D3DXCreateTextureFromFileEx( g_pd3dDevice, loadingBGpath,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&LoadBGTexture);		
	m_LoadPanel.Create(g_pd3dDevice,	LoadBGTexture, true);
	char imagename[99];
	Rom *rom = g_romList.GetRomAt(actualrom);
	sprintf(imagename,"%s%s",szPathMedia,rom->m_szBoxArtFilename.c_str());
	if ( PathFileExists( imagename )) {
		m_BoxPanel.Destroy();
		D3DXCreateTextureFromFileEx( g_pd3dDevice, imagename,D3DX_DEFAULT, D3DX_DEFAULT,	1, 0, D3DFMT_LIN_A8R8G8B8 ,	D3DPOOL_MANAGED,D3DX_FILTER_NONE , D3DX_FILTER_NONE, 0x00000000,NULL, NULL,&BoxTexture);		
		m_BoxPanel.Create(g_pd3dDevice,	BoxTexture, true);
		
	
	}
}

void RenderProgress(int progress)
{
	g_d3d.BeginRender();
	m_BgPanel.Render(0,0);
	extern int iLoadBoxPosX;
	extern int iLoadBoxPosY;
	extern int iLoadPanelPosX;
	extern int iLoadPanelPosY;
	extern int iLoadPanelTxtPosX;
	extern int iLoadPanelTxtPosY;
	extern int iLoadPanelTxtAlign;
	extern int iLoadPanelBarPosX;
	extern int iLoadPanelBarPosY;
	extern int iLoadPanelTrunc;
	
	//Boxart
	char imagename[99];
	Rom *rom = g_romList.GetRomAt(actualrom);
	sprintf(imagename,"%s%s",szPathMedia,rom->m_szBoxArtFilename.c_str());
	
	int x = 92; // 64
	if(fontcenter == 640)
		x = 384;
	if ( PathFileExists( imagename ))
	m_BoxPanel.Render((float)iLoadBoxPosX, (float)iLoadBoxPosY);//435,35	
	
	//weinerschnitzel Center the Loading Panel
	int x2 = 92;
	if(fontcenter == 640)
		x2 = 412;
	m_LoadPanel.Render((float)iLoadPanelPosX, (float)iLoadPanelPosY);//92,95 412,95
	float LoadTrunc = m_LoadPanel.GetWidth();
	m_Font.Begin();
	
	//weinerschnitzel - get rom name why reinvent the wheel?
	
	WCHAR m_romname[120];

	swprintf(m_romname, L"Loading %S", rom->GetCleanName().c_str());

	m_Font.DrawText((float)iLoadPanelTxtPosX, (float)iLoadPanelTxtPosY, dwTitleColor, m_romname , XBFONT_TRUNCATED|GetFontAlign(iLoadPanelTxtAlign), (float)iLoadPanelTrunc);//320,200

	m_Font.End();

	for (int i=0;i<=progress;i++){
		m_PgPanel.Render((float)((iLoadPanelBarPosX)+(i*4)), (float)iLoadPanelBarPosY);
	}

	g_d3d.EndRender();
}

extern bool compatible[3];
// 0=1964,1=pj64,2=ultrahle 
// not working apparently , ini not done
/*
void display_compatible()
{
	char compatib[99];
	WCHAR compat[99];
	if (compatible[0]) {
		if (compatible[1]){

			if (compatible[2]){
				sprintf(compatib,"Compatible With all emulators"); }
			else {
				sprintf(compatib,"Compatible With 1964 and Project64"); }
		}
		else {

			if (compatible[2]){
				sprintf(compatib,"Compatible With 1964 and UltraHLE"); }
			else {
				sprintf(compatib,"Compatible With 1964"); }
		}
	}
	else{

		if (compatible[1]){

			if (compatible[2]){
				sprintf(compatib,"Compatible With Project64 and UltraHLE"); }
			else {
				sprintf(compatib,"Compatible With Project64"); }
		}
		else {

			if (compatible[2]){
				sprintf(compatib,"Compatible With UltraHLE"); }
			else {
				sprintf(compatib,"No information"); }
		}
	}
swprintf(compat,L"%S",compatib);
m_Font.Begin();
m_Font.DrawText(320, 100, 0xFFFFFFFF,compat , XBFONT_CENTER_X);
m_Font.End(); 

}
*/