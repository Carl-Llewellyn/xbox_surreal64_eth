/*
 * Project 64 - A Nintendo 64 emulator.
 *
 * (c) Copyright 2001 zilmar (zilmar@emulation64.com) and 
 * Jabo (jabo@emulation64.com).
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
#include "main.h"
#include "cpu.h"
#include "memory.h"
#include "debugger.h"
#include "plugin.h"
#include "unzip.h"
#include "resource.h"
#include "stubs.h"
//#include "../LaunchData.h"	// Ez0n3 - launchdata isn't used anymore
#include "Plugin.h"
#include "IOSupport.h"

#include "../Plugins.h"

int InitalizeApplication ( HINSTANCE hInstance );
//weinerschnitzel - determine memory size for rompaging method
//extern int RAM_IS_128;
//extern BOOL PhysRam128(); //May need this here for clean paging
// Ez0n3 - reinstate max video mem until freakdave finishes this
extern void _VIDEO_SetMaxTextureMem(DWORD mem);


extern void _INPUT_LoadButtonMap(int *cfgData); 

LARGE_INTEGER Frequency, Frames[NoOfFrames], LastFrame;
BOOL HaveDebugger, AutoLoadMapFile, ShowUnhandledMemory, ShowTLBMisses, 
	ShowDListAListCount, ShowCompMem, Profiling, IndvidualBlock, AutoStart, 
	AutoSleep, DisableRegCaching, UseIni, UseTlb, UseLinking, RomBrowser,
	IgnoreMove, Rercursion, ShowPifRamErrors, LimitFPS, ShowCPUPer, AutoZip, 
	AutoFullScreen, SystemABL;
DWORD CurrentFrame, CPU_Type, SystemCPU_Type, SelfModCheck, SystemSelfModCheck, 
	RomsToRemember, RomDirsToRemember;
HWND hMainWindow, hHiddenWin, hStatusWnd;
char CurrentSave[256];
HMENU hMainMenu;
HINSTANCE hInst;


#define MenuLocOfUsedFiles	9
#define MenuLocOfUsedDirs	(MenuLocOfUsedFiles + 1)

DWORD RomFileSize, RomRamSize, RomSaveUsing, RomCPUType, RomSelfMod, 
	RomUseTlb, RomUseLinking, RomCF, RomUseLargeBuffer, RomUseCache,
	RomReadRomDirect;
char CurrentFileName[MAX_PATH+1], RomName[MAX_PATH+1], RomHeader[64];
//char LastRoms[10][MAX_PATH+1], LastDirs[10][MAX_PATH+1];

BOOL IsValidRomImage ( BYTE Test[4] );
void Enable128MegCaching( void );

int CurrentPercent;
 
void ByteSwapRom (void) {
	DWORD count;

	//SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"Byte swapping image" );
	switch (*((DWORD *)&ROM[0])) {
	case 0x12408037:
		for( count = 0 ; count < RomFileSize; count += 4 ) {
			ROM[count] ^= ROM[count+2];
			ROM[count + 2] ^= ROM[count];
			ROM[count] ^= ROM[count+2];			
			ROM[count + 1] ^= ROM[count + 3];
			ROM[count + 3] ^= ROM[count + 1];
			ROM[count + 1] ^= ROM[count + 3];			
		}
		break;
	case 0x40123780:
		for( count = 0 ; count < RomFileSize; count += 4 ) {
			ROM[count] ^= ROM[count+3];
			ROM[count + 3] ^= ROM[count];
			ROM[count] ^= ROM[count+3];			
			ROM[count + 1] ^= ROM[count + 2];
			ROM[count + 2] ^= ROM[count + 1];
			ROM[count + 1] ^= ROM[count + 2];			
		}
		break;
	case 0x80371240: break;
	default:
		break;
		//DisplayError("ByteSwapRom: %X",ROM[0]);
	}
}

int ChooseN64RomToOpen ( void ) {
 
	return TRUE;
}

void EnableOpenMenuItems (void) {
 
}

void GetRomDirectory ( char * Directory ) {
 
}

BOOL IsValidRomImage ( BYTE Test[4] ) {
	if ( *((DWORD *)&Test[0]) == 0x40123780 ) { return TRUE; }
	if ( *((DWORD *)&Test[0]) == 0x12408037 ) { return TRUE; }
	if ( *((DWORD *)&Test[0]) == 0x80371240 ) { return TRUE; }
	return FALSE;
}

BOOL LoadDataFromRomFile(char * FileName,BYTE * Data,int DataLen, int * RomSize) {
	BYTE Test[4];
	int count;

	if (strnicmp(&FileName[strlen(FileName)-4], ".ZIP",4) == 0 ){ 
		int len, port = 0, FoundRom;
	    unz_file_info info;
		char zname[132];
		unzFile file;
		file = unzOpen(FileName);
		if (file == NULL) { return FALSE; }

		port = unzGoToFirstFile(file);
		FoundRom = FALSE; 
		while(port == UNZ_OK && FoundRom == FALSE) {
			unzGetCurrentFileInfo(file, &info, zname, 128, NULL,0, NULL,0);
		    if (unzLocateFile(file, zname, 1) != UNZ_OK ) {
				unzClose(file);
				return FALSE;
			}
			if( unzOpenCurrentFile(file) != UNZ_OK ) {
				unzClose(file);
				return FALSE;
			}
			unzReadCurrentFile(file,Test,4);
			if (IsValidRomImage(Test)) {
				FoundRom = TRUE;
				RomFileSize = info.uncompressed_size;
				memcpy(Data,Test,4);
				len = unzReadCurrentFile(file,&Data[4],DataLen - 4) + 4;

				if ((int)DataLen != len) {
					unzCloseCurrentFile(file);
					unzClose(file);
					return FALSE;
				}
				*RomSize = info.uncompressed_size;
				if(unzCloseCurrentFile(file) == UNZ_CRCERROR) {
					unzClose(file);
					return FALSE;
				}
				unzClose(file);
			}
			if (FoundRom == FALSE) {
				unzCloseCurrentFile(file);
				port = unzGoToNextFile(file);
			}

		}
		if (FoundRom == FALSE) {
			return FALSE;
		}
	} else {
		DWORD dwRead;
		HANDLE hFile;
		
		hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,
			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);
		
		if (hFile == INVALID_HANDLE_VALUE) {  return FALSE; }

		SetFilePointer(hFile,0,0,FILE_BEGIN);
		ReadFile(hFile,Test,4,&dwRead,NULL);
		if (!IsValidRomImage(Test)) { CloseHandle( hFile ); return FALSE; }
		SetFilePointer(hFile,0,0,FILE_BEGIN);
		if (!ReadFile(hFile,Data,DataLen,&dwRead,NULL)) { CloseHandle( hFile ); return FALSE; }
		*RomSize = GetFileSize(hFile,NULL);
		CloseHandle( hFile ); 		
	}	

	switch (*((DWORD *)&Data[0])) {
	case 0x12408037:
		for( count = 0 ; count < DataLen; count += 4 ) {
			Data[count] ^= Data[count+2];
			Data[count + 2] ^= Data[count];
			Data[count] ^= Data[count+2];			
			Data[count + 1] ^= Data[count + 3];
			Data[count + 3] ^= Data[count + 1];
			Data[count + 1] ^= Data[count + 3];			
		}
		break;
	case 0x40123780:
		for( count = 0 ; count < DataLen; count += 4 ) {
			Data[count] ^= Data[count+3];
			Data[count + 3] ^= Data[count];
			Data[count] ^= Data[count+3];			
			Data[count + 1] ^= Data[count + 2];
			Data[count + 2] ^= Data[count + 1];
			Data[count + 1] ^= Data[count + 2];			
		}
		break;
	case 0x80371240: break;
	}
	return TRUE;
}

void LoadRecentDirs (HWND hParent) {
	 
}

void LoadRecentFiles (HWND hParent) {
	 
}

void LoadRecentRom (DWORD Index) {
	 
}


void LoadRomOptions ( void ) {
	DWORD NewRamSize;

	ReadRomOptions();

	if (RomUseLargeBuffer)
		RomUseLargeBuffer = 0;

	NewRamSize = RomRamSize;
	if ((int)RomRamSize < 0) { NewRamSize = SystemRdramSize; }

	if (RomReadRomDirect) {
		if(VirtualAlloc(N64MEM + 0x10000000, RomFileSize, MEM_COMMIT, PAGE_READWRITE)==NULL) {
			//DisplayError("Not enough memory for N64MEM!");
			ExitThread(0);
		}
		memcpy(N64MEM + 0x10000000,ROM,RomFileSize);
		if (VirtualAlloc((BYTE *)XBOXDelaySlotTable + (0x10000000 >> 0xA), (RomFileSize >> 0xA), MEM_COMMIT, PAGE_READWRITE)==NULL) {
			//DisplayError("Failed to Extend Delay Slot Table to 8mb");
			ExitThread(0);
		}
	}

	if (RomUseLargeBuffer) {
		if (VirtualAlloc(XBOXRecompCode, LargeCompileBufferSize, MEM_COMMIT, PAGE_READWRITE)==NULL) {
			//DisplayError("Failed to allocate for Recomp Buffer");
			ExitThread(0);
		}
	} else {
		VirtualFree(XBOXRecompCode, LargeCompileBufferSize,MEM_DECOMMIT);
		if (VirtualAlloc(XBOXRecompCode, g_dwNormalCompileBufferSize, MEM_COMMIT, PAGE_READWRITE)==NULL) {
			//DisplayError("Failed to Recomp Buffer");
			ExitThread(0);
		}
	}
	if (NewRamSize != RdramSize) {
		if (RdramSize == 0x400000) { 
			if (VirtualAlloc(N64MEM + 0x400000, 0x400000, MEM_COMMIT, PAGE_READWRITE)==NULL) {
				//DisplayError("Failed to Extend memory to 8mb");
				ExitThread(0);
			}
			if (VirtualAlloc((BYTE *)XBOXJumpTable + 0x400000, 0x400000, MEM_COMMIT, PAGE_READWRITE)==NULL) {
				//DisplayError("Failed to Extend Jump Table to 8mb");
				ExitThread(0);
			}
			if (VirtualAlloc((BYTE *)XBOXDelaySlotTable + (0x400000 >> 0xA), (0x400000 >> 0xA), MEM_COMMIT, PAGE_READWRITE)==NULL) {
				//DisplayError("Failed to Extend Delay Slot Table to 8mb");
				ExitThread(0);
			}
		} else {
			VirtualFree(N64MEM + 0x400000, 0x400000,MEM_DECOMMIT);
			VirtualFree((BYTE *)XBOXJumpTable + 0x400000, 0x400000,MEM_DECOMMIT);
			VirtualFree((BYTE *)XBOXDelaySlotTable + (0x400000 >> 0xA), (0x400000 >> 0xA),MEM_DECOMMIT);
		}
	}
	RdramSize = NewRamSize;
	CPU_Type = SystemCPU_Type;
	if (RomCPUType != CPU_Default) { CPU_Type = RomCPUType; }
	CountPerOp = RomCF;
	if (CountPerOp < 1)  { CountPerOp = Default_CountPerOp; }
	if (CountPerOp > 6)  { CountPerOp = Default_CountPerOp; }
	
	SaveUsing = RomSaveUsing;
	SelfModCheck = SystemSelfModCheck;
	if (RomSelfMod != ModCode_Default) { SelfModCheck = RomSelfMod; }
	UseTlb = RomUseTlb;
	UseLinking = SystemABL;
	DisableRegCaching = !RomUseCache;
	if (UseIni && RomUseLinking == 0 ) { UseLinking = TRUE; }
	if (UseIni && RomUseLinking == 1 ) { UseLinking = FALSE; }
	//switch (*(ROM + 0x3D)) {
	switch (*(tmpBuf + 0x3D)) {
	case 0x44: //Germany
	case 0x46: //french
	case 0x49: //Italian
	case 0x50: //Europe
	case 0x53: //Spanish
	case 0x55: //Australia
	case 0x58: // X (PAL)
	case 0x59: // X (PAL)
		Timer_Initialize((double)50);
		break;
	case 0x37: // 7 (Beta)
	case 0x41: // A (NTSC)
	case 0x45: //USA
	case 0x4A: //Japan
	default:
		Timer_Initialize((double)60);
		break;
	}
}

void RemoveRecentDirList (HWND hWnd) {
 
}

void RemoveRecentList (HWND hWnd) {
 
}

void ReadRomOptions (void) {
	RomRamSize        = -1;
	RomSaveUsing      = Auto;
	RomCF             = -1;
	RomCPUType        = CPU_Recompiler;
	RomSelfMod        = ModCode_Default;
	RomUseTlb         = TRUE;
	RomUseCache       = TRUE;
	RomUseLargeBuffer = FALSE;
	RomReadRomDirect  = FALSE;
	RomUseLinking     = -1;
	//NormalCompileBufferSize = 0x01200000;

	UseIni = 1;
	RomRamSize = 4;
	 
	if (strlen(RomName) != 0) {
		char Identifier[100];
		LPSTR IniFileName;
		char String[100];

		IniFileName = GetIniFileName();
	
		sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(tmpBuf + 0x10),*(DWORD *)(tmpBuf + 0x14),*(tmpBuf + 0x3D));
	
		if (UseIni) { RomRamSize = _GetPrivateProfileInt(Identifier,"RDRAM Size",-1,IniFileName); }
		if (RomRamSize == 4 || RomRamSize == 8) { 
			RomRamSize *= 0x100000; 
		} else {
			RomRamSize = -1; 
		}

		RomCF = _GetPrivateProfileInt(Identifier,"Counter Factor",-1,IniFileName);
		if (RomCF > 6) { RomCF = -1; }
		

		_GetPrivateProfileString(Identifier,"Save Type","",String,sizeof(String),IniFileName);
		if (strcmp(String,"4kbit Eeprom") == 0)       { RomSaveUsing = Eeprom_4K; } 
		else if (strcmp(String,"16kbit Eeprom") == 0) { RomSaveUsing = Eeprom_16K; } 
		else if (strcmp(String,"Sram") == 0)          { RomSaveUsing = Sram; } 
		else if (strcmp(String,"FlashRam") == 0)      { RomSaveUsing = FlashRam; } 
		else                                          { RomSaveUsing = Auto; } 

		if (UseIni) {
			_GetPrivateProfileString(Identifier,"CPU Type","",String,sizeof(String),IniFileName);
			if (strcmp(String,"Interpreter") == 0)       { RomCPUType = CPU_Interpreter; } 
			else if (strcmp(String,"Recompiler") == 0)   { RomCPUType = CPU_Recompiler; } 
			else if (strcmp(String,"SyncCores") == 0)    { RomCPUType = CPU_SyncCores; } 
			else                                         { RomCPUType = CPU_Default; } 

			_GetPrivateProfileString(Identifier,"Self-modifying code Method","",String,sizeof(String),IniFileName);
			if (strcmp(String,"None") == 0)                      { RomSelfMod = ModCode_None; } 
			else if (strcmp(String,"Cache") == 0)                { RomSelfMod = ModCode_Cache; } 
			else if (strcmp(String,"Protected Memory") == 0)     { RomSelfMod = ModCode_ProtectedMemory; } 
			else if (strcmp(String,"Check Memory") == 0)         { RomSelfMod = ModCode_CheckMemoryCache; } 
			else if (strcmp(String,"Check Memory & cache") == 0) { RomSelfMod = ModCode_CheckMemoryCache; } 
			else if (strcmp(String,"Change Memory") == 0)        { RomSelfMod = ModCode_ChangeMemory; } 
			else                                                 { RomSelfMod = ModCode_Default; } 
		}
		_GetPrivateProfileString(Identifier,"Use TLB","",String,sizeof(String),IniFileName);
		if (strcmp(String,"No") == 0) { RomUseTlb = FALSE; } 
		_GetPrivateProfileString(Identifier,"Reg Cache","",String,sizeof(String),IniFileName);
		if (strcmp(String,"No") == 0) { RomUseCache = FALSE; } 
		_GetPrivateProfileString(Identifier,"Use Large Buffer","",String,sizeof(String),IniFileName);
		if (strcmp(String,"Yes") == 0) { RomUseLargeBuffer = TRUE; } 
		_GetPrivateProfileString(Identifier,"Linking","",String,sizeof(String),IniFileName);
		if (strcmp(String,"On") == 0) { RomUseLinking = 0; } 
		if (strcmp(String,"Off") == 0) { RomUseLinking = 1; }
	}
 
	 
	 
}

void OpenN64Image ( void ) {
	DWORD ThreadID;
	 
	//hMenu = GetMenu(hMainWindow);
	//EnableMenuItem(hMenu,ID_FILE_OPEN_ROM,MFS_DISABLED|MF_BYCOMMAND);
	//SendMessage( hStatusWnd, SB_SETTEXT, 0, (LPARAM)"Choosing N64 image" );
	if (ChooseN64RomToOpen()) {
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)OpenChosenFile,NULL,0, &ThreadID);	
	} else {
		EnableOpenMenuItems();
	}
}

void OpenChosenFile ( void ) {
#define ReadFromRomSection	0x400000
	char FileName[_MAX_DIR]; //drive[_MAX_DRIVE] ,FileName[_MAX_DIR],dir[_MAX_DIR], ext[_MAX_EXT];
	char WinTitle[70], MapFile[_MAX_PATH];
	char Message[100];
	BYTE Test[4];
	HANDLE outputZipFile = NULL;
	DWORD dwBytesWritten = 0;
	int count;
	int iPos = 1;
	int iPos2 = 0;

	Sleep(1000);
	CloseCpu();
#if (!defined(EXTERNAL_RELEASE))
	ResetMappings();
#endif
	strcpy(MapFile,CurrentFileName);

//	InitMessageFont();

//	OutputTextToScreen("Sureal64 V0.01 Alpha",60, 20);
//	OutputTextToScreen("====================",60, 40);
	
	//ChangePowerLEDColor(0xf0);
		
	if (strnicmp(&CurrentFileName[strlen(CurrentFileName)-4], ".ZIP",4) == 0 ){ 
		int len, port = 0, FoundRom;
	    unz_file_info info;
		char zname[132];
		unzFile file;
		file = unzOpen(CurrentFileName);
		if (file == NULL) {
			return;
		}

		port = unzGoToFirstFile(file);
		FoundRom = FALSE; 
		while(port == UNZ_OK && FoundRom == FALSE) {
			unzGetCurrentFileInfo(file, &info, zname, 128, NULL,0, NULL,0);
		    if (unzLocateFile(file, zname, 1) != UNZ_OK ) {
				unzClose(file);
//				OutputTextToScreen("Zip Error: unzLocateFile() failed\n", 60, 60);
				EnableOpenMenuItems();			
				return;
			}
			if( unzOpenCurrentFile(file) != UNZ_OK ) {
				unzClose(file);
				//DisplayError("Error in zip file\n");
				EnableOpenMenuItems();			
				//ShowRomList(hMainWindow);
				return;
			}
			unzReadCurrentFile(file,Test,4);
			if (IsValidRomImage(Test)) {
				FoundRom = TRUE;
				RomFileSize = info.uncompressed_size;
				/*if (!Allocate_ROM()) {
					unzCloseCurrentFile(file);
					unzClose(file);
					//DisplayError("Not enough memory for rom");
					EnableOpenMenuItems();			
					//ShowRomList(hMainWindow);
					return;
				}
				memcpy(ROM,Test,4); */
				//len = unzReadCurrentFile(file,&ROM[4],RomFileSize - 4) + 4;

				strcpy(FileName,"T:\\rom.v64");

				outputZipFile = CreateFile(FileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,
									CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
									NULL);
				
				ROM = (BYTE *)malloc(0x10);
				memcpy(ROM,Test,4);
				WriteFile(outputZipFile, ROM, 4, &dwBytesWritten, NULL);
				free(ROM);

				len = 4;
				for (count = 4; count < (int)RomFileSize; count += ReadFromRomSection) {
					ROM = (BYTE *)malloc(ReadFromRomSection);
					len += unzReadCurrentFile(file,ROM,ReadFromRomSection);
					sprintf(Message,"Loaded: %.2f%c...",((float)len/(float)RomFileSize) * 100.0f,'%');
					iPos2 = (40) + (20*iPos);
//					OutputTextToScreen(Message, 60, iPos2);
					if (outputZipFile) {
						WriteFile(outputZipFile, ROM, ReadFromRomSection, &dwBytesWritten, NULL);						 
					}
					
					iPos++;

					free(ROM);
				}
				
				CloseHandle(outputZipFile);

				strcpy(CurrentFileName, FileName);
				if ((int)RomFileSize != len) {
					unzCloseCurrentFile(file);
					unzClose(file);
					switch (len) {
					case UNZ_ERRNO:
						//DisplayError("Errno:...");
						break;
					case UNZ_EOF:
						//DisplayError("Unexpected end of file.");
						break;
					case UNZ_PARAMERROR:
						//DisplayError("Parameter error.");
						break;
					case UNZ_BADZIPFILE:
						//DisplayError("Bad zipfile.");
						break;
					case UNZ_INTERNALERROR:
						//DisplayError("Internal error.");
						break;
					case UNZ_CRCERROR:
						//DisplayError("CRC error.");
						break;
					}
					EnableOpenMenuItems();			
					//ShowRomList(hMainWindow);
					return;
				}
				if(unzCloseCurrentFile(file) == UNZ_CRCERROR) {
					unzClose(file);
					//DisplayError("CRC error in zip file.\n");
					//EnableOpenMenuItems();			
					//ShowRomList(hMainWindow);
					return;
				}
				//AddRecentFile(hMainWindow,CurrentFileName);
				//_splitpath( CurrentFileName, drive, dir, FileName, ext );
				unzClose(file);
			}
			if (FoundRom == FALSE) {
				unzCloseCurrentFile(file);
				port = unzGoToNextFile(file);
			}
		}
		if (FoundRom == FALSE) {
		    //DisplayError("Zip Error: file contains no n64 image\n");
		    unzClose(file);
			EnableOpenMenuItems();			 
			return;
		}
