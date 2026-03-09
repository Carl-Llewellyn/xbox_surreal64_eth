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

// SP ucode for GBI0

void DLParser_GBI1_SpNoop(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_SpNoop);

	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	if( dwCmd2>>24 == 0x00 )
	{
		RDP_GFX_PopDL();
		RDP_NOIMPL("Double SPNOOP, Skip remain ucodes, PC=%08X, Cmd1=%08X", dwPC, dwCmd1);
	}
}



void DLParser_GBI0_Mtx(DWORD dwCmd0, DWORD dwCmd1)
{	
	SP_Timing(DLParser_GBI0_Mtx);

	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwCommand = (dwCmd0>>16)&0xFF;
	DWORD dwLength  = (dwCmd0)    &0xFFFF;

	DL_PF("    Command: %s %s %s Length %d Address 0x%08x",
		(dwCommand & G_MTX_PROJECTION) ? "Projection" : "ModelView",
		(dwCommand & G_MTX_LOAD) ? "Load" : "Mul",	
		(dwCommand & G_MTX_PUSH) ? "Push" : "NoPush",
		dwLength, dwAddress);

	if (dwAddress + 64 > g_dwRamSize)
	{
		TRACE1("Mtx: Address invalid (0x%08x)", dwAddress);
		return;
	}

	// Load matrix from dwAddress
	DaedalusMatrix mat;
	DWORD dwI;
	DWORD dwJ;
	const float fRecip = 1.0f / 65536.0f;
	
	for (dwI = 0; dwI < 4; dwI++) {
		for (dwJ = 0; dwJ < 4; dwJ++) {

			SHORT nDataHi = *(SHORT *)(g_pu8RamBase + ((dwAddress+(dwI<<3)+(dwJ<<1)     )^0x2));
			WORD  nDataLo = *(WORD  *)(g_pu8RamBase + ((dwAddress+(dwI<<3)+(dwJ<<1) + 32)^0x2));

			mat.m[dwI][dwJ] = (float)(((LONG)nDataHi<<16) | (nDataLo))*fRecip;
		}
	}

	LONG nLoadCommand = dwCommand & G_MTX_LOAD ? RENDER_LOAD_MATRIX : RENDER_MUL_MATRIX;
	BOOL bPush = dwCommand & G_MTX_PUSH ? TRUE : FALSE;

#ifdef USING_INT_MATRIX
	N64IntMatrix intmtx((DWORD)(g_pu8RamBase+dwAddress));
#endif

	if (dwCommand & G_MTX_PROJECTION)
	{
		// So far only Extreme-G seems to Push/Pop projection matrices	
#ifdef USING_INT_MATRIX
		CDaedalusRender::g_pRender->SetProjection(mat, intmtx, bPush, nLoadCommand);
#else
		CDaedalusRender::g_pRender->SetProjection(mat, bPush, nLoadCommand);
#endif
	}
	else
	{
#ifdef USING_INT_MATRIX
		CDaedalusRender::g_pRender->SetWorldView(mat, intmtx, bPush, nLoadCommand);
#else
		CDaedalusRender::g_pRender->SetWorldView(mat, bPush, nLoadCommand);
#endif
	}
	
#ifdef _DEBUG
	char *loadstr = nLoadCommand==RENDER_LOAD_MATRIX?"Load":"Mul";
	char *pushstr = bPush?"Push":"Nopush";
	int projlevel = CDaedalusRender::g_pRender->GetProjectMatrixLevel();
	int worldlevel = CDaedalusRender::g_pRender->GetWorldViewMatrixLevel();
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		pauseAtNext = false;
		debuggerPause = true;
		if (dwCommand & G_MTX_PROJECTION)
		{
			TRACE3("Pause after %s and %s Matrix: Projection, level=%d\n", loadstr, pushstr, projlevel );
		}
		else
		{
			TRACE3("Pause after %s and %s Matrix: WorldView level=%d\n", loadstr, pushstr, worldlevel);
		}
	}
	else
	{
		if( (pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) ) && logTriDetailsWithVertexMtx ) 
		{
			if (dwCommand & G_MTX_PROJECTION)
			{
				TRACE3("Matrix: %s and %s Projection level=%d\n", loadstr, pushstr, projlevel);
			}
			else
			{
				TRACE3("Matrix: %s and %s WorldView\n level=%d", loadstr, pushstr, worldlevel);
			}
		}
	}
