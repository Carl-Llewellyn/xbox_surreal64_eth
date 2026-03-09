////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, Epsilon and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// ultrahle.c


#include "ultrahle.h"
#include "Controller.h"
#include "IoSupport.h"
#include "../config.h"

#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>

#include <xbapp.h>
#include <xbresource.h>

// GogoAckman config file
#include "../config.h"

void __EMU_LoadState(int index);
void __EMU_SaveState(int index);

extern "C" void _INPUT_LoadButtonMap(int *cfgData);

// weinersch UHLE AntiAliasing? I think so!
DWORD SetAntiAliasMode(int AAMode);
extern int AntiAliasMode;

char szBuffer[ MAX_PATH ];             
HANDLE hInst;                          
HWND hwndMain;                         
HWND hwndStatus;              

HANDLE mainthread;                     
LPDWORD mainthreadid;
D3DPRESENT_PARAMETERS d3dpp; 

bool g_exitCmd = false;
BOOL menuWaiting = FALSE;


IDirect3D8				*g_pDirect3D;	
IDirect3DDevice8		*g_pDevice;
IDirect3DVertexBuffer8  *g_pVertexBuffer;

//extern bool g_HD_UHLE_720;

// OSD
extern BOOL bIsUltra;
char emuvidname[128]; // none
extern "C" {
char emuname[256];
}

//fd - undef for 1080i WARNING! SLOWDOWN! IGM CRASH!
//also needs to be enabled in your dashboard!
//#define TEST_HD_UHLE_1080

HRESULT Direct3DInit();

int emuthread(int value)
{
    main_thread();
	g_exitCmd = true;
    return(0);
}

HRESULT Direct3DInit()
{
    // Create the D3D object.
    if( NULL == ( g_pDirect3D = Direct3DCreate8( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice.
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.BackBufferWidth        = 640;
    d3dpp.BackBufferHeight       = 480;
    d3dpp.BackBufferFormat       = D3DFMT_A8R8G8B8;
    d3dpp.BackBufferCount        = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.MultiSampleType		 = SetAntiAliasMode(AntiAliasMode);
	switch (VSync){
		case 0 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			break;
		case 1 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			break;
		case 2 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
			break;
/*		case 2 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_TWO;
			break;
		case 3 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_THREE;
			break;
		case 4 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
			break;
		case 5 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
			break;
		case 6 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_TWO_OR_IMMEDIATE;
			break;
		case 7 : 	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_THREE_OR_IMMEDIATE;
			break;*/
	}
	d3dpp.FullScreen_RefreshRateInHz = 60;


	DWORD videoFlags = XGetVideoFlags();
	if(XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I)
	{
		if(videoFlags & XC_VIDEO_FLAGS_PAL_60Hz)		// PAL 60 user
			d3dpp.FullScreen_RefreshRateInHz = 60;
		else
			d3dpp.FullScreen_RefreshRateInHz = 50;
	}

	 if((videoFlags & XC_VIDEO_FLAGS_WIDESCREEN) !=0)
	 {
		d3dpp.Flags = D3DPRESENTFLAG_WIDESCREEN;
	 }

	  if(XGetAVPack() == XC_AV_PACK_HDTV)
	  {		
				//1080i
			#ifdef TEST_HD_UHLE_1080
				if(videoFlags & XC_VIDEO_FLAGS_HDTV_1080i)
                {
                        d3dpp.Flags            = D3DPRESENTFLAG_WIDESCREEN | D3DPRESENTFLAG_INTERLACED;
                        d3dpp.BackBufferWidth  = 1920;
                        d3dpp.BackBufferHeight = 1080;
						d3dpp.BackBufferWidth  = init.gfxwid = 1920;
                        d3dpp.BackBufferHeight = init.gfxhig = 1080;
                }
				else
			#endif
				//720p
                 if((videoFlags & XC_VIDEO_FLAGS_HDTV_720p)&& bEnableHDTV)
                {
                        
							d3dpp.Flags            = D3DPRESENTFLAG_PROGRESSIVE | D3DPRESENTFLAG_WIDESCREEN;
							d3dpp.BackBufferWidth  = init.gfxwid = 1280;
							d3dpp.BackBufferHeight = init.gfxhig = 720;
                }

				//480p
				else if(videoFlags & XC_VIDEO_FLAGS_HDTV_480p)
                {
                        d3dpp.Flags            = D3DPRESENTFLAG_PROGRESSIVE;
                        d3dpp.BackBufferWidth  = init.gfxwid = 640;
                        d3dpp.BackBufferHeight = init.gfxhig = 480;
                }
	  }

//The user can now choose between different vertex processing modes - freakdave

	if(VertexMode == 0){

    // Create the Direct3D device.
    if( FAILED( g_pDirect3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
                                      D3DCREATE_PUREDEVICE,
                                      &d3dpp, &g_pDevice ) ) )
        return E_FAIL;
	}


	else if(VertexMode == 1){

    // Create the Direct3D device.
    if( FAILED( g_pDirect3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pDevice ) ) )
        return E_FAIL;
	}


	else if(VertexMode == 2){

    // Create the Direct3D device.
    if( FAILED( g_pDirect3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pDevice ) ) )
        return E_FAIL;
	}

	else if(VertexMode == 3){

    // Create the Direct3D device.
    if( FAILED( g_pDirect3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
                                      D3DCREATE_MIXED_VERTEXPROCESSING,
                                      &d3dpp, &g_pDevice ) ) )
        return E_FAIL;
	}

	// setup the view matrices
	D3DXMATRIX mat;
	
	//fd : Aspect ratio
	if(!bEnableHDTV){
		//4:3 for SD
		D3DXMatrixPerspectiveFovLH(&mat, D3DXToRadian(90.0f), (float)(init.gfxwid/init.gfxhig), 0.9f, 32768.0f);
	}else{
		//4:3 for HD
		D3DXMatrixPerspectiveFovLH(&mat, D3DXToRadian(90.0f), (float)(960/720), 0.9f, 32768.0f);
	}
	g_pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&mat);

	D3DXMatrixIdentity(&mat);
	g_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&mat);

	D3DXMatrixTranslation(&mat, 0, 0, 0);
	g_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&mat);
	
    g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// freakdave - edited by GogoAckman - reedited by freakdave
	g_pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, TextureMode);
	g_pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, TextureMode);


	g_pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	g_pDevice->SetRenderState( D3DRS_SHADEMODE,  D3DSHADE_GOURAUD );
	g_pDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(0, 0, 0, 0));

	g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	g_pDevice->CreateVertexBuffer(sizeof(CVertex)*4, NULL, D3DFVF_CVertex, D3DPOOL_MANAGED, &g_pVertexBuffer);