#if (!defined(EXTERNAL_RELEASE))
		if (AutoLoadMapFile) {
			OpenZipMapFile(MapFile);
		}
#endif
	} else {
 
	}
	
	if (RomBrowser) { SetupPlugins(hMainWindow); }

	iPos2 += 20;
//	OutputTextToScreen("Building Pagefile, Please Wait.....", 60, iPos2);
	InitVirtualRomData(CurrentFileName);
	LoadVirtualRomData();
	DeleteFile(FileName);			// Delete our temp file
	InitPageAndFrameTables();

	sprintf(WinTitle,"Loaded - [ %s ]",CurrentFileName);
 
	//ChangePowerLEDColor(0x0f);

	if (AutoStart) { StartEmulation(); }
}

void SaveRecentDirs (void) {
	 
}

void SaveRecentFiles (void) {
	 
}

void SaveRomOptions (void) {

	/*
	char Identifier[100];
	LPSTR IniFileName;
	char String[100];

	if (strlen(RomName) == 0) { return; }

	IniFileName = GetIniFileName();
	sprintf(Identifier,"%08X-%08X-C:%X",*(DWORD *)(ROM + 0x10),*(DWORD *)(ROM + 0x14),*(ROM + 0x3D));
	_WritePrivateProfileString(Identifier,"Internal Name",RomName,IniFileName);

	switch (RomRamSize) {
	case 0x400000: strcpy(String,"4"); break;
	case 0x800000: strcpy(String,"8"); break;
	default: strcpy(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"RDRAM Size",String,GetIniFileName());

	switch (RomCF) {
	case 1: case 2: case 3: case 4: case 5: case 6: sprintf(String,"%d",RomCF); break;
	default: sprintf(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"Counter Factor",String,GetIniFileName());

	switch (RomSaveUsing) {
	case Eeprom_4K: sprintf(String,"4kbit Eeprom"); break;
	case Eeprom_16K: sprintf(String,"16kbit Eeprom"); break;
	case Sram: sprintf(String,"Sram"); break;
	case FlashRam: sprintf(String,"FlashRam"); break;
	default: sprintf(String,"First Save Type"); break;
	}
	_WritePrivateProfileString(Identifier,"Save Type",String,GetIniFileName());

	switch (RomCPUType) {
	case CPU_Interpreter: sprintf(String,"Interpreter"); break;
	case CPU_Recompiler: sprintf(String,"Recompiler"); break;
	case CPU_SyncCores: sprintf(String,"SyncCores"); break;
	default: sprintf(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"CPU Type",String,GetIniFileName());
	
	switch (RomSelfMod) {
	case ModCode_None: sprintf(String,"None"); break;
	case ModCode_Cache: sprintf(String,"Cache"); break;
	case ModCode_ProtectedMemory: sprintf(String,"Protected Memory"); break;
	case ModCode_CheckMemoryCache: sprintf(String,"Check Memory & cache"); break;
	case ModCode_ChangeMemory: sprintf(String,"Change Memory"); break;
	default: sprintf(String,"Default"); break;
	}
	_WritePrivateProfileString(Identifier,"Self-modifying code Method",String,GetIniFileName());

	_WritePrivateProfileString(Identifier,"Reg Cache",RomUseCache?"Yes":"No",GetIniFileName());
	_WritePrivateProfileString(Identifier,"Use TLB",RomUseTlb?"Yes":"No",GetIniFileName());
	_WritePrivateProfileString(Identifier,"Use Large Buffer",RomUseLargeBuffer?"Yes":"No",GetIniFileName());
	_WritePrivateProfileString(Identifier,"Linking","Global",GetIniFileName()); 
	if (RomUseLinking == 0) { _WritePrivateProfileString(Identifier,"Linking","On",GetIniFileName()); }
	if (RomUseLinking == 1) { _WritePrivateProfileString(Identifier,"Linking","Off",GetIniFileName()); } */
}

