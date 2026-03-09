#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>

#define MAX_FILE_PATH 256
#define MAX_SAVE_STATES 5

// Ez0n3 - plugins
#include "Plugins.h"

#include "../simpleini.h" // ini parser

#define QUOTE_MAX (128)						// Maximum length of "quoted strings"
int QuoteRead(char** pszQuote, char** pszEnd, char* szSrc);					// Read a quoted string from szSrc and point to the end

char *LabelCheck(char* s, char* szLabel);
// Skin
char *LabelCheckNew( char* s, char* szLabel );
//
#define SKIP_WS(s) for (;;) { if (*s != ' ' && *s != '\t') break; s++; }	// skip whitespace
#define SKIP_EQ(e) for (;;) { if (*e != '=') break; e++; }	// skip equals
int ConfigAppLoad();				// char* lpszName = NULL
int ConfigAppSave();
int ConfigAppLoad2();				
int ConfigAppSave2();
int ConfigAppLoadTemp();				
int ConfigAppSaveTemp();
//freakdave
extern int dw1964DynaMem;
extern int dw1964PagingMem;
extern int dwPJ64DynaMem;
extern int dwPJ64PagingMem;
extern int iAudioPlugin; //use iAudioPlugin instead to determine if basic audio is used
extern int iRspPlugin;
extern bool bUseRspAudio; // control a listing

extern int iPagingMethod;

// ultrahle mem settings
extern int dwUltraCodeMem;
extern int dwUltraGroupMem;

// use iAudioPlugin instead, but leave this in case it's set in ini
extern bool bUseLLERSP;
extern bool bUseBasicAudio;

// Ez0n3 - reinstate max video mem until freakdave finishes this
//extern int dwFreeMem;
extern int dwMaxVideoMem;

extern bool HideLaunchScreens;
extern bool FrameSkip;
extern int VSync;
extern int AntiAliasMode;
//extern int RefreshRateInHz;
extern int FlickerFilter;
extern bool SoftDisplayFilter;
extern int TextureMode;
extern int VertexMode;
extern float XBOX_CONTROLLER_DEAD_ZONE;
//#define XBOX_CONTROLLER_DEAD_ZONE 8600
extern float Deadzone;
extern int Sensitivity;
extern int DefaultPak;
extern bool EnableController1;
extern bool EnableController2;
extern bool EnableController3;
extern bool EnableController4;
extern int preferedemu;
extern int videoplugin;
extern bool bEnableHDTV;
extern bool bFullScreen;

extern char szPathRoms[MAX_FILE_PATH];
extern char szPathMedia[MAX_FILE_PATH];
extern char szPathSkins[MAX_FILE_PATH];
extern char szPathSaves[MAX_FILE_PATH];
extern char szPathScreenshots[MAX_FILE_PATH];

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
extern bool PathFileExists(const char *pszPath);