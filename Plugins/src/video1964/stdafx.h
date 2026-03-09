// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(_STDAFX_H_)
#define _STDAFX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _XBOX
#define EXPORT
#else
#define EXPORT				__declspec(dllexport)
#endif
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0400
#define WINVER 0x0400
#include <stdio.h>

#ifdef _XBOX
#include <xtl.h>
#include "..\..\..\Include\xbox_depp.h"

#else //win32
#include <windows.h>
#include <windowsx.h>			// Button_* etc
#include <shlwapi.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <ShellAPI.h>
#include <winnt.h>			// For 32x32To64 multiplies
#endif //_XBOX

#include <math.h>			// For sqrt()
#include <iostream>
#include <fstream>
#include <istream>

#if _MSC_VER > 1200
#include "xmmintrin.h"
#endif


#include <process.h>

#if defined(_XBOX) || defined(USEDX8)
#define DX_VERSION 8
#else
#define DX_VERSION 9
#endif

#if DX_VERSION == 8
#include <d3d8.h>
#include <d3dx8.h>
#include <d3d8types.h>
#include <D3dx8math.h>
#include <xgraphics.h>
#elif DX_VERSION == 9
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>
#include <D3dx9math.h>
#include <dxerr9.h> // Used on googlecode, fix for debug info?
#endif

// DirectX Common Names - staying consistent with 1964video on googlecode for potential fixes

#if DX_VERSION == 8
#define MYLPDIRECT3DTEXTURE			LPDIRECT3DTEXTURE8
#define MYD3DADAPTER_IDENTIFIER		D3DADAPTER_IDENTIFIER8 //D3DADPTER_IDENTIFIER8
#define MYIDirect3DSurface			IDirect3DSurface8
#define MYLPDIRECT3DBASETEXTURE		LPDIRECT3DBASETEXTURE8
#define MYLPDIRECT3DSURFACE			LPDIRECT3DSURFACE8
#define MYD3DVIEWPORT				D3DVIEWPORT8
#define MYD3DCAPS					D3DCAPS8
#define MYLPDIRECT3DDEVICE			LPDIRECT3DDEVICE8
#define MYLPDIRECT3D				LPDIRECT3D8
#define MYIDirect3DBaseTexture		IDirect3DBaseTexture8 //MYIDirect3DBaseTexture

#elif DX_VERSION == 9
#define MYLPDIRECT3DTEXTURE			LPDIRECT3DTEXTURE9
#define MYD3DADAPTER_IDENTIFIER		D3DADAPTER_IDENTIFIER9
#define MYIDirect3DSurface			IDirect3DSurface9
#define MYLPDIRECT3DBASETEXTURE		LPDIRECT3DBASETEXTURE9
#define MYLPDIRECT3DSURFACE			LPDIRECT3DSURFACE9
#define MYD3DVIEWPORT				D3DVIEWPORT9
#define MYD3DCAPS					D3DCAPS9
#define MYLPDIRECT3DDEVICE			LPDIRECT3DDEVICE9
#define MYLPDIRECT3D				LPDIRECT3D9
#define MYIDirect3DBaseTexture		IDirect3DBaseTexture9
#define D3DRS_ZBIAS					D3DRS_DEPTHBIAS

#endif // DX_VERSION

#include <vector>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_CHECK
#define SAFE_CHECK(a)	if( (a) == NULL ) {ErrorMsg("Creater out of memory"); /*throw new exception();*/}
#endif

#include "typedefs.h"
#include "gfx.h"
#include "Video.h"
#include "Config.h"
#include "resource.h"
#include "Debugger.h"
#include "RSP_S2DEX.h"
#include "RSP_Parser.h"

#include "TextureManager.h"
#include "ConvertImage.h"
#include "Texture.h"
#include "DirectXTexture.h"
#include "OGLTexture.h"

#include "CombinerDefs.h"
#include "DecodedMux.h"
#include "OGLDecodedMux.h"
#include "DirectXDecodedMux.h"

#include "blender.h"
#include "DirectXBlender.h"


#include "combiner.h"
#include "GeneralCombiner.h"
#include "DirectXCombiner.h"

#include "DirectXCombinerVariants.h"

#ifndef _XBOX //win32
// TNT
#include "CNvTNTCombiner.h"
#include "CNvTNTDirectXCombiner.h"
// OGL
#include "gl/gl.h"
#include "glext.h"
#endif //_XBOX //win32

#include "GraphicsContext.h"
#include "DXGraphicsContext.h"
#include "DeviceBuilder.h"
#ifndef _XBOX //win32
#include "OGLGraphicsContext.h"

#include "OGLCombiner.h"
#include "OGLExtCombiner.h"
#include "OGLCombinerNV.h"
#include "OGLCombinerTNT2.h"
#endif //_XBOX //win32

#include "RenderBase.h"
#include "ExtendedRender.h"
#include "Render.h"
#include "D3DRender.h"
#ifndef _XBOX //win32
#include "OGLRender.h"
#include "OGLExtRender.h"
#endif //_XBOX //win32

#include "resource.h"

#include "icolor.h"

#include "CSortedList.h"
#include "CritSect.h"
#include "Timing.h"

extern WindowSettingStruct windowSetting;

void __cdecl MsgInfo (char * Message, ...);
void __cdecl ErrorMsg (char * Message, ...);

#define MI_INTR_DP          0x00000020  
#define MI_INTR_SP          0x00000001  

extern uint32 g_dwRamSize;

extern uint32 * g_pRDRAMu32;
extern signed char* g_pRDRAMs8;
extern unsigned char *g_pRDRAMu8;

extern GFX_INFO g_GraphicsInfo;


extern char *project_name;

#endif // _STDAFX_H_
