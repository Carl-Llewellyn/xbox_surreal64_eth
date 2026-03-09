/*$T Dll_Video.c GC 1.136 03/09/02 17:41:40 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Video plugin interface functions
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

#include <xtl.h>

#include "../globals.h"
#include "../memory.h"
#include "registry.h"
#include "DLL_Video.h"
#include "wingui.h"
#include "../emulator.h"

GFX_INFO	Gfx_Info;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_GetDllInfo(PLUGIN_INFO *Plugin_Info)
{
	__try
	{
		_VIDEO_GetDllInfo(Plugin_Info);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("GettDllInfo Failed.");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

BOOL VIDEO_InitiateGFX(GFX_INFO Gfx_Info)
{
	__try
	{
		_VIDEO_InitiateGFX(Gfx_Info);
	}
	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		/* DisplayError("Cannot Initialize Graphics"); */
	}

	return(1);	/* why not for now.. */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_ProcessDList(void)
{
	_VIDEO_ProcessDList();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_RomOpen(void)
{
	__try
	{
		_VIDEO_RomOpen();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("Video RomOpen Failed.");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_RomClosed(void)
{
	__try
	{
		_VIDEO_RomClosed();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("Video RomClosed Failed.");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_ChangeWindow(int window)
{
	__try
	{
		_VIDEO_ChangeWindow();
		guistatus.IsFullScreen ^= 1;
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("VIDEO ChangeWindow failed");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_DllClose(void)
{
	__try
	{
		_VIDEO_CloseDLL();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("VIDEO DllClose failed");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*void CloseVideoPlugin(void)
{
	VIDEO_DllClose();
	DestroyGraphicsPlugin();
}*/

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_DllConfig(HWND hParent)
{
	_VIDEO_DllConfig(hParent);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_About(HWND hParent)
{
	_VIDEO_DllAbout(hParent);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_Test(HWND hParent)
{
	_VIDEO_DllTest(hParent);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_MoveScreen(int x, int y)
{
	_VIDEO_MoveScreen(x, y);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
#include "../n64rcp.h"
void VIDEO_UpdateScreen(void)
{
	//static int recall = 0x04000000+307200*2;
	//static int k=0;

	__try
	{
		_VIDEO_UpdateScreen();
	}
	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("Video UpdateScreen failed.");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_DrawScreen(void)
{
	__try
	{
		_VIDEO_DrawScreen();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("Video DrawScreen failed.");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_ViStatusChanged(void)
{
	__try
	{
		_VIDEO_ViStatusChanged();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("Exception in ViStatusChanged");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_ViWidthChanged(void)
{
	__try
	{
		_VIDEO_ViWidthChanged();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("Exception in ViWidthChanged");
	}
}

/*
 =======================================================================================================================
    changes for spec 1.3
 =======================================================================================================================
 */
void VIDEO_CaptureScreen(char *Directory)
{
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_ProcessRDPList(void)
{
	__try
	{
		_VIDEO_ProcessRDPList();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		DisplayError("Exception in Processing RDP List");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VIDEO_ShowCFB(void)
{
}
