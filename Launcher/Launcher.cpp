 /**
 * Surreal 64 Launcher (C) 2003
 * 
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: buttza@hotmail.com, lantus@lantus-x.com
 */

#include "Launcher.h"
#include "IniFile.h"
#include "BoxArtTable.h"
#include "VideoTable.h"
#include "IoSupport.h"

#include "RomList.h"
#include "config.h"
#include <xbapp.h>
#include <xbresource.h>
#include <xbfont.h>
#include "../ingamemenu/xlmenu.h"
#include "128meg.h"
#include "musicmanager.h"
#include "Panel.h"

#define VERSION L"Surreal64 CE (B5.5)"


//weinerschnitzel Skin Control
extern DWORD dwRomListColor;
extern DWORD dwSelectedRomColor;
extern DWORD dwTitleColor;
extern int iRomListPosX;
extern int iRomListPosY;
extern int iRomListAlign;
extern int iRLBorderPosX;
extern int iRLBorderPosY;
extern int iInfoPosX;
extern int iInfoPosY;
extern int iTitleX;
extern int iTitleY;
extern int RomListTrunc;
extern int fontcenter;
extern CPanel m_BgPanel;
extern CPanel m_RLPanel;
extern CPanel m_RLPanel;
extern CPanel m_BoxBG;
extern CPanel m_InfoPanel;
extern CPanel m_ControlsPanel;
extern CPanel m_LaunchPanel;
extern CPanel m_MenuBgPanel;
extern CPanel m_MenuBg2Panel;
extern CPanel m_LogoPanel;
extern CPanel m_SynopsisPanel;
char nameofrom[120];
bool isScrolling = false;
extern int iLineSpacing;

extern int iTempMessagePosX;
extern int iTempMessagePosY;
extern int iTempMessageAlign;

extern DWORD GetFontAlign(int iAlign);

float fGameSelect;
float fCursorPos;
float fMaxCount;
float m_fFrameTime;			// amount of time per frame

int	  LastPos;
int	  m_iMaxWindowList;
int	  m_iWindowMiddle;

extern int GAMESEL_MaxWindowList;		 
int GAMESEL_WindowMiddle = 6;	
const float	GAMESEL_cfDeadZone = 0.3f;
const float	GAMESEL_cfMaxThresh	= 0.93f;
const float	GAMESEL_cfMaxPossible =	1.0f;
const float	GAMESEL_cfRectifyScale = GAMESEL_cfMaxPossible/(GAMESEL_cfMaxPossible-GAMESEL_cfDeadZone);
const float	GAMESEL_cfSpeedBandFastest = 2.0f;	// seconds (don`t worry	for	PAL	NTSC dif xbapp handles that)
const float	GAMESEL_cfFastestScrollMult	= 6.0f;
const float	GAMESEL_cfSpeedBandMedium =	1.0f;	// if the pad is held at max for given seconds list	will move faster
const float	GAMESEL_cfMediumScrollMult = 5.0f;
const float	GAMESEL_cfSpeedBandLowest =	0.5f;
const float	GAMESEL_cfLowestScrollMult = 2.0f;	 

extern "C" void Enable128MegCaching();
extern void InitLogo();
extern void DrawLogo(bool Menu);
extern void LoadSkinFile();
extern void WriteSkinFile();
extern int romcounter; // for surreal config
int actualrom = 0;
extern void LaunchMenu();
bool g_bQuit = false;
extern void MainMenu(void);
CXBFont		m_Font;					// Font	for	text display
CXBFont		m_MSFont;					// Font	for	buttons
extern char romCRC[32];
//extern DWORD dwLastRomCRC;
extern char romname[256];
void ConstructCredits();
CMusicManager  music;
LPDIRECTSOUND8  m_pDSound;          // DirectSound object
extern char skinname[32];
extern bool EnableBGMusic;
extern bool RandomBGMusic;

extern char BoxartName[32];
extern bool CheckSkin(const char *szSkin);
extern void ExitToDash(void);
extern void OpenRomBrowser();

// needed for user pref and CMD launch
//extern int m_emulator; // Ez0n3 - why was this used at all?
extern int preferedemu;
extern int videoplugin;
extern int iAudioPlugin;
extern int iRspPlugin;
extern void Launch();
//extern int ConfigAppLoad3();
extern bool has128ram;

extern void LaunchHideScreens(); // launch rom no screens
void MoveCursorToRom(int iPos); // move cursor to specified pos
char *RemovePath(char *szFilename); // remove the path from the filename
void BuildDirectories();

bool bTempMessage = false;
DWORD dwTempMessageStart = 0;
char szTempMessage[100];

#define CUSTOM_LAUNCH_MAGIC 0xEE456777
typedef struct //_CUSTOM_LAUNCH_DATA
{
	DWORD magic; //populate this with CUSTOM_LAUNCH_MAGIC so we know we are using this special structure
	CHAR szFilename[300]; //this is the path to the game to load upon startup
	CHAR szLaunchXBEOnExit[100]; //this is the XBE name that should be launched when exiting the emu  ( "FILE.XBE" )
	CHAR szRemap_D_As[350]; //this is what D drive should be mapped to in order to launch the XBE specified in szLaunchXBEOnExit  ( "\\Device\\Harddisk0\\Partition1\\GAMES" )
	BYTE country; //country code to use
	BYTE launchInsertedMedia; //should we auto-run the inserted CD/DVD ?
	BYTE executionType; //generic variable that determines how the emulator is run - for example, if you wish to run FMSXBOX as MSX1 or MSX2 or MSX2+
	CHAR reserved[MAX_LAUNCH_DATA_SIZE-757]; //MAX_LAUNCH_DATA_SIZE is 3KB 
} CUSTOM_LAUNCH_DATA, *PCUSTOM_LAUNCH_DATA;

class CXBoxSample :	public CXBApplication
{

public:
	HRESULT	Initialize();		// Initialize the sample
	HRESULT	Render();			// Render the scene
	HRESULT	FrameMove();		// Perform per-frame updates
	CXBoxSample();
	CXBPackedResource		m_xprResource;

private:
	void MoveCursor();
	CIoSupport m_IOSupport;
};

VOID __cdecl main()
{
	CXBoxSample	xbApp;
	if(	FAILED(	xbApp.Create() ) )
		return;
	xbApp.Run();

	fGameSelect	= 0.0f;
	fCursorPos = 0.0f;
	fMaxCount =	0.0f;
}

