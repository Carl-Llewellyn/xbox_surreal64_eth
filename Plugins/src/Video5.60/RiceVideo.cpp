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

extern bool bEnableHDTV;
extern bool bFullScreen;

BOOL g_bTempMessage = FALSE;
DWORD g_dwTempMessageStart = 0;
char g_szTempMessage[100];

// reinstate max video mem
extern bool g_bUseSetTextureMem;
extern DWORD g_maxTextureMemUsage;

PluginStatus status;
char generalText[256];

IniFile *g_pIniFile = NULL;
GFX_INFO g_GraphicsInfo;

DWORD g_dwRamSize = 0x400000;
DWORD* g_pRDRAMu32 = NULL;
signed char *g_pRDRAMs8 = NULL;
unsigned char *g_pRDRAMu8 = NULL;

CCritSect g_CritialSection;

///#define USING_THREAD

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
std::vector<RECT> frameWriteByCPURects;
RECT frameWriteByCPURectArray[20][20];
bool frameWriteByCPURectFlag[20][20];
std::vector<DWORD> frameWriteRecord;

//---------------------------------------------------------------------------------------

BOOL APIENTRY DllMain(HINSTANCE hinstDLL,  // DLL module handle
                      DWORD fdwReason,              // reason called
                      LPVOID lpvReserved)           // reserved
{ 
	windowSetting.myhInst = hinstDLL;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: 
		CProfiler::Create();
		InitConfiguration();
		break;
	case DLL_THREAD_ATTACH: 
		break; 
	case DLL_THREAD_DETACH: 
		break; 
	case DLL_PROCESS_DETACH: 
		CProfiler::Destroy();

		delete g_pIniFile;		// Will write ini file if changed
		g_pIniFile = NULL;
		break; 
	} 
	
	
	return TRUE; 
} 

void GetPluginDir( char * Directory ) 
{
#ifdef _XBOX
	//strcpy(Directory,"D:\\");
	strcpy(Directory,"T:\\");
#else
#endif
}

//-------------------------------------------------------------------------------------
EXPORT void CALL _VIDEO_GetDllInfo ( PLUGIN_INFO * PluginInfo )
{
#ifdef _DEBUG
#if _MSC_VER > 1200
	sprintf(PluginInfo->Name, "Rice's Video Plugin Debug %d.%d.%d",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
#else
	sprintf(PluginInfo->Name, "Rice's Video Plugin Debug %d.%d.%d - Win98",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
#endif
#else
#if _MSC_VER > 1200
	sprintf(PluginInfo->Name, "Rice's Video Plugin %d.%d.%d",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
	//sprintf(PluginInfo->Name, "Rice's Video Plugin %d.%d.%d (%s)",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2,BUILD_DATE);
#else
	sprintf(PluginInfo->Name, "Rice's Video Plugin %d.%d.%d - Win98",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
#endif
#endif
	PluginInfo->Version        = 0x0103;
	PluginInfo->Type           = PLUGIN_TYPE_GFX;
	PluginInfo->NormalMemory   = FALSE;
	PluginInfo->MemoryBswaped  = TRUE;
}

//---------------------------------------------------------------------------------------

EXPORT void CALL _VIDEO_DllAbout ( HWND hParent )
{
	char temp[300];
	sprintf(temp,"Rice's Video Plugin %d.%d.%d (build %d) \nDirectX 8.1+\nOpenGL 1.1-1.4/ATI/Nvidia TNT/Geforce Extension",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2,BUILD_NUMBER) ;
	MsgInfo(temp);
}


//---------------------------------------------------------------------------------------

EXPORT void CALL _VIDEO_DllTest ( HWND hParent )
{
	MsgInfo("TODO: Test");
}

EXPORT void CALL _VIDEO_DllConfig ( HWND hParent )
{
	
}

void ChangeWindowStep2()
{
	status.bDisableFPS = true;
	windowSetting.bDisplayFullscreen = 1-windowSetting.bDisplayFullscreen;
	g_CritialSection.Lock();
	windowSetting.bDisplayFullscreen = CGraphicsContext::Get()->ToggleFullscreen();
 
	CGraphicsContext::Get()->Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);
	CGraphicsContext::Get()->UpdateFrame();
	CGraphicsContext::Get()->Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);
	CGraphicsContext::Get()->UpdateFrame();
	CGraphicsContext::Get()->Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);
	CGraphicsContext::Get()->UpdateFrame();
	g_CritialSection.Unlock();
	status.bDisableFPS = false;
	status.ToToggleFullScreen = FALSE;
}

EXPORT void CALL _VIDEO_ChangeWindow ()
{
	if( status.ToToggleFullScreen )
		status.ToToggleFullScreen = FALSE;
	else
		status.ToToggleFullScreen = TRUE;
}

void ChangeWinSize( void ) 
{
	 
}
//---------------------------------------------------------------------------------------

EXPORT void CALL _VIDEO_DrawScreen (void)
{
}

//---------------------------------------------------------------------------------------

EXPORT void CALL _VIDEO_MoveScreen (int xpos, int ypos)
{ 
}

void Ini_GetRomOptions(LPROMINFO pRomInfo);
void Ini_StoreRomOptions(LPROMINFO pRomInfo);
void GenerateCurrentRomOptions();
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if( windowSetting.lastSecDlistCount != 0xFFFFFFFF )
	{
		if( windowSetting.dps < 0 )
			windowSetting.dps = (float)(status.gDlistCount-windowSetting.lastSecDlistCount);
		else
			windowSetting.dps = windowSetting.dps * 0.2f + (status.gDlistCount-windowSetting.lastSecDlistCount)*0.8f;
	}
	windowSetting.lastSecDlistCount = status.gDlistCount;

	if( windowSetting.lastSecFrameCount != 0xFFFFFFFF )
	{
		if( windowSetting.fps < 0 )
			windowSetting.fps = (float)(status.gFrameCount-windowSetting.lastSecFrameCount);
		else
			windowSetting.fps = windowSetting.fps * 0.2f + (status.gFrameCount-windowSetting.lastSecFrameCount)*0.8f;
	}
	windowSetting.lastSecFrameCount = status.gFrameCount;
}

