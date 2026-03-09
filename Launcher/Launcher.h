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

#pragma once

#include <xtl.h>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
using namespace std;

#include "Common.h"
#include "D3D.h"
#include "..\Log.h"


enum Emulators
{
	_1964,
	_Project64,
	_UltraHLE,
	_None
};

// Ez0n3
extern int preferedemu;
extern int videoplugin;
extern int iAudioPlugin;
extern int iRspPlugin;
int ConfigAppLoad3(); // load user pref func
bool ConfigAppLoadPaths();
bool PhysRam128(); // determine if the current phys ram is greater than 100MB

bool EmuDoesNoRsp(int p_iEmulator);
bool RspDoesAlist(int p_iRspPlugin);

extern bool HideLaunchScreens;
extern bool FrameSkip;

extern bool g_bQuit;

extern void ShowTempMessage(const char *msg);
extern void DrawTempMessage();