CXBoxSample::CXBoxSample() 
:CXBApplication()
{

}

extern bool onhd;
extern void BuildSynopsis();
HRESULT	CXBoxSample::Initialize()
{
	// Ez0n3 - determine if the current phys ram is greater than 100MB
	if ( PhysRam128() ) has128ram = true;
	else has128ram = false;

	Enable128MegCaching();
	XSetFileCacheSize(8 * 1024 * 1024);

	// initialise direct 3d
	if (!g_d3d.Create())
		return S_OK;

	// mount drives
	g_IOSupport.Mount("A:","cdrom0");
	g_IOSupport.Mount("E:","Harddisk0\\Partition1");
	//g_IOSupport.Mount("C:","Harddisk0\\Partition2");
	//g_IOSupport.Mount("X:","Harddisk0\\Partition3");
	//g_IOSupport.Mount("Y:","Harddisk0\\Partition4");
	//g_IOSupport.Mount("Z:","Harddisk0\\Partition5");
	g_IOSupport.Mount("F:","Harddisk0\\Partition6");
	g_IOSupport.Mount("G:","Harddisk0\\Partition7");
	
	// utility shoud be mounted automatically
	if(XGetDiskSectorSize("Z:\\") == 0)
		g_IOSupport.Mount("Z:","Harddisk0\\Partition5");

	//FIXME: Running the application from HDD while having the roms 
	//on CD/DVD does only work through editing the path in Surreal.ini.
	//This needs to be automated.
	if(XGetDiskSectorSize("D:\\") == 2048){
		onhd = FALSE;
	}else{
		onhd = TRUE;
	}
	
	// load the main ini file
	g_iniFile.Load("Surreal.ini"); // try T first, then D

	ConfigAppLoadPaths(); // get paths from main ini

	ConfigAppLoad(); // get user settings and paths
	ConfigAppSave();

	ConfigAppLoadTemp(); // load history
	
	BuildDirectories();
	
	// build synopsis files - onhd only
	if (PathFileExists("D:\\synopsis.txt") && onhd)
		BuildSynopsis();

	// weinerschnitzel - create a new Skin.ini based on defaults if no skin.ini exists
	char skininipath[256];
	sprintf(skininipath,"%s%s\\Skin.ini",szPathSkins,skinname);
	if (!PathFileExists(skininipath)) {
		WriteSkinFile();
	}

	//reboot to dash to avoid crash, missing files will be logged
	if (!CheckSkin(skinname)) {
		ExitToDash();
	}

	LoadSkinFile();

	char fontname[256];
	sprintf(fontname,"%s%s\\Launcher\\Font.xpr",szPathSkins,skinname);
	if(	FAILED(	m_Font.Create(fontname) ) )
		return XBAPPERR_MEDIANOTFOUND;
	sprintf(fontname,"%s%s\\MsFont.xpr",szPathSkins,skinname);
	if(	FAILED(	m_MSFont.Create(fontname) ) )
		return XBAPPERR_MEDIANOTFOUND;
	
	//Make sure Surreal10.ini does not exist
	if (PathFileExists("T:\\Surreal10.ini"))
		DeleteFile("T:\\Surreal10.ini");


	InitLogo();
	
	char szMediaTest[256];
	sprintf(szMediaTest, "%s%s", szPathMedia, BoxartName);
	if (PathFileExists(szMediaTest))
		g_boxArtTable.Build();
		
	sprintf(szMediaTest, "%sMovies", szPathMedia);
	if (PathFileExists(szMediaTest))
		g_VideoTable.Build();
	

	// load the rom list if it isn't already loaded
	// we do it here so that the user can see the rom list is loading
	//g_romList.m_bUseCache = true; // to enable cache file
	if (!g_romList.IsLoaded()) //freakdave - in this case something went wrong
		g_romList.Load(); // cache disabled
		//g_romList.Refresh(); //Do a full refresh (delete, create new and load)
	

	romcounter = g_romList.GetRomListSize();	

	if (romcounter < GAMESEL_MaxWindowList)
	{
		m_iMaxWindowList = romcounter;
		m_iWindowMiddle	 = romcounter/2;
	}
	else
	{
		m_iMaxWindowList = GAMESEL_MaxWindowList;
		m_iWindowMiddle	 = GAMESEL_MaxWindowList/2;
	}

	
	// rom history
	if (strlen(romCRC))
	{
		DWORD dwLastRomCRC = strtoul(romCRC, NULL, 16);
		for (int ii = 0, n = g_romList.GetRomListSize(); ii < n; ii++) {
			Rom* rom = g_romList.GetRomAt(ii);
			if(rom->GetCrc1() == dwLastRomCRC){
				MoveCursorToRom(ii);
				break;
			}
		}
	}

	
	if (!PathFileExists("D:\\Credits.txt") && !PathFileExists("T:\\Credits.txt"))
		ConstructCredits();

	if(FAILED(DirectSoundCreate(NULL, &m_pDSound, NULL)))
	return E_FAIL;

	if (RandomBGMusic)
		music.SetRandom(true);
	
    music.Initialize();

	if(EnableBGMusic){
		music.Play();
	}
	

//#define DEBUG_CMD 1
#ifndef DEBUG_CMD
	// CMD launch code
	DWORD launchDataType;
	LAUNCH_DATA launchData;
	char *szCmdLine; //[300];
	
	if(XGetLaunchInfo (&launchDataType, &launchData) == ERROR_SUCCESS)
	{
		// Evox
		if(launchDataType == LDT_FROM_DEBUGGER_CMDLINE) {
			szCmdLine = ((PLD_FROM_DEBUGGER_CMDLINE)&launchData)->szCmdLine;
		}
		// XBMC
		else if(launchDataType == LDT_TITLE && ((PCUSTOM_LAUNCH_DATA)&launchData)->magic == CUSTOM_LAUNCH_MAGIC) {
			szCmdLine = (char*)((PCUSTOM_LAUNCH_DATA)&launchData)->szFilename;
		}

		if (strlen(szCmdLine)) {
#else
			char szCmdLineFake[300];
			sprintf(szCmdLineFake, "E:\\DEVKIT\\Surreal64 CE\\Roms\\Super Mario 64.z64");
			//sprintf(szCmdLineFake, "Super Mario 64.z64");
			char *szCmdLine = szCmdLineFake;
#endif

			char szCmdFilename[300];
			sprintf(szCmdFilename, RemovePath(szCmdLine));

			//bool bRomMatch = false;
			for (int ii = 0, n = g_romList.GetRomListSize(); ii < n; ii++) {
				Rom* rom = g_romList.GetRomAt(ii);

				char szRomFilename[300];
				char szRomFilenameReal[300];
				sprintf(szRomFilenameReal, "%s", rom->GetFileName().c_str());
				sprintf(szRomFilename, RemovePath(szRomFilenameReal));
				
				if (strcmp(szCmdFilename, szRomFilename) == 0) {
					actualrom = ii;
					//bRomMatch = true;
					ConfigAppLoad3(); // load user pref for rom
					
					// Here we "clean" our name
					WCHAR m_currentname[120];
					sprintf(nameofrom, "%s", rom->GetCleanName().c_str());
					swprintf(m_currentname, L"%S", nameofrom);
					sprintf(romname, "%S", m_currentname);
					sprintf(romCRC, "%08x", rom->m_dwCrc1);

					ConfigAppSave2(); // save the rom ini

					Launch();
					
					break;
				}
			}
			
#ifndef DEBUG_CMD
			// shouldn't need to reboot with new method
			/*if (!bRomMatch) {
				XLaunchNewImage("D:\\default.xbe", NULL);
			}*/
		}
	}
#endif

	if (romcounter == 0)
		OpenRomBrowser();

return S_OK;

}

