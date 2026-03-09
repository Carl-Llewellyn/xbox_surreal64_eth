/*
Copyright (C) 2002 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "stdafx.h"
#include "_BldNum.h"
#include "TextureCache.h"
//#include "./Menu/MenuMain.h"

BOOL g_bTempMessage = FALSE;
DWORD g_dwTempMessageStart = 0;
char g_szTempMessage[100];

PluginStatus status;

//HINSTANCE myhInst = NULL;

IniFile *g_pIniFile = NULL;
GFX_INFO g_GraphicsInfo;
DWORD dwTvSystem = TV_SYSTEM_NTSC;
float fRatio = 0.75f;

DWORD g_dwRamSize = 0x400000;
DWORD* g_pu32RamBase = NULL;
signed char *g_ps8RamBase = NULL;
unsigned char *g_pu8RamBase = NULL;
unsigned char * g_pu8SpMemBase = NULL;

CCritSect g_CritialSection;

//#define USING_THREAD

#ifdef USING_THREAD
HANDLE			videoThread;
HANDLE			threadMsg[5];
HANDLE			threadFinished;

#define RSPMSG_CLOSE			0
#define RSPMSG_SWAPBUFFERS		1
#define RSPMSG_PROCESSDLIST		2
#define RSPMSG_CHANGEWINDOW		3
#define RSPMSG_PROCESSRDPLIST	4
#endif


//=======================================================
// User Options
RECT frameWriteByCPURect;
std::vector<u32> frameWriteRecord;

int g_DlistCount=0;

void InitGammaValues();


// Ez0n3 - reinstate max video mem
extern bool g_bUseSetTextureMem;
extern DWORD g_maxTextureMemUsage;


void GetPluginDir( char * Directory ) 
{
#ifdef _XBOX
	//strcpy(Directory,"D:\\");
	strcpy(Directory,"T:\\");
#else
#endif
}

//-------------------------------------------------------------------------------------
void _VIDEO_GetDllInfo ( PLUGIN_INFO * PluginInfo )
{
#ifdef _DEBUG
	sprintf(PluginInfo->Name, "Rice's Daedalus Debug %d.%d.%d",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
#else
	sprintf(PluginInfo->Name, "Rice's Daedalus %d.%d.%d",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
#endif
	PluginInfo->Version        = 0x0102;
	PluginInfo->Type           = PLUGIN_TYPE_GFX;
	PluginInfo->NormalMemory   = FALSE;
	PluginInfo->MemoryBswaped  = TRUE;
}

//---------------------------------------------------------------------------------------

void _VIDEO_DllAbout ( HWND hParent )
{
	 
}


//---------------------------------------------------------------------------------------

void _VIDEO_DllTest ( HWND hParent )
{
	 
}

void _VIDEO_DllConfig ( HWND hParent )
{
	 
}

void ChangeWindowStep2()
{
	windowSetting.bDisplayFullscreen = 1-windowSetting.bDisplayFullscreen;
	g_CritialSection.Lock();
	windowSetting.bDisplayFullscreen = CGraphicsContext::Get()->ToggleFullscreen();
	if( windowSetting.bDisplayFullscreen )
	{
		 
	}
	else
	{
		 
	}
	g_CritialSection.Unlock();
}

void _VIDEO_ChangeWindow ()
{
	ChangeWindowStep2();

}

void ChangeWinSize( void ) 
{
	 
}
//---------------------------------------------------------------------------------------

void _VIDEO_DrawScreen (void)
{
}

//---------------------------------------------------------------------------------------

void _VIDEO_MoveScreen (int xpos, int ypos)
{ 
}

void Ini_GetRomOptions(LPROMINFO pRomInfo);
void Ini_StoreRomOptions(LPROMINFO pRomInfo);
void GenerateCurrentRomOptions();

void StartVideo(void)
{
	g_CritialSection.Lock();
	
	memcpy(&g_curRomInfo.rh, g_GraphicsInfo.HEADER, sizeof(ROMHeader));
	ROM_ByteSwap_3210( &g_curRomInfo.rh, sizeof(ROMHeader) );
	ROM_GetRomNameFromHeader(g_curRomInfo.szGameName, &g_curRomInfo.rh);
	Ini_GetRomOptions(&g_curRomInfo);
	GenerateCurrentRomOptions();
	dwTvSystem = CountryCodeToTVSystem(g_curRomInfo.rh.nCountryID);
	if( dwTvSystem == TV_SYSTEM_NTSC )
		fRatio = 0.75f;
	else
		fRatio = 9/11.0f;;


	if (g_curRomInfo.bDisableTextureCRC)
		g_bCRCCheck = FALSE;
	else
		g_bCRCCheck = TRUE;
	
	__try{
		uint32 dummy;
		dummy = g_GraphicsInfo.RDRAM[0x400000];
		dummy = g_GraphicsInfo.RDRAM[0x500000];
		dummy = g_GraphicsInfo.RDRAM[0x600000];
		dummy = g_GraphicsInfo.RDRAM[0x700000];
		dummy = g_GraphicsInfo.RDRAM[0x7FFFFC];
		g_dwRamSize = 0x800000;
	}
	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		g_dwRamSize = 0x400000;
	}

	ChangeWinSize();

	g_GraphicsInfo.hStatusBar = FALSE;

	CDeviceBuilder::GetBuilder()->CreateGraphicsContext();
	HRESULT hr = CGraphicsContext::Get()->Initialize(g_GraphicsInfo.hWnd, g_GraphicsInfo.hStatusBar, 640, 480, FALSE);
	
	CDeviceBuilder::GetBuilder()->CreateRender();
	CDaedalusRender::GetRender()->Initialize();

	if( SUCCEEDED(hr) )
	{
		DLParser_Init();
	}

	status.bGameIsRunning = true;
	g_DlistCount = 0;
	
	//status.gRDPFrame = 0;

	g_CritialSection.Unlock();

			
//	CProfiler::Get()->Reset();
}

void StopVideo()
{
	g_CritialSection.Lock();
	status.bGameIsRunning = false;

	try {
		gTextureCache.DropTextures();
		RDP_Cleanup();
	}
	catch(...)
	{
		TRACE0("Some exceptions during RomClosed");
	}

	if (CGraphicsContext::g_pGraphicsContext)
		CGraphicsContext::Get()->CleanUp();

	CDeviceBuilder::GetBuilder()->DeleteRender();
	CDeviceBuilder::GetBuilder()->DeleteGraphicsContext();

	g_CritialSection.Unlock();

#ifdef ENABLE_PROFILER
	CProfiler::Get()->Display();
#endif
	DEBUGGER_ONLY({delete surfTlut;});
}

#ifdef USING_THREAD
void ChangeWindowStep2();
void UpdateScreenStep2 (void);
void ProcessDListStep2(void);

//BOOL WINAPI SwitchToThread(VOID);
DWORD WINAPI VideoThreadProc( LPVOID lpParameter )
{
	BOOL res;
	StartVideo();
	SetEvent( threadFinished );

	while(true)
	{
		switch (WaitForMultipleObjects( 5, threadMsg, FALSE, INFINITE ))
		{
		case (WAIT_OBJECT_0 + RSPMSG_PROCESSDLIST):
			ProcessDListStep2();
			SetEvent( threadFinished );
			break;
		case (WAIT_OBJECT_0 + RSPMSG_SWAPBUFFERS):
			//res = SwitchToThread();
			//Sleep(1);
			UpdateScreenStep2();
			SetEvent( threadFinished );
			break;
		case (WAIT_OBJECT_0 + RSPMSG_CLOSE):
			StopVideo();
			SetEvent( threadFinished );
			return 1;
		case (WAIT_OBJECT_0 + RSPMSG_CHANGEWINDOW):
			ChangeWindowStep2();
			SetEvent( threadFinished );
			break;
		case (WAIT_OBJECT_0 + RSPMSG_PROCESSRDPLIST):
			g_DlistCount++;
			try
			{
				RDP_DLParser_Process();
			}
			catch (...)
			{
				ErrorMsg("Unknown Error in Daedalus plugin ProcessRDPList");
				//TriggerDPInterrupt();
			}
			SetEvent( threadFinished );
			break;
		}
	}
	return 0;
}
#endif



//---------------------------------------------------------------------------------------
void _VIDEO_RomClosed (void)
{
#ifdef USING_THREAD
	if(videoThread)
	{
		SetEvent( threadMsg[RSPMSG_CLOSE] );
		WaitForSingleObject( threadFinished, INFINITE );
		for (int i = 0; i < 5; i++)
		{
			if (threadMsg[i])	CloseHandle( threadMsg[i] );
		}
		CloseHandle( threadFinished );
		CloseHandle( videoThread );
	}
	videoThread = NULL;
#else
	StopVideo();
#endif
	//Ini_StoreRomOptions(&g_curRomInfo);
}

void _VIDEO_RomOpen (void)
{
	//InitGammaValues();

__try{
		uint32 dummy;
		dummy = g_GraphicsInfo.RDRAM[0x400000];
		dummy = g_GraphicsInfo.RDRAM[0x500000];
		dummy = g_GraphicsInfo.RDRAM[0x600000];
		dummy = g_GraphicsInfo.RDRAM[0x700000];
		dummy = g_GraphicsInfo.RDRAM[0x7FFFFC];
		g_dwRamSize = 0x800000;
	}
	__except(NULL, EXCEPTION_EXECUTE_HANDLER)
	{
		g_dwRamSize = 0x400000;
	}

#ifdef _DEBUG
	if( debuggerPause )
	{
		debuggerPause = FALSE;
		Sleep(100);
	}
#endif


#ifdef USING_THREAD
	DWORD threadID;
	for(int i = 0; i < 5; i++) 
	{ 
		threadMsg[i] = CreateEvent( NULL, FALSE, FALSE, NULL );
		if (threadMsg[i] == NULL)
		{ 
			ErrorMsg( "Error creating thread message events");
			return;
		} 
	} 
	threadFinished = CreateEvent( NULL, FALSE, FALSE, NULL );
	if (threadFinished == NULL)
	{ 
		ErrorMsg( "Error creating video thread finished event");
		return;
	} 
	videoThread = CreateThread( NULL, 4096, VideoThreadProc, NULL, NULL, &threadID );

#else
	StartVideo();
#endif
}


void SetVIScales()
{
	if( g_curRomInfo.VIHeight>0 && g_curRomInfo.VIWidth>0 )
	{
		windowSetting.fViWidth = windowSetting.uViWidth = g_curRomInfo.VIWidth;
		windowSetting.fViHeight = windowSetting.uViHeight = g_curRomInfo.VIHeight;
	}
	else
	{
		float xscale, yscale;
		DWORD val = *g_GraphicsInfo.VI_X_SCALE_RG & 0xFFF;
		xscale = (float)val / (1<<10);
		DWORD start = *g_GraphicsInfo.VI_H_START_RG >> 16;
		DWORD end = *g_GraphicsInfo.VI_H_START_RG&0xFFFF;
		DWORD width = *g_GraphicsInfo.VI_WIDTH_RG;
		windowSetting.fViWidth = (end-start)*xscale;
		if( abs(windowSetting.fViWidth - width ) < 8 ) 
		{
			windowSetting.fViWidth = (float)width;
		}
		else
		{
			//DebuggerAppendMsg("fViWidth = %f, Width Reg=%d", windowSetting.fViWidth, width);
		}

		val = (*g_GraphicsInfo.VI_Y_SCALE_RG & 0xFFF);// - ((*g_GraphicsInfo.VI_Y_SCALE_RG>>16) & 0xFFF);
		if( val == 0x3FF )	val = 0x400;
		yscale = (float)val / (1<<10);
		start = *g_GraphicsInfo.VI_V_START_RG >> 16;
		end = *g_GraphicsInfo.VI_V_START_RG&0xFFFF;
		windowSetting.fViHeight = (end-start)/2*yscale;

		if( yscale == 0 )
		{
			windowSetting.fViHeight = windowSetting.fViWidth*fRatio;
		}
		else
		{
			if( *g_GraphicsInfo.VI_WIDTH_RG > 0x300 ) 
				windowSetting.fViHeight *= 2;

			if( windowSetting.fViWidth*fRatio > windowSetting.fViHeight && (*g_GraphicsInfo.VI_X_SCALE_RG & 0xFF) != 0 )
			{
				if( abs(int(windowSetting.fViWidth*fRatio - windowSetting.fViHeight)) < 8 )
				{
					windowSetting.fViHeight = windowSetting.fViWidth*fRatio;
				}
				/*
				else
				{
					if( abs(windowSetting.fViWidth*fRatio-windowSetting.fViHeight) > windowSetting.fViWidth*0.1f )
					{
						if( fRatio > 0.8 )
							windowSetting.fViHeight = windowSetting.fViWidth*3/4;
						//windowSetting.fViHeight = (*g_GraphicsInfo.VI_V_SYNC_RG - 0x2C)/2;
					}
				}
				*/
			}
			
			if( windowSetting.fViHeight<100 || windowSetting.fViWidth<100 )
			{
				//At sometime, value in VI_H_START_RG or VI_V_START_RG are 0
				windowSetting.fViWidth = (float)*g_GraphicsInfo.VI_WIDTH_RG;
				windowSetting.fViHeight = windowSetting.fViWidth*fRatio;
			}
		}

		windowSetting.uViWidth = (unsigned short)(windowSetting.fViWidth/4);
		windowSetting.fViWidth = windowSetting.uViWidth *= 4;

		windowSetting.uViHeight = (unsigned short)(windowSetting.fViHeight/4);
		windowSetting.fViHeight = windowSetting.uViHeight *= 4;
		u16 optimizeHeight = u16(windowSetting.uViWidth*fRatio);
		if( windowSetting.uViHeight != optimizeHeight )
		{
			if( abs(windowSetting.uViHeight-optimizeHeight) <= 8 )
				windowSetting.fViHeight = windowSetting.uViHeight = optimizeHeight;
		}


		if( gRDP.scissor.left == 0 && gRDP.scissor.top == 0 && gRDP.scissor.right != 0 )
		{
			if( (*g_GraphicsInfo.VI_X_SCALE_RG & 0xFF) != 0x0 && gRDP.scissor.right == windowSetting.uViWidth )
			{
				// Mario Tennis
				if( abs(int( windowSetting.fViHeight - gRDP.scissor.bottom )) < 8 )
				{
					windowSetting.fViHeight = windowSetting.uViHeight = gRDP.scissor.bottom;
				}
				else if( windowSetting.fViHeight < gRDP.scissor.bottom )
				{
					windowSetting.fViHeight = windowSetting.uViHeight = gRDP.scissor.bottom;
				}
				windowSetting.fViHeight = windowSetting.uViHeight = gRDP.scissor.bottom;
			}
			else if( gRDP.scissor.right == windowSetting.uViWidth - 1 && gRDP.scissor.bottom != 0 )
			{
				if( !(fRatio == 0.75 && windowSetting.uViHeight == optimizeHeight) )
					windowSetting.fViHeight = windowSetting.uViHeight = gRDP.scissor.bottom + gRDP.scissor.top + 1;
			}
			else if( gRDP.scissor.right == windowSetting.uViWidth && gRDP.scissor.bottom != 0  && fRatio != 0.75 )
			{
				if( !(fRatio == 0.75 && windowSetting.uViHeight == optimizeHeight) )
					windowSetting.fViHeight = windowSetting.uViHeight = gRDP.scissor.bottom;
			}
		}
	}
	SetScreenMult(windowSetting.uDisplayWidth/windowSetting.fViWidth, windowSetting.uDisplayHeight/windowSetting.fViHeight);
}