#endif
}




void DLParser_GBI1_Reserved(DWORD dwCmd0, DWORD dwCmd1)
{		
	SP_Timing(DLParser_GBI1_Reserved);

	RDP_NOIMPL("RDP: Reserved (0x%08x 0x%08x)", dwCmd0, dwCmd1);
}



void DLParser_GBI1_MoveMem(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_MoveMem);

	DWORD dwType    = (dwCmd0>>16)&0xFF;
	DWORD dwLength  = (dwCmd0)&0xFFFF;
	DWORD dwAddress = RDPSegAddr(dwCmd1);

	switch (dwType) {
		case G_MV_VIEWPORT:
			{
				DL_PF("    G_MV_VIEWPORT. Address: 0x%08x, Length: 0x%04x", dwAddress, dwLength);
				RDP_MoveMemViewport(dwAddress);
			}
			break;
		case G_MV_LOOKATY:
			DL_PF("    G_MV_LOOKATY");
			//RDP_NOIMPL_WARN("G_MV_LOOKATY Not Implemented");
			break;
		case G_MV_LOOKATX:
			DL_PF("    G_MV_LOOKATX");
			//RDP_NOIMPL_WARN("G_MV_LOOKATX Not Implemented");
			break;
		case G_MV_L0:
		case G_MV_L1:
		case G_MV_L2:
		case G_MV_L3:
		case G_MV_L4:
		case G_MV_L5:
		case G_MV_L6:
		case G_MV_L7:
			{
				DWORD dwLight = (dwType-G_MV_L0)/2;
				DL_PF("    G_MV_L%d", dwLight);

				// Ensure dwType == G_MV_L0 -- G_MV_L7
				//	if (dwType < G_MV_L0 || dwType > G_MV_L7 || ((dwType & 0x1) != 0))
				//		break;

				DL_PF("    Light%d: Length:0x%04x, Address: 0x%08x", dwLight, dwLength, dwAddress);

				RDP_MoveMemLight(dwLight, dwAddress);
			}
			break;
		case G_MV_TXTATT:
			DL_PF("    G_MV_TXTATT");
			//RDP_NOIMPL_WARN("G_MV_TXTATT Not Implemented");
			break;
		case G_MV_MATRIX_1:
			RDP_GFX_Force_Matrix(dwAddress);
			//DL_PF("    G_MV_MATRIX_1");
			//RDP_NOIMPL_WARN("G_MV_MATRIX_1 ");
			break;
		case G_MV_MATRIX_2:
			//DL_PF("    G_MV_MATRIX_2");
			//RDP_NOIMPL_WARN("G_MV_MATRIX_2 ");
			break;
		case G_MV_MATRIX_3:
			//DL_PF("    G_MV_MATRIX_3");
			//RDP_NOIMPL_WARN("G_MV_MATRIX_3 ");
			break;
		case G_MV_MATRIX_4:
			//DL_PF("    G_MV_MATRIX_4");
			//RDP_NOIMPL_WARN("G_MV_MATRIX_4 ");
			//RDP_GFX_Force_Matrix(dwAddress-48);
			break;
		default:
			RDP_NOIMPL("MoveMem: Unknown Move Type, cmd=%08X, %08X", dwCmd0, dwCmd1);
			break;
	}
}


void DLParser_GBI0_Vtx(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI0_Vtx);

	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwV0 =  (dwCmd0>>16)&0x0F;
	DWORD dwN  = ((dwCmd0>>20)&0x0F)+1;
	DWORD dwLength = (dwCmd0)&0xFFFF;

	DL_PF("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", dwAddress, dwV0, dwN, dwLength);

	if ((dwV0 + dwN) > 80)
	{
		TRACE3("Warning, invalid vertex positions, N=%d, v0=%d, Addr=0x%08X", dwN, dwV0, dwAddress);
		dwN = 32 - dwV0;
	}

	// Check that address is valid...
	if ((dwAddress + (dwN*16)) > g_dwRamSize)
	{
		TRACE1("SetNewVertexInfo: Address out of range (0x%08x)", dwAddress);
	}
	else
	{
		SetNewVertexInfo(dwAddress, dwV0, dwN);

		status.dwNumVertices += dwN;

		RDP_DumpVtxInfo(dwAddress, dwV0, dwN);
	}
}