HRESULT	CXBoxSample::FrameMove()
{ 
	static bool bDpadUp = false;
	static bool bDpadDown = false;
	WORD wButtons;

		MoveCursor();

			if ((bDpadUp || bDpadDown) && XBUtil_Timer(TIMER_GETAPPTIME) > 0.5f) {
				XBUtil_Timer(TIMER_STOP);
				wButtons = m_DefaultGamepad.wButtons;
			} else {
				wButtons = m_DefaultGamepad.wPressedButtons;
			}

			if(/*m_DefaultGamepad.wPressedButtons*/wButtons	& XINPUT_GAMEPAD_DPAD_DOWN)
			{
				if (wButtons == m_DefaultGamepad.wPressedButtons) XBUtil_Timer(TIMER_RESET);

				bDpadUp = false;
				bDpadDown = true;
				isScrolling = true;
				// default don`t clamp cursor
				bool bClampCursor =	FALSE;

				fCursorPos ++;

				if(	fCursorPos > m_iWindowMiddle )
				{
					// clamp cursor	position
					bClampCursor = TRUE;

					// advance gameselect
					if(fGameSelect == 0) fGameSelect +=	(fCursorPos	- m_iWindowMiddle);
					else fGameSelect ++;

					// clamp game window range (high)
					if((fGameSelect	+ m_iMaxWindowList)	> romcounter)
					{

						// clamp to	end
						fGameSelect	= (float)(romcounter - m_iMaxWindowList);

						// advance cursor pos after	all!
						bClampCursor = FALSE;

						// clamp cursor	to end
						if((fGameSelect	+ fCursorPos) >= romcounter)
							fCursorPos = (float)(m_iMaxWindowList-1);
					}
				}

				// check for cursor	clamp
				if(	bClampCursor )
					fCursorPos = (float)m_iWindowMiddle;
					
				actualrom = (int)fGameSelect+(int)fCursorPos;


			}
			else if( /*m_DefaultGamepad.wPressedButtons*/wButtons &	XINPUT_GAMEPAD_DPAD_UP)
			{
				if (wButtons == m_DefaultGamepad.wPressedButtons) XBUtil_Timer(TIMER_RESET);

				bDpadUp = true;
				bDpadDown = false;
				isScrolling = true;

				// default don`t clamp cursor
				bool bClampCursor =	FALSE;

				fCursorPos --;
				if(	fCursorPos < m_iWindowMiddle )
				{
					// clamp cursor	position
					bClampCursor = TRUE;

					// backup window pos
					fGameSelect	--;

					// clamp game window range (low)
					if(fGameSelect < 0)
					{
						// clamp to	start
						fGameSelect	= 0;

						// backup cursor pos after all!
						bClampCursor = FALSE;

						// clamp cursor	to end
						if(	fCursorPos < 0 )
							fCursorPos = 0;
					}
				}else{
					isScrolling = false;
				}

				// check for cursor	clamp
				if(	bClampCursor )
					fCursorPos = (float)m_iWindowMiddle;	
					
				actualrom = (int)fGameSelect+(int)fCursorPos;


			}
		if(m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_A])
		{
			if(!HideLaunchScreens)
			{
				LaunchMenu();
			}
			else
			{
				LaunchHideScreens();
			}
		}
	
		if(m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_X])
		{
			MainMenu();
		}


		if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB){
			if(music.IsPlaying()){
				music.Pause();
			}else{
				music.Play();
			}
		}
		else if(m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_LEFT_THUMB){
			music.NextSong();
		}

		if(m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_Y])
		{
            g_romList.Refresh();
			romcounter = g_romList.GetRomListSize();

			if (romcounter <	GAMESEL_MaxWindowList)
			{
				m_iMaxWindowList = romcounter;
				m_iWindowMiddle	 = romcounter/2;
			}
			else
			{
				m_iMaxWindowList = GAMESEL_MaxWindowList;
				m_iWindowMiddle	 = GAMESEL_MaxWindowList/2;
			}
		}

		
		
		// All/Favourites Rom List Toggle
		if(m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_BLACK])
		{
			// reset cursor pos
			int iListPosMove = 0;
			bool bRefreshList = false;
			Rom *rom = g_romList.GetRomAt(actualrom);
		
			// switch list mode
			if (g_romList.GetRomListMode() == All) // all current, switch to fav
			{
				if (g_romList.IsListEmpty(Favourite)) { // needs something to show
					ShowTempMessage("List: No Favorites Found");
				} else {
					MoveCursorToRom(0);
					g_romList.SetRomListMode(Favourite);
					iListPosMove = g_romList.FindRom(rom, Favourite);
					ShowTempMessage("List: Favorites");
					bRefreshList = true;
				}
			}
			else // fav current, switch to all
			{
				MoveCursorToRom(0);
				g_romList.SetRomListMode(All);
				iListPosMove = g_romList.FindRom(rom, All);
				ShowTempMessage("List: All");
				bRefreshList = true;
			}
			
			if (bRefreshList)
			{
				//g_romList.Save(); // save out dat files
				//g_romList.Refresh();
				
				romcounter = g_romList.GetRomListSize();

				if (romcounter < GAMESEL_MaxWindowList)
				{
					m_iMaxWindowList = romcounter;
					m_iWindowMiddle	 = romcounter/2;
				}
				else
				{
					m_iMaxWindowList = GAMESEL_MaxWindowList;
					m_iWindowMiddle	 = GAMESEL_MaxWindowList/2;
				}
				
				if (iListPosMove > -1)
					MoveCursorToRom(iListPosMove);
			}
			
		}
		// Favourite Rom Toggle
		else if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_WHITE])
		{
			Rom *rom = g_romList.GetRomAt(actualrom);
			
			bool bRefreshList = false;
			if (g_romList.FindRom(rom, Favourite) > -1) // it's a favorite
			{
				int iListPosMove = -1;
				
				if (g_romList.GetRomListMode() == Favourite) // rem fav while fav's are showing, need to move the cursor
				{
					// reset cursor pos
					iListPosMove = actualrom;
					MoveCursorToRom(0); // put it at the beginning
					bRefreshList = true;
				}
				
				g_romList.RemoveRomFromList(rom, Favourite); // rem fav
				
				if (iListPosMove > 0)
					MoveCursorToRom(iListPosMove-1); // put it up one rom after this one gets removed
				
				char favmsg[256];
				sprintf(favmsg, "Favorite Removed: %s", rom->GetCleanName().c_str());
				ShowTempMessage(favmsg);
			}
			else
			{
				g_romList.AddRomToList(rom, Favourite); // add fav
				
				char favmsg[256];
				sprintf(favmsg, "Favorite Added: %s", rom->GetCleanName().c_str());
				ShowTempMessage(favmsg);
			}
			
			//g_romList.Save(); // save out the dat file
			
			if (bRefreshList) // fav list changed while in it
			{
				if (g_romList.GetRomListSize() == 0) // no roms left in fav list, switch to all
				{
					MoveCursorToRom(0);
					g_romList.SetRomListMode(All);
				}
				
				//Sleep(100);
				//g_romList.Refresh(); // refresh list to pop out removed roms
				romcounter = g_romList.GetRomListSize();

				if (romcounter < GAMESEL_MaxWindowList)
				{
					m_iMaxWindowList = romcounter;
					m_iWindowMiddle	 = romcounter/2;
				}
				else
				{
					m_iMaxWindowList = GAMESEL_MaxWindowList;
					m_iWindowMiddle	 = GAMESEL_MaxWindowList/2;
				}
			}
			
		}

		

			return S_OK;

}
bool compatible[3];
HRESULT	CXBoxSample::Render()
{
		g_d3d.BeginRender();

		WCHAR	   m_currentname[120];	// Game	name written to	a string

	    int iGameidx;
	    int	iTempGameSel;

	    // printf Gamelist
		DrawLogo(false);

		//Draw Launcher images
		m_RLPanel.Render((float)iRLBorderPosX, (float)iRLBorderPosY);
		


		m_Font.Begin();

		// draw	game list entries

		iTempGameSel = (int)fGameSelect;
		actualrom = (int)fGameSelect+(int)fCursorPos;

		if (romcounter == 0)
			m_Font.DrawText(320, 202, 0xFFEEEEEE,	L"No Roms Found\nPlease edit surreal.ini and refresh by pressing \403", XBFONT_CENTER_X);	 
		for	(iGameidx=0; iGameidx<m_iMaxWindowList;	iGameidx++)
		{
			Rom *rom = g_romList.GetRomAt(iTempGameSel++);
		
		
			// Here we "clean" our name
			//weinerschnitzel - truncate the name to fit in GUI
			WCHAR m_currentname_trunc[120];
			swprintf( m_currentname, L"%S", rom->GetCleanName().c_str() );
			swprintf( m_currentname_trunc, L"%S", rom->GetCleanTruncName(RomListTrunc).c_str() ); 
		
	
			int itemh = (int)m_Font.GetFontHeight();

			if (iGameidx==(int)fCursorPos){
                m_Font.DrawText((float)iRomListPosX, (float)(iRomListPosY+((itemh+iLineSpacing)*iGameidx)), dwSelectedRomColor, m_currentname_trunc, GetFontAlign(iRomListAlign), 530);//45,45
				sprintf(romCRC,"%08x",rom->m_dwCrc1);
				sprintf(romname,"%S",m_currentname);
				
				// don't think this is working yet
				/*for (int i=0;i<3;i++){
					compatible[i]=rom->GetIniEntry()->pbEmuSupported[i];}*/
			}
			else
			{
				m_Font.DrawText((float)iRomListPosX, (float)(iRomListPosY+((itemh+iLineSpacing)*iGameidx)), dwRomListColor, m_currentname_trunc, GetFontAlign(iRomListAlign), 530);//45,45
			}

		}
		

		//Title and Version
		m_Font.DrawText((float)iTitleX, (float)iTitleY, dwTitleColor, VERSION, XBFONT_CENTER_X);//So the RomList doesn't cover the title

		m_Font.End();
		
		// Temporary Message
		DrawTempMessage(); // on top of the panels

     	g_d3d.EndRender();

			return S_OK;
}