//---------------------------------------------------------------------------------------
void SetAddrUsedByVIOrigin(u32 addr);
void UpdateScreenStep2 (void)
{
	status.bVIOriginIsUpdated = false;

	if( g_bUsingFakeCI )
		SaveFakeFrameBuffer();
	
	SetAddrUsedByVIOrigin(*g_GraphicsInfo.VI_ORIGIN_RG);

	if( g_DlistCount == 0 )
	{
		// CPU frame buffer update
		u32 width = *g_GraphicsInfo.VI_WIDTH_RG;
		if( (*g_GraphicsInfo.VI_ORIGIN_RG & (g_dwRamSize-1) ) > width*2 &&
			*g_GraphicsInfo.VI_H_START_RG != 0 && width != 0 )
		{
			SetVIScales();
			CDaedalusRender::GetRender()->DrawFrameBuffer(true);
			CGraphicsContext::Get()->SwapBuffer();
		}
		return;
	}

	if( currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_VI_UPDATE )
	{
		CGraphicsContext::Get()->SwapBuffer();

#ifdef _DEBUG
		if( pauseAtNext )
		{
			DebuggerAppendMsg("Update Screen: VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_RG);
		}
		DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_FRAME);
		DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_SET_CIMG);
#endif
		return;
	}

	if( currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_VI_CHANGE )
	{
		if( *g_GraphicsInfo.VI_ORIGIN_RG != status.curVIOriginReg )
		{
			if( *g_GraphicsInfo.VI_ORIGIN_RG < status.curDisplayBuffer || *g_GraphicsInfo.VI_ORIGIN_RG > status.curDisplayBuffer+0x2000  )
			{
				status.curDisplayBuffer = *g_GraphicsInfo.VI_ORIGIN_RG;
				status.curVIOriginReg = status.curDisplayBuffer;
				//status.curRenderBuffer = NULL;

				CGraphicsContext::Get()->SwapBuffer();
#ifdef _DEBUG
				if( pauseAtNext )
				{
					DebuggerAppendMsg("Update Screen: VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_RG);
				}
				DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_FRAME);
				DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_SET_CIMG);
#endif
			}
			else
			{
				status.curDisplayBuffer = *g_GraphicsInfo.VI_ORIGIN_RG;
				status.curVIOriginReg = status.curDisplayBuffer;
				DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FRAME, {DebuggerAppendMsg("Skip Screen Update, closed to the display buffer, VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_RG);});
			}
		}
		else
		{
			DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FRAME, {DebuggerAppendMsg("Skip Screen Update, the same VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_RG);});
		}

		return;
	}

	if( currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1ST_CI_CHANGE ||currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_1st_PRIMITIVE )
	{
		status.bVIOriginIsUpdated=true;
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FRAME, {DebuggerAppendMsg("VI ORIG is updated to %08X", *g_GraphicsInfo.VI_ORIGIN_RG);});
		return;
	}

#ifdef _DEBUG
	if( pauseAtNext )
	{
		DebuggerAppendMsg("VI is updated, No screen update: VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_RG);
	}
	DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_FRAME);
	DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_SET_CIMG);
#endif
}