void SetRecentRomDir (DWORD Index) {
	 
}

void SetRomDirectory ( char * Directory, BOOL IgnoreDefaultDir ) {
	 
}

//extern BOOL PathFileExists(const char *pszPath);
BOOL PathFileExists(const char *pszPath)
{   
    return GetFileAttributes(pszPath) != INVALID_FILE_ATTRIBUTES;   
}

char * GetIniFileName(void) {
	//return "D:\\Project64.rdb";
	
	if(PathFileExists("T:\\Project64.rdb")) {
		OutputDebugString("T:\\Project64.rdb Loading!\n");
		return "T:\\Project64.rdb";
	}
	else {
		OutputDebugString("T:\\Project64.rdb Failed to Load!\n");
		
		if(PathFileExists("D:\\Project64.rdb"))
			OutputDebugString("D:\\Project64.rdb Loading!\n");
		else
			OutputDebugString("D:\\Project64.rdb Failed to Load!\n");
			
		return "D:\\Project64.rdb";
	}
}


void SetCurrentSaveState (HWND hWnd, int State) {
	 
}


void DisplayFPS (void) {
 
}

void StopTimer (BOOL ContinueOld) {
	 
}


void ResetTimerList (void) {
 
}

void StartTimer (char * Label) {
	 
}

 
void GenerateTimerResults (void) {
	 
}

