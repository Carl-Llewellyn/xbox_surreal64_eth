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
/*#define DefaultGFXDll				"Jabo_Direct3D7.dll"
#define DefaultRSPDll				"RSP.dll"
#define DefaultAudioDll				"Jabo_Dsound.dll"
#define DefaultControllerDll		"Jabo_DInput.dll"*/

#pragma once

#define PLUGIN_TYPE_RSP				1
#define PLUGIN_TYPE_GFX				2
#define PLUGIN_TYPE_AUDIO			3
#define PLUGIN_TYPE_CONTROLLER		4

#define SYSTEM_NTSC					0
#define SYSTEM_PAL					1
#define SYSTEM_MPAL					2

typedef struct {
	WORD Version;        /* Should be set to 1 */
	WORD Type;           /* Set to PLUGIN_TYPE_GFX */
	char Name[100];      /* Name of the DLL */

	/* If DLL supports memory these memory options then set them to TRUE or FALSE
	   if it does not support it */
	BOOL NormalMemory;   /* a normal BYTE array */ 
	BOOL MemoryBswaped;  /* a normal BYTE array where the memory has been pre
	                          bswap on a dword (32 bits) boundry */
} PLUGIN_INFO;

typedef struct {
	HWND hWnd;			/* Render window */
	HWND hStatusBar;    /* if render window does not have a status bar then this is NULL */

	BOOL MemoryBswaped;    // If this is set to TRUE, then the memory has been pre
	                       //   bswap on a dword (32 bits) boundry 
						   //	eg. the first 8 bytes are stored like this:
	                       //        4 3 2 1   8 7 6 5

	BYTE * HEADER;	// This is the rom header (first 40h bytes of the rom
					// This will be in the same memory format as the rest of the memory.
	BYTE * RDRAM;
	BYTE * DMEM;
	BYTE * IMEM;

	DWORD * MI__INTR_REG;

	DWORD * DPC__START_REG;
	DWORD * DPC__END_REG;
	DWORD * DPC__CURRENT_REG;
	DWORD * DPC__STATUS_REG;
	DWORD * DPC__CLOCK_REG;
	DWORD * DPC__BUFBUSY_REG;
	DWORD * DPC__PIPEBUSY_REG;
	DWORD * DPC__TMEM_REG;

	DWORD * VI__STATUS_REG;
	DWORD * VI__ORIGIN_REG;
	DWORD * VI__WIDTH_REG;
	DWORD * VI__INTR_REG;
	DWORD * VI__V_CURRENT_LINE_REG;
	DWORD * VI__TIMING_REG;
	DWORD * VI__V_SYNC_REG;
	DWORD * VI__H_SYNC_REG;
	DWORD * VI__LEAP_REG;
	DWORD * VI__H_START_REG;
	DWORD * VI__V_START_REG;
	DWORD * VI__V_BURST_REG;
	DWORD * VI__X_SCALE_REG;
	DWORD * VI__Y_SCALE_REG;

	void ( __cdecl *CheckInterrupts)( void );
} GFX_INFO;

typedef struct {
	HINSTANCE hInst;
	BOOL MemoryBswaped;    /* If this is set to TRUE, then the memory has been pre
	                          bswap on a dword (32 bits) boundry */
	BYTE * RDRAM;
	BYTE * DMEM;
	BYTE * IMEM;

	DWORD * MI__INTR_REG;

	DWORD * SP__MEM_ADDR_REG;
	DWORD * SP__DRAM_ADDR_REG;
	DWORD * SP__RD_LEN_REG;
	DWORD * SP__WR_LEN_REG;
	DWORD * SP__STATUS_REG;
	DWORD * SP__DMA_FULL_REG;
	DWORD * SP__DMA_BUSY_REG;
	DWORD * SP__PC_REG;
	DWORD * SP__SEMAPHORE_REG;

	DWORD * DPC__START_REG;
	DWORD * DPC__END_REG;
	DWORD * DPC__CURRENT_REG;
	DWORD * DPC__STATUS_REG;
	DWORD * DPC__CLOCK_REG;
	DWORD * DPC__BUFBUSY_REG;
	DWORD * DPC__PIPEBUSY_REG;
	DWORD * DPC__TMEM_REG;

	void ( __cdecl *CheckInterrupts)( void );
	void ( *ProcessDlist)( void );
	void ( *ProcessAlist)( void );
	void ( *ProcessRdpList)( void );
} RSP_INFO_1_0;

