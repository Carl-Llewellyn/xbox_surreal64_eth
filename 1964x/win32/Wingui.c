/*
 * 1964 Copyright (C) 1999-2002 Joel Middendorf, <schibo@emulation64.com> This
 * program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: schibo@emulation64.com, rice1964@yahoo.com
 */

#include <xtl.h>

#include "wingui.h"
#include "../debug_option.h"
#include "../hardware.h"
#include "../fileio.h"
#include "../emulator.h"
#include "../interrupt.h"
#include "../memory.h"
#include "../network.h"
#include "../iPIF.h"
#include "../gamesave.h"
#include "DLL_Video.h"
#include "DLL_Audio.h"
#include "DLL_Input.h"
#include "DLL_RSP.h"
#include "registry.h"
#include "../r4300i.h"
#include "../timer.h"
#include "../rompaging.h"
//#include "../../LaunchData.h"	// Ez0n3 - launchdata isn't used anymore
#include "../dynarec/x86.h"
#include "../IOSupport.h"
#include "../128meg.h"

struct EMU1964GUI	gui;
struct DIRECTORIES	directories;
struct GUISTATUS	guistatus;

int					ActiveApp;

char				game_country_name[10];
int					game_country_tvsystem = 0;

int					Audio_Is_Initialized = 0;
int					timer;
int					StateFileNumber = 0;

extern int			selected_rom_index;
extern BOOL			Is_Reading_Rom_File;;
extern BOOL			To_Stop_Reading_Rom_File;
extern BOOL			opcode_debugger_memory_is_allocated;
extern HINSTANCE	hinstControllerPlugin;
BOOL				NeedFreshromListAfterStop = TRUE;

LRESULT APIENTRY		OptionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);

void					SelectVISpeed(WPARAM wParam);
void					SetupAdvancedMenus(void);
void					RegenerateAdvancedUserMenus(void);
void					DeleteAdvancedUserMenus(void);
void					RegenerateStateSelectorMenus(void);
void					DeleteStateSelectorMenus(void);
void					RegerateRecentGameMenus(void);
void					DeleteRecentGameMenus(void);
void					RegerateRecentRomDirectoryMenus(void);
void					DeleteRecentRomDirectoryMenus(void);
void					RefreshRecentRomDirectoryMenus(char *newromdirectory);
void					ChangeToRecentDirectory(int id);
void					OpenRecentGame(int id);
void					UpdateCIC(void);
void					init_debug_options(void);
extern LRESULT APIENTRY PluginsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
long					OnNotifyStatusBar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
long					OnPopupMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
long					OnOpcodeDebuggerCommands(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void					OnFreshRomList();
void					DisableNetplayMemu();

/*//weinerschnitzel - lets do this for rom paging now
// Ez0n3 - determine if the current phys ram is greater than 100MB
extern int	RAM_IS_128 = 0; //assume we are 64mb
extern BOOL PhysRam128(){
  MEMORYSTATUS memStatus;
  GlobalMemoryStatus( &memStatus );
  if( memStatus.dwTotalPhys < (100 * 1024 * 1024) ){ 
	  RAM_IS_128 = 0;
	  return FALSE;
  }else{
	  RAM_IS_128 = 1;
	  return TRUE;
  }
}*/

#ifdef DEBUG
char tracemessage[256];
#endif

char g_szPathSaves[256] = "D:\\Saves\\";

// Ez0n3 - reinstate max video mem until freakdave finishes this
extern void _VIDEO_SetMaxTextureMem(DWORD mem);

BOOL MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	OutputDebugString(lpText);
	return 0;

}

void __EMU_SaveState(int index)
{
	char buf[_MAX_PATH];
	sprintf(buf, "%s%08x\\%08X-%08X-%02X.%i", g_szPathSaves, currentromoptions.crc1, currentromoptions.crc1, currentromoptions.crc2, currentromoptions.countrycode, index);
	FileIO_ExportPJ64State(buf);
}