void _VIDEO_UpdateScreen (void)
{
#ifdef USING_THREAD
	if (videoThread)
	{
		SetEvent( threadMsg[RSPMSG_SWAPBUFFERS] );
		WaitForSingleObject( threadFinished, INFINITE );
	}
#else
	UpdateScreenStep2();
#endif
}

//---------------------------------------------------------------------------------------

void _VIDEO_ViStatusChanged (void)
{
	SetVIScales();
	CDaedalusRender::g_pRender->UpdateClipRectangle();
}

//---------------------------------------------------------------------------------------
void _VIDEO_ViWidthChanged (void)
{
	SetVIScales();
	CDaedalusRender::g_pRender->UpdateClipRectangle();
}


BOOL _VIDEO_InitiateGFX(GFX_INFO Gfx_Info)
{
 
	windowSetting.bDisplayFullscreen = TRUE;
	memcpy(&g_GraphicsInfo, &Gfx_Info, sizeof(GFX_INFO));
	
	g_pu8RamBase			= Gfx_Info.RDRAM;
	g_pu32RamBase			= (DWORD*)Gfx_Info.RDRAM;
	g_ps8RamBase			= (signed char *)Gfx_Info.RDRAM;
	g_pu8SpMemBase			= (u8*)Gfx_Info.DMEM;

	windowSetting.fViWidth = 320;
	windowSetting.fViHeight = 240;

	InitConfiguration();

	return(TRUE);
}


