/*
 * RSP Compiler plug in for Project 64 (A Nintendo 64 emulator).
 *
 * (c) Copyright 2001 jabo (jabo@emulation64.com) and
 * zilmar (zilmar@emulation64.com)
 *
 * pj64 homepage: www.pj64.net
 * 
 * Permission to use, copy, modify and distribute Project64 in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Project64 is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Project64 or software derived from Project64.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so if they want them.
 *
 */

#include <xtl.h>
#include <stdio.h>
#include "Rsp.h"
#include "CPU.h"
#include "Recompiler CPU.h"
#include "Rsp Command.h"
#include "Rsp Registers.h"
#include "rspmemory.h"
#include "breakpoint.h"
#include "resource.h"
 

UWORD   RSP_GPR[32], RSP_Flags[4];
UDWORD  RSP_ACCUM[8];
VECTOR  RSP_Vect[32];
 
BOOL GraphicsHle=1, AudioHle=0;
BOOL ConditionalMove;
BOOL Profiling, IndvidualBlock, ShowErrors;
DWORD CPUCore = RecompilerCPU;

HANDLE hMutex = NULL;

//DEBUG_INFO DebugInfo;
RSP_INFO RSPInfo;
HINSTANCE hinstDLL;
HMENU hRSPMenu = NULL;

DWORD AsciiToHex (char * HexValue) {
	DWORD Count, Finish, Value = 0;

	Finish = strlen(HexValue);
	if (Finish > 8 ) { Finish = 8; }

	for (Count = 0; Count < Finish; Count++){
		Value = (Value << 4);
		switch( HexValue[Count] ) {
		case '0': break;
		case '1': Value += 1; break;
		case '2': Value += 2; break;
		case '3': Value += 3; break;
		case '4': Value += 4; break;
		case '5': Value += 5; break;
		case '6': Value += 6; break;
		case '7': Value += 7; break;
		case '8': Value += 8; break;
		case '9': Value += 9; break;
		case 'A': Value += 10; break;
		case 'a': Value += 10; break;
		case 'B': Value += 11; break;
		case 'b': Value += 11; break;
		case 'C': Value += 12; break;
		case 'c': Value += 12; break;
		case 'D': Value += 13; break;
		case 'd': Value += 13; break;
		case 'E': Value += 14; break;
		case 'e': Value += 14; break;
		case 'F': Value += 15; break;
		case 'f': Value += 15; break;
		default: 
			Value = (Value >> 4);
			Count = Finish;
		}
	}
	return Value;
}
 

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/ 
void _RSP_CloseDLL (void) 
{
	if (!AudioHle)
		FreeMemory();
}
 
 
/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
void _RSP_GetDllInfo ( PLUGIN_INFO * PluginInfo ) {
	PluginInfo->Version = 0x0101;
	PluginInfo->Type = PLUGIN_TYPE_RSP;
	sprintf(PluginInfo->Name,"RSP emulation Plugin");
	PluginInfo->NormalMemory = FALSE;
	PluginInfo->MemoryBswaped = TRUE;
}

