/*$T wingui.h GC 1.136 02/28/02 07:53:40 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


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
#ifndef _WINGUI_H__1964_
#define _WINGUI_H__1964_

/* Functions in the wingui.c */
int APIENTRY		WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow);
HWND				InitWin98UI(HANDLE hInstance, int nCmdShow);
LRESULT APIENTRY	MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY	About(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
void __cdecl		DisplayError(char *Message, ...);
void __cdecl		DisplayCriticalMessage(char *Message, ...);

BOOL				WinLoadRom(void);
BOOL				WinLoadRomStep2(char *szFileName);
void				Pause(void);
void				Resume(void);
void				Kill(void);
void				Play(BOOL WithFullScreen);
void				Stop(void);
void				KailleraPlay(void);
void				OpenROM(void);
void				CloseROM(void);
void				ChangeDirectory(void);
void				SaveState(void);
void				LoadState(void);
void				SaveStateByNumber(WPARAM wparam);
void				LoadStateByNumber(WPARAM wparam);
void				SaveStateByDialog(int format);
void				LoadStateByDialog(int format);
void				EnableStateMenu(void);
void				DisableStateMenu(void);
void				PrepareBeforePlay(int IsFullScreen);
void				KillCPUThread(void);
void				SetCounterFactor(int);
void				SetCodeCheckMethod(int);
void				InitPluginData(void);
void				Set_1964_Directory(void);
void				CountryCodeToCountryName_and_TVSystem(int countrycode, char *countryname, int *tvsystem);
void				CaptureScreenToFile(void);
//void				Set_Ready_Message(void);
void				DisableDebugMenu(void);
void				SetupDebuger(void);
//void				SaveCmdLineParameter(char *cmdline);
BOOL				StartGameByCommandLine();

void				StateSetNumber(int number);
void				Exit1964(void);

#define MAXFILENAME 256					/* maximum length of file pathname */

struct EMU1964GUI
{
	char		*szBaseWindowTitle;
	HINSTANCE	hInst;
	HWND		hwnd1964main;			/* handle to main window */
	HWND		hClientWindow;			/* Window handle of the client child window */
	char	szWindowTitle[80];
};

extern struct EMU1964GUI	gui;

struct GUISTATUS
{
	int		clientwidth;				/* Client window width */
	int		clientheight;				/* Client window height */
	RECT	window_position;			/* 1964 main window location */
	BOOL	WindowIsMaximized;
	BOOL	window_is_moving;
	BOOL	window_is_maximized;
	BOOL	window_is_minimized;
	BOOL	block_menu;					/* Block all menu command while 1964 is busy */
	int		IsFullScreen;
};
extern struct GUISTATUS guistatus;

struct DIRECTORIES
{
	char	main_directory[256];
	char	save_directory_to_use[256];
};

extern struct DIRECTORIES	directories;

extern char					game_country_name[10];
extern int					game_country_tvsystem;

void LoadPlugins();

extern void __EMU_GetStateFilename(int index, char *filename, int mode);

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
extern BOOL PathFileExists(const char *pszPath);

extern char g_szPathSaves[256];

#endif