void DLParser_GBI0_DL(DWORD dwCmd0, DWORD dwCmd1)
{	
	SP_Timing(DLParser_GBI0_DL);

	DWORD dwPush = (dwCmd0 >> 16) & 0xFF;
	DWORD dwAddr = RDPSegAddr(dwCmd1);

	DL_PF("    Address=0x%08x Push: 0x%02x", dwAddr, dwPush);
	if( dwAddr > g_dwRamSize )
	{
		RDP_NOIMPL("Error: DL addr = %08X out of range, PC=%08X", dwAddr, g_dwPCStack[g_dwPCindex].addr );
		dwAddr &= (g_dwRamSize-1);
		DebuggerPauseCountN( NEXT_DLIST );
	}

	{
		switch (dwPush)
		{
		case G_DL_PUSH:
			DL_PF("    Pushing DisplayList 0x%08x", dwAddr);
			g_dwPCindex++;
			g_dwPCStack[g_dwPCindex].addr = dwAddr;
			g_dwPCStack[g_dwPCindex].limit = ~0;

			break;
		case G_DL_NOPUSH:
			DL_PF("    Jumping to DisplayList 0x%08x", dwAddr);
			g_dwPCStack[g_dwPCindex].addr = dwAddr;
			g_dwPCStack[g_dwPCindex].limit = ~0;
			break;
		}

		DL_PF("Level=%d", g_dwPCindex+1);
		DL_PF("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
		DL_PF("############################################");
	}
}


void DLParser_GBI1_RDPHalf_Cont(DWORD dwCmd0, DWORD dwCmd1)	
{
	SP_Timing(DLParser_GBI1_RDPHalf_Cont);

	/*DL_PF("RDPHalf_Cont: (Ignored)");*/ 
}
void DLParser_GBI1_RDPHalf_2(DWORD dwCmd0, DWORD dwCmd1)		
{ 
	SP_Timing(DLParser_GBI1_RDPHalf_2);

	/*DL_PF("RDPHalf_2: (Ignored)");*/ 
}
void DLParser_GBI1_RDPHalf_1(DWORD dwCmd0, DWORD dwCmd1)		
{
	SP_Timing(DLParser_GBI1_RDPHalf_1);

	/*DL_PF("RDPHalf_1: (Ignored)");*/ 
}

#define SUPPORT_LINE_3D

void DLParser_GBI1_Line3D(DWORD dwCmd0, DWORD dwCmd1)
{
	status.primitiveType = PRIM_LINE3D;

	// While the next command pair is Tri1, add vertices
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	if( (dwCmd1>>24) == 0 )
	{
		// Flying Dragon
		DWORD dwV0		= ((dwCmd1>>16)&0xFF)/gRSP.vertexMult;
		DWORD dwV1		= ((dwCmd1>>8 )&0xFF)/gRSP.vertexMult;
		DWORD dwWidth   = ((dwCmd1    )&0xFF);
		DWORD dwFlag	= ((dwCmd1>>24)&0xFF)/gRSP.vertexMult;	
		
		CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();

		status.dwNumTrisRendered++;

		CDaedalusRender::g_pRender->Line3D(dwV0, dwV1, dwWidth);
		SP_Timing(DLParser_GBI1_Line3D);
		DP_Timing(DLParser_GBI1_Line3D);
	}
	else
	{
		do {
			DWORD dwV3   = ((dwCmd1>>24)&0xFF)/gRSP.vertexMult;		
			DWORD dwV0   = ((dwCmd1>>16)&0xFF)/gRSP.vertexMult;
			DWORD dwV1	 = ((dwCmd1>>8 )&0xFF)/gRSP.vertexMult;
			DWORD dwV2   = ((dwCmd1    )&0xFF)/gRSP.vertexMult;

			DL_PF("    Line3D: V0: %d, V1: %d, V2: %d, V3: %d", dwV0, dwV1, dwV2, dwV3);

			// Do first tri
			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("Line3D 1/2", dwV0, dwV1, dwV2);
				if (!bTrisAdded && CDaedalusRender::g_pRender->IsTextureEnabled())
				{
					SetupTextures();
					InitVertexTextureConstants();
				}

				if( !bTrisAdded )
				{
					CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();
				}

				bTrisAdded = TRUE;
				AddTri(dwV0, dwV1, dwV2);
			}

			// Do second tri
			if (TestTri(dwV2, dwV3, dwV0))
			{
				DEBUG_DUMP_VERTEXES("Line3D 2/2", dwV0, dwV1, dwV2);
				if (!bTrisAdded && CDaedalusRender::g_pRender->IsTextureEnabled())
				{
					SetupTextures();
					InitVertexTextureConstants();
				}

				if( !bTrisAdded )
				{
					CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();
				}

				bTrisAdded = TRUE;
				AddTri(dwV2, dwV3, dwV0);
			}


			dwCmd0			= *(DWORD *)(g_pu8RamBase + dwPC+0);
			dwCmd1			= *(DWORD *)(g_pu8RamBase + dwPC+4);
			dwPC += 8;
#ifdef _DEBUG
		} while ((dwCmd0>>24) == (BYTE)G_LINE3D && !(pauseAtNext && eventToPause==NEXT_FLUSH_TRI));
#else
		} while ((dwCmd0>>24) == (BYTE)G_LINE3D);
#endif

		g_dwPCStack[g_dwPCindex].addr = dwPC-8;

		if (bTrisAdded)	
		{
			CDaedalusRender::g_pRender->FlushTris();
		}
	}
}


