#include "stdafx_video.h"
#include "..\..\..\..\config.h"

#include <xbfont.h>
#include <xfont.h>

#define XFONT_TRUETYPE
#define DEBUGLINES 10
#define DEBUGSPACING 20


extern BOOL g_bTempMessage;
extern DWORD g_dwTempMessageStart;
extern char g_szTempMessage[100];

XFONT *g_defaultTrueTypeFont = NULL;

CXBFont m_Font;		// Font	for	IGM
CXBFont m_MSFont;	// Font for buttons

MEMORYSTATUS memStat;

//fd - more debug output
extern int iAudioPlugin;
extern int iRspPlugin;
extern bool FrameSkip;
extern int VSync;
extern int DefaultPak;
extern int iPagingMethod;
extern int AntiAliasMode;

extern char skinname[32];
extern bool showdebug;
extern bool bEnableHDTV;
extern bool bFullScreen;

extern "C" char emuname[256];
extern char emuvidname[128];
char emuresname[128];

int iWindowHeight;
char emuinfo[128];

BOOL bInitOSD = false;
BOOL bIsUltra = false;

void XboxDrawDebugInfo();
void XboxDrawTemporaryMessage();
void XboxDrawLowMemWarning();

void XboxDrawOSD()
{
	if (!bInitOSD) {
		bInitOSD = true;

		char fontname[256];	
		if(bEnableHDTV)
		sprintf(fontname,"%s%s\\IGMHD\\Font.xpr",szPathSkins,skinname);
		else
		sprintf(fontname,"%s%s\\IGM\\Font.xpr",szPathSkins,skinname);
		m_Font.Create(fontname);
		sprintf(fontname,"%s%s\\MsFont.xpr",szPathSkins,skinname);
		m_MSFont.Create(fontname);
		
		DWORD videoFlags = XGetVideoFlags();
		if(XGetAVPack() == XC_AV_PACK_HDTV){
			//720p
			if( videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV) {
				sprintf(emuresname,"720p");
				iWindowHeight = 720;
			}
			//480p
			else if( videoFlags & XC_VIDEO_FLAGS_HDTV_480p) {
				sprintf(emuresname,"480p");
				iWindowHeight = 480;
			}
			//480i
			else {
				sprintf(emuresname,"480i");
				iWindowHeight = 480;
			}
		}else{
			//480i
			sprintf(emuresname,"480i");
			iWindowHeight = 480;
		}

		sprintf(emuinfo,"%s",emuname);
		if (strlen(emuvidname)) {
			char tmp[128];
			sprintf(tmp," %s",emuvidname); // prepend a space
			strcat(emuinfo,tmp);
		}
		if (strlen(emuresname)) {
			char tmp[128];
			sprintf(tmp," %s",emuresname); // prepend a space
			strcat(emuinfo,tmp);
		}
	}

	XboxDrawLowMemWarning();
	
	if (showdebug)
		XboxDrawDebugInfo();
	
	if (g_bTempMessage)
		XboxDrawTemporaryMessage();

	if (!showdebug && !g_bTempMessage)
	{
		switch (AntiAliasMode)	// Revert FSAA from ttf messages
		{
			case 2:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR );		break;
			case 3:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX );	break;
			case 4:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR );		break;
			case 5:		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN );	break;
		}

		g_pd3dDevice->SetBackBufferScale( 1.0f, 1.0f );
	}
	
	// not sure why this fixes it, but it does
	m_Font.Begin();
	m_Font.End();
}

__forceinline void XboxDrawLowMemWarning()
{	
	//Check Memory, Warn User, Return to Launcher
	GlobalMemoryStatus(&memStat);
	if (memStat.dwAvailPhys / 1024 / 1024 < 1)
	{
		WCHAR szMemWarning[128];
		swprintf(szMemWarning,L"Out of Memory! Returning to Launcher...");

		//Clear to save memory 
		g_pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_ARGB(0,0,0,0),0,0);

		// Flick AA off before drawing Temp Messages
		if(AntiAliasMode > 3)		// 4x FSAA  
		{
			g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
			g_pd3dDevice->SetBackBufferScale( 0.5f, 0.5f );
		}
		else if(AntiAliasMode > 1)	// 2x FSAA
		{
			g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
			g_pd3dDevice->SetBackBufferScale( 0.5f, 1.0f );
		}

		// Draw Temp Message
		if (g_defaultTrueTypeFont == NULL)
		{
			XFONT_OpenDefaultFont(&g_defaultTrueTypeFont);
			g_defaultTrueTypeFont->SetBkMode(XFONT_OPAQUE);
			g_defaultTrueTypeFont->SetBkColor(D3DCOLOR_XRGB(0,0,0));
		}

		D3DSurface *pBackBuffer, *pFrontBuffer;

		g_pd3dDevice->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &pFrontBuffer);
		g_pd3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

		g_defaultTrueTypeFont->TextOut(pFrontBuffer, szMemWarning, (unsigned)-1, 30, (iWindowHeight - 50));
		g_defaultTrueTypeFont->TextOut(pBackBuffer, szMemWarning, (unsigned)-1, 30, (iWindowHeight - 50));

		pFrontBuffer->Release();
		pBackBuffer->Release();
		
		g_pd3dDevice->EndScene();
		g_pd3dDevice->Present(NULL,NULL,NULL,NULL);

		//Lock in a loop until its time to exit
		while(1)
		{
			if (GetTickCount() > g_dwTempMessageStart + 1000)
			{
#ifndef DEBUG
				XLaunchNewImage("D:\\default.xbe", NULL);
#else
				OutputDebugString("OSD: WARNING! Low Memory!\n");
				return;
#endif
			}
		}
	}
}