void __cdecl MsgInfo (char * Message, ...)
{
#ifndef _XBOX
#else

	OutputDebugString("Rice MSG: ");
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
#endif
}

void __cdecl ErrorMsg (char * Message, ...)
{
#ifndef _XBOX
#else

	OutputDebugString("Rice ERR: ");
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
#endif
}

//---------------------------------------------------------------------------------------

void _VIDEO_CloseDLL (void)
{ 
	delete g_pIniFile;		// Will write ini file if changed
	g_pIniFile = NULL;
#ifdef _DEBUG
	 
#endif
}

void ProcessDListStep2(void)
{
	g_DlistCount++;
	g_CritialSection.Lock();
#ifdef _DEBUG
	DLParser_Process((OSTask *)(g_GraphicsInfo.DMEM + 0x0FC0));			
#else
	try
	{
		DLParser_Process((OSTask *)(g_GraphicsInfo.DMEM + 0x0FC0));			
	}
	catch (...)
	{
#ifdef _DEBUG
		ErrorMsg("Unknown Error in Daedalus D3D plugin ProcessDList");
#else

#endif
		TriggerDPInterrupt();
	}
#endif
	g_CritialSection.Unlock();
}	

DWORD _VIDEO_ProcessDListCountCycles(void)
{
#ifdef USING_THREAD
	if (videoThread)
	{
		SetEvent( threadMsg[RSPMSG_PROCESSDLIST] );
		WaitForSingleObject( threadFinished, INFINITE );
	}
	return 0;
#else
	g_DlistCount++;
	status.SPCycleCount = 100;
	status.DPCycleCount = 0;
#ifdef _DEBUG
	DLParser_Process((OSTask *)(g_GraphicsInfo.DMEM + 0x0FC0));
#else
	try
	{
		DLParser_Process((OSTask *)(g_GraphicsInfo.DMEM + 0x0FC0));
	}
	catch (...)
	{
#ifdef _DEBUG
		ErrorMsg("Unknown Error in Rice's Daedalus plugin ProcessDListCountCycles");
#endif
		TriggerDPInterrupt();
	}
#endif
	status.SPCycleCount *= 6;
	//status.DPCycleCount += status.SPCycleCount;
	//status.DPCycleCount *=4;
	//status.DPCycleCount = min(200,status.DPCycleCount);
	status.DPCycleCount *= 15;
	status.DPCycleCount += status.SPCycleCount;
	return (status.DPCycleCount<<16)+status.SPCycleCount;
#endif
}	

