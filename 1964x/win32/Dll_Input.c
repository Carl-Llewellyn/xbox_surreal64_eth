/*$T Dll_Input.c GC 1.136 03/09/02 17:41:27 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Input plugin interface functions
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
#include "registry.h"
#include "../plugins.h"
#include "DLL_Input.h"

CONTROL		Controls[4];

//void (__cdecl *_CONTROLLER_Under_Selecting_DllAbout) (HWND) = NULL;
//void (__cdecl *_CONTROLLER_Under_Selecting_DllTest) (HWND) = NULL;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*BOOL LoadControllerPlugin(char *libname)
{
	return TRUE;
}*/

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*void CloseControllerPlugin(void)
{
	_INPUT_RomClosed();

	_INPUT_CloseDLL();
}*/

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_CloseDLL(void)
{
	__try
	{
		_INPUT_CloseDLL();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_ControllerCommand(int _Control, BYTE *_Command)
{
	__try
	{
		_INPUT_ControllerCommand(_Control, _Command);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_DllAbout(HWND _hWnd)
{
	__try
	{
		_INPUT_DllAbout(_hWnd);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_DllConfig(HWND _hWnd)
{
	__try
	{
		_INPUT_DllConfig(_hWnd);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_DllTest(HWND _hWnd)
{
	__try
	{
		//_INPUT_DllTest(_hWnd);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_GetDllInfo(PLUGIN_INFO *_plugin)
{
	__try
	{
		_INPUT_GetDllInfo(_plugin);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_GetKeys(int _Control, BUTTONS *_Keys)
{
	__try
	{
		_INPUT_GetKeys(_Control, _Keys);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_InitiateControllers(HWND _hMainWindow, CONTROL _Controls[4])
{
	__try
	{
		_INPUT_InitiateControllers(_hMainWindow, _Controls);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}

	/*
	 * Add mempak support in 1964 by configure the control 4 as mempak £
	 * no matter if the control plugin support it or not
	 */
	if(_Controls[0].Plugin == PLUGIN_NONE) 
		_Controls[0].Plugin = PLUGIN_MEMPAK;

	/*
	 * Controls[1].Present = FALSE; £
	 * _Controls[2].Present = FALSE; £
	 * _Controls[3].Present = FALSE;
	 */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_ReadController(int _Control, BYTE *_Command)
{
	__try
	{
		_INPUT_ReadController(_Control, _Command);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_RomClosed(void)
{
	__try
	{
		_INPUT_RomClosed();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_RomOpen(void)
{
	__try
	{
		_INPUT_RomOpen();
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_WM_KeyDown(WPARAM _wParam, LPARAM _lParam)
{
	__try
	{
		_INPUT_WM_KeyDown(_wParam, _lParam);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CONTROLLER_WM_KeyUp(WPARAM _wParam, LPARAM _lParam)
{
	__try
	{
		_INPUT_WM_KeyUp(_wParam, _lParam);
	}

	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/*
 =======================================================================================================================
    Use when selecting plugin
 =======================================================================================================================
 */
/*void CONTROLLER_Under_Selecting_DllAbout(HWND _hWnd)
{
	if(_CONTROLLER_Under_Selecting_DllAbout != NULL)
	{
		__try
		{
			_CONTROLLER_Under_Selecting_DllAbout(_hWnd);
		}

		__except(NULL, EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
}*/

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*void CONTROLLER_Under_Selecting_DllTest(HWND _hWnd)
{
	if(_CONTROLLER_Under_Selecting_DllTest != NULL)
	{
		__try
		{
			_CONTROLLER_Under_Selecting_DllTest(_hWnd);
		}

		__except(NULL, EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
}*/