// check for move cursor and move accordingly (with	clamp etc)
void CXBoxSample::MoveCursor()
{
	// get right trigger state (convert	to float & scale to	0.0f - 1.0f)
	float fWindowVelocity =	(float)((m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER])/256.0f);

	// subract in left trigger state (convert to float)
	fWindowVelocity	-= (float)((m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER])/256.0f);

	// get left stick state
	fWindowVelocity  +=	(float)((-m_DefaultGamepad.sThumbLY)/32256.0f);

	//setup	static locals
	static float fFastestThresh	= m_d3dpp.FullScreen_RefreshRateInHz*GAMESEL_cfSpeedBandFastest;
	static float fMediumThresh = m_d3dpp.FullScreen_RefreshRateInHz*GAMESEL_cfSpeedBandMedium;
	static float fLowestThresh = m_d3dpp.FullScreen_RefreshRateInHz*GAMESEL_cfSpeedBandLowest;

	// do dead zone	and	gamma curve
	if(( fWindowVelocity  >	-GAMESEL_cfDeadZone	) && ( fWindowVelocity < GAMESEL_cfDeadZone	))
	{
		// zero	dead zone (for spring slack	& noise)
		fWindowVelocity	= 0.0f;
		fMaxCount =	0.0f;
	}
	else
	{
		//start	at 0.0f	after deadzone
		if(	fWindowVelocity	< 0.0f )
			//adjust for deadzone
			fWindowVelocity	+= GAMESEL_cfDeadZone;
		else
			//adjust for deadzone
			fWindowVelocity	-= GAMESEL_cfDeadZone;

		//rescale to correct range
		fWindowVelocity	*= GAMESEL_cfRectifyScale;

		//increase max held	count (for speed up	bands)
		if(	fabs(fWindowVelocity) >	GAMESEL_cfMaxThresh	)
		{
			//increase count by	frame time and clamp count (if necc.)
			if(fMaxCount +=	m_fFrameTime)
				fMaxCount =	fFastestThresh;
		}

		//check	for	speed scale	banding
		if(	fMaxCount >	fFastestThresh)
			fWindowVelocity	*= GAMESEL_cfFastestScrollMult;
		else if( fMaxCount > fMediumThresh)
			fWindowVelocity	*= GAMESEL_cfMediumScrollMult;
		else if( fMaxCount > fLowestThresh)
			fWindowVelocity	*= GAMESEL_cfLowestScrollMult;
	}

	// default don`t clamp cursor
	bool bClampCursor =	FALSE;
	isScrolling = false;
	// check if	triggers pressed and move window
	fCursorPos += fWindowVelocity;
	if(	fWindowVelocity	> 0.0f )
	{
		isScrolling = true;
		if(	fCursorPos > m_iWindowMiddle )
		{
			// clamp cursor	position
			bClampCursor = TRUE;

			// advance gameselect
			fGameSelect	+= fWindowVelocity;

			// clamp game window range (high)
			if((fGameSelect	+ m_iMaxWindowList)	> romcounter)
			{
				// clamp to	end
				fGameSelect	= (float)(romcounter - m_iMaxWindowList);

				// advance cursor pos after	all!
				bClampCursor = FALSE;

				// clamp cursor	to end
				if(fCursorPos >	m_iMaxWindowList -1)
					fCursorPos = (float)(m_iMaxWindowList-1);
			}
		}

	}
	else if( fWindowVelocity < 0.0f	)	// ok to do	this! because of deadzone clamp
	{
		isScrolling = true;
		if(	fCursorPos < m_iWindowMiddle )
		{
			// clamp cursor	position
			bClampCursor = TRUE;

			// backup window pos
			fGameSelect	+= fWindowVelocity;

			// clamp game window range (low)
			if(fGameSelect < 0)
			{
				// clamp to	start
				fGameSelect	= 0;

				// backup cursor pos after all!
				bClampCursor = FALSE;

				// clamp cursor	to end
				if(	fCursorPos < 0 )
					fCursorPos = 0;
			}
		}
	}

	// check for cursor	clamp
	if(	bClampCursor )
		fCursorPos = (float)m_iWindowMiddle;	

	actualrom = (int)fGameSelect+(int)fCursorPos;
}