void DLParser_GBI1_ClearGeometryMode(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_ClearGeometryMode);
	DWORD dwMask = (dwCmd1);

#ifdef _DEBUG
	DL_PF("    Mask=0x%08x", dwMask);
	if (dwMask & G_ZBUFFER)						DL_PF("  Disabling ZBuffer");
	if (dwMask & G_TEXTURE_ENABLE)				DL_PF("  Disabling Texture");
	if (dwMask & G_SHADE)						DL_PF("  Disabling Shade");
	if (dwMask & G_SHADING_SMOOTH)				DL_PF("  Disabling Smooth Shading");
	if (dwMask & G_CULL_FRONT)					DL_PF("  Disabling Front Culling");
	if (dwMask & G_CULL_BACK)					DL_PF("  Disabling Back Culling");
	if (dwMask & G_FOG)							DL_PF("  Disabling Fog");
	if (dwMask & G_LIGHTING)					DL_PF("  Disabling Lighting");
	if (dwMask & G_TEXTURE_GEN)					DL_PF("  Disabling Texture Gen");
	if (dwMask & G_TEXTURE_GEN_LINEAR)			DL_PF("  Disabling Texture Gen Linear");
	if (dwMask & G_LOD)							DL_PF("  Disabling LOD (no impl)");
#endif

	gRDP.geometryMode &= ~dwMask;

	RDP_GFX_InitGeometryMode();
}



void DLParser_GBI1_SetGeometryMode(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_SetGeometryMode);
	DWORD dwMask = (dwCmd1);

#ifdef _DEBUG
	DL_PF("    Mask=0x%08x", dwMask);
	if (dwMask & G_ZBUFFER)						DL_PF("  Enabling ZBuffer");
	if (dwMask & G_TEXTURE_ENABLE)				DL_PF("  Enabling Texture");
	if (dwMask & G_SHADE)						DL_PF("  Enabling Shade");
	if (dwMask & G_SHADING_SMOOTH)				DL_PF("  Enabling Smooth Shading");
	if (dwMask & G_CULL_FRONT)					DL_PF("  Enabling Front Culling");
	if (dwMask & G_CULL_BACK)					DL_PF("  Enabling Back Culling");
	if (dwMask & G_FOG)							DL_PF("  Enabling Fog");
	if (dwMask & G_LIGHTING)					DL_PF("  Enabling Lighting");
	if (dwMask & G_TEXTURE_GEN)					DL_PF("  Enabling Texture Gen");
	if (dwMask & G_TEXTURE_GEN_LINEAR)			DL_PF("  Enabling Texture Gen Linear");
	if (dwMask & G_LOD)							DL_PF("  Enabling LOD (no impl)");
#endif // _DEBUG
	gRDP.geometryMode |= dwMask;

	//g_bTextureEnable = (gRDP.geometryMode & G_TEXTURE_ENABLE);

	RDP_GFX_InitGeometryMode();
}




void DLParser_GBI1_EndDL(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_EndDL);
	RDP_GFX_PopDL();
}