void StartVideo(void)
{
 
	g_CritialSection.Lock();

	memcpy(&g_curRomInfo.romheader, g_GraphicsInfo.HEADER, sizeof(ROMHeader));
	ROM_ByteSwap_3210( &g_curRomInfo.romheader, sizeof(ROMHeader) );
	ROM_GetRomNameFromHeader(g_curRomInfo.szGameName, &g_curRomInfo.romheader);
	Ini_GetRomOptions(&g_curRomInfo);
	GenerateCurrentRomOptions();
	status.dwTvSystem = CountryCodeToTVSystem(g_curRomInfo.romheader.nCountryID);
	if( status.dwTvSystem == TV_SYSTEM_NTSC )
		status.fRatio = 0.75f;
	else
		status.fRatio = 9/11.0f;;
	
	ChangeWinSize();
		
	try {
		CDeviceBuilder::GetBuilder()->CreateGraphicsContext();
		CGraphicsContext::InitWindowInfo();
		
		windowSetting.bDisplayFullscreen = FALSE;
		bool res = CGraphicsContext::Get()->Initialize(g_GraphicsInfo.hWnd, g_GraphicsInfo.hStatusBar, 640, 480, TRUE);
		
		CDeviceBuilder::GetBuilder()->CreateRender();
		CRender::GetRender()->Initialize();
		
		if( res )
		{
			DLParser_Init();
		}
		
		status.bGameIsRunning = true;
	}
	catch(...)
	{
		ErrorMsg("Error to start video");
		throw 0;
	}

	g_CritialSection.Unlock();
	 
}