// move cursor to the specified pos
void MoveCursorToRom(int iPos) {

	/*char McDbgMsg[255];
	sprintf(McDbgMsg, "MCDBG1: fGameSelect=%.0f | fCursorPos=%.0f | iPos=%i \n", fGameSelect, fCursorPos, iPos);
	OutputDebugString(McDbgMsg);*/
	
	actualrom = iPos;
	int iPadTop = 0;
	int iPadBot = 0;
	
	romcounter = g_romList.GetRomListSize();
	
	if (romcounter <= GAMESEL_MaxWindowList) {
		iPadTop = romcounter;
		iPadBot = romcounter;
	} else {
		iPadTop = GAMESEL_MaxWindowList / 2;
		iPadBot = romcounter - iPadTop;
	}

	if (iPos <= iPadTop) { // top
		fGameSelect = 0.0f;
		fCursorPos = (float)iPos;
		//OutputDebugString("top\n");
	}	
	else if (iPos < iPadBot) { // mid
		fGameSelect = (float)(iPos - iPadTop);
		fCursorPos = (float)iPadTop;
		//OutputDebugString("mid\n");
	}
	else { // bot
		fGameSelect = (float)(romcounter - GAMESEL_MaxWindowList);
		fCursorPos = (float)(iPos - (int)fGameSelect);
		//OutputDebugString("bot\n");
	}
	
	/*sprintf(McDbgMsg, "MCDBG2: fGameSelect=%.0f | fCursorPos=%.0f | iPos=%i \n", fGameSelect, fCursorPos, iPos);
	OutputDebugString(McDbgMsg);*/
	
	return;
}

// remove the path from the filename
char *RemovePath(char *szFilename)
{
	if ( strchr(szFilename, '\\') ) {
		int cmdcounterbackslash;
		for (int i=0;i<300;i++){ // find last backslash
			if (szFilename[i]=='\\')  cmdcounterbackslash=i;
			if (szFilename[i] == '\0') break;
		}
		for (int i=0;i<300;i++){ // remove it
			for (int j= cmdcounterbackslash+1;j<300;j++){
				szFilename[i]=szFilename[j];
				i++;
				if (szFilename[j] == '\0') break;
			}
			szFilename[i+1] = '\0';
			break;
		}
	}
			
    return szFilename;
}


void ReloadSkin() {
	// fonts
	char fontname[256];
	m_MSFont.Destroy();
	m_Font.Destroy();
	
	
	//Refresh images
	m_BgPanel.Destroy();
	m_RLPanel.Destroy();
	m_BoxBG.Destroy();
	m_InfoPanel.Destroy();
	m_ControlsPanel.Destroy();
	m_LaunchPanel.Destroy();
	m_MenuBgPanel.Destroy();
	m_MenuBg2Panel.Destroy();
	m_LogoPanel.Destroy();

	LoadSkinFile();
	Sleep(300);
	sprintf(fontname,"%s%s\\Launcher\\Font.xpr",szPathSkins,skinname);
	m_Font.Create(fontname);
	sprintf(fontname,"%s%s\\MsFont.xpr",szPathSkins,skinname);
	m_MSFont.Create(fontname);

	InitLogo();

	// music
	music.Stop();
	music.Initialize();
	music.Play();
}