#ifdef _XBOX

	if(AntiAliasMode>1){
		g_pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , TRUE);
	}else if(AntiAliasMode==1){
		g_pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , FALSE);
		g_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , TRUE);
		g_pDevice->SetRenderState( D3DRS_EDGEANTIALIAS , TRUE);
	}else{
		g_pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , FALSE);
	}

#endif


	return S_OK;
}

extern int ControllerConfig[76];
extern bool bloadstate[MAX_SAVE_STATES];
extern bool bsavestate[MAX_SAVE_STATES];
extern bool bSatesUpdated;
//extern bool onhd;
extern "C" void loadinis();

char g_szPathSaves[256] = "D:\\Saves\\";
extern "C" void GetPathSaves(char *pszPathSaves);

#ifndef DEFAULT_MAX
extern "C" int loaddwUltraCodeMem();
extern "C" int loaddwUltraGroupMem();
#endif

extern "C" int loadbAudioBoost();

void __cdecl main()
{
	// mount the common drives
	g_IOSupport.Mount("A:","cdrom0");
	g_IOSupport.Mount("E:","Harddisk0\\Partition1");
	//g_IOSupport.Mount("C:","Harddisk0\\Partition2");
	//g_IOSupport.Mount("X:","Harddisk0\\Partition3");
	//g_IOSupport.Mount("Y:","Harddisk0\\Partition4");
	//g_IOSupport.Mount("Z:","Harddisk0\\Partition5");
	g_IOSupport.Mount("F:","Harddisk0\\Partition6");
	g_IOSupport.Mount("G:","Harddisk0\\Partition7");
	
	// utility shoud be mounted automatically
	if(XGetDiskSectorSize("Z:\\") == 0)
		g_IOSupport.Mount("Z:","Harddisk0\\Partition5");

	init.gfxwid = 640;
    init.gfxhig = 480;

	loadinis();
	
	g_bAudioBoost = (loadbAudioBoost() == 1 ? true : false); // set before audio init
	
#ifndef DEFAULT_MAX
	{
		COMPILER_CODEMAX = (loaddwUltraCodeMem() * 1024 * 1024); //0x500000;
		
		char buffer[64];
		sprintf(buffer, "%i", (loaddwUltraGroupMem() * 1000));
		COMPILER_GROUPMAX = strtoul(buffer, NULL, 16); //0x10000;
	}
#endif

	_INPUT_LoadButtonMap(ControllerConfig);

    // Initialize Direct3D
    if( FAILED( Direct3DInit() ) )
        return;

	g_pd3dDevice = g_pDevice;

	sprintf(emuname,"UltraHLE");
	bIsUltra = true; // tell the OSD it's ultra - no audio/video plugins etc
	
	GetPathSaves(g_szPathSaves);

	main_startup();

    int mainthreadid;
    mainthread = CreateThread(NULL, 0,
							 (LPTHREAD_START_ROUTINE)emuthread,
							  NULL, 0, (LPDWORD)&mainthreadid);

	bool loaded = false;

	CHAR cmd[MAX_PATH];
	
	// make sure there's a temp rom
	if (PathFileExists("Z:\\TemporaryRom.dat")) {
		OutputDebugString("Z:\\TemporaryRom.dat File Found!\n");
		strcpy(cmd, "rom Z:\\TemporaryRom.dat");
	}
	else {
		OutputDebugString("Z:\\TemporaryRom.dat File Not Found!\n");
		
		// if debugging, a temp rom can be placed in T to skip the launcher
		if (PathFileExists("T:\\Data\\TemporaryRom.dat")) {
			OutputDebugString("T:\\Data\\TemporaryRom.dat File Found!\n");
			strcpy(cmd, "rom T:\\Data\\TemporaryRom.dat");
		}
		else {
			OutputDebugString("T:\\Data\\TemporaryRom.dat File Not Found!\n");
			Sleep(100);
			XLaunchNewImage("D:\\default.xbe", NULL);
		}
	}
	
	while(TRUE)
	{
#ifdef DEBUG
		char buf[100];
		MEMORYSTATUS stat;
		GlobalMemoryStatus(&stat);
		sprintf(buf, "\n%.2fMB of RAM available\n", (float)stat.dwAvailPhys / 1024.0f / 1024.0f);
		OutputDebugString(buf);
#endif
		Sleep(1000);

		if (bSatesUpdated) {
			bSatesUpdated = false;
			
			for (int i=0; i<MAX_SAVE_STATES; i++) {
				if (bloadstate[i]) {
					__EMU_LoadState(i+1);
					bloadstate[i]=false;
					break;
				}
				else if (bsavestate[i]) {
					__EMU_SaveState(i+1);
					bsavestate[i]=false;
					break;
				}
			}
		}
		if (!loaded)
		{
			main_command(cmd);
			if(main_commanderrors()>1)				break;
			else  main_start();
			loaded = true;
			
			{ // create the save directory if it doesn't exist
				char szPathSaves[_MAX_PATH];
				sprintf(szPathSaves, "%s%08x", g_szPathSaves, cart.crc1);
				if (!PathFileExists(szPathSaves)) {
					if (!CreateDirectory(szPathSaves, NULL)) {
						OutputDebugString(szPathSaves);
						OutputDebugStringA(" Could Not Be Created!\n");
					}
				}
			}
		}

// Menu moved to RDP.cpp , more fast :) - GogoAckman

		if (g_exitCmd)
		{
			//LAUNCH_DATA ld;
			//ZeroMemory(&ld, sizeof(LAUNCH_DATA));

			Controller_Finalise();
			
			//XLaunchNewImage("D:\\default.xbe", &ld);
			XLaunchNewImage("D:\\default.xbe", NULL);
			break;
		}
	}
}

