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

#ifndef _DLLINTERFACE_H_
#define _DLLINTERFACE_H_

#include "typedefs.h"

typedef struct {
	HINSTANCE myhInst;
	
	float	fViWidth;
	float	fViHeight;

	unsigned __int16		uViWidth;
	unsigned __int16		uViHeight;
	
	unsigned __int16		uDisplayWidth;
	unsigned __int16		uDisplayHeight;

	unsigned __int16		uFullScreenDisplayWidth;
	unsigned __int16		uFullScreenDisplayHeight;
	
	unsigned __int16		uWindowDisplayWidth;
	unsigned __int16		uWindowDisplayHeight;
	
	BOOL	bDisplayFullscreen;

	int		uFullScreenFrequency;

	float	fMultX, fMultY;
	int		vpLeftW, vpTopW, vpRightW, vpBottomW, vpWidthW, vpHeightW;

}WindowSettingStruct;

extern WindowSettingStruct windowSetting;

typedef enum 
{
	PRIM_TRI1,
	PRIM_TRI2,
	PRIM_TRI3,
	PRIM_DMA_TRI,
	PRIM_LINE3D,
	PRIM_TEXTRECT,
	PRIM_TEXTRECTFLIP,
	PRIM_FILLRECT,
} PrimitiveType;

typedef struct {
	bool	bGameIsRunning;

	BOOL	frameReadByCPU;
	BOOL	frameWriteByCPU;

	DWORD	SPCycleCount;		// Count how many CPU cycles SP used in this DLIST
	DWORD	DPCycleCount;		// Count how many CPU cycles DP used in this DLIST

	DWORD	dwNumTrisRendered;
	DWORD	dwNumDListsCulled;
	DWORD	dwNumTrisClipped;
	DWORD	dwNumVertices;

	DWORD	gRDPFrame;
	DWORD	gRDPTime;
	DWORD	gTexUcodeCount;
	BOOL	bRDPHalted;

	bool	useModifiedMap;
	bool	ucodeHasBeenSet;
	BOOL	bDetermineduCode;

	DWORD	curRenderBuffer;
	DWORD	curDisplayBuffer;
	DWORD	curVIOriginReg;

	PrimitiveType primitiveType;

	DWORD	lastPurgeTimeTime;		// Time textures were last purged

	bool	UseLargerTile[2];		// This is a speed up for large tile loading,
	DWORD	LargerTileRealLeft[2];	// works only for TexRect, LoadTile, large width, large pitch

	bool	bVIOriginIsUpdated;
	bool	bCIBufferIsRendered;

	bool	isMMXSupported;
	bool	isSSESupported;

	bool	isMMXEnabled;
	bool	isSSEEnabled;

	bool	toShowCFB;
	bool	toCaptureScreen;
	char	screenCaptureFilename[MAX_PATH];

	// Frame buffer simulation related status variables
	bool	bN64FrameBufferIsUsed;		// Frame buffer is used in the frame
	bool	bN64IsDrawingTextureBuffer;	// The current N64 game is rendering into texture buffer, to create self-rendering texture
	bool	bHandleN64TextureBuffer;	// Do we need to handle of the N64 texture buffer stuff?
	bool	bDirectWriteIntoRDRAM;		// When drawing into texture buffer, this value =
										// = true	don't render, but write real N64 graphic value into RDRAM
										// = false  rendering into texture buffer of DX or OGL, the texture buffer
										//			will be copied into RDRAM at the end
	bool	bTextureBufferIsDrawn;		// flag to mark if the texture buffer is ever drawn
	bool	bTextureBufferDrawnByTriangles;	// flag to tell if the buffer is even drawn by Triangle cmds

} PluginStatus;

extern PluginStatus status;


#endif