void _VIDEO_ProcessRDPList(void)
{
#ifdef USING_THREAD
	if (videoThread)
	{
		SetEvent( threadMsg[RSPMSG_PROCESSRDPLIST] );
		WaitForSingleObject( threadFinished, INFINITE );
	}
#else
	g_DlistCount++;
	try
	{
		RDP_DLParser_Process();
	}
	catch (...)
	{
		ErrorMsg("Unknown Error in Daedalus plugin ProcessRDPList");
		//TriggerDPInterrupt();
	}
#endif
}	

void _VIDEO_ProcessDList(void)
{
#ifdef USING_THREAD
	if (videoThread)
	{
		SetEvent( threadMsg[RSPMSG_PROCESSDLIST] );
		WaitForSingleObject( threadFinished, INFINITE );
	}
#else
	ProcessDListStep2();
#endif
}	

//---------------------------------------------------------------------------------------

void TriggerDPInterrupt(void)
{
	*(g_GraphicsInfo.MI_INTR_RG) |= MI_INTR_DP;
	g_GraphicsInfo.CheckInterrupts();
}

/******************************************************************
  Function: FrameBufferWriteList
  Purpose:  This function is called to notify the dll that the
            frame buffer has been modified by CPU at the given address.
  input:    FrameBufferModifyEntry *plist
			size = size of the plist, max = 1024
  output:   none
*******************************************************************/ 
void _VIDEO_FBWList(FrameBufferModifyEntry *plist, DWORD size)
{
}