/*
// a or c
#define	G_BL_CLR_IN		0
#define	G_BL_CLR_MEM	1
#define	G_BL_CLR_BL		2
#define	G_BL_CLR_FOG	3

//b
#define	G_BL_A_IN		0
#define	G_BL_A_FOG		1
#define	G_BL_A_SHADE	2
#define	G_BL_0			3

//d
#define	G_BL_1MA		0
#define	G_BL_A_MEM		1
#define	G_BL_1			2
*/

static const char * sc_szBlClr[4] = { "In", "Mem", "Bl", "Fog" };
static const char * sc_szBlA1[4] = { "AIn", "AFog", "AShade", "0" };
static const char * sc_szBlA2[4] = { "1-A", "AMem", "1", "?" };
/*
#define	GBL_c1(m1a, m1b, m2a, m2b)	\
	(m1a) << 30 | (m1b) << 26 | (m2a) << 22 | (m2b) << 18
#define	GBL_c2(m1a, m1b, m2a, m2b)	\
	(m1a) << 28 | (m1b) << 24 | (m2a) << 20 | (m2b) << 16
*/

/*
  Blend: 0x0050: In*AIn + Mem*AIn    | In*AIn + Mem*AIn
  Blend: 0x0150: In*AIn + Mem*AIn    | In*AFog + Mem*AIn
  Blend: 0x0c18: In*0 + In*AShade    | In*AIn + Mem*AIn
  Blend: 0x0c19: In*0 + In*AShade    | In*AIn + Mem*AFog
  Blend: 0xc810: Fog*AShade + In*AIn | In*AIn + Mem*AIn
  Blend: 0xc811: Fog*AShade + In*AIn | In*AIn + Mem*AFog
*/


void DLParser_GBI1_SetOtherModeL(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_SetOtherModeL);

	DWORD dwShift = (dwCmd0>>8)&0xFF;
	DWORD dwLength= (dwCmd0   )&0xFF;
	DWORD dwData  = dwCmd1;

	DWORD dwMask = ((1<<dwLength)-1)<<dwShift;

	DWORD modeL = gRDP.otherModeL;
	modeL = (modeL&(~dwMask)) | dwData;

	DLParser_RDPSetOtherMode(gRDP.otherModeH, modeL );
}


void DLParser_GBI1_SetOtherModeH(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_SetOtherModeH);

	DWORD dwShift = (dwCmd0>>8)&0xFF;
	DWORD dwLength= (dwCmd0   )&0xFF;
	DWORD dwData  = dwCmd1;

	DWORD dwMask = ((1<<dwLength)-1)<<dwShift;
	DWORD dwModeH = gRDP.otherModeH;

	// Mask off bits that don't apply. Might not be necessary
	//	dwData &= dwMask;

	dwModeH = (dwModeH&(~dwMask)) | dwData;
	DLParser_RDPSetOtherMode(dwModeH, gRDP.otherModeL );
}


void DLParser_GBI1_Texture(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_Texture);

	bool  bTextureEnable = ((dwCmd0)&0x01)?true:false;
	DWORD dwLevel  = (dwCmd0>>11)&0x07;
	DWORD dwTile= (dwCmd0>>8 )&0x07;

	float fTextureScaleS = (float)((dwCmd1>>16)&0xFFFF) / (65536.0f * 32.0f);
	float fTextureScaleT = (float)((dwCmd1    )&0xFFFF) / (65536.0f * 32.0f);

	if( ((dwCmd1>>16)&0xFFFF) == 0xFFFF )
	{
		fTextureScaleS = 1/32.0f;
	}
	else if( ((dwCmd1>>16)&0xFFFF) == 0x8000 )
	{
		fTextureScaleS = 1/64.0f;
	}
#ifdef _DEBUG
	else if( ((dwCmd1>>16)&0xFFFF) != 0 )
	{
		//DebuggerAppendMsg("Warning, texture scale = %08X is not integer", (dwCmd1>>16)&0xFFFF);
	}
#endif

	if( ((dwCmd1    )&0xFFFF) == 0xFFFF )
	{
		fTextureScaleT = 1/32.0f;
	}
	else if( ((dwCmd1    )&0xFFFF) == 0x8000 )
	{
		fTextureScaleT = 1/64.0f;
	}