/******************************************************************
  Function: GetRspDebugInfo
  Purpose:  This function allows the emulator to gather information
            about the debug capabilities of the dll by filling in
			the DebugInfo structure.
  input:    a pointer to a RSPDEBUG_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 

#if !defined(EXTERNAL_RELEASE)

void   GetRspDebugInfo ( RSPDEBUG_INFO * DebugInfo ) {
	HKEY hKeyResults = 0;
	char String[200];
	long lResult;

	hRSPMenu = LoadMenu(hinstDLL,"RspMenu");
	DebugInfo->hRSPMenu = hRSPMenu;
	DebugInfo->ProcessMenuItem = ProcessMenuItem;

	DebugInfo->UseBPoints = TRUE;
	sprintf(DebugInfo->BPPanelName," RSP ");
	DebugInfo->Add_BPoint = Add_BPoint;
	DebugInfo->CreateBPPanel = CreateBPPanel;
	DebugInfo->HideBPPanel = HideBPPanel;
	DebugInfo->PaintBPPanel = PaintBPPanel;
	DebugInfo->RefreshBpoints = RefreshBpoints;
	DebugInfo->RemoveAllBpoint = RemoveAllBpoint;
	DebugInfo->RemoveBpoint = RemoveBpoint;
	DebugInfo->ShowBPPanel = ShowBPPanel;
	
	DebugInfo->Enter_RSP_Commands_Window = Enter_RSP_Commands_Window;

	sprintf(String,"Software\\N64 Emulation\\DLL\\%s",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0,KEY_ALL_ACCESS,&hKeyResults);
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Bytes = 4;

		lResult = RegQueryValueEx(hKeyResults,"Profiling On",0,&Type,(LPBYTE)(&Profiling),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { 
			Profiling = Default_ProfilingOn;
		}
		lResult = RegQueryValueEx(hKeyResults,"Log Indvidual Blocks",0,&Type,(LPBYTE)(&IndvidualBlock),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { 
			IndvidualBlock = Default_IndvidualBlock;
		}		
		lResult = RegQueryValueEx(hKeyResults,"Show Compiler Errors",0,&Type,(LPBYTE)(&ShowErrors),&Bytes);
		if (Type != REG_DWORD || lResult != ERROR_SUCCESS) { 
			ShowErrors = Default_ShowErrors;
		}
		RegCloseKey(hKeyResults);
	}
	if (Profiling) {
		CheckMenuItem( hRSPMenu, ID_PROFILING_ON, MF_BYCOMMAND | MFS_CHECKED );
	} else {
		CheckMenuItem( hRSPMenu, ID_PROFILING_OFF, MF_BYCOMMAND | MFS_CHECKED );
	}
	if (IndvidualBlock) {
		CheckMenuItem( hRSPMenu, ID_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND | MFS_CHECKED );
	}
	if (ShowErrors) {
		CheckMenuItem( hRSPMenu, ID_SHOWCOMPILERERRORS, MF_BYCOMMAND | MFS_CHECKED );
	}
}

#endif

 

/******************************************************************
  Function: InitiateRSP
  Purpose:  This function is called when the DLL is started to give
            information from the emulator that the n64 RSP 
			interface needs
  input:    Rsp_Info is passed to this function which is defined
            above.
			CycleCount is the number of cycles between switching
			control between teh RSP and r4300i core.
  output:   none
*******************************************************************/ 

RSP_COMPILER Compiler;

void DetectCpuSpecs(void) {
	DWORD Intel_Features = 0;
	DWORD AMD_Features = 0;

	__try {
		_asm {
			/* Intel features */
			mov eax, 1
			cpuid
			mov [Intel_Features], edx

			/* AMD features */
			mov eax, 80000001h
			cpuid
			or [AMD_Features], edx
		}
    } __except ( EXCEPTION_EXECUTE_HANDLER) {
		AMD_Features = Intel_Features = 0;
    }

	if (Intel_Features & 0x02000000) {
		Compiler.mmx2 = TRUE;
		Compiler.sse = TRUE;
	}
	if (Intel_Features & 0x00800000) {
		Compiler.mmx = TRUE;
	}
	if (AMD_Features & 0x40000000) {
		Compiler.mmx2 = TRUE;
	}
	if (Intel_Features & 0x00008000) {
		ConditionalMove = TRUE;
	} else {
		ConditionalMove = FALSE;
	}
}

void _RSP_InitiateRSP ( RSP_INFO Rsp_Info, DWORD * CycleCount) {
	//char RegPath[256];
	HKEY hKey = NULL;
	//DWORD dwSize, lResult, Disposition;

	RSPInfo = Rsp_Info;

	memset(&Compiler, 0, sizeof(Compiler));
	
	Compiler.bAlignGPR = TRUE;
	Compiler.bAlignVector = TRUE;
	Compiler.bFlags = TRUE;
	Compiler.bReOrdering = TRUE;
	Compiler.bSections = TRUE;
	Compiler.bDest = TRUE;
	Compiler.bAccum = TRUE;
	Compiler.bGPRConstants = TRUE;
	
	DetectCpuSpecs();

	if (RSPInfo.ProcessAList != NULL)
		AudioHle = 1;

	hMutex = CreateMutex(NULL, FALSE, NULL);
 
	*CycleCount = 0;

	if (!AudioHle)
		AllocateMemory();

	//InitilizeRSPRegisters();
	Build_RSP();
	#ifdef GenerateLog
	Start_Log();
	#endif
}

/******************************************************************
  Function: InitiateRSPDebugger
  Purpose:  This function is called when the DLL is started to give
            information from the emulator that the n64 RSP 
			interface needs to intergrate the debugger with the
			rest of the emulator.
  input:    DebugInfo is passed to this function which is defined
            above.
  output:   none
*******************************************************************/ 

#if !defined(EXTERNAL_RELEASE)

void InitiateRSPDebugger ( DEBUG_INFO Debug_Info) {
	DebugInfo = Debug_Info;
}