void DisplayCPUPer (void) {
 
	return;	
}

void LoadCheats (void) {

}

// Ez0n3 - launchdata isn't used anymore
/*
char *GetCommandLine( void )
{
		DWORD error=0;
		DWORD  pdwLaunchDataType;
		static LAUNCH_DATA  pLaunchData;

		error = XGetLaunchInfo( &pdwLaunchDataType,&pLaunchData);
		if( error == ERROR_NOT_FOUND )
			return 0;

		if( error == ERROR_SUCCESS )
		{
			switch(pdwLaunchDataType)
			{
			case LDT_FROM_DASHBOARD:
				return (char*)&pLaunchData;
			break;
			case LDT_FROM_DEBUGGER_CMDLINE:
				return (char*)&pLaunchData;
			break;
			case LDT_TITLE:
				return (char*)&pLaunchData;
			break;
			default:
				return (char*)&pLaunchData;
			break;
			}
		}
		else
			return 0;
}
*/

int taille;
extern void loadinis();
extern int loaddwPJ64PagingMem();
extern int loaddwPJ64DynaMem();

// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
//extern int loadbUseLLERSP(); // not used anymore
extern int loadbUseRspAudio(); // control a listing
extern int loadiRspPlugin();
extern int loadiAudioPlugin();

// Ez0n3 - reinstate max video mem
extern int loaddwMaxVideoMem();