/******************************************************************
  Function: FrameBufferRead
  Purpose:  This function is called to notify the dll that the
            frame buffer memory is beening read at the given address.
			DLL should copy content from its render buffer to the frame buffer
			in N64 RDRAM
			DLL is responsible to maintain its own frame buffer memory addr list
			DLL should copy 4KB block content back to RDRAM frame buffer.
			Emulator should not call this function again if other memory
			is read within the same 4KB range
  input:    addr		rdram address
			val			val
			size		1 = BYTE, 2 = WORD, 4 = DWORD
  output:   none
*******************************************************************/ 

void FrameBufferWriteByCPU(u32 addr, u32 size);
void FrameBufferReadByCPU( u32 addr );

extern SetImgInfo g_CI;

 void _VIDEO_FBRead(DWORD addr)
{
	FrameBufferReadByCPU(addr);
}


/******************************************************************
  Function: FrameBufferWrite
  Purpose:  This function is called to notify the dll that the
            frame buffer has been modified by CPU at the given address.
  input:    addr		rdram address
			val			val
			size		1 = BYTE, 2 = WORD, 4 = DWORD
  output:   none
*******************************************************************/ 

void _VIDEO_FBWrite(DWORD addr, DWORD size)
{
	FrameBufferWriteByCPU(addr, size);
}

