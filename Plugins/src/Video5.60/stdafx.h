// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D745D35D_8415_4821_9789_CD847CAF9B48__INCLUDED_)
#define AFX_STDAFX_H__D745D35D_8415_4821_9789_CD847CAF9B48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define EXPORT
#include <stdio.h>
#include <xtl.h>
#include <math.h>			// For sqrt()
#include <iostream>

#if _MSC_VER > 1200
#include "xmmintrin.h"
#endif


#include <process.h>

#include <d3d8.h>
#include <d3dx8.h>
#include <d3d8types.h>
#include <D3dx8math.h>

#include <vector>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_CHECK
#define SAFE_CHECK(a)	if( (a) == NULL ) {ErrorMsg("Creater out of memory"); throw new exception();}
#endif

#include "typedefs.h"
#include "gfx.h"
#include "RiceVideo.h"
#include "Config.h"
#include "resource.h"
#include "Debugger.h"
#include "RDP_S2DEX.h"
#include "DLParser.h"

#include "TextureCache.h"
#include "ConvertImage.h"
#include "Texture.h"
#include "DirectXTexture.h"
//#include "OGLTexture.h"

#include "Profiler.h"

#include "CombinerDefs.h"
#include "DecodedMux.h"
//#include "OGLDecodedMux.h"
#include "DirectXDecodedMux.h"

#include "blender.h"
#include "DirectXBlender.h"


#include "combiner.h"
#include "GeneralCombiner.h"
#include "DirectXCombiner.h"
#include "CNvTNTCombiner.h"
#include "CNvTNTDirectXCombiner.h"
#include "DirectXCombinerVariants.h"

//#include "gl/gl.h"
//#include "glext.h"

#include "GraphicsContext.h"
#include "DXGraphicsContext.h"
#include "DeviceBuilder.h"
//#include "OGLGraphicsContext.h"

//#include "OGLCombiner.h"
//#include "OGLExtCombiner.h"
//#include "OGLCombinerNV.h"
//#include "OGLCombinerTNT2.h"

#include "TextureCache.h"
#include "RenderBase.h"
#include "ExtendedRender.h"
#include "Render.h"
#include "D3DRender.h"
//#include "OGLRender.h"
//#include "OGLExtRender.h"

#include "resource.h"

#include "profiler.h"

#include "icolor.h"

#include "CSortedList.h"
#include "CritSect.h"
#include "Timing.h"

extern WindowSettingStruct windowSetting;

void __cdecl MsgInfo (char * Message, ...);
void __cdecl ErrorMsg (char * Message, ...);

#define MI_INTR_DP          0x00000020  

extern DWORD g_dwRamSize;

extern DWORD * g_pRDRAMu32;
extern signed char* g_pRDRAMs8;
extern unsigned char *g_pRDRAMu8;

extern GFX_INFO g_GraphicsInfo;


//#define ENABLE_PROFILER

#ifdef ENABLE_PROFILER
#define StartProfiler(item)	CProfiler::Get()->StartTiming(item)
#define StopProfiler(item)	CProfiler::Get()->StopTiming(item)
#else
#define StartProfiler(item)
#define StopProfiler(item)
#endif

//#include "ResourceString.h"		// Useful everywhere
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D745D35D_8415_4821_9789_CD847CAF9B48__INCLUDED_)