__forceinline void XboxDrawDebugInfo()
{
	WCHAR szMemStatus[128];
	GlobalMemoryStatus(&memStat);
	
	static DWORD lastTick = GetTickCount() / 1000;
	static int lastTickFPS = 0;
	static int frameCount = 0;

	if (lastTick != GetTickCount() / 1000)
	{
		lastTickFPS = frameCount;
		frameCount = 0;
		lastTick = GetTickCount() / 1000;
	}
	frameCount++;
	WCHAR str[128];
	swprintf(str,L"%i fps", lastTickFPS);
	swprintf(szMemStatus,L"%d MB free",(memStat.dwAvailPhys /1024 /1024));
	WCHAR debugemu[256];
	swprintf(debugemu,L"%S",emuinfo);

	//check audio plugins
	WCHAR debugaudio[256];
	switch (iAudioPlugin)
	{
		case _AudioPluginNone:		swprintf(debugaudio,L"Audio Plugin: None");		break;
		case _AudioPluginLleRsp:	swprintf(debugaudio,L"Audio Plugin: LLE RSP");	break;
		case _AudioPluginBasic:		swprintf(debugaudio,L"Audio Plugin: Basic");	break;
		case _AudioPluginJttl:		swprintf(debugaudio,L"Audio Plugin: JttL");		break;
		case _AudioPluginAzimer:	swprintf(debugaudio,L"Audio Plugin: Azimer");	break;
		case _AudioPluginMusyX:		swprintf(debugaudio,L"Audio Plugin: MusyX");	break;
		case _AudioPluginM64p:		swprintf(debugaudio,L"Audio Plugin: Mupen64");	break;
	}

	//check RSP plugins
	WCHAR debugrsp[256];
	switch (iRspPlugin)
	{
		case _RSPPluginNone:	swprintf(debugrsp,L"RSP Plugin: None");		break;
		case _RSPPluginLLE:		swprintf(debugrsp,L"RSP Plugin: LLE");		break;
		case _RSPPluginHLE:		swprintf(debugrsp,L"RSP Plugin: HLE");		break;
		case _RSPPluginM64p:	swprintf(debugrsp,L"RSP Plugin: Mupen64");	break;
	}

	//check Default Paks
	WCHAR debugpak[256];
	switch (DefaultPak)
	{
		case 1:		swprintf(debugpak,L"Current Pak: None");		break;
		case 2:		swprintf(debugpak,L"Current Pak: Memory Pak");	break;
		case 3:		swprintf(debugpak,L"Current Pak: Rumble Pak");	break;
	}

	//Check VSYNC
	WCHAR debugvsync[256];
	switch (VSync)
	{
		case 0:		swprintf(debugvsync,L"VSync: No");		break; //L"VSYNC: INTERVAL_IMMEDIATE");
		case 1:		swprintf(debugvsync,L"VSync: Yes");		break; //L"VSYNC: INTERVAL_ONE");
		case 2:		swprintf(debugvsync,L"VSync: Auto");	break; //L"VSYNC: INTERVAL_ONE_OR_IMMEDIATE");
	}

	//Check FrameSkip
	WCHAR debugframeskip[256];
	if(FrameSkip){
		swprintf(debugframeskip,L"Skip Frames: Yes");
	}else{
		swprintf(debugframeskip,L"Skip Frames: No");
	}

	WCHAR debugpaging[256];
	switch (iPagingMethod)
	{
		case _PagingXXX: 	swprintf(debugpaging,L"Page: XXX"); break;
		case _PagingS10: 	swprintf(debugpaging,L"Page: 1.0"); break;
	}
	
	WCHAR debugaa[256];
	switch (AntiAliasMode)
	{
		case 0:		swprintf(debugaa,L"AntiAliasing: None");		break;
		case 1:		swprintf(debugaa,L"AntiAliasing: Edge");		break;
		case 2:		swprintf(debugaa,L"AntiAliasing: 2x Linear");	break;
		case 3:		swprintf(debugaa,L"AntiAliasing: 2x Quincunx");	break;
		case 4:		swprintf(debugaa,L"AntiAliasing: 4x Linear");	break;
		case 5:		swprintf(debugaa,L"AntiAliasing: 4x Gaussian");	break;
	}
	
	if (bIsUltra) {
		swprintf(debugaudio,		L"Audio Plugin: NA");
		swprintf(debugrsp,			L"RSP Plugin: NA");
		swprintf(debugframeskip,	L"Skip Frames: NA");
		swprintf(debugpaging,		L"Page: NA");
	}

	// Flick AA off before drawing Debug Info
	if(AntiAliasMode > 3)		// 4x FSAA
	{
		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
		g_pd3dDevice->SetBackBufferScale( 0.5f, 0.5f );
	}
	else if(AntiAliasMode > 1)	// 2x FSAA
	{
		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
		g_pd3dDevice->SetBackBufferScale( 0.5f, 1.0f );
	}
	
	if (g_defaultTrueTypeFont == NULL)
	{
		XFONT_OpenDefaultFont(&g_defaultTrueTypeFont);
		g_defaultTrueTypeFont->SetBkMode(XFONT_TRANSPARENT);
		g_defaultTrueTypeFont->SetBkColor(D3DCOLOR_ARGB(100,0,0,0));
	}

	D3DSurface *pBackBuffer, *pFrontBuffer;

	g_pd3dDevice->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &pFrontBuffer);
	g_pd3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

	g_defaultTrueTypeFont->SetTextHeight(14);
	g_defaultTrueTypeFont->SetTextAntialiasLevel(g_defaultTrueTypeFont->GetTextAntialiasLevel());

	for(int NumDebugLines = 1; NumDebugLines <= DEBUGLINES; NumDebugLines++){

		WCHAR buf[200];
		
		switch(NumDebugLines)
		{
			case 1	: swprintf(buf, szMemStatus);		break;
			case 2	: swprintf(buf,	str);				break;
			case 3	: swprintf(buf, debugemu);			break;
			case 4	: swprintf(buf, debugaudio);		break;
			case 5	: swprintf(buf, debugrsp);			break;
			case 6	: swprintf(buf, debugpak);			break;
			case 7	: swprintf(buf, debugvsync);			break;
			case 8	: swprintf(buf, debugframeskip);		break;
			case 9	: swprintf(buf, debugpaging);		break;
			case 10	: swprintf(buf, debugaa);			break;
		}

		g_defaultTrueTypeFont->TextOut(pFrontBuffer, buf, (unsigned)-1, 30, ((DEBUGSPACING * NumDebugLines) + 15 ) );
		g_defaultTrueTypeFont->TextOut(pBackBuffer,	 buf, (unsigned)-1, 30, ((DEBUGSPACING * NumDebugLines) + 15 ) );

	}

	pFrontBuffer->Release();
	pBackBuffer->Release();

}