void ConstructCredits()
{
	//write out the Credits
	FILE *fp;

	if ((fp = fopen("T:\\Credits.txt","w+")) == NULL) {
		OutputDebugString("Failed to Open T:\\Credits.txt\n");
		return;
	}

	fprintf(fp,"Surreal64 CE 'Community Edition' Modification\n");
	fprintf(fp,"Based on Surreal64 XXX\n");
	fprintf(fp,"\n");
	fprintf(fp,"Authors:\n");
	fprintf(fp,"freakdave, Ez0n3, weinerschnitzel\n");
	fprintf(fp,"\n");
	fprintf(fp,"Additional Rice Video Fixes:\n");
	fprintf(fp,"death2droid, microdev\n");
	fprintf(fp,"\n");
	fprintf(fp,"Skin Developers:\n");
	fprintf(fp,"cbagy, FrankMorris, Neil222");
	fprintf(fp,"\n");
	fprintf(fp,"Synopsis:\n");
	fprintf(fp,"MegaMan(?)");
	fprintf(fp,"\n");
	fprintf(fp,"Beta Testers:\n");
	fprintf(fp,"Bigby, Scorp316, YRUSirius\n");
	fprintf(fp,"\n");
	fprintf(fp,"Custom N64 Boxart packs:\n");
	fprintf(fp,"cbagy\n");
	fprintf(fp,"Hyperspin-FE 3D Boxart:\n");
	fprintf(fp,"Azzbarb, http://www.hyperspin-fe.com/ \n");
	fprintf(fp,"Contributed by Bigby\n");
	fprintf(fp,"\n");
	fprintf(fp,"Community .ini file tweaks:\n");
	fprintf(fp,"Darknior, Monkeys69, scorp316, Spbaabck, Xmodder deluX\n");
	fprintf(fp,"Xpfrags, Greatant, Mwaterbu, Dnyce1987\n");
	fprintf(fp,"\n");
	fprintf(fp,"Previous Testing / Ideas / Support:\n");
	fprintf(fp,"Neo369, Bomb Bloke, greatant, edwardar, Darknior, que2eqe \n");
	fprintf(fp,"\n");
	fprintf(fp,"Special thanks to:\n");
	fprintf(fp,"Schibo, madmab, Hyper_Eye, nes6502, Iriez\n");
	fprintf(fp,"The EmuXtras Team");
	fprintf(fp,"\n");
	fprintf(fp,"The ones who made it all happen:\n");
	fprintf(fp,"oDD & Lantus\n");
	fprintf(fp,"\n");
	fprintf(fp,"Visit us at:\n");
	fprintf(fp,"http://surreal64ce.wikidot.com\n");
	fprintf(fp,"http://forums.xbox-scene.com\n");
	fprintf(fp,"#surreal64ce on EFNET\n");
	fprintf(fp,"\n\n\n");
	fprintf(fp,"Surreal64 XXX Modification\n");
	fprintf(fp,"\n");
	fprintf(fp,"Authors:\n");
	fprintf(fp,"Team XXX\n");
	fprintf(fp,"GogoAckman, freakdave, Artik\n");
	fprintf(fp,"http://www.logic-sunrise.com/forums\n");
	fprintf(fp,"http://freakdave.xbox-scene.com\n");
	fprintf(fp,"\n");
	fprintf(fp,"Skin by:\n");
	fprintf(fp,"NeverWill\n");
	fprintf(fp,"\n");
	fprintf(fp,"Music found on OCRemix:\n");
	fprintf(fp,"http://www.ocremix.org\n");
	fprintf(fp,"\n");
	fprintf(fp,"Beta Tester:\n");
	fprintf(fp,"Dukedoom\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"Shouts go out to:\n");
	fprintf(fp,"oDD and Lantus for bringing N64 emulation to the XBOX\n");
	fprintf(fp,"XBINS\n");
	fprintf(fp,"Xantium\n");
	fprintf(fp,"Iriez\n");
	fprintf(fp,"XPort\n");
	fprintf(fp,"the XBOX-SCENE community for waiting too long for this\n");
	fprintf(fp,"\n\n\n\n");
	fprintf(fp,"and last but not least:\n");
	fprintf(fp,"\n\n\n\n");
	fprintf(fp,"the #A-Shock Kindergarden!!!!1111Shift(1)\n");
	fprintf(fp,"and the rest of our beloved scene whiners...\n");
	fprintf(fp,"\n\n\n");
	fprintf(fp,"1964 / UltraHLE / Launcher /Plugins\n");
	fprintf(fp,"oDD\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"Project64 / Plugins\n");
	fprintf(fp,"Lantus\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"Testing / Ideas / Support\n");
	fprintf(fp,"Iriez\n");
	fprintf(fp,"Very Special Thanks\n");
	fprintf(fp,"Azimer for his HLE audio code\n");
	fprintf(fp,"Rice for his support with his video plugin\n");
	fprintf(fp,"Greets and Thanks\n");
	fprintf(fp,"Carcharius, Xport, TJ, DragonZ\n");
	fprintf(fp,"and anyone else we forgot\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"1964 Credits\n");
	fprintf(fp,"Authors\n");
	fprintf(fp,"Schibo and Rice\n");
	fprintf(fp,"Copyright (C) 1989 - 1991\n");
	fprintf(fp,"Thanks\n");
	fprintf(fp,"steb, zilmar, anarko, Azimer, Jabo\n");
	fprintf(fp,"Greets\n");
	fprintf(fp,"Angelo, ContraSF, Cricket,\n");
	fprintf(fp,"_Demo_, duddie, Emul8or, epsilon,\n");
	fprintf(fp,"F|RES, FlyingNR6, hartec, Houe, hWnd,\n");
	fprintf(fp,"icepir8, LaC, Lemmy Lionel, Niki Waibel,\n");
	fprintf(fp,"Phrodide, rcp, Scav, slacka, Smiff,\n");
	fprintf(fp,"SpectRe, Spikez, subice, StrmnNrmn,\n");
	fprintf(fp,"TNSe\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"PJ64 Credits\n");
	fprintf(fp,"Authors\n");
	fprintf(fp,"Zilmar and Jabo\n");
	fprintf(fp,"Copyright (C) 1998 - 2001\n");
	fprintf(fp,"Contributors\n");
	fprintf(fp,"Tooie and Witten\n");
	fprintf(fp,"Support\n");
	fprintf(fp,"Smiff and The Gentleman\n");
	fprintf(fp,"Thanks and Greets\n");
	fprintf(fp,"Anarko, Breakpoint, Cricket, _Demo_,\n");
	fprintf(fp,"Duddie, F|ReS, Icepir8, LaC,\n");
	fprintf(fp,"Lemmy, rcp, schibo, slacka,\n");
	fprintf(fp,"smiff and anyone i missed\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"UltraHLE Credits\n");
	fprintf(fp,"Authors\n");
	fprintf(fp,"Epsilon and RealityMan\n");
	fprintf(fp,"Copyright (C) Epsilon and RealityMan, 1998\n");
	fprintf(fp,"Greets\n");
	fprintf(fp,"Fox and _64Bit_ (Emulators Unlimited),\n");
	fprintf(fp,"JoseQ (EmuViews), Atila (Retrogames),\n");
	fprintf(fp,"Dextrose, Niki Waibel, Mike Tedder,\n");
	fprintf(fp,"Anarko, Romberman 64, Brian Faber,\n");
	fprintf(fp,"Jim at EmuNews and the whole\n");
	fprintf(fp,"emulation community.\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"Rice Video Plugins\n");
	fprintf(fp,"Author\n");
	fprintf(fp,"Rice\n");
	fprintf(fp,"Copyright (C) 2003 Rice1964\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"Daedalus Video Plugins\n");
	fprintf(fp,"Author\n");
	fprintf(fp,"StrmnNrmn\n");
	fprintf(fp,"Copyright (C) 2001 StrmnNrmn\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"Input Plugin\n");
	fprintf(fp,"Authors\n");
	fprintf(fp,"oDD and Lantus\n");
	fprintf(fp,"Copyright (C) 1989 - 1991\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"SDL Sound Plugin\n");
	fprintf(fp,"Author\n");
	fprintf(fp,"JttL\n");
	fprintf(fp,"Copyright (C) 2003 by Juha Luotio aka JttL\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"HLE Sound Plugin\n");
	fprintf(fp,"Author\n");
	fprintf(fp,"Azimer\n");
	fprintf(fp,"Copyright (C) 2000 - 2001 Azimer\n");
	fprintf(fp,"\n\n");
	fprintf(fp,"RSP Plugin\n");
	fprintf(fp,"Author\n");
	fprintf(fp,"Hacktarux\n");
	fprintf(fp,"Copyright (C) 2001 Jabo and Zilmar\n");
	fprintf(fp,"\n\n");
	fclose(fp);
}

// emulators that support no rsp should pass this check
bool EmuDoesNoRsp(int p_iEmulator)
{
	bool bEmuNoRsp = false;;
	
	// add supporting emulators here
	switch (p_iEmulator)
	{
		case _1964: 		bEmuNoRsp = true;
			break;
		case _Project64: 	bEmuNoRsp = false;
			break;
		case _UltraHLE: 	bEmuNoRsp = false;
			break;
	}
	
	return bEmuNoRsp;
}

// rsp plugins that support process alisting
bool RspDoesAlist(int p_iRspPlugin)
{
	bool bRspAlist = false;
	
	// add supporting rsp plugins here
	switch (p_iRspPlugin)
	{
		case _RSPPluginLLE: 	bRspAlist = true;
			break;
		case _RSPPluginHLE: 	bRspAlist = false;
			break;
		case _RSPPluginM64p: 	bRspAlist = true;
			break;
	}
	
	return bRspAlist;
}

// load any and all user settings from surreal.ini and xxxxxxxx.ini for the selected rom
// needed for main menu, to ensure user config is correct when igm (ConfigAppLoad2) reads it
// this is basically a substitute for ConfigAppLoad2, but for the main menu because it needs to check the surreal.ini hash
int ConfigAppLoad3() //LoadRomConfig()
{
	int iLoad2Res = ConfigAppLoad2(); // returns 1 if there is no xxxxxxxx.ini
	if (iLoad2Res == 1 && romcounter > 0) // has roms and no rom ini found. load defaults and then get user pref BEFORE ConfigAppSave2.
	{
	
		Rom *sRom = g_romList.GetRomAt(actualrom); // get data for currently selected rom in menu
		
		// get the setting from the surreal.ini hash if any
		// int's will return -1 if not set, bools will return NULL
		
		preferedemu = sRom->GetIniEntry()->preferedemu;
			
		videoplugin = sRom->GetIniEntry()->videoplugin;

		iAudioPlugin = sRom->GetIniEntry()->iAudioPlugin;
		
		iRspPlugin = sRom->GetIniEntry()->iRspPlugin;
		
		iPagingMethod = sRom->GetIniEntry()->iPagingMethod;
		
		dw1964DynaMem = sRom->GetIniEntry()->dw1964DynaMem;
		
		dw1964PagingMem = sRom->GetIniEntry()->dw1964PagingMem;

		dwPJ64DynaMem = sRom->GetIniEntry()->dwPJ64DynaMem;

		dwPJ64PagingMem = sRom->GetIniEntry()->dwPJ64PagingMem;

		// ultrahle mem settings
		dwUltraCodeMem = sRom->GetIniEntry()->dwUltraCodeMem;
		
		dwUltraGroupMem = sRom->GetIniEntry()->dwUltraGroupMem;

		dwMaxVideoMem = sRom->GetIniEntry()->dwMaxVideoMem;
		
		bUseRspAudio = sRom->GetIniEntry()->bUseRspAudio;
			
			
		// leave for backward compatibility
		bUseLLERSP = sRom->GetIniEntry()->bUseLLERSP;

		bUseBasicAudio = sRom->GetIniEntry()->bUseBasicAudio;
	}
	
	// some basic checks before we commit user pref
	// most of these were being done in IniFile.cpp before it entered the hash
	//---------------------------------------------------------------
	
	// also in IniFile.cpp for defaults
	int iMaxVideoMem = 10;
	int iMaxDynaMem = 20;
	int iMaxPagingMem = 20;
	if (has128ram) {
		iMaxDynaMem = 32;
		iMaxPagingMem = 64;
	}
	
	if (preferedemu < 0 || preferedemu >= _None)
		preferedemu = _1964;
		
	if (videoplugin < 0 || videoplugin >= _VideoPluginMissing)
		videoplugin = _VideoPluginRice560;
	
	if (iAudioPlugin < 0 || iAudioPlugin >= _AudioPluginMissing)
		iAudioPlugin = _AudioPluginJttl;
	
	if (iRspPlugin < 0 || iRspPlugin >= _RSPPluginMissing)
		iRspPlugin = _RSPPluginHLE;
	
	if (iPagingMethod < 0 || iPagingMethod >= _PagingMissing)
		iPagingMethod = _PagingXXX;
	
	if (dw1964DynaMem < 0 || dw1964DynaMem > iMaxDynaMem)
		dw1964DynaMem = 8;
		
	if (dw1964PagingMem < 0 || dw1964PagingMem > iMaxPagingMem)
		dw1964PagingMem = 4;
		
	if (dwPJ64DynaMem < 0 || dwPJ64DynaMem > iMaxDynaMem)
		dwPJ64DynaMem = 16;
		
	if (dwPJ64PagingMem < 0 || dwPJ64PagingMem > iMaxPagingMem)
		dwPJ64PagingMem = 4;
		
	// ultrahle mem settings
	if (dwUltraCodeMem < 0 || dwUltraCodeMem > iMaxDynaMem)
		dwUltraCodeMem = 5;
		
	if (dwUltraGroupMem < 0 || dwUltraGroupMem > iMaxDynaMem)
		dwUltraGroupMem = 10;

	if (dwMaxVideoMem < 0 || dwMaxVideoMem > iMaxVideoMem) // allow "0" for auto
		dwMaxVideoMem = 5;
		
	//---------------------------------------------------------------
	// leave these in case it's set in the ini - just force new setting
	//---------------------------------------------------------------//
	
	// - deprecated vars - //
	
	if (!iLoad2Res) // let user config override these - if it's set, ignore it
	{
		if (bUseLLERSP)
			iRspPlugin = _RSPPluginLLE; // force rsp lle plugin

		if (bUseBasicAudio)
			iAudioPlugin = _AudioPluginBasic; // force basic audio plugin
	}
	
	// - deprecated plugins - //
	
	if (iAudioPlugin == _AudioPluginLleRsp)
		iAudioPlugin = _AudioPluginBasic; // force basic audio plugin
	
	// - force other settings - //
	
	if (!EmuDoesNoRsp(preferedemu) && iRspPlugin == _RSPPluginNone)
		iRspPlugin = _RSPPluginHLE; // force rsp hle plugin
		
	if (!RspDoesAlist(iRspPlugin) && bUseRspAudio)
		bUseRspAudio = false; // force off if rsp plugins doesn't support it
		
	/*if (iPagingMethod == _PagingNone && !has128ram)
		iPagingMethod = _PagingXXX; // force xxx if they don't have enough ram*/
	
	//-------------------------------------------------------------//
	
	return iLoad2Res;
}

bool ConfigAppLoadPaths()
{
	if (!g_iniFile.IsLoaded())
		return false;
	
	sprintf(szPathRoms, "%s", g_iniFile.GetRomPath().c_str());
	sprintf(szPathMedia, "%s", g_iniFile.GetMediaPath().c_str());
	sprintf(szPathSkins, "%s", g_iniFile.GetSkinPath().c_str());
	sprintf(szPathSaves, "%s", g_iniFile.GetSavePath().c_str());
	sprintf(szPathScreenshots, "%s", g_iniFile.GetScreenshotPath().c_str());

	if (!onhd && strcmp(szPathSaves, "D:\\Saves\\") == 0)
		sprintf(szPathSaves, "T:\\Saves\\");
		
	if (!onhd && strcmp(szPathScreenshots, "D:\\Screenshots\\") == 0)
		sprintf(szPathScreenshots, "T:\\Screenshots\\");
		
	return true;
}

// Ez0n3 - determine if the current phys ram is greater than 100MB
bool PhysRam128()
{
  MEMORYSTATUS memStatus;
  GlobalMemoryStatus( &memStatus );
  if( memStatus.dwTotalPhys < (100 * 1024 * 1024) ) return false;
  else return true;  
}

void ShowTempMessage(const char *msg)
{
	bTempMessage = true;
	strncpy(szTempMessage, msg, 99);
	dwTempMessageStart = GetTickCount();
}

void DrawTempMessage()
{
	if (bTempMessage) {
		if (GetTickCount() > dwTempMessageStart + 3000)
			bTempMessage = false;
		
		WCHAR buf[200];
		memset(buf, 0, sizeof(WCHAR) * 200);
		mbstowcs(buf, szTempMessage, strlen(szTempMessage));
		
		m_Font.Begin();
		m_Font.DrawText((float)iTempMessagePosX, (float)iTempMessagePosY, dwTitleColor, buf, GetFontAlign(iTempMessageAlign));
		m_Font.End();
	}
}

void BuildDirectories()
{
	if (!PathFileExists("T:\\Data")) { // .dat files
		if (!CreateDirectory("T:\\Data", NULL)) {
			OutputDebugString("T:\\Data Could Not Be Created!\n");
		}
	}
	if (!PathFileExists("T:\\Misc")) { // logs and stuff
		if (!CreateDirectory("T:\\Misc", NULL)) {
			OutputDebugString("T:\\Misc Could Not Be Created!\n");
		}
	}
	if (!PathFileExists("T:\\Temp")) { // temporary
		if (!CreateDirectory("T:\\Temp", NULL)) {
			OutputDebugString("T:\\Temp Could Not Be Created!\n");
		}
	}
	if (!PathFileExists(szPathRoms)) {
		if (!CreateDirectory(szPathRoms, NULL)) {
			OutputDebugString(szPathRoms);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	if (!PathFileExists(szPathMedia)) {
		if (!CreateDirectory(szPathMedia, NULL)) {
			OutputDebugString(szPathMedia);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	if (!PathFileExists(szPathSkins)) {
		if (!CreateDirectory(szPathSkins, NULL)) {
			OutputDebugString(szPathSkins);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	if (!PathFileExists(szPathSaves)) {
		if (!CreateDirectory(szPathSaves, NULL)) {
			OutputDebugString(szPathSaves);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	if (!PathFileExists(szPathScreenshots)) {
		if (!CreateDirectory(szPathScreenshots, NULL)) {
			OutputDebugString(szPathScreenshots);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	
	// sub dirs
	char subdir[256];
	sprintf(subdir, "%sboxart", szPathMedia);
	if (!PathFileExists(subdir)) {
		if (!CreateDirectory(subdir, NULL)) {
			OutputDebugString(subdir);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	sprintf(subdir, "%sui", szPathMedia);
	if (!PathFileExists(subdir)) {
		if (!CreateDirectory(subdir, NULL)) {
			OutputDebugString(subdir);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	sprintf(subdir, "%sui\\controller", szPathMedia);
	if (!PathFileExists(subdir)) {
		if (!CreateDirectory(subdir, NULL)) {
			OutputDebugString(subdir);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
	
	sprintf(subdir, "%sSynopsis", szPathMedia);
	if (!PathFileExists(subdir)) {
		if (!CreateDirectory(subdir, NULL)) {
			OutputDebugString(subdir);
			OutputDebugStringA(" Could Not Be Created!\n");
		}
	}
}