extern int loadiPagingMethod();
extern int loadbAudioBoost();

char g_szPathSaves[256] = "D:\\Saves\\";
extern void GetPathSaves(char *pszPathSaves);


char emuname[256];

VOID __cdecl main()
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
		OutputDebugString("Z:\\TemporaryRom.dat File Found!\n");
		strcpy(g_temporaryRomPath, "Z:\\TemporaryRom.dat");
	}
	else {
		OutputDebugString("Z:\\TemporaryRom.dat File Not Found!\n");
		
		// if debugging, a temp rom can be placed in T to skip the launcher
		if (PathFileExists("T:\\Data\\TemporaryRom.dat")) {
			OutputDebugString("T:\\Data\\TemporaryRom.dat File Found!\n");
			strcpy(g_temporaryRomPath, "T:\\Data\\TemporaryRom.dat");
		}
		else {
			OutputDebugString("T:\\Data\\TemporaryRom.dat File Not Found!\n");
			Sleep(100);
			XLaunchNewImage("D:\\default.xbe", NULL);
		}
	}
	
	loadinis();

	//freakdave - check for 128mb
	//PhysRam128();

    sprintf(emuname,"Project64x");
	
	GetPathSaves(g_szPathSaves);

	g_dwNormalCompileBufferSize = loaddwPJ64DynaMem() * 1024 * 1024;
	
	
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

	
	InitalizeApplication(NULL);