__forceinline void XboxDrawTemporaryMessage()
{
	// Flick AA off before drawing Temp Messages
	if(AntiAliasMode > 3)		// 4x FSAA  
	{
		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
		g_pd3dDevice->SetBackBufferScale( 0.5f, 0.5f );
	}
	else if(AntiAliasMode > 1)	// 2x FSAA
	{
		g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEMODE, D3DMULTISAMPLEMODE_1X );
		g_pd3dDevice->SetBackBufferScale( 0.5f, 1.0f );
	}

	if (g_defaultTrueTypeFont == NULL)
	{
		XFONT_OpenDefaultFont(&g_defaultTrueTypeFont);
		g_defaultTrueTypeFont->SetBkMode(XFONT_OPAQUE);
		g_defaultTrueTypeFont->SetBkColor(D3DCOLOR_XRGB(0,0,0));
	}

	if (GetTickCount() > g_dwTempMessageStart + 3000)
	{
		g_bTempMessage = FALSE;
	}

	WCHAR buf[200];
	D3DSurface *pBackBuffer, *pFrontBuffer;

	memset(buf, 0, sizeof(WCHAR) * 200);
	


	g_pd3dDevice->GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, &pFrontBuffer);
	g_pd3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

	mbstowcs(buf, g_szTempMessage, strlen(g_szTempMessage));

	g_defaultTrueTypeFont->TextOut(pFrontBuffer, buf, (unsigned)-1, 30, (iWindowHeight - 50));
	g_defaultTrueTypeFont->TextOut(pBackBuffer, buf, (unsigned)-1, 30, (iWindowHeight - 50));

	pFrontBuffer->Release();
	pBackBuffer->Release();

}