void __EMU_LoadState(int index)
{
	char buf[_MAX_PATH];
	sprintf(buf, "%s%08x\\%08X-%08X-%02X.%i", g_szPathSaves, currentromoptions.crc1, currentromoptions.crc1, currentromoptions.crc2, currentromoptions.countrycode, index);
	FileIO_ImportPJ64State(buf);
}

void __EMU_GetStateFilename(int index, char *filename, int mode)
{
	if(mode == 0){
		sprintf(filename, "%s%08x\\%08X-%08X-%02X.%i", g_szPathSaves, currentromoptions.crc1, currentromoptions.crc1, currentromoptions.crc2, currentromoptions.countrycode, index);
	}
	else if(mode == 1){
		sprintf(filename, "%s%08x\\%08X-%08X-%02X.%i.bmp", g_szPathSaves, currentromoptions.crc1, currentromoptions.crc1, currentromoptions.crc2, currentromoptions.countrycode, index);
	}
	return;
}

int avail;
MEMORYSTATUS stat;
char buf[100];
extern void loadinis();
extern int loaddw1964PagingMem();
extern int loaddw1964DynaMem();

// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
//extern int loadbUseLLERSP(); // not used anymore
extern int loadbUseRspAudio();
extern int loadiRspPlugin();
extern int loadiAudioPlugin();

// Ez0n3 - reinstate max video mem
extern int loaddwMaxVideoMem();

extern int loadiPagingMethod();
extern int loadbAudioBoost();

extern void GetPathSaves(char *pszPathSaves);


char emuname[256];

void __cdecl main()
{
	// mount the common drives
	Mount("A:","cdrom0");
	Mount("E:","Harddisk0\\Partition1");
	//Mount("C:","Harddisk0\\Partition2");
	//Mount("X:","Harddisk0\\Partition3");
	//Mount("Y:","Harddisk0\\Partition4");
	//Mount("Z:","Harddisk0\\Partition5");
	Mount("F:","Harddisk0\\Partition6");
	Mount("G:","Harddisk0\\Partition7");
	
	// utility shoud be mounted automatically
	if(XGetDiskSectorSize("Z:\\") == 0)
		Mount("Z:","Harddisk0\\Partition5");
	
	// make sure there's a temp rom
	if (PathFileExists("Z:\\TemporaryRom.dat")) {
		DisplayError("Z:\\TemporaryRom.dat File Found!");
		strcpy(g_temporaryRomPath, "Z:\\TemporaryRom.dat");
	}
	else {
		DisplayError("Z:\\TemporaryRom.dat File Not Found!");
		
		// if debugging, a temp rom can be placed in T to skip the launcher
		if (PathFileExists("T:\\Data\\TemporaryRom.dat")) {
			DisplayError("T:\\Data\\TemporaryRom.dat File Found!");
			strcpy(g_temporaryRomPath, "T:\\Data\\TemporaryRom.dat");
		}
		else {
			DisplayError("T:\\Data\\TemporaryRom.dat File Not Found!");
			Sleep(100);
			XLaunchNewImage("D:\\default.xbe", NULL);
		}
	}

	loadinis();
	sprintf(emuname,"1964x");
	
	GetPathSaves(g_szPathSaves);

	//freakdave - check for 128mb
	//PhysRam128();

	g_dwRecompCodeSize = loaddw1964DynaMem() * 1024 * 1024;
	
	
	g_iPagingMethod = loadiPagingMethod();
	if (g_iPagingMethod == _PagingXXX) {
		g_dwPageSize = 0x40000;
	}
	else if (g_iPagingMethod == _PagingS10) {
		g_dwPageSize = 0x10000;
	}
	Enable128MegCaching();

	
	// max texture mem, 0 = auto
	_VIDEO_SetMaxTextureMem(loaddwMaxVideoMem());
	
	g_bAudioBoost = (loadbAudioBoost() == 1 ? TRUE : FALSE); // set before audio init
	
	// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
	//g_bUseLLERspPlugin = loadbUseLLERSP(); // not used anymore
	g_bUseRspAudio = loadbUseRspAudio(); // control a listing
	g_iRspPlugin = loadiRspPlugin();
	g_iAudioPlugin = loadiAudioPlugin();
	
	LoadAudioPlugin();
	LoadRSPPlugin();

	// start the emulator
	gui.szBaseWindowTitle = "1964 0.8.5";
	gui.hwnd1964main = NULL;		/* handle to main window */
	gui.hClientWindow = NULL;		/* Window handle of the client child window */

	Rom_Loaded = FALSE;
	guistatus.block_menu = TRUE;	/* block all menu commands during startip */
	emustatus.cpucore = DYNACOMPILER;
	emustatus.Emu_Is_Resetting = FALSE;
	guistatus.IsFullScreen = FALSE;

	p_gHardwareState = (HardwareState *) &gHardwareState;
	p_gMemoryState = (MemoryState *) &gMemoryState;

	SetDefaultOptions();

	Set_1964_Directory();
	ReadConfiguration();			/* System registry settings */

	InitVirtualMemory();

	InitPluginData();

	emustatus.cpucore = defaultoptions.Emulator;

	LoadPlugins();

	r4300i_Init();

//	if(WinLoadRomStep2(pLd->szFilename))
//	{
	WinLoadRomStep2(g_temporaryRomPath);
	
		Play(emuoptions.auto_full_screen); /* autoplay*/ 
/*	}
	else
	{
		OutputDebugString("Rom load failed");
	}*/
	
	{ // create the save directory if it doesn't exist
		char szPathSaves[_MAX_PATH];
		sprintf(szPathSaves, "%s%08x", g_szPathSaves, currentromoptions.crc1);
		if (!PathFileExists(szPathSaves)) {
			if (!CreateDirectory(szPathSaves, NULL)) {
				DisplayError("%s Could Not Be Created!", szPathSaves);
			}
		}
	}
	
	while (TRUE)
	{
#ifdef DEBUG
		char buf[100];
		MEMORYSTATUS stat;
		GlobalMemoryStatus(&stat);
		sprintf(buf, "\n%.2fMB of RAM available\n", (float)(stat.dwAvailPhys / 1024.0f / 1024.0f));
		OutputDebugString(buf);
#endif
		Sleep(1000);
	}
}

