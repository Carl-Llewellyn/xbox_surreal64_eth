////////////////////////////////////////////////////////////////////////////////
// UltraHLE - Ultra64 High Level Emulator
// Copyright (c) 1999, XXX and RealityMan
// THIS IS A PRIVATE NONPUBLIC VERSION. NOT FOR PUBLIC DISTRIBUTION!
// ultrahle.h

#pragma once

#include "stdsdk.h"                    // Standard Win32 API Includes, etc.
#include "version.h"                   // Version Information
#include "main.h"

   // Prototypes

double readdouble(int reg);
void writedouble(int reg,double value);
void c_execop( dword opcode );
void c_exec( void );

   //BOOL CALLBACK AboutDialog( HWND, UINT, WPARAM, LPARAM );
   //int ControllerProperties( void );
   //BOOL APIENTRY Controller1Prop( HWND, UINT, UINT, LONG );

   //extern void CreateListView( void );
   //extern BOOL UpdateROMList( void );
  // extern BOOL LoadImageState( BOOL );
  // extern BOOL SaveState( void );

   // Globals

   //extern char szBuffer[ MAX_PATH ];             // Temporary String Buffer
   //extern HANDLE hInst;                          // Global Application Instance
   //extern HWND hwndMain;                         // Handle to the Main App Window
   //extern HWND hwndStatus;                       // Handle to Status Bar

   extern HANDLE mainthread;                     
   extern LPDWORD mainthreadid;

   extern IDirect3D8		*g_pDirect3D;	// Pointer to Direct3D
   extern IDirect3DDevice8	*g_pDevice;		// Pointer to the Direct3D device
   extern IDirect3DVertexBuffer8 *g_pVertexBuffer;
   //extern IDirect3DVertexBuffer8 *g_pProcessedVertexBuffer;

	//extern HWND hwndList;                  // Handle to Rom List View
	//extern HWND hwndDebug;                 // Handle to Debug List View
   //extern ROMLIST *romList;               // Pointer to Rom List Information

	extern BOOL menuWaiting;
	extern bool g_exitCmd;

    extern Init init;

	extern DWORD  m_xiInsertions;
	extern DWORD  m_xiRemovals;
	extern HANDLE m_hXPort[4];

const DWORD D3DFVF_CVertex = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2;

// our vertex class
class CVertex
{
public:
	// Its coordinates
	float fX, fY, fZ;

	// Its colour
	DWORD dwColor;

	// 1st texture coordinates
	float fU, fV;

	// 2nd texture coordinates
	float fU2, fV2;

	// A function to simplify initialization
	void Create(float fp_fX, float fp_fY, float fp_fZ, 
				float fp_fR, float fp_fG, float fp_fB, float fp_fA,
				float fp_fU, float fp_fV, 
				float fp_fU2, float fp_fV2)
	{
		fX = fp_fX;
		fY = fp_fY;
		fZ = fp_fZ;

		dwColor = D3DCOLOR_ARGB((BYTE)(fp_fA*255), (BYTE)(fp_fR*255), (BYTE)(fp_fG*255), (BYTE)(fp_fB*255));

		fU = fp_fU;
		fV = fp_fV;

		fU2 = fp_fU2;
		fV2 = fp_fV2;
	}
};

// Our processed vertex class
class CProcessedVertex
{
public:
	// Processed position
	float fX, fY, fZ, fRHW;
	
	// Its colour
	DWORD dwColor;

	// 1st texture coordinates
	float fU, fV;

	// 2nd texture coordinates
	float fU2, fV2;
};
// The CProcessedVertex FVF

const DWORD D3DFVF_CProcessedVertex=D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2;

extern "C" void __EMU_GetStateFilename(int index, char *filename, int mode);

extern char g_szPathSaves[256];