void ProcessMenuItem(int ID) {
	DWORD Disposition;
	HKEY hKeyResults;
	char String[200];
	long lResult;
	UINT uState;

	switch (ID) {
	case ID_RSPCOMMANDS: Enter_RSP_Commands_Window(); break;
	case ID_RSPREGISTERS: Enter_RSP_Register_Window(); break;
	case ID_DUMP_RSPCODE: DumpRSPCode(); break;
	case ID_DUMP_DMEM: DumpRSPData(); break;
	case ID_PROFILING_ON:
	case ID_PROFILING_OFF:
		uState = GetMenuState( hRSPMenu, ID_PROFILING_ON, MF_BYCOMMAND);			
		hKeyResults = 0;
		Disposition = 0;

		if ( uState & MFS_CHECKED ) {								
			CheckMenuItem( hRSPMenu, ID_PROFILING_ON, MF_BYCOMMAND | MFS_UNCHECKED );
			CheckMenuItem( hRSPMenu, ID_PROFILING_OFF, MF_BYCOMMAND | MFS_CHECKED );
			GenerateTimerResults();
			Profiling = FALSE;
		} else {
			CheckMenuItem( hRSPMenu, ID_PROFILING_ON, MF_BYCOMMAND | MFS_CHECKED );
			CheckMenuItem( hRSPMenu, ID_PROFILING_OFF, MF_BYCOMMAND | MFS_UNCHECKED );
			ResetTimerList();
			Profiling = TRUE;
		}
			
		sprintf(String,"Software\\N64 Emulation\\DLL\\%s",AppName);
		lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
			REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
		if (lResult == ERROR_SUCCESS) { 
			RegSetValueEx(hKeyResults,"Profiling On",0,REG_DWORD,(BYTE *)&Profiling,sizeof(DWORD));
		}
		RegCloseKey(hKeyResults);
		break;
	case ID_PROFILING_RESETSTATS: ResetTimerList(); break;
	case ID_PROFILING_GENERATELOG: GenerateTimerResults(); break;
	case ID_PROFILING_LOGINDIVIDUALBLOCKS:
		uState = GetMenuState( hRSPMenu, ID_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND);
		hKeyResults = 0;
		Disposition = 0;

		ResetTimerList();
		if ( uState & MFS_CHECKED ) {								
			CheckMenuItem( hRSPMenu, ID_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND | MFS_UNCHECKED );
			IndvidualBlock = FALSE;
		} else {
			CheckMenuItem( hRSPMenu, ID_PROFILING_LOGINDIVIDUALBLOCKS, MF_BYCOMMAND | MFS_CHECKED );
			IndvidualBlock = TRUE;
		}
			
		sprintf(String,"Software\\N64 Emulation\\DLL\\%s",AppName);
		lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
		REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
		if (lResult == ERROR_SUCCESS) { 
			RegSetValueEx(hKeyResults,"Log Indvidual Blocks",0,REG_DWORD,
				(BYTE *)&IndvidualBlock,sizeof(DWORD));
		}
		RegCloseKey(hKeyResults);
		break;
	case ID_SHOWCOMPILERERRORS:
		uState = GetMenuState(hRSPMenu, ID_SHOWCOMPILERERRORS, MF_BYCOMMAND);
		
		if (uState & MFS_CHECKED) {
			CheckMenuItem( hRSPMenu, ID_SHOWCOMPILERERRORS, MF_BYCOMMAND | MFS_UNCHECKED );
			ShowErrors = FALSE;
		} else {
			CheckMenuItem( hRSPMenu, ID_SHOWCOMPILERERRORS, MF_BYCOMMAND | MFS_CHECKED );
			ShowErrors = TRUE;
		}
		sprintf(String,"Software\\N64 Emulation\\DLL\\%s",AppName);
		lResult = RegCreateKeyEx( HKEY_CURRENT_USER,String,0,"",
		REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyResults,&Disposition);
			
		if (lResult == ERROR_SUCCESS) { 
			RegSetValueEx(hKeyResults,"Show Compiler Errors",0,REG_DWORD,
				(BYTE *)&ShowErrors, sizeof(DWORD));
		}
		RegCloseKey(hKeyResults);
		break;
	case ID_COMPILER:		
		DialogBox(hinstDLL, "RSPCOMPILER", HWND_DESKTOP, CompilerDlgProc);
		break;
	}
}

#endif

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
void  _RSP_RomClosed (void) {
	void ClearAllx86Code(void);

 
	//InitilizeRSPRegisters();
	if (!AudioHle)
		ClearAllx86Code();

	#ifdef GenerateLog
	Stop_Log();
	#endif
	#ifdef Log_x86Code
	Stop_x86_Log();
	#endif
}

BOOL GetBooleanCheck(HWND hDlg, DWORD DialogID) {

	return TRUE;
 
}

BOOL CALLBACK CompilerDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	 
	return TRUE;
}

BOOL CALLBACK ConfigDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	 
	return TRUE;
}

void _RSP_DllConfig (HWND hWnd) {
	 
}