//	strcpy(CurrentFileName, pLd->szFilename);

	OpenChosenFile();
	
	{ // create the save directory if it doesn't exist
		char szPathSaves[_MAX_PATH];
		sprintf(szPathSaves, "%s%08x", g_szPathSaves, *(DWORD *)(tmpBuf + 0x10));
		if (!PathFileExists(szPathSaves)) {
			if (!CreateDirectory(szPathSaves, NULL)) {
				OutputDebugString(szPathSaves);
				OutputDebugStringA(" Could Not Be Created!\n");
			}
		}
	}
	
	switch (CPU_Type) {
	case CPU_Interpreter:  
		StartInterpreterCPU();
		break;
	case CPU_Recompiler:
		StartRecompilerCPU();
		break;

	} 
}


int InitalizeApplication ( HINSTANCE hInstance ) {
	 
	 
	if (!Allocate_Memory()) { 
		 
		return FALSE; 
	}

	//hPauseMutex = CreateMutex(NULL,FALSE,NULL);
#if (!defined(EXTERNAL_RELEASE))
	HaveDebugger = FALSE;
	sprintf(String,"Software\\N64 Emulation\\%s",AppName);
	lResult = RegOpenKeyEx( HKEY_CURRENT_USER,String,0, KEY_ALL_ACCESS,&hKeyResults);	
	if (lResult == ERROR_SUCCESS) {
		DWORD Type, Value, Bytes = 4;

		lResult = RegQueryValueEx(hKeyResults,"Debugger",0,&Type,(LPBYTE)(&Value),&Bytes);
		if (Type == REG_DWORD && lResult == ERROR_SUCCESS) { 
			if (Value == 0x9348ae97) {
				HaveDebugger = TRUE;
			}
		}
	}
#endif
	
	IgnoreMove = FALSE;
	CPU_Type = Default_CPU;
	SystemCPU_Type = Default_CPU;
	SystemSelfModCheck = Default_SelfModCheck;
	SystemRdramSize = Default_RdramSize;
	SystemABL = Default_AdvancedBlockLink;
	AutoStart = Default_AutoStart;
	AutoSleep = Default_AutoSleep;
	DisableRegCaching = Default_DisableRegCaching;
	UseIni = Default_UseIni;
	AutoZip = Default_AutoZip;
	AutoFullScreen = FALSE;
	RomsToRemember = Default_RomsToRemember;
	RomDirsToRemember = Default_RomsDirsToRemember;
	AutoLoadMapFile = Default_AutoMap;
	ShowUnhandledMemory = Default_ShowUnhandledMemory;
	ShowCPUPer = Default_ShowCPUPer;
	LimitFPS = Default_LimitFPS;
	ShowTLBMisses = Default_ShowTLBMisses;
	Profiling = Default_ProfilingOn;
	IndvidualBlock = Default_IndvidualBlock;
	RomBrowser = Default_UseRB;
	Rercursion = Default_Rercursion;

	
	SetupRegisters(&Registers);
	QueryPerformanceFrequency(&Frequency);
#if (!defined(EXTERNAL_RELEASE))
	LoadLogOptions(&LogOptions, FALSE);
	StartLog();
#endif
	return TRUE;
}