#ifdef _DEBUG
	else if( (dwCmd1&0xFFFF) != 0 )
	{
		//DebuggerAppendMsg("Warning, texture scale = %08X is not integer", (dwCmd1)&0xFFFF);
	}
#endif

	if( gRSP.ucode == 6 )
	{
		if( fTextureScaleS == 0 )	fTextureScaleS = 1.0f/32.0f;
		if( fTextureScaleT == 0 )	fTextureScaleT = 1.0f/32.0f;
	}

	CDaedalusRender::g_pRender->SetTextureEnableAndScale(dwTile, bTextureEnable, fTextureScaleS, fTextureScaleT);

	// What happens if these are 0? Interpret as 1.0f?

#ifdef _DEBUG
	if( logTextureDetails || (pauseAtNext && eventToPause==NEXT_SET_TEXTURE))
	{
		DebuggerAppendMsg("SetTexture: Level: %d Tile: %d %s\n", dwLevel, dwTile, bTextureEnable ? "enabled":"disabled");
		DebuggerAppendMsg("            ScaleS: %f, ScaleT: %f\n", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_SET_TEXTURE);
#endif
	DL_PF("    Level: %d Tile: %d %s", dwLevel, dwTile, bTextureEnable ? "enabled":"disabled");
	DL_PF("    ScaleS: %f, ScaleT: %f", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
}


void DLParser_GBI1_MoveWord(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_MoveWord);

	// Type of movement is in low 8bits of cmd0.

	DWORD dwIndex = dwCmd0 & 0xFF;
	DWORD dwOffset = (dwCmd0 >> 8) & 0xFFFF;


	switch (dwIndex)
	{
	case G_MW_MATRIX:
		DL_PF("    G_MW_MATRIX");
		RDP_NOIMPL_WARN("G_MW_MATRIX Not Implemented");

		break;
	case G_MW_NUMLIGHT:
		{
			DWORD dwNumLights = ((dwCmd1-0x80000000)/32)-1;
			//DWORD dwNumLights = ((dwCmd1-0x80000000)/32);
			DL_PF("    G_MW_NUMLIGHT: Val:%d", dwNumLights);

			gRSP.ambientLightIndex = dwNumLights;
			SetNumLights(dwNumLights);
		}
		break;
	case G_MW_CLIP:
		{
			switch (dwOffset)
			{
			case G_MWO_CLIP_RNX:
			case G_MWO_CLIP_RNY:
			case G_MWO_CLIP_RPX:
			case G_MWO_CLIP_RPY:
				CDaedalusRender::g_pRender->SetClipRatio(dwOffset, dwCmd1);
				break;
			default:
				DL_PF("    G_MW_CLIP  ?   : 0x%08x", dwCmd1);
				break;
			}

			//RDP_NOIMPL_WARN("G_MW_CLIP Not Implemented");
		}
		break;
	case G_MW_SEGMENT:
		{
			DWORD dwSegment = (dwOffset >> 2) & 0xF;
			DWORD dwBase = dwCmd1&0x00FFFFFF;
			DL_PF("    G_MW_SEGMENT Seg[%d] = 0x%08x", dwSegment, dwBase);
			if( dwBase > g_dwRamSize )
			{
#ifdef _DEBUG
				if( pauseAtNext )
					DebuggerAppendMsg("warning: Segment %d addr is %8X", dwSegment, dwBase);
#endif
			}
			else
			{
				gRSP.segments[dwSegment] = dwBase;
			}
		}
		break;
	case G_MW_FOG:
		{
			WORD wMult = (WORD)((dwCmd1 >> 16) & 0xFFFF);
			WORD wOff  = (WORD)((dwCmd1      ) & 0xFFFF);

			float fMult = (float)(short)wMult;
			float fOff = (float)(short)wOff;

			float rng = 128000.0f / fMult;
			float fMin = 500.0f - (fOff*rng/256.0f);
			float fMax = rng + fMin;
			DL_PF("    G_MW_FOG/Mul=%d: Off=%d", wMult, wOff);
			SetFogMinMax(fMin, fMax, fMult, fOff);
		}
		//RDP_NOIMPL_WARN("G_MW_FOG Not Implemented");
		break;
	case G_MW_LIGHTCOL:
		{
			DWORD dwLight = dwOffset / 0x20;
			DWORD dwField = (dwOffset & 0x7);

			DL_PF("    G_MW_LIGHTCOL/0x%08x: 0x%08x", dwOffset, dwCmd1);

			switch (dwField)
			{
			case 0:
				//gRSP.n64lights[dwLight].dwRGBA = dwCmd1;
				// Light col, not the copy
				if (dwLight == gRSP.ambientLightIndex)
				{
					SetAmbientLight( (dwCmd1>>8) );
				}
				else
				{
					SetLightCol(dwLight, dwCmd1);
				}
				break;

			case 4:
				break;

			default:
				TRACE1("G_MW_LIGHTCOL with unknown offset 0x%08x", dwField);
				break;
			}
		}
		break;
	case G_MW_POINTS:
		{
			DWORD offset = (dwCmd0>>8)&0xFFFF;
			DWORD vtx = offset/40;
			DWORD where = offset - vtx*40;
			ModifyVertexInfo(where, vtx, dwCmd1);
		}
		break;
	case G_MW_PERSPNORM:
		DL_PF("    G_MW_PERSPNORM");
		//if( dwCmd1 != 0x1A ) DebuggerAppendMsg("PerspNorm: 0x%04x", (short)dwCmd1);	
		break;
	default:
		RDP_NOIMPL("Unknown MoveWord, %08X, %08X", dwCmd0, dwCmd1);
		break;
	}

}