BOOL PathFileExists(const char *pszPath)
{   
    return GetFileAttributes(pszPath) != INVALID_FILE_ATTRIBUTES;   
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Pause(void)
{
	PauseEmulator();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Resume(void)
{
	if(emustatus.Emu_Is_Running && emustatus.Emu_Is_Paused)
	{
		ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
	}
}

void AfterStop(void);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void KillCPUThread(void)
{
	if(emustatus.Emu_Is_Running)
	{
		DWORD exitCode;

		SuspendThread(CPUThreadHandle);
		GetExitCodeThread(CPUThreadHandle, &exitCode);
		ExitThread(exitCode);
		CloseHandle(CPUThreadHandle);

		if(currentromoptions.Code_Check == CODE_CHECK_PROTECT_MEMORY) 
			UnprotectAllBlocks();

		AUDIO_RomClosed();
		CONTROLLER_RomClosed();
		VIDEO_RomClosed();

		AfterStop();
	}

	//To finish off the window placement and settings
	Rom_Loaded = TRUE;
	CloseROM(); 
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Kill(void)
{
	NetProbe_Shutdown();
	StopEmulator();
	CPUThreadHandle = NULL;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void Play(BOOL WithFullScreen)
{
	/*~~~~~*/
	int core;
	/*~~~~~*/

	if(Rom_Loaded)
	{
		if(emustatus.Emu_Is_Running) 
		{
			Stop();
		}

		PrepareBeforePlay(guistatus.IsFullScreen);

		core = currentromoptions.Emulator;
		if(core == DYNACOMPILER)
		{					/* Dynarec */
			emustatus.cpucore = DYNACOMPILER;
		}
		else
		{					/* Interpreter */
			emustatus.cpucore = INTERPRETER;
		}

		NetProbe_Startup();
		r4300i_Reset();
		RunEmulator(emustatus.cpucore);

		if(WithFullScreen && (emustatus.Emu_Is_Resetting == 0))
		{
			if(guistatus.IsFullScreen == 0)
			{
				VIDEO_ChangeWindow(guistatus.IsFullScreen);
			}
		}
	}
	else
		DisplayError("Please load a ROM first.");
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Stop()
{
	if(emustatus.Emu_Is_Running)
	{
		NetProbe_Shutdown();
		StopEmulator();

		if (emustatus.Emu_Is_Resetting == 0)
		{
			if(guistatus.IsFullScreen)
			{
				VIDEO_ChangeWindow(guistatus.IsFullScreen);
			}

			AfterStop();
		}
	}
	else if(Is_Reading_Rom_File)
	{
		To_Stop_Reading_Rom_File = TRUE;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CloseROM(void)
{
	if(emustatus.Emu_Is_Running)
	{
		Stop();

		Close_iPIF();
		FreeVirtualRomMemory();
		r4300i_Init();

		Rom_Loaded = FALSE;
	}
}

/*
=======================================================================================================================
=======================================================================================================================
*/

BOOL WinLoadRomStep2(char *szFileName)
{
	lstrcpy(gui.szWindowTitle, gui.szBaseWindowTitle);
	lstrcat(gui.szWindowTitle, " - ");
	
	if(ReadRomData(szFileName) == FALSE) 
	{
		Rom_Loaded = FALSE;
		return FALSE;
	}
	
	

	lstrcat(gui.szWindowTitle, rominfo.name);
	
	// added by oDD
	// replaces: memcpy(, &gMemoryState.ROM_Image[0], 0x40);
	{
		FILE *tmpFile = fopen(g_temporaryRomPath, "rb");
		fseek(tmpFile, 0, SEEK_SET);
		fread(&HeaderDllPass[0], sizeof(uint8), 0x40, tmpFile);
		fclose(tmpFile);
	}

	Rom_Loaded = TRUE;
	gHWS_pc = 0xA4000040;	/* We do it in r4300i_inithardware */
	
	UpdateCIC();
	
	return TRUE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SaveState(void)
{
	if(Rom_Loaded)
	{
		if(emustatus.Emu_Is_Running)
		{
			if(PauseEmulator())
			{
				FileIO_gzSaveState();
				ResumeEmulator(DO_NOTHING_AFTER_PAUSE);
			}
		}
		else
		{
			FileIO_gzSaveState();
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void LoadState(void)
{
	/*~~~~~~~~~~~~~~~~~~~~*/
	int was_running = FALSE;
	/*~~~~~~~~~~~~~~~~~~~~*/

	if(Rom_Loaded)
	{
		if(emustatus.Emu_Is_Running)
		{
			if(PauseEmulator())
			{
				FileIO_gzLoadState();
				Init_Count_Down_Counters();
				ResumeEmulator(REFRESH_DYNA_AFTER_PAUSE);
			}
		}
		else
		{
			FileIO_gzLoadState();
			Init_Count_Down_Counters();
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void StateSetNumber(int number)
{
	StateFileNumber = number;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetCounterFactor(int factor)
{
	if(CounterFactor != factor)
	{
		if(emustatus.Emu_Is_Running)
		{
			if(PauseEmulator())
			{
				CounterFactor = factor;
				ResumeEmulator(REFRESH_DYNA_AFTER_PAUSE);	// Need to init emu
			}
		}

		CounterFactor = factor;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetCodeCheckMethod(int method)
{
	if(emustatus.Emu_Is_Running)
	{
		if(PauseEmulator())
		{
			Dyna_Check_Codes = Dyna_Code_Check[method - 1];
			emustatus.CodeCheckMethod = method;
			ResumeEmulator(REFRESH_DYNA_AFTER_PAUSE);	// Need to init emu
		}
	}
	else
	{
		emustatus.CodeCheckMethod = method;
		defaultoptions.Code_Check = method;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void PrepareBeforePlay(int IsFullScreen)
{
	/* Setting options */
	//RomListSelectLoadedRomEntry();
	GenerateCurrentRomOptions();
	init_whole_mem_func_array();					/* Needed here. The tlb function pointers change. */
	ResetRdramSize(currentromoptions.RDRAM_Size);
	//if(strcpy(current_cheatcode_rom_internal_name, currentromoptions.Game_Name) != 0)
	//	CodeList_ReadCode(currentromoptions.Game_Name);

	CounterFactor = currentromoptions.Counter_Factor;
	emustatus.CodeCheckMethod = currentromoptions.Code_Check;

	/*
	 * Using the Check_QWORD to boot, will switch to ROM specified
	 * emustatus.CodeCheckMethod £
	 * at first FPU exception. I don't know why use NoCheck method will not boot £
	 * Game like SuperMario should not need to do DynaCodeCheck but how the ROM does
	 * not boot £
	 * with DynaCodeCheck, need debug
	 */
	if(emustatus.CodeCheckMethod == CODE_CHECK_NONE || emustatus.CodeCheckMethod == CODE_CHECK_DMA_ONLY)
	{
		Dyna_Check_Codes = Dyna_Code_Check_None_Boot;
		TRACE0("Set code check method = Dyna_Code_Check_None_Boot / Check_DMA_only");
	}
	else
	{
		Dyna_Check_Codes = Dyna_Code_Check[emustatus.CodeCheckMethod - 1];
	}

	emustatus.cpucore = currentromoptions.Emulator;

	/* About FPU usage exceptions */
	if(currentromoptions.FPU_Hack == USEFPUHACK_YES)
	{
		EnableFPUUnusableException();
	}
	else
	{
		DisableFPUUnusableException();
	}

	Flashram_Init();
	Init_iPIF();

	emustatus.DListCount = 0;
	emustatus.AListCount = 0;
	emustatus.PIDMACount = 0;
	emustatus.ControllerReadCount = 0;

	if(!QueryPerformanceFrequency(&Freq))
	{
		currentromoptions.Max_FPS = MAXFPS_NONE;	/* ok, this computer does not support */
		/* accurate timer, don't use speed limiter */
	}
	else
	{
		if(rominfo.TV_System == TV_SYSTEM_PAL)					/* PAL */
		{
			vips_speed_limits[MAXFPS_AUTO_SYNC] = vips_speed_limits[MAXFPS_PAL_50];
		}
		else	/* NTSC */
		{
			vips_speed_limits[MAXFPS_AUTO_SYNC] = vips_speed_limits[MAXFPS_NTSC_60];
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AfterStop(void)
{
#ifdef CHEATCODE_LOCK_MEMORY
	CloseCheatCodeEngineMemoryLock();
#endif

	Close_Save();
	Close_iPIF();

	emustatus.Emu_Is_Running = FALSE;

	/* Reset some of the default options */
	defaultoptions.Emulator = DYNACOMPILER;
	emustatus.cpucore = defaultoptions.Emulator;

	SetCounterFactor(defaultoptions.Counter_Factor);
	SetCodeCheckMethod(defaultoptions.Code_Check);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CaptureScreenToFile(void)
{
}

static BOOL exiting_1964 = FALSE;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Exit1964(void)
{
	if(exiting_1964) 
		exit(0);

	exiting_1964 = TRUE;

	if(emustatus.Emu_Is_Running) 
		Stop();

	Close_iPIF();			/* save mempak and eeprom */

	FreeVirtualMemory();

	/*
	 * Here is the fix for the problem that 1964 crash when exiting if using opengl
	 * plugins. £
	 * I don't know why 1964 crash, looks like crash is not happen in 1964, but dll
	 * related. £
	 * just doing exit(0) will not crash,(maybe we have left some resource not
	 * released, donno)
	 */
	exit(0);
}


