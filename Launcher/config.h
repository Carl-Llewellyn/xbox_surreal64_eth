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

// lets try relying on ./config.cpp instead
#include "../Config.h"
#include "RomList.h" // only diff is we need the romlist in the main menu

/*
#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>

// Ez0n3 - plugins
#include "../Plugins.h"

#define QUOTE_MAX (128)						// Maximum length of "quoted strings"
int QuoteRead(char** pszQuote, char** pszEnd, char* szSrc);					// Read a quoted string from szSrc and point to the end

char *LabelCheck(char* s, char* szLabel);
// Skin
char *LabelCheckNew( char* s, char* szLabel );
//
#define SKIP_WS(s) for (;;) { if (*s != ' ' && *s != '\t') break; s++; }	// skip whitespace
int ConfigAppLoad();				// char* lpszName = NULL
int ConfigAppSave();
int ConfigAppLoad2();				
int ConfigAppSave2();
//freakdave
extern int dw1964DynaMem;
extern int dw1964PagingMem;
extern int dwPJ64DynaMem;
extern int dwPJ64PagingMem;
extern bool bUseLLERSP; // use iAudioPlugin instead, but leave this in case it's set in ini

// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
extern int iAudioPlugin;
extern int videoplugin;
extern int iRspPlugin;
extern bool bUseRspAudio; // control a listing

// Ez0n3 - reinstate max video mem until freakdave finishes this
//extern int dwFreeMem;
extern int dwMaxVideoMem;

extern int FlickerFilter;
extern bool SoftDisplayFilter;
extern int TextureMode;
extern int VertexMode;
extern bool FrameSkip;
extern int VSync;
extern float XBOX_CONTROLLER_DEAD_ZONE;
extern float Deadzone;
extern unsigned int Sensitivity;
extern int DefaultPak;
extern bool EnableController1;
extern bool EnableController2;
extern bool EnableController3;
extern bool EnableController4;
*/