void DLParser_GBI1_PopMtx(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_PopMtx);

	BYTE nCommand = (BYTE)(dwCmd1 & 0xFF);

	DL_PF("    Command: 0x%02x (%s)",
		nCommand,  (nCommand & G_MTX_PROJECTION) ? "Projection" : "ModelView");

	// Do any of the other bits do anything?
	// So far only Extreme-G seems to Push/Pop projection matrices

	if (nCommand & G_MTX_PROJECTION)
	{
		CDaedalusRender::g_pRender->PopProjection();
	}
	else
	{
		CDaedalusRender::g_pRender->PopWorldView();
	}
#ifdef _DEBUG
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		pauseAtNext = false;
		debuggerPause = true;
		DebuggerAppendMsg("Pause after Pop Matrix: %s\n", (nCommand & G_MTX_PROJECTION) ? "Proj":"World");
	}
	else
	{
		if( (pauseAtNext /*&& (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) */) && logTriDetailsWithVertexMtx ) 
		{
			DebuggerAppendMsg("Pause after Pop Matrix: %s\n", (nCommand & G_MTX_PROJECTION) ? "Proj":"World");
		}
	}
#endif
}



void DLParser_GBI1_CullDL(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_CullDL);

#ifdef _DEBUG
	if( !debuggerEnableCullFace )
	{
		return;	//Disable Culling
	}
#endif

	DWORD i;
	DWORD dwVFirst = ((dwCmd0) & 0xFFF) / gRSP.vertexMult;
	DWORD dwVLast  = ((dwCmd1) & 0xFFF) / gRSP.vertexMult;

	DL_PF("    Culling using verts %d to %d", dwVFirst, dwVLast);

	// Mask into range
	dwVFirst &= 0x1f;
	dwVLast &= 0x1f;

	if( dwVLast < dwVFirst )	return;
	if( !gRSP.bRejectVtx )	return;

	for (i = dwVFirst; i <= dwVLast; i++)
	{
		if (g_clipFlag[i] == 0)
		{
			DL_PF("    Vertex %d is visible, continuing with display list processing", i);
			return;
		}
	}

	status.dwNumDListsCulled++;

	DL_PF("    No vertices were visible, culling rest of display list");

	RDP_GFX_PopDL();
}