void __EMU_SaveState(int index)
{
	CHAR stateFileName[255];
	CHAR tmp[350];
	sprintf(stateFileName, "%s%08X\\%08X-%08X-%02X.sav%i", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country, index);

	sprintf(tmp, "save %s", stateFileName);
	main_command(tmp);
	if(main_commanderrors()>1)
	{
		g_exitCmd = true;
	}
	else
	{
		main_start();
	}
}

void __EMU_LoadState(int index)
{
	CHAR stateFileName[255];
	CHAR tmp[350];
	sprintf(stateFileName, "%s%08X\\%08X-%08X-%02X.sav%i", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country, index);

	sprintf(tmp, "load %s", stateFileName);
	main_command(tmp);
	if(main_commanderrors()>1)
	{
		g_exitCmd = true;
	}
	else
	{
		main_start();
	}
}

void __EMU_GetStateFilename(int index, char *filename, int mode)
{
	if(mode == 0){
		sprintf(filename, "%s%08X\\%08X-%08X-%02X.sav%i", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country, index);
	}
	else if(mode == 1){
		sprintf(filename, "%s%08X\\%08X-%08X-%02X.sav%i.bmp", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country, index);
	}
	return;
}


//-----------------------------------------------------------------------------
// Name: SetAntiAliasMode()
// Desc: Surreal64 function to set the antialiasing mode determined by the
//       Launcher. Edge AntiAliasing may work better on the xbox compared to
//	     the FSAA modes. 4x Gaussian is the reccomended then to 2x Quincunx if 
//	     the framerate plunges. Linear modes are also available if those
//	     methods are prefered.
//-----------------------------------------------------------------------------
DWORD SetAntiAliasMode(int AAMode){
	DWORD useAAMode;
	switch (AAMode)
	{
		case 0:
			useAAMode = D3DMULTISAMPLE_NONE;
		break;

		case 1:
			useAAMode = D3DMULTISAMPLE_NONE;
		break;

		case 2:
			useAAMode = D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR;
		break;

		case 3:
			useAAMode = D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX;
		break;

		case 4:
			useAAMode = D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR;
		break;

		case 5:
			useAAMode = D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN;
		break;
	}
	return useAAMode;
}