typedef struct {
	BOOL Present;
	BOOL RawData;
	int  Plugin;
} CONTROL;

typedef struct {
	HWND hMainWindow;
	HINSTANCE hinst;

	BOOL MemoryBswaped;		// If this is set to TRUE, then the memory has been pre
							//   bswap on a dword (32 bits) boundry, only effects header. 
							//	eg. the first 8 bytes are stored like this:
							//        4 3 2 1   8 7 6 5
	BYTE * HEADER;			// This is the rom header (first 40h bytes of the rom)
	CONTROL *Controls;		// A pointer to an array of 4 controllers .. eg:
							// CONTROL Controls[4];
} CONTROL_INFO;

typedef struct {
	HINSTANCE hInst;
	BOOL MemoryBswaped;    /* If this is set to TRUE, then the memory has been pre
	                          bswap on a dword (32 bits) boundry */
	BYTE * RDRAM;
	BYTE * DMEM;
	BYTE * IMEM;

	DWORD * MI__INTR_REG;

	DWORD * SP__MEM_ADDR_REG;
	DWORD * SP__DRAM_ADDR_REG;
	DWORD * SP__RD_LEN_REG;
	DWORD * SP__WR_LEN_REG;
	DWORD * SP__STATUS_REG;
	DWORD * SP__DMA_FULL_REG;
	DWORD * SP__DMA_BUSY_REG;
	DWORD * SP__PC_REG;
	DWORD * SP__SEMAPHORE_REG;

	DWORD * DPC__START_REG;
	DWORD * DPC__END_REG;
	DWORD * DPC__CURRENT_REG;
	DWORD * DPC__STATUS_REG;
	DWORD * DPC__CLOCK_REG;
	DWORD * DPC__BUFBUSY_REG;
	DWORD * DPC__PIPEBUSY_REG;
	DWORD * DPC__TMEM_REG;

	void (  __cdecl *CheckInterrupts)( void );
	void ( *ProcessDlist)( void );
	void ( *ProcessAlist)( void );
	void ( *ProcessRdpList)( void );
	void ( *ShowCFB)( void );
} RSP_INFO_1_1;



  

typedef struct {
	HWND hwnd;
	HINSTANCE hinst;

	BOOL MemoryBswaped;    // If this is set to TRUE, then the memory has been pre
	                       //   bswap on a dword (32 bits) boundry 
						   //	eg. the first 8 bytes are stored like this:
	                       //        4 3 2 1   8 7 6 5
	BYTE * HEADER;	// This is the rom header (first 40h bytes of the rom
					// This will be in the same memory format as the rest of the memory.
	BYTE * RDRAM;
	BYTE * DMEM;
	BYTE * IMEM;

	DWORD * MI__INTR_REG;

	DWORD * AI__DRAM_ADDR_REG;
	DWORD * AI__LEN_REG;
	DWORD * AI__CONTROL_REG;
	DWORD * AI__STATUS_REG;
	DWORD * AI__DACRATE_REG;
	DWORD * AI__BITRATE_REG;

	void ( __cdecl *CheckInterrupts)( void );
} AUDIO_INFO;


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

/*** Conteroller plugin's ****/
#define PLUGIN_NONE					1
#define PLUGIN_MEMPAK				2
#define PLUGIN_RUMBLE_PAK			3 
#define PLUGIN_TANSFER_PAK			4 // not implemeted for non raw data
#define PLUGIN_RAW					5 // the controller plugin is passed in raw data

/******** All DLLs have this function **************/
void ( *GetDllInfo)             ( PLUGIN_INFO * PluginInfo );