void DLParser_GBI1_Tri1(DWORD dwCmd0, DWORD dwCmd1)
{
	status.primitiveType = PRIM_TRI1;
	bool bTrisAdded = false;
	bool textureIsEnabled = CDaedalusRender::g_pRender->IsTextureEnabled();

	// While the next command pair is Tri1, add vertices
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;
	DWORD * pCmdBase = (DWORD *)(g_pu8RamBase + dwPC);
	
	do
	{
		//DWORD dwFlag = (dwCmd1>>24)&0xFF;
		// Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
		DWORD dwV0 = ((dwCmd1>>16)&0xFF)/gRSP.vertexMult;
		DWORD dwV1 = ((dwCmd1>>8 )&0xFF)/gRSP.vertexMult;
		DWORD dwV2 = ((dwCmd1    )&0xFF)/gRSP.vertexMult;

		if (TestTri(dwV0, dwV1, dwV2))
		{
			DEBUG_DUMP_VERTEXES("Tri1", dwV0, dwV1, dwV2);
			DL_PF("    Tri1: 0x%08x 0x%08x %d,%d,%d", dwCmd0, dwCmd1, dwV0, dwV1, dwV2);

			if (!bTrisAdded)
			{
				if( textureIsEnabled )
			{
				SetupTextures();
				InitVertexTextureConstants();
			}
				CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();
				bTrisAdded = true;
			}
			AddTri(dwV0, dwV1, dwV2);
		}

		dwCmd0			= *pCmdBase++;
		dwCmd1			= *pCmdBase++;
		dwPC += 8;

#ifdef _DEBUG
	} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (dwCmd0>>24) == (BYTE)G_TRI1);
	if( pauseAtNext && eventToPause==NEXT_TRIANGLE ) 
	{
		eventToPause = NEXT_FLUSH_TRI;
	}
#else
	} while ((dwCmd0>>24) == (BYTE)G_TRI1);
#endif

	g_dwPCStack[g_dwPCindex].addr = dwPC-8;

	if (bTrisAdded)	
	{
		CDaedalusRender::g_pRender->FlushTris();
	}
}


void DLParser_GBI0_Tri4(DWORD dwCmd0, DWORD dwCmd1)
{
	status.primitiveType = PRIM_TRI2;

	// While the next command pair is Tri2, add vertices
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	do {
		DWORD dwFlag = (dwCmd0>>16)&0xFF;
		DL_PF("    PD Tri4: 0x%08x 0x%08x Flag: 0x%02x", dwCmd0, dwCmd1, dwFlag);

		BOOL bVisible;
		for( int i=0; i<4; i++)
		{
			DWORD v0 = (dwCmd1>>(4+(i<<3))) & 0xF;
			DWORD v1 = (dwCmd1>>(  (i<<3))) & 0xF;
			DWORD v2 = (dwCmd0>>(  (i<<2))) & 0xF;
			bVisible = TestTri(v0, v2, v1);
			DL_PF("       (%d, %d, %d) %s", v0, v1, v2, bVisible ? "": "(clipped)");
			if (bVisible)
			{
				DEBUG_DUMP_VERTEXES("Tri4_PerfectDark 1/2", v0, v1, v2);
				if (!bTrisAdded && CDaedalusRender::g_pRender->IsTextureEnabled())
				{
					SetupTextures();
					InitVertexTextureConstants();
				}

				if( !bTrisAdded )
				{
					CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();
				}

				bTrisAdded = TRUE;
				AddTri(v0, v2, v1);
			}
		}
		
		dwCmd0			= *(DWORD *)(g_pu8RamBase + dwPC+0);
		dwCmd1			= *(DWORD *)(g_pu8RamBase + dwPC+4);
		dwPC += 8;

#ifdef _DEBUG
	} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (dwCmd0>>24) == (BYTE)G_TRI2);
	if(pauseAtNext && eventToPause==NEXT_TRIANGLE )
	{
		eventToPause = NEXT_FLUSH_TRI;
	}

#else
	} while ((dwCmd0>>24) == (BYTE)G_TRI2);
#endif


	g_dwPCStack[g_dwPCindex].addr = dwPC-8;


	if (bTrisAdded)	
	{
		CDaedalusRender::g_pRender->FlushTris();
	}
	
	gRSP.DKRVtxCount=0;
}

//Nintro64 uses Sprite2d 


void DLParser_Nothing(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_Nothing);

#ifdef _DEBUG
	if( logUnknownOpcode )
	{
		TRACE0("Stack Trace");
		for( int i=0; i<g_dwPCindex; i++ )
		{
			DebuggerAppendMsg("  %08X", g_dwPCStack[i].addr);
		}
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DebuggerAppendMsg("PC=%08X",dwPC);
		DebuggerAppendMsg("Warning, unknown ucode PC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
	}
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_UNKNOWN_OP, {TRACE0("Paused at unknown ucode\n");})
	if( debuggerContinueWithUnknown )
	{
		return;
	}
#endif
		
	if( options.enableHacks )
		return;
	
	StopRDP();
	g_dwPCindex=-1;
}