// not in 510? code from 531
/*typedef struct
{
	uint32	addr;
	uint32	size;
	uint32	width;
	uint32	height;
} FrameBufferInfo;
extern SetImgInfo g_DI;
extern RecentCIInfo g_RecentCIInfo[];
void _VIDEO_FBGetFrameBufferInfo(void *p)
{
	FrameBufferInfo * pinfo = (FrameBufferInfo *)p;
	if( g_DI.dwAddr == 0 )
	{
		memset(pinfo,0,sizeof(FrameBufferInfo)*6);
	}
	else
	{
		int idx=0;
		for (int i=0; i<5; i++ )
		{
			if( status.gRDPFrame-g_RecentCIInfo[i].lastUsedFrame > 30 || g_RecentCIInfo[i].lastUsedFrame == 0 )
			{
				memset(&pinfo[i],0,sizeof(FrameBufferInfo));
			}
			else
			{
				pinfo[i].addr = g_RecentCIInfo[i].dwAddr;
				pinfo[i].size = 2;
				pinfo[i].width = g_RecentCIInfo[i].dwWidth;
				pinfo[i].height = g_RecentCIInfo[i].dwHeight;
				pinfo[5].width = g_RecentCIInfo[i].dwWidth;
				pinfo[5].height = g_RecentCIInfo[i].dwHeight;
			}
		}

		pinfo[5].addr = g_DI.dwAddr;
		//pinfo->size = g_RecentCIInfo[5].dwSize;
		pinfo[5].size = 2;
	}
}*/

void _VIDEO_SetMaxTextureMem(DWORD mem)
{
	if (mem == 0) // auto mem
	{
		g_bUseSetTextureMem = false;
	}
	else // set mem
	{
		g_bUseSetTextureMem = true;
		g_maxTextureMemUsage = mem * 1024 * 1024;
	}
}

void _VIDEO_DisplayTemporaryMessage(const char *msg)
{
	g_bTempMessage = TRUE;
	strncpy(g_szTempMessage, msg, 99);
	g_dwTempMessageStart = GetTickCount();
}

void InitGammaValues()
{
	/*float gamma = 1;
	switch( options.gamma_correction )
	{
	case 0:	// No gamma correction
		gamma = 1;
		break;
	case 1:	// Lighter
		gamma = 0.9f;
		break;
	case 2:	// Lighter
		gamma = 0.8f;
		break;
	case 3:	// Lighter
		gamma = 0.65f;
		break;
	case 4: // Lightest
		gamma = 0.5f;
		break;
	case 5: // darker
		gamma = 1.25f;
		break;
	case 6: // Darkest
		gamma = 1.5f;
		break;
	}

	if( options.gamma_correction == 0 )
	{
		for( int i=0; i<256; i++ )
		{
			g_gammaValues[i] = i;
		}
	}
	else
	{
		for( int i=0; i<256; i++ )
		{
			double val = pow((i/255.0f),gamma)*256;
			g_gammaValues[i] = BYTE(val);
		}
	} */
}

