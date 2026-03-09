/**********************************************************************************
Common Controller plugin spec, version #1.0 maintained by 
zilmar (zilmar@emulation64.com)

All questions or suggestions should go through the mailing list.
http://www.egroups.com/group/Plugin64-Dev
**********************************************************************************/
#pragma once

#include "Common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Note: BOOL, BYTE, WORD, DWORD, TRUE, FALSE are defined in windows.h */

/*** Conteroller plugin's ****/
#define PLUGIN_NONE					1
#define PLUGIN_MEMPAK				2
#define PLUGIN_RUMBLE_PAK			3 // not implemeted for non raw data
#define PLUGIN_TANSFER_PAK			4 // not implemeted for non raw data

/********************************************************************************* 
 Note about Conteroller plugin's: 
 the rumble pak needs a function for the force feed back joystick and tranfer pak 
 probaly needs a function for the plugin to be able to select the GB rom and 
 eeprom... maybe this should be done by the emu instead of the plugin, but I think
 it probaly should be done by the plugin. I will see about adding these functions 
 in the next spec
**********************************************************************************/

/***** Structures *****/
typedef struct {
	BOOL Present;
	BOOL RawData;
	int  Plugin;
} CONTROL;

typedef union {
	DWORD Value;
	struct {
		unsigned R_DPAD       : 1;
		unsigned L_DPAD       : 1;
		unsigned D_DPAD       : 1;
		unsigned U_DPAD       : 1;
		unsigned START_BUTTON : 1;
		unsigned Z_TRIG       : 1;
		unsigned B_BUTTON     : 1;
		unsigned A_BUTTON     : 1;

		unsigned R_CBUTTON    : 1;
		unsigned L_CBUTTON    : 1;
		unsigned D_CBUTTON    : 1;
		unsigned U_CBUTTON    : 1;
		unsigned R_TRIG       : 1;
		unsigned L_TRIG       : 1;
		unsigned Reserved1    : 1;
		unsigned Reserved2    : 1;

		signed   Y_AXIS       : 8;

		signed   X_AXIS       : 8;
	};
} BUTTONS;

/******************************************************************
  Function: _INPUT_CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/ 
void _INPUT_CloseDLL (void);

/******************************************************************
  Function: _INPUT_ControllerCommand
  Purpose:  To process the raw data that has just been sent to a 
            specific controller.
  input:    - Controller Number (0 to 3) and -1 signalling end of 
              processing the pif ram.
			- Pointer of data to be processed.
  output:   none
  
  note:     This function is only needed if the DLL is allowing raw
            data.

            the data that is being processed looks like this:
            initilize controller: 01 03 00 FF FF FF 
            read controller:      01 04 01 FF FF FF FF
*******************************************************************/
void _INPUT_ControllerCommand ( int Control, BYTE * Command);

/******************************************************************
  Function: _INPUT_DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
void _INPUT_DllAbout ( HWND hParent );

/******************************************************************
  Function: _INPUT_DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
void _INPUT_DllConfig ( HWND hParent );

/******************************************************************
  Function: _INPUT_DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
void _INPUT_DllTest ( HWND hParent );

/******************************************************************
  Function: _INPUT_GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
void _INPUT_GetDllInfo ( PLUGIN_INFO * PluginInfo );

/******************************************************************
  Function: _INPUT_GetKeys
  Purpose:  To get the current state of the controllers buttons.
  input:    - Controller Number (0 to 3)
            - A pointer to a BUTTONS structure to be filled with 
			the controller state.
  output:   none
*******************************************************************/  	
void _INPUT_GetKeys(int Control, BUTTONS * Keys );

/******************************************************************
  Function: _INPUT_InitiateControllers
  Purpose:  This function initialises how each of the controllers 
            should be handled.
  input:    - The handle to the main window.
            - A controller structure that needs to be filled for 
			  the emulator to know how to handle each controller.
  output:   none
*******************************************************************/  
void _INPUT_InitiateControllers (HWND hMainWindow, CONTROL Controls[4]);

/******************************************************************
  Function: _INPUT_ReadController
  Purpose:  To process the raw data in the pif ram that is about to
            be read.
  input:    - Controller Number (0 to 3) and -1 signalling end of 
              processing the pif ram.
			- Pointer of data to be processed.
  output:   none  
  note:     This function is only needed if the DLL is allowing raw
            data.
*******************************************************************/
void _INPUT_ReadController ( int Control, BYTE * Command );

/******************************************************************
  Function: _INPUT_RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
void _INPUT_RomClosed (void);

/******************************************************************
  Function: _INPUT_RomOpen
  Purpose:  This function is called when a rom is open. (from the 
            emulation thread)
  input:    none
  output:   none
*******************************************************************/ 
void _INPUT_RomOpen (void);

/******************************************************************
  Function: _INPUT_WM_KeyDown
  Purpose:  To pass the _INPUT_WM_KeyDown message from the emulator to the 
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/ 
void _INPUT_WM_KeyDown( WPARAM wParam, LPARAM lParam );

/******************************************************************
  Function: _INPUT_WM_KeyUp
  Purpose:  To pass the WM_KEYUP message from the emulator to the 
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/ 
void _INPUT_WM_KeyUp( WPARAM wParam, LPARAM lParam );

/******************************************************************
  Function: _INPUT_SetRumble
  Purpose:  Turns the rumble on a controller on or off.
  input:    the controller and whether rumble is on or not
  output:   none
*******************************************************************/ 
void _INPUT_SetRumble(int Control, BOOL on);

/******************************************************************
  Function: _INPUT_LoadButtonMap
  Purpose:  Loads a button configuration for the current game
  input:    a 72byte array of cfg data
  output:   none
*******************************************************************/ 
void _INPUT_LoadButtonMap(int *cfgData);

#if defined(__cplusplus)
}
#endif

/******************************************************************
  Function: _INPUT_GetXGamepadHandle
  Purpose:  Returns a handle to the specified xbox gamepad
  input:    controller needed
  output:   handle to the controller
*******************************************************************/
HANDLE _INPUT_GetXGamepadHandle(int controller);

/******************************************************************
  Function: _INPUT_IsIngameMenuWaiting
  Purpose:  When the right stick is pressed, this function will return true once.
  input:    none
  output:   none
*******************************************************************/ 
BOOL _INPUT_IsIngameMenuWaiting();

/******************************************************************
  Function: _INPUT_UpdatePaks
  Purpose:  If exiting IngameMenu, this function will update the Controller Slot and return true.
  input:    none
  output:   none
*******************************************************************/ 
BOOL _INPUT_UpdatePaks();

/******************************************************************
  Function: _INPUT_UpdateControllerStates
  Purpose:  Enables/Disables Controller
  input:    none
  output:   none
*******************************************************************/ 
BOOL _INPUT_UpdateControllerStates();