void READMSRREG( UINT32 reg, LARGE_INTEGER *val ) 
{
  UINT32 lowPart, highPart;
  _asm {
    mov   ecx, reg
    rdmsr
    mov   lowPart, eax
    mov   highPart, edx
  };

  val->LowPart = lowPart;
  val->HighPart = highPart;
}

	//-------------------------------------------------------------------
  // WRITEMSRREG64
  //! \brief    Write to a 64 bit MSR register
	//-------------------------------------------------------------------
void WRITEMSRREG( UINT32 reg, LARGE_INTEGER val ) 
{
  _asm {
    mov   ecx, reg
    mov   eax, val.LowPart
    mov   edx, val.HighPart
    wrmsr
  };
}


void Enable128MegCaching( void )
{
  LARGE_INTEGER regVal;
  long filesize;
  FILE *fp;

    // Verify that we have 128 megs available
  MEMORYSTATUS memStatus;
  GlobalMemoryStatus( &memStatus );
  if( memStatus.dwTotalPhys < (100 * 1024 * 1024) ) {
  	g_dwNumFrames = ((loaddwPJ64PagingMem() * 1024 * 1024) / g_dwPageSize);
	return;
  }

    // Grab the existing default type
  READMSRREG( IA32_MTRR_DEF_TYPE, &regVal );
  
    // Set the default to WriteBack (0x06)
  regVal.LowPart = (regVal.LowPart & ~0xFF) | 0x06;
  WRITEMSRREG( IA32_MTRR_DEF_TYPE, regVal );

  fp = fopen(g_temporaryRomPath,"r");
  //fp = fopen("T:\\Data\\TemporaryRom.dat","r");
  rewind(fp);
  fseek(fp, 0, SEEK_END);
  filesize = ftell(fp);
  fclose(fp);

  g_dwNumFrames = (DWORD)((filesize) / g_dwPageSize);
}

void DisplayError (char * Message, ...)
{
//#ifdef DEBUG
	char Msg[400];
	va_list ap;

	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );
	
	OutputDebugString(Msg);
/*#else
	OutputDebugString(Message);
#endif*/
	OutputDebugString("\n");
}