/********** RSP DLL: Functions *********************/

void ( *RSPCloseDLL)        ( void );
void ( *RSPDllAbout)        ( HWND hWnd );
void ( *RSPDllConfig)       ( HWND hWnd );
void ( *RSPRomClosed)       ( void );
DWORD ( *DoRspCycles)       ( DWORD );
void ( *InitiateRSP_1_0)    ( RSP_INFO_1_0 Rsp_Info, DWORD * Cycles);
void ( *InitiateRSP_1_1)    ( RSP_INFO_1_1 Rsp_Info, DWORD * Cycles);
 

/********** GFX DLL: Functions *********************/
void ( *CaptureScreen)      ( char * );
void ( *ChangeWindow)       ( void );
void ( *GFXCloseDLL)        ( void );
void ( *GFXDllAbout)        ( HWND hParent );
void ( *GFXDllConfig)       ( HWND hParent );
void ( *GfxRomClosed)       ( void );
void ( *GfxRomOpen)         ( void );
void ( *DrawScreen)         ( void );
BOOL ( *InitiateGFX)        ( GFX_INFO Gfx_Info );
void ( *MoveScreen)         ( int xpos, int ypos );
void ( *ProcessDList)       ( void );
void ( *ProcessRDPList)     ( void );
void ( *ShowCFB)			   ( void );
void ( *UpdateScreen)       ( void );
void ( *ViStatusChanged)    ( void );
void ( *ViWidthChanged)     ( void );

/************ Audio DLL: Functions *****************/
void ( *AiCloseDLL)       ( void );
void ( *AiDacrateChanged) ( int SystemType );
void ( *AiLenChanged)     ( void );
void ( *AiDllAbout)       ( HWND hParent );
void ( *AiDllConfig)      ( HWND hParent );
void ( *AiDllTest)        ( HWND hParent );
DWORD ( *AiReadLength)    ( void );
void ( *AiRomClosed)      ( void );
void ( *AiUpdate)         ( BOOL Wait );
BOOL ( *InitiateAudio)    ( AUDIO_INFO Audio_Info );
void ( *ProcessAList)     ( void );

/********** Controller DLL: Functions **************/
void ( *ContCloseDLL)     ( void );
void ( *ControllerCommand)( int Control, BYTE * Command );
void ( *ContDllAbout)     ( HWND hParent );
void ( *ContConfig)       ( HWND hParent );
void ( *InitiateControllers_1_0)( HWND hMainWindow, CONTROL Controls[4] );
void ( *InitiateControllers_1_1)( CONTROL_INFO ControlInfo );
void ( *GetKeys)          ( int Control, BUTTONS * Keys );
void ( *ReadController)   ( int Control, BYTE * Command );
void ( *ContRomOpen)      ( void );
void ( *ContRomClosed)    ( void );
void ( *WM_KeyDown)       ( WPARAM wParam, LPARAM lParam );
void ( *WM_KeyUp)         ( WPARAM wParam, LPARAM lParam );


/********** Plugin: Functions *********************/
void GetPluginDir        ( char * Directory );
void GetSnapShotDir      ( char * Directory );
void PluginConfiguration ( HWND hWnd );
void SetupPlugins        ( HWND hWnd );
void SetupPluginScreen   ( HWND hDlg );
void ShutdownPlugins     ( void );

/********** External Global Variables ***************/
//#define MaxDlls	100
//extern char RspDLL[100], GfxDLL[100], AudioDLL[100],ControllerDLL[100], * PluginNames[MaxDlls];
extern DWORD PluginCount, RspTaskValue, AudioIntrReg;
extern CONTROL Controllers[4];
extern WORD RSPVersion;


// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
//extern BOOL g_bUseLLERspPlugin; // not used anymore
//extern BOOL g_bUseBasicAudio; // not used anymore
extern BOOL g_bUseRspAudio; // control a listing

extern int g_iRspPlugin;
extern int g_iAudioPlugin;
extern BOOL g_bAudioBoost;