void StopVideo()
{
	g_CritialSection.Lock();
	status.bGameIsRunning = false;

	try {
		// Kill all textures?
		gTextureManager.DropTextures();
		gTextureManager.CleanUp();
		RDP_Cleanup();
	}
	catch(...)
	{
		TRACE0("Some exceptions during RomClosed");
	}

	CDeviceBuilder::GetBuilder()->DeleteRender();
	if (CGraphicsContext::g_pGraphicsContext)
		CGraphicsContext::Get()->CleanUp();
	CDeviceBuilder::GetBuilder()->DeleteGraphicsContext();

	g_CritialSection.Unlock();
	windowSetting.dps = windowSetting.fps = -1;
	windowSetting.lastSecDlistCount = windowSetting.lastSecFrameCount = 0xFFFFFFFF;
	status.gDlistCount = status.gFrameCount = 0;

 
	 
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
			try
			{
				RDP_DLParser_Process();
			}
			catch (...)
			{
				ErrorMsg("Unknown Error in ProcessRDPList");
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
EXPORT void CALL _VIDEO_RomClosed (void)
{
	TRACE0("To stop video");
	Ini_StoreRomOptions(&g_curRomInfo);
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
	TRACE0("Video is stopped");
}

EXPORT void CALL _VIDEO_RomOpen (void)
{
	if( g_CritialSection.IsLocked() )
	{
		g_CritialSection.Unlock();
		TRACE0("g_CritialSection is locked when game is starting, unlock it now.");
	}
	status.bDisableFPS=false;

	__try{
		DWORD dummy;
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
	else if( g_curRomInfo.UseCIWidthAndRatio && g_CI.dwWidth )
	{
		windowSetting.fViWidth = windowSetting.uViWidth = g_CI.dwWidth;
		windowSetting.fViHeight = windowSetting.uViHeight = 
			g_curRomInfo.UseCIWidthAndRatio == USE_CI_WIDTH_AND_RATIO_FOR_NTSC ? g_CI.dwWidth/4*3 : g_CI.dwWidth/11*9;
	}
	else
	{
		float xscale, yscale;
		DWORD val = *g_GraphicsInfo.VI_X_SCALE_REG & 0xFFF;
		xscale = (float)val / (1<<10);
		DWORD start = *g_GraphicsInfo.VI_H_START_REG >> 16;
		DWORD end = *g_GraphicsInfo.VI_H_START_REG&0xFFFF;
		DWORD width = *g_GraphicsInfo.VI_WIDTH_REG;
		windowSetting.fViWidth = (end-start)*xscale;
		if( abs(windowSetting.fViWidth - width ) < 8 ) 
		{
			windowSetting.fViWidth = (float)width;
		}
		else
		{
			//DebuggerAppendMsg("fViWidth = %f, Width Reg=%d", windowSetting.fViWidth, width);
		}

		val = (*g_GraphicsInfo.VI_Y_SCALE_REG & 0xFFF);// - ((*g_GraphicsInfo.VI_Y_SCALE_REG>>16) & 0xFFF);
		if( val == 0x3FF )	val = 0x400;
		yscale = (float)val / (1<<10);
		start = *g_GraphicsInfo.VI_V_START_REG >> 16;
		end = *g_GraphicsInfo.VI_V_START_REG&0xFFFF;
		windowSetting.fViHeight = (end-start)/2*yscale;

		if( yscale == 0 )
		{
			windowSetting.fViHeight = windowSetting.fViWidth*status.fRatio;
		}
		else
		{
			if( *g_GraphicsInfo.VI_WIDTH_REG > 0x300 ) 
				windowSetting.fViHeight *= 2;

			if( windowSetting.fViWidth*status.fRatio > windowSetting.fViHeight && (*g_GraphicsInfo.VI_X_SCALE_REG & 0xFF) != 0 )
			{
				if( abs(int(windowSetting.fViWidth*status.fRatio - windowSetting.fViHeight)) < 8 )
				{
					windowSetting.fViHeight = windowSetting.fViWidth*status.fRatio;
				}
				/*
				else
				{
					if( abs(windowSetting.fViWidth*status.fRatio-windowSetting.fViHeight) > windowSetting.fViWidth*0.1f )
					{
						if( status.fRatio > 0.8 )
							windowSetting.fViHeight = windowSetting.fViWidth*3/4;
						//windowSetting.fViHeight = (*g_GraphicsInfo.VI_V_SYNC_REG - 0x2C)/2;
					}
				}
				*/
			}
			
			if( windowSetting.fViHeight<100 || windowSetting.fViWidth<100 )
			{
				//At sometime, value in VI_H_START_REG or VI_V_START_REG are 0
				windowSetting.fViWidth = (float)*g_GraphicsInfo.VI_WIDTH_REG;
				windowSetting.fViHeight = windowSetting.fViWidth*status.fRatio;
			}
		}

		windowSetting.uViWidth = (unsigned short)(windowSetting.fViWidth/4);
		windowSetting.fViWidth = windowSetting.uViWidth *= 4;

		windowSetting.uViHeight = (unsigned short)(windowSetting.fViHeight/4);
		windowSetting.fViHeight = windowSetting.uViHeight *= 4;
		uint16 optimizeHeight = uint16(windowSetting.uViWidth*status.fRatio);
		uint16 optimizeHeight2 = uint16(windowSetting.uViWidth*3/4);
		if( windowSetting.uViHeight != optimizeHeight && windowSetting.uViHeight != optimizeHeight2 )
		{
			if( abs(windowSetting.uViHeight-optimizeHeight) <= 8 )
				windowSetting.fViHeight = windowSetting.uViHeight = optimizeHeight;
			else if( abs(windowSetting.uViHeight-optimizeHeight2) <= 8 )
				windowSetting.fViHeight = windowSetting.uViHeight = optimizeHeight2;
		}


		if( gRDP.scissor.left == 0 && gRDP.scissor.top == 0 && gRDP.scissor.right != 0 )
		{
			if( (*g_GraphicsInfo.VI_X_SCALE_REG & 0xFF) != 0x0 && gRDP.scissor.right == windowSetting.uViWidth )
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
				if( windowSetting.uViHeight != optimizeHeight && windowSetting.uViHeight != optimizeHeight2 )
				{
					if( status.fRatio != 0.75 && windowSetting.fViHeight > optimizeHeight/2 )
					{
						windowSetting.fViHeight = windowSetting.uViHeight = gRDP.scissor.bottom + gRDP.scissor.top + 1;
					}
				}
			}
			else if( gRDP.scissor.right == windowSetting.uViWidth && gRDP.scissor.bottom != 0  && status.fRatio != 0.75 )
			{
				if( windowSetting.uViHeight != optimizeHeight && windowSetting.uViHeight != optimizeHeight2 )
				{
					if( status.fRatio != 0.75 && windowSetting.fViHeight > optimizeHeight/2 )
					{
						windowSetting.fViHeight = windowSetting.uViHeight = gRDP.scissor.bottom + gRDP.scissor.top + 1;
					}
				}
			}
		}
	}
	SetScreenMult(windowSetting.uDisplayWidth/windowSetting.fViWidth, windowSetting.uDisplayHeight/windowSetting.fViHeight);
}

//---------------------------------------------------------------------------------------
void SetAddrUsedByVIOrigin(DWORD addr);
void UpdateScreenStep2 (void)
{
	status.bVIOriginIsUpdated = false;

	if( status.ToToggleFullScreen && status.gDlistCount > 0 )
	{
		ChangeWindowStep2();
		return;
	}

	g_CritialSection.Lock();
	if( status.bHandleN64TextureBuffer )
		CGraphicsContext::g_pGraphicsContext->CloseTextureBuffer(true);
	
	SetAddrUsedByVIOrigin(*g_GraphicsInfo.VI_ORIGIN_REG);

	if( status.gDlistCount == 0 )
	{
		// CPU frame buffer update
		DWORD width = *g_GraphicsInfo.VI_WIDTH_REG;
		if( (*g_GraphicsInfo.VI_ORIGIN_REG & (g_dwRamSize-1) ) > width*2 && *g_GraphicsInfo.VI_H_START_REG != 0 && width != 0 )
		{
			SetVIScales();
			CRender::GetRender()->DrawFrameBuffer(true);
			CGraphicsContext::Get()->UpdateFrame();
		}
		g_CritialSection.Unlock();
		return;
	}

	if( status.toCaptureScreen )
	{
		status.toCaptureScreen = false;
		// Capture screen here
		CRender::g_pRender->CaptureScreen(status.screenCaptureFilename);
	}

	if( currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_VI_UPDATE )
	{
		CGraphicsContext::Get()->UpdateFrame();

#ifdef _DEBUG
		if( pauseAtNext )
		{
			DebuggerAppendMsg("Update Screen: VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_REG);
		}
		DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_FRAME);
		DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_SET_CIMG);
#endif
		g_CritialSection.Unlock();
		return;
	}

	if( currentRomOptions.screenUpdateSetting==SCREEN_UPDATE_AT_VI_CHANGE )
	{
		if( *g_GraphicsInfo.VI_ORIGIN_REG != status.curVIOriginReg )
		{
			if( *g_GraphicsInfo.VI_ORIGIN_REG < status.curDisplayBuffer || *g_GraphicsInfo.VI_ORIGIN_REG > status.curDisplayBuffer+0x2000  )
			{
				status.curDisplayBuffer = *g_GraphicsInfo.VI_ORIGIN_REG;
				status.curVIOriginReg = status.curDisplayBuffer;
				//status.curRenderBuffer = NULL;

				CGraphicsContext::Get()->UpdateFrame();
#ifdef _DEBUG
				if( pauseAtNext )
				{
					DebuggerAppendMsg("Update Screen: VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_REG);
				}
				DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_FRAME);
				DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_SET_CIMG);
#endif
			}
			else
			{
				status.curDisplayBuffer = *g_GraphicsInfo.VI_ORIGIN_REG;
				status.curVIOriginReg = status.curDisplayBuffer;
				DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FRAME, {DebuggerAppendMsg("Skip Screen Update, closed to the display buffer, VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_REG);});
			}
		}
		else
		{
			DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FRAME, {DebuggerAppendMsg("Skip Screen Update, the same VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_REG);});
		}

		g_CritialSection.Unlock();
		return;
	}

	if( currentRomOptions.screenUpdateSetting >= SCREEN_UPDATE_AT_1ST_CI_CHANGE )
	{
		status.bVIOriginIsUpdated=true;
		DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_FRAME, {DebuggerAppendMsg("VI ORIG is updated to %08X", *g_GraphicsInfo.VI_ORIGIN_REG);});
		g_CritialSection.Unlock();
		return;
	}

#ifdef _DEBUG
	if( pauseAtNext )
	{
		DebuggerAppendMsg("VI is updated, No screen update: VIORIG=%08X", *g_GraphicsInfo.VI_ORIGIN_REG);
	}
	DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_FRAME);
	DEBUGGER_PAUSE_COUNT_N_WITHOUT_UPDATE(NEXT_SET_CIMG);
#endif

	g_CritialSection.Unlock();
}

EXPORT void CALL _VIDEO_UpdateScreen (void)
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

EXPORT void CALL _VIDEO_ViStatusChanged (void)
{
	g_CritialSection.Lock();
	SetVIScales();
	CRender::g_pRender->UpdateClipRectangle();
	g_CritialSection.Unlock();
}

//---------------------------------------------------------------------------------------
EXPORT void CALL _VIDEO_ViWidthChanged (void)
{
	g_CritialSection.Lock();
	SetVIScales();
	CRender::g_pRender->UpdateClipRectangle();
	g_CritialSection.Unlock();
}
EXPORT BOOL CALL GetFullScreenStatus(void);
EXPORT void CALL SetOnScreenText (char *msg)
{
	status.CPUCoreMsgIsSet = true;
	memset(&status.CPUCoreMsgToDisplay, 0, 256);
	strncpy(status.CPUCoreMsgToDisplay, msg, 255);
}

EXPORT BOOL CALL GetFullScreenStatus(void)
{
	if( CGraphicsContext::g_pGraphicsContext )
	{
		return CGraphicsContext::g_pGraphicsContext->IsWindowed() ? FALSE : TRUE;
	}
	else
	{
		return FALSE;
	}
}

EXPORT BOOL CALL _VIDEO_InitiateGFX(GFX_INFO Gfx_Info)
{
#ifdef _DEBUG
	OpenDialogBox();
#endif

	memset(&status, 0, sizeof(status));
	windowSetting.bDisplayFullscreen = FALSE;
	memcpy(&g_GraphicsInfo, &Gfx_Info, sizeof(GFX_INFO));
	
	g_pRDRAMu8			= Gfx_Info.RDRAM;
	g_pRDRAMu32			= (DWORD*)Gfx_Info.RDRAM;
	g_pRDRAMs8			= (signed char *)Gfx_Info.RDRAM;

	windowSetting.fViWidth = 320;
	windowSetting.fViHeight = 240;
	status.ToToggleFullScreen = FALSE;
	status.bDisableFPS=false;

	InitConfiguration();
	CGraphicsContext::InitWindowInfo();
	CGraphicsContext::InitDeviceParameters();

	return(TRUE);
}


void __cdecl MsgInfo (char * Message, ...)
{
#ifndef _XBOX
	char Msg[400];
	va_list ap;

	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );

	sprintf(generalText, "Rice's Video Plugin %d.%d.%d",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
	MessageBox(NULL,Msg,generalText,MB_OK|MB_ICONINFORMATION);
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
	char Msg[400];
	va_list ap;
	
	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );
	
	sprintf(generalText, "Rice's Video Plugin %d.%d.%d",FILE_VERSION0,FILE_VERSION1,FILE_VERSION2);
	if( status.ToToggleFullScreen || (CGraphicsContext::g_pGraphicsContext && !CGraphicsContext::g_pGraphicsContext->IsWindowed()) )
		SetWindowText(g_GraphicsInfo.hStatusBar,Msg);
	else
		MessageBox(NULL,Msg,generalText,MB_OK|MB_ICONERROR);
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

EXPORT void CALL _VIDEO_CloseDLL (void)
{ 
	if( status.bGameIsRunning )
	{
		_VIDEO_RomClosed();
	}

	delete g_pIniFile;		// Will write ini file if changed
	g_pIniFile = NULL;
#ifdef _DEBUG
	CloseDialogBox();
#endif
}

void ProcessDListStep2(void)
{
	g_CritialSection.Lock();
	if( status.toShowCFB )
	{
		CRender::GetRender()->DrawFrameBuffer(true);
		status.toShowCFB = false;
	}

	try
	{
		DLParser_Process((OSTask *)(g_GraphicsInfo.DMEM + 0x0FC0));
	}
	catch (...)
	{
		TRACE0("Unknown Error in ProcessDList");
		TriggerDPInterrupt();
	}

	g_CritialSection.Unlock();
}	

EXPORT DWORD CALL _VIDEO_ProcessDListCountCycles(void)
{
#ifdef USING_THREAD
	if (videoThread)
	{
		SetEvent( threadMsg[RSPMSG_PROCESSDLIST] );
		WaitForSingleObject( threadFinished, INFINITE );
	}
	return 0;
#else
	g_CritialSection.Lock();
	status.SPCycleCount = 100;
	status.DPCycleCount = 0;
	try
	{
		DLParser_Process((OSTask *)(g_GraphicsInfo.DMEM + 0x0FC0));
	}
	catch (...)
	{
		TRACE0("Unknown Error in Rice's Video Plugin plugin ProcessDListCountCycles");
		TriggerDPInterrupt();
	}
	status.SPCycleCount *= 6;
	//status.DPCycleCount += status.SPCycleCount;
	//status.DPCycleCount *=4;
	//status.DPCycleCount = min(200,status.DPCycleCount);
	//status.DPCycleCount *= 15;
	status.DPCycleCount *= 5;
	status.DPCycleCount += status.SPCycleCount;

	g_CritialSection.Unlock();
	return (status.DPCycleCount<<16)+status.SPCycleCount;
#endif
}	

EXPORT void CALL _VIDEO_ProcessRDPList(void)
{
#ifdef USING_THREAD
	if (videoThread)
	{
		SetEvent( threadMsg[RSPMSG_PROCESSRDPLIST] );
		WaitForSingleObject( threadFinished, INFINITE );
	}
#else
	try
	{
		RDP_DLParser_Process();
	}
	catch (...)
	{
		TRACE0("Unknown Error in ProcessRDPList");
		TriggerDPInterrupt();
	}
#endif
}	

EXPORT void CALL _VIDEO_ProcessDList(void)
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
	*(g_GraphicsInfo.MI_INTR_REG) |= MI_INTR_DP;
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
EXPORT  void CALL _VIDEO_FBWList(FrameBufferModifyEntry *plist, DWORD size)
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

			Since depth buffer is also being watched, the reported addr
			may belong to depth buffer
  input:    addr		rdram address
			val			val
			size		1 = BYTE, 2 = WORD, 4 = DWORD
  output:   none
*******************************************************************/ 

void FrameBufferWriteByCPU(DWORD addr, DWORD size);
void FrameBufferReadByCPU( DWORD addr );

EXPORT  void CALL _VIDEO_FBRead(DWORD addr)
{
	FrameBufferReadByCPU(addr);
}


/******************************************************************
  Function: FrameBufferWrite
  Purpose:  This function is called to notify the dll that the
            frame buffer has been modified by CPU at the given address.

			Since depth buffer is also being watched, the reported addr
			may belong to depth buffer

  input:    addr		rdram address
			val			val
			size		1 = BYTE, 2 = WORD, 4 = DWORD
  output:   none
*******************************************************************/ 

EXPORT void CALL _VIDEO_FBWrite(DWORD addr, DWORD size)
{
	FrameBufferWriteByCPU(addr, size);
}

/************************************************************************
Function: FBGetFrameBufferInfo
Purpose:  This function is called by the emulator core to retrieve frame
		  buffer information from the video plugin in order to be able
		  to notify the video plugin about CPU frame buffer read/write
		  operations

		  size:
			= 1		byte
			= 2		word (16 bit) <-- this is N64 default depth buffer format
			= 4		dword (32 bit)

		  when frame buffer information is not available yet, set all values
		  in the FrameBufferInfo structure to 0

input:    FrameBufferInfo pinfo[6]
		  pinfo is pointed to a FrameBufferInfo structure which to be
		  filled in by this function
output:   Values are return in the FrameBufferInfo structure
		  Plugin can return up to 6 frame buffer info
/************************************************************************/

/******************************************************************
  Function: FrameBufferWrite
  Purpose:  This function is called to notify the dll that the
            frame buffer has been modified by CPU at the given address.
  input:    addr		rdram address
			val			val
			size		1 = BYTE, 2 = WORD, 4 = DWORD
  output:   none
*******************************************************************/ 

/*void __VIDEO_FBWrite(DWORD addr, DWORD size)
{
	FrameBufferWriteByCPU(addr, size);
}*/

void _VIDEO_SetMaxTextureMem(DWORD mem) // supposed to be double underscore?? //	__VIDEO_SetMaxTextureMem(DWORD mem)
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

typedef struct
{
	DWORD	addr;
	DWORD	size;
	DWORD	width;
	DWORD	height;
} FrameBufferInfo;
extern RecentCIInfo g_RecentCIInfo[];
EXPORT void CALL _VIDEO_FBGetFrameBufferInfo(void *p)
{
	FrameBufferInfo * pinfo = (FrameBufferInfo *)p;
	if( g_ZI.dwAddr == 0 )
	{
		memset(pinfo,0,sizeof(FrameBufferInfo)*6);
	}
	else
	{
		int idx=0;
		for (int i=0; i<5; i++ )
		{
			if( status.gDlistCount-g_RecentCIInfo[i].lastUsedFrame > 30 || g_RecentCIInfo[i].lastUsedFrame == 0 )
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

		pinfo[5].addr = g_ZI.dwAddr;
		//pinfo->size = g_RecentCIInfo[5].dwSize;
		pinfo[5].size = 2;
	}
}

//__declspec(dllexport) void CALL FBWrite2(DWORD addr, DWORD size)
//{
//	FrameBufferWriteByCPU(addr, size);
//}

// Plugin spec 1.3 functions
#ifndef _XBOX
void CALL ShowCFB (void)
{
	status.toShowCFB = true;
}
#endif

void CALL CaptureScreen ( char * Directory )
{
#ifndef _XBOX
	if( status.bGameIsRunning && status.gDlistCount > 0 )
	{
		if( !PathFileExists(Directory) )
		{
			if( !CreateDirectory(Directory, NULL) )
			{
				//DisplayError("Can not create new folder: %s", pathname);
				return;
			}
		}

		strcpy(status.screenCaptureFilename, Directory);
		if( Directory[strlen(Directory)-1] != '\\' && Directory[strlen(Directory)-1] != '/'  )
		{
			strcat(status.screenCaptureFilename,"\\");
		}
		
		strcat(status.screenCaptureFilename, g_curRomInfo.szGameName);

		char tempname[MAX_PATH];
		for( int i=0; ; i++)
		{
			sprintf(tempname, "%s-%d.bmp", status.screenCaptureFilename, i);
			if( !PathFileExists(tempname) )
			{
				break;
			}
		}

		strcpy(status.screenCaptureFilename, tempname);
		status.toCaptureScreen = true;
	}
#endif
}