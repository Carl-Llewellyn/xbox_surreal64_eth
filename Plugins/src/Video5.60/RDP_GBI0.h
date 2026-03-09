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

void RSP_GBI1_SpNoop(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_SpNoop);

	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
	uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	uint32 dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	if( dwCmd2>>24 == 0x00 )
	{
		RDP_GFX_PopDL();
		RSP_RDP_NOIMPL("Double SPNOOP, Skip remain ucodes, PC=%08X, Cmd1=%08X", dwPC, word1);
	}
}



void RSP_GBI0_Mtx(uint32 word0, uint32 word1)
{	
	SP_Timing(RSP_GBI0_Mtx);

	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwCommand = (word0>>16)&0xFF;
	uint32 dwLength  = (word0)    &0xFFFF;

	LOG_DL("    Command: %s %s %s Length %d Address 0x%08x",
		(dwCommand & RSP_MATRIX_PROJECTION) ? "Projection" : "ModelView",
		(dwCommand & RSP_MATRIX_LOAD) ? "Load" : "Mul",	
		(dwCommand & RSP_MATRIX_PUSH) ? "Push" : "NoPush",
		dwLength, dwAddr);

	if (dwAddr + 64 > g_dwRamSize)
	{
		TRACE1("Mtx: Address invalid (0x%08x)", dwAddr);
		return;
	}

	// Load matrix from dwAddr
	Matrix mat;
	uint32 dwI;
	uint32 dwJ;
	const float fRecip = 1.0f / 65536.0f;
	
	for (dwI = 0; dwI < 4; dwI++) {
		for (dwJ = 0; dwJ < 4; dwJ++) {

			short nDataHi = *(short *)(g_pRDRAMu8 + ((dwAddr+(dwI<<3)+(dwJ<<1)     )^0x2));
			WORD  nDataLo = *(WORD  *)(g_pRDRAMu8 + ((dwAddr+(dwI<<3)+(dwJ<<1) + 32)^0x2));

			mat.m[dwI][dwJ] = (float)(((LONG)nDataHi<<16) | (nDataLo))*fRecip;
		}
	}

	LONG nLoadCommand = dwCommand & RSP_MATRIX_LOAD ? RENDER_LOAD_MATRIX : RENDER_MUL_MATRIX;
	BOOL bPush = dwCommand & RSP_MATRIX_PUSH ? TRUE : FALSE;

	if (dwCommand & RSP_MATRIX_PROJECTION)
	{
		// So far only Extreme-G seems to Push/Pop projection matrices	
		CRender::g_pRender->SetProjection(mat, bPush, nLoadCommand);
	}
	else
	{
		CRender::g_pRender->SetWorldView(mat, bPush, nLoadCommand);
	}
	
#ifdef _DEBUG
	char *loadstr = nLoadCommand==RENDER_LOAD_MATRIX?"Load":"Mul";
	char *pushstr = bPush?"Push":"Nopush";
	int projlevel = CRender::g_pRender->GetProjectMatrixLevel();
	int worldlevel = CRender::g_pRender->GetWorldViewMatrixLevel();
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		pauseAtNext = false;
		debuggerPause = true;
		if (dwCommand & RSP_MATRIX_PROJECTION)
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
		if( pauseAtNext && logMatrix ) 
		{
			if (dwCommand & RSP_MATRIX_PROJECTION)
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




void RSP_GBI1_Reserved(uint32 word0, uint32 word1)
{		
	SP_Timing(RSP_GBI1_Reserved);

	RSP_RDP_NOIMPL("RDP: Reserved (0x%08x 0x%08x)", word0, word1);
}



void RSP_GBI1_MoveMem(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_MoveMem);

	uint32 dwType    = (word0>>16)&0xFF;
	uint32 dwLength  = (word0)&0xFFFF;
	uint32 dwAddr = RSPSegmentAddr(word1);

	switch (dwType) {
		case RSP_GBI1_MV_MEM_VIEWPORT:
			{
				LOG_DL("    RSP_GBI1_MV_MEM_VIEWPORT. Address: 0x%08x, Length: 0x%04x", dwAddr, dwLength);
				RSP_MoveMemViewport(dwAddr);
			}
			break;
		case RSP_GBI1_MV_MEM_LOOKATY:
			LOG_DL("    RSP_GBI1_MV_MEM_LOOKATY");
			break;
		case RSP_GBI1_MV_MEM_LOOKATX:
			LOG_DL("    RSP_GBI1_MV_MEM_LOOKATX");
			break;
		case RSP_GBI1_MV_MEM_L0:
		case RSP_GBI1_MV_MEM_L1:
		case RSP_GBI1_MV_MEM_L2:
		case RSP_GBI1_MV_MEM_L3:
		case RSP_GBI1_MV_MEM_L4:
		case RSP_GBI1_MV_MEM_L5:
		case RSP_GBI1_MV_MEM_L6:
		case RSP_GBI1_MV_MEM_L7:
			{
				uint32 dwLight = (dwType-RSP_GBI1_MV_MEM_L0)/2;
				LOG_DL("    RSP_GBI1_MV_MEM_L%d", dwLight);
				LOG_DL("    Light%d: Length:0x%04x, Address: 0x%08x", dwLight, dwLength, dwAddr);

				RSP_MoveMemLight(dwLight, dwAddr);
			}
			break;
		case RSP_GBI1_MV_MEM_TXTATT:
			LOG_DL("    RSP_GBI1_MV_MEM_TXTATT");
			break;
		case RSP_GBI1_MV_MEM_MATRIX_1:
			RSP_GFX_Force_Matrix(dwAddr);
			break;
		case RSP_GBI1_MV_MEM_MATRIX_2:
			break;
		case RSP_GBI1_MV_MEM_MATRIX_3:
			break;
		case RSP_GBI1_MV_MEM_MATRIX_4:
			break;
		default:
			RSP_RDP_NOIMPL("MoveMem: Unknown Move Type, cmd=%08X, %08X", word0, word1);
			break;
	}
}


void RSP_GBI0_Vtx(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI0_Vtx);

	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwV0 =  (word0>>16)&0x0F;
	uint32 dwN  = ((word0>>20)&0x0F)+1;
	uint32 dwLength = (word0)&0xFFFF;

	LOG_DL("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", dwAddr, dwV0, dwN, dwLength);

	if ((dwV0 + dwN) > 80)
	{
		TRACE3("Warning, invalid vertex positions, N=%d, v0=%d, Addr=0x%08X", dwN, dwV0, dwAddr);
		dwN = 32 - dwV0;
	}

	// Check that address is valid...
	if ((dwAddr + (dwN*16)) > g_dwRamSize)
	{
		TRACE1("ProcessVertexData: Address out of range (0x%08x)", dwAddr);
	}
	else
	{
		ProcessVertexData(dwAddr, dwV0, dwN);

		status.dwNumVertices += dwN;

		DisplayVertexInfo(dwAddr, dwV0, dwN);
	}
}


void RSP_GBI0_DL(uint32 word0, uint32 word1)
{	
	SP_Timing(RSP_GBI0_DL);

	uint32 dwPush = (word0 >> 16) & 0xFF;
	uint32 dwAddr = RSPSegmentAddr(word1);

	LOG_DL("    Address=0x%08x Push: 0x%02x", dwAddr, dwPush);
	if( dwAddr > g_dwRamSize )
	{
		RSP_RDP_NOIMPL("Error: DL addr = %08X out of range, PC=%08X", dwAddr, g_dwPCStack[g_dwPCindex].addr );
		dwAddr &= (g_dwRamSize-1);
		DebuggerPauseCountN( NEXT_DLIST );
	}

	{
		switch (dwPush)
		{
		case RSP_DLIST_PUSH:
			LOG_DL("    Pushing DisplayList 0x%08x", dwAddr);
			g_dwPCindex++;
			g_dwPCStack[g_dwPCindex].addr = dwAddr;
			g_dwPCStack[g_dwPCindex].limit = ~0;

			break;
		case RSP_DLIST_NOPUSH:
			LOG_DL("    Jumping to DisplayList 0x%08x", dwAddr);
			g_dwPCStack[g_dwPCindex].addr = dwAddr;
			g_dwPCStack[g_dwPCindex].limit = ~0;
			break;
		}

		LOG_DL("Level=%d", g_dwPCindex+1);
		LOG_DL("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
	}
}


void RSP_GBI1_RDPHalf_Cont(uint32 word0, uint32 word1)	
{
	SP_Timing(RSP_GBI1_RDPHalf_Cont);

	LOG_DL("RDPHalf_Cont: (Ignored)"); 
}
void RSP_GBI1_RDPHalf_2(uint32 word0, uint32 word1)		
{ 
	SP_Timing(RSP_GBI1_RDPHalf_2);

	LOG_DL("RDPHalf_2: (Ignored)"); 
}

uint32 g_Half1Val;
void RSP_GBI1_RDPHalf_1(uint32 word0, uint32 word1)		
{
	SP_Timing(RSP_GBI1_RDPHalf_1);

	g_Half1Val = word1;

	LOG_DL("RDPHalf_1: (Ignored)"); 
}

#define SUPPORT_LINE_3D

void RSP_GBI1_Line3D(uint32 word0, uint32 word1)
{
	status.primitiveType = PRIM_LINE3D;

	// While the next command pair is Tri1, add vertices
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	if( (word1>>24) == 0 )
	{
		// Flying Dragon
		uint32 dwV0		= ((word1>>16)&0xFF)/gRSP.vertexMult;
		uint32 dwV1		= ((word1>>8 )&0xFF)/gRSP.vertexMult;
		uint32 dwWidth   = ((word1    )&0xFF);
		uint32 dwFlag	= ((word1>>24)&0xFF)/gRSP.vertexMult;	
		
		CRender::g_pRender->SetCombinerAndBlender();

		status.dwNumTrisRendered++;

		CRender::g_pRender->Line3D(dwV0, dwV1, dwWidth);
		SP_Timing(RSP_GBI1_Line3D);
		DP_Timing(RSP_GBI1_Line3D);
	}
	else
	{
		do {
			uint32 dwV3   = ((word1>>24)&0xFF)/gRSP.vertexMult;		
			uint32 dwV0   = ((word1>>16)&0xFF)/gRSP.vertexMult;
			uint32 dwV1	 = ((word1>>8 )&0xFF)/gRSP.vertexMult;
			uint32 dwV2   = ((word1    )&0xFF)/gRSP.vertexMult;

			LOG_DL("    Line3D: V0: %d, V1: %d, V2: %d, V3: %d", dwV0, dwV1, dwV2, dwV3);

			// Do first tri
			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("Line3D 1/2", dwV0, dwV1, dwV2);
				if (!bTrisAdded && CRender::g_pRender->IsTextureEnabled())
				{
					PrepareTextures();
					InitVertexTextureConstants();
				}

				if( !bTrisAdded )
				{
					CRender::g_pRender->SetCombinerAndBlender();
				}

				bTrisAdded = TRUE;
				AddTri(dwV0, dwV1, dwV2);
			}

			// Do second tri
			if (TestTri(dwV2, dwV3, dwV0))
			{
				DEBUG_DUMP_VERTEXES("Line3D 2/2", dwV0, dwV1, dwV2);
				if (!bTrisAdded && CRender::g_pRender->IsTextureEnabled())
				{
					PrepareTextures();
					InitVertexTextureConstants();
				}

				if( !bTrisAdded )
				{
					CRender::g_pRender->SetCombinerAndBlender();
				}

				bTrisAdded = TRUE;
				AddTri(dwV2, dwV3, dwV0);
			}


			word0			= *(uint32 *)(g_pRDRAMu8 + dwPC+0);
			word1			= *(uint32 *)(g_pRDRAMu8 + dwPC+4);
			dwPC += 8;
#ifdef _DEBUG
		} while ((word0>>24) == (BYTE)RSP_LINE3D && !(pauseAtNext && eventToPause==NEXT_FLUSH_TRI));
#else
		} while ((word0>>24) == (BYTE)RSP_LINE3D);
#endif

		g_dwPCStack[g_dwPCindex].addr = dwPC-8;

		if (bTrisAdded)	
		{
			CRender::g_pRender->FlushTris();
		}
	}
}


void RSP_GBI1_ClearGeometryMode(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_ClearGeometryMode);
	uint32 dwMask = (word1);

#ifdef _DEBUG
	LOG_DL("    Mask=0x%08x", dwMask);
	if (dwMask & G_ZBUFFER)						LOG_DL("  Disabling ZBuffer");
	if (dwMask & G_TEXTURE_ENABLE)				LOG_DL("  Disabling Texture");
	if (dwMask & G_SHADE)						LOG_DL("  Disabling Shade");
	if (dwMask & G_SHADING_SMOOTH)				LOG_DL("  Disabling Smooth Shading");
	if (dwMask & G_CULL_FRONT)					LOG_DL("  Disabling Front Culling");
	if (dwMask & G_CULL_BACK)					LOG_DL("  Disabling Back Culling");
	if (dwMask & G_FOG)							LOG_DL("  Disabling Fog");
	if (dwMask & G_LIGHTING)					LOG_DL("  Disabling Lighting");
	if (dwMask & G_TEXTURE_GEN)					LOG_DL("  Disabling Texture Gen");
	if (dwMask & G_TEXTURE_GEN_LINEAR)			LOG_DL("  Disabling Texture Gen Linear");
	if (dwMask & G_LOD)							LOG_DL("  Disabling LOD (no impl)");
#endif

	gRDP.geometryMode &= ~dwMask;

	RSP_GFX_InitGeometryMode();
}



void RSP_GBI1_SetGeometryMode(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_SetGeometryMode);
	uint32 dwMask = (word1);

#ifdef _DEBUG
	LOG_DL("    Mask=0x%08x", dwMask);
	if (dwMask & G_ZBUFFER)						LOG_DL("  Enabling ZBuffer");
	if (dwMask & G_TEXTURE_ENABLE)				LOG_DL("  Enabling Texture");
	if (dwMask & G_SHADE)						LOG_DL("  Enabling Shade");
	if (dwMask & G_SHADING_SMOOTH)				LOG_DL("  Enabling Smooth Shading");
	if (dwMask & G_CULL_FRONT)					LOG_DL("  Enabling Front Culling");
	if (dwMask & G_CULL_BACK)					LOG_DL("  Enabling Back Culling");
	if (dwMask & G_FOG)							LOG_DL("  Enabling Fog");
	if (dwMask & G_LIGHTING)					LOG_DL("  Enabling Lighting");
	if (dwMask & G_TEXTURE_GEN)					LOG_DL("  Enabling Texture Gen");
	if (dwMask & G_TEXTURE_GEN_LINEAR)			LOG_DL("  Enabling Texture Gen Linear");
	if (dwMask & G_LOD)							LOG_DL("  Enabling LOD (no impl)");
#endif // _DEBUG
	gRDP.geometryMode |= dwMask;

	RSP_GFX_InitGeometryMode();
}




void RSP_GBI1_EndDL(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_EndDL);
	RDP_GFX_PopDL();
}


static const char * sc_szBlClr[4] = { "In", "Mem", "Bl", "Fog" };
static const char * sc_szBlA1[4] = { "AIn", "AFog", "AShade", "0" };
static const char * sc_szBlA2[4] = { "1-A", "AMem", "1", "?" };

void RSP_GBI1_SetOtherModeL(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_SetOtherModeL);

	uint32 dwShift = (word0>>8)&0xFF;
	uint32 dwLength= (word0   )&0xFF;
	uint32 dwData  = word1;

	uint32 dwMask = ((1<<dwLength)-1)<<dwShift;

	uint32 modeL = gRDP.otherModeL;
	modeL = (modeL&(~dwMask)) | dwData;

	DLParser_RDPSetOtherMode(gRDP.otherModeH, modeL );
}


void RSP_GBI1_SetOtherModeH(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_SetOtherModeH);

	uint32 dwShift = (word0>>8)&0xFF;
	uint32 dwLength= (word0   )&0xFF;
	uint32 dwData  = word1;

	uint32 dwMask = ((1<<dwLength)-1)<<dwShift;
	uint32 dwModeH = gRDP.otherModeH;

	dwModeH = (dwModeH&(~dwMask)) | dwData;
	DLParser_RDPSetOtherMode(dwModeH, gRDP.otherModeL );
}


void RSP_GBI1_Texture(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_Texture);

	bool  bTextureEnable = ((word0)&0x01)?true:false;
	uint32 dwLevel  = (word0>>11)&0x07;
	uint32 dwTile= (word0>>8 )&0x07;

	float fTextureScaleS = (float)((word1>>16)&0xFFFF) / (65536.0f * 32.0f);
	float fTextureScaleT = (float)((word1    )&0xFFFF) / (65536.0f * 32.0f);

	if( ((word1>>16)&0xFFFF) == 0xFFFF )
	{
		fTextureScaleS = 1/32.0f;
	}
	else if( ((word1>>16)&0xFFFF) == 0x8000 )
	{
		fTextureScaleS = 1/64.0f;
	}
#ifdef _DEBUG
	else if( ((word1>>16)&0xFFFF) != 0 )
	{
		//DebuggerAppendMsg("Warning, texture scale = %08X is not integer", (word1>>16)&0xFFFF);
	}
#endif

	if( ((word1    )&0xFFFF) == 0xFFFF )
	{
		fTextureScaleT = 1/32.0f;
	}
	else if( ((word1    )&0xFFFF) == 0x8000 )
	{
		fTextureScaleT = 1/64.0f;
	}
#ifdef _DEBUG
	else if( (word1&0xFFFF) != 0 )
	{
		//DebuggerAppendMsg("Warning, texture scale = %08X is not integer", (word1)&0xFFFF);
	}
#endif

	if( gRSP.ucode == 6 )
	{
		if( fTextureScaleS == 0 )	fTextureScaleS = 1.0f/32.0f;
		if( fTextureScaleT == 0 )	fTextureScaleT = 1.0f/32.0f;
	}

	CRender::g_pRender->SetTextureEnableAndScale(dwTile, bTextureEnable, fTextureScaleS, fTextureScaleT);

	// What happens if these are 0? Interpret as 1.0f?

#ifdef _DEBUG
	if( logTextures || (pauseAtNext && eventToPause==NEXT_SET_TEXTURE))
	{
		DebuggerAppendMsg("SetTexture: Level: %d Tile: %d %s\n", dwLevel, dwTile, bTextureEnable ? "enabled":"disabled");
		DebuggerAppendMsg("            ScaleS: %f, ScaleT: %f\n", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
	}

	DEBUGGER_PAUSE_COUNT_N(NEXT_SET_TEXTURE);
#endif
	LOG_DL("    Level: %d Tile: %d %s", dwLevel, dwTile, bTextureEnable ? "enabled":"disabled");
	LOG_DL("    ScaleS: %f, ScaleT: %f", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
}

extern void RSP_RDP_InsertMatrix(uint32 word0, uint32 word1);
void RSP_GBI1_MoveWord(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_MoveWord);

	// Type of movement is in low 8bits of cmd0.

	uint32 dwIndex = word0 & 0xFF;
	uint32 dwOffset = (word0 >> 8) & 0xFFFF;


	switch (dwIndex)
	{
	case RSP_MOVE_WORD_MATRIX:
		RSP_RDP_InsertMatrix(word0, word1);
		break;
	case RSP_MOVE_WORD_NUMLIGHT:
		{
			uint32 dwNumLights = ((word1-0x80000000)/32)-1;
			//uint32 dwNumLights = ((word1-0x80000000)/32);
			LOG_DL("    RSP_MOVE_WORD_NUMLIGHT: Val:%d", dwNumLights);

			gRSP.ambientLightIndex = dwNumLights;
			SetNumLights(dwNumLights);
		}
		break;
	case RSP_MOVE_WORD_CLIP:
		{
			switch (dwOffset)
			{
			case RSP_MV_WORD_OFFSET_CLIP_RNX:
			case RSP_MV_WORD_OFFSET_CLIP_RNY:
			case RSP_MV_WORD_OFFSET_CLIP_RPX:
			case RSP_MV_WORD_OFFSET_CLIP_RPY:
				CRender::g_pRender->SetClipRatio(dwOffset, word1);
				break;
			default:
				LOG_DL("    RSP_MOVE_WORD_CLIP  ?   : 0x%08x", word1);
				break;
			}
		}
		break;
	case RSP_MOVE_WORD_SEGMENT:
		{
			uint32 dwSegment = (dwOffset >> 2) & 0xF;
			uint32 dwBase = word1&0x00FFFFFF;
			LOG_DL("    RSP_MOVE_WORD_SEGMENT Seg[%d] = 0x%08x", dwSegment, dwBase);
			if( dwBase > g_dwRamSize )
			{
				gRSP.segments[dwSegment] = dwBase;
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
	case RSP_MOVE_WORD_FOG:
		{
			WORD wMult = (WORD)((word1 >> 16) & 0xFFFF);
			WORD wOff  = (WORD)((word1      ) & 0xFFFF);

			float fMult = (float)(short)wMult;
			float fOff = (float)(short)wOff;

			float rng = 128000.0f / fMult;
			float fMin = 500.0f - (fOff*rng/256.0f);
			float fMax = rng + fMin;

			if( fMult <= 0 || fMin > fMax || fMax < 0 || fMin > 1000 )
			{
				// Hack
				fMin = 996;
				fMax = 1000;
				fMult = 0;
				fOff = 1;
			}

			LOG_DL("    RSP_MOVE_WORD_FOG/Mul=%d: Off=%d", wMult, wOff);
			SetFogMinMax(fMin, fMax, fMult, fOff);
		}
		break;
	case RSP_MOVE_WORD_LIGHTCOL:
		{
			uint32 dwLight = dwOffset / 0x20;
			uint32 dwField = (dwOffset & 0x7);

			LOG_DL("    RSP_MOVE_WORD_LIGHTCOL/0x%08x: 0x%08x", dwOffset, word1);

			switch (dwField)
			{
			case 0:
				if (dwLight == gRSP.ambientLightIndex)
				{
					SetAmbientLight( (word1>>8) );
				}
				else
				{
					SetLightCol(dwLight, word1);
				}
				break;

			case 4:
				break;

			default:
				TRACE1("RSP_MOVE_WORD_LIGHTCOL with unknown offset 0x%08x", dwField);
				break;
			}
		}
		break;
	case RSP_MOVE_WORD_POINTS:
		{
			uint32 offset = (word0>>8)&0xFFFF;
			uint32 vtx = offset/40;
			uint32 where = offset - vtx*40;
			ModifyVertexInfo(where, vtx, word1);
		}
		break;
	case RSP_MOVE_WORD_PERSPNORM:
		LOG_DL("    RSP_MOVE_WORD_PERSPNORM");
		//if( word1 != 0x1A ) DebuggerAppendMsg("PerspNorm: 0x%04x", (short)word1);	
		break;
	default:
		RSP_RDP_NOIMPL("Unknown MoveWord, %08X, %08X", word0, word1);
		break;
	}

}


void RSP_GBI1_PopMtx(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_PopMtx);

	BYTE nCommand = (BYTE)(word1 & 0xFF);

	LOG_DL("    Command: 0x%02x (%s)",	nCommand,  (nCommand & RSP_MATRIX_PROJECTION) ? "Projection" : "ModelView");

	// Do any of the other bits do anything?
	// So far only Extreme-G seems to Push/Pop projection matrices

	if (nCommand & RSP_MATRIX_PROJECTION)
	{
		CRender::g_pRender->PopProjection();
	}
	else
	{
		CRender::g_pRender->PopWorldView();
	}
#ifdef _DEBUG
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		pauseAtNext = false;
		debuggerPause = true;
		DebuggerAppendMsg("Pause after Pop Matrix: %s\n", (nCommand & RSP_MATRIX_PROJECTION) ? "Proj":"World");
	}
	else
	{
		if( pauseAtNext && logMatrix ) 
		{
			DebuggerAppendMsg("Pause after Pop Matrix: %s\n", (nCommand & RSP_MATRIX_PROJECTION) ? "Proj":"World");
		}
	}
#endif
}



void RSP_GBI1_CullDL(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_CullDL);

#ifdef _DEBUG
	if( !debuggerEnableCullFace )
	{
		return;	//Disable Culling
	}
#endif
	if( g_curRomInfo.bDisableCulling )
	{
		return;	//Disable Culling
	}

	uint32 i;
	uint32 dwVFirst = ((word0) & 0xFFF) / gRSP.vertexMult;
	uint32 dwVLast  = ((word1) & 0xFFF) / gRSP.vertexMult;

	LOG_DL("    Culling using verts %d to %d", dwVFirst, dwVLast);

	// Mask into range
	dwVFirst &= 0x1f;
	dwVLast &= 0x1f;

	if( dwVLast < dwVFirst )	return;
	if( !gRSP.bRejectVtx )	return;

	for (i = dwVFirst; i <= dwVLast; i++)
	{
		if (g_clipFlag[i] == 0)
		{
			LOG_DL("    Vertex %d is visible, continuing with display list processing", i);
			return;
		}
	}

	status.dwNumDListsCulled++;

	LOG_DL("    No vertices were visible, culling rest of display list");

	RDP_GFX_PopDL();
}



void RSP_GBI1_Tri1(uint32 word0, uint32 word1)
{
	status.primitiveType = PRIM_TRI1;
	bool bTrisAdded = false;
	bool textureIsEnabled = CRender::g_pRender->IsTextureEnabled();

	// While the next command pair is Tri1, add vertices
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;
	uint32 * pCmdBase = (uint32 *)(g_pRDRAMu8 + dwPC);
	
	do
	{
		//uint32 dwFlag = (word1>>24)&0xFF;
		// Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
		uint32 dwV0 = ((word1>>16)&0xFF)/gRSP.vertexMult;
		uint32 dwV1 = ((word1>>8 )&0xFF)/gRSP.vertexMult;
		uint32 dwV2 = ((word1    )&0xFF)/gRSP.vertexMult;

		if (TestTri(dwV0, dwV1, dwV2))
		{
			DEBUG_DUMP_VERTEXES("Tri1", dwV0, dwV1, dwV2);
			LOG_DL("    Tri1: 0x%08x 0x%08x %d,%d,%d", word0, word1, dwV0, dwV1, dwV2);

			if (!bTrisAdded)
			{
				if( textureIsEnabled )
			{
				PrepareTextures();
				InitVertexTextureConstants();
			}
				CRender::g_pRender->SetCombinerAndBlender();
				bTrisAdded = true;
			}
			AddTri(dwV0, dwV1, dwV2);
		}

		word0			= *pCmdBase++;
		word1			= *pCmdBase++;
		dwPC += 8;

#ifdef _DEBUG
	} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (word0>>24) == (BYTE)RSP_TRI1);
	if( pauseAtNext && eventToPause==NEXT_TRIANGLE ) 
	{
		eventToPause = NEXT_FLUSH_TRI;
	}
#else
	} while ((word0>>24) == (BYTE)RSP_TRI1);
#endif

	g_dwPCStack[g_dwPCindex].addr = dwPC-8;

	if (bTrisAdded)	
	{
		CRender::g_pRender->FlushTris();
	}
}


void RSP_GBI0_Tri4(uint32 word0, uint32 word1)
{
	status.primitiveType = PRIM_TRI2;

	// While the next command pair is Tri2, add vertices
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	do {
		uint32 dwFlag = (word0>>16)&0xFF;
		LOG_DL("    PD Tri4: 0x%08x 0x%08x Flag: 0x%02x", word0, word1, dwFlag);

		BOOL bVisible;
		for( int i=0; i<4; i++)
		{
			uint32 v0 = (word1>>(4+(i<<3))) & 0xF;
			uint32 v1 = (word1>>(  (i<<3))) & 0xF;
			uint32 v2 = (word0>>(  (i<<2))) & 0xF;
			bVisible = TestTri(v0, v2, v1);
			LOG_DL("       (%d, %d, %d) %s", v0, v1, v2, bVisible ? "": "(clipped)");
			if (bVisible)
			{
				DEBUG_DUMP_VERTEXES("Tri4_PerfectDark 1/2", v0, v1, v2);
				if (!bTrisAdded && CRender::g_pRender->IsTextureEnabled())
				{
					PrepareTextures();
					InitVertexTextureConstants();
				}

				if( !bTrisAdded )
				{
					CRender::g_pRender->SetCombinerAndBlender();
				}

				bTrisAdded = TRUE;
				AddTri(v0, v2, v1);
			}
		}
		
		word0			= *(uint32 *)(g_pRDRAMu8 + dwPC+0);
		word1			= *(uint32 *)(g_pRDRAMu8 + dwPC+4);
		dwPC += 8;

#ifdef _DEBUG
	} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (word0>>24) == (BYTE)RSP_TRI2);
	if(pauseAtNext && eventToPause==NEXT_TRIANGLE )
	{
		eventToPause = NEXT_FLUSH_TRI;
	}

#else
	} while ((word0>>24) == (BYTE)RSP_TRI2);
#endif


	g_dwPCStack[g_dwPCindex].addr = dwPC-8;


	if (bTrisAdded)	
	{
		CRender::g_pRender->FlushTris();
	}
	
	gRSP.DKRVtxCount=0;
}

//Nintro64 uses Sprite2d 


void RSP_RDP_Nothing(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_RDP_Nothing);

#ifdef _DEBUG
	if( logUnknownUcode )
	{
		TRACE0("Stack Trace");
		for( int i=0; i<g_dwPCindex; i++ )
		{
			DebuggerAppendMsg("  %08X", g_dwPCStack[i].addr);
		}
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DebuggerAppendMsg("PC=%08X",dwPC);
		DebuggerAppendMsg("Warning, unknown ucode PC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
	}
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_UNKNOWN_OP, {TRACE0("Paused at unknown ucode\n");})
	if( debuggerContinueWithUnknown )
	{
		return;
	}
#endif
		
	if( options.bEnableHacks )
		return;
	
	status.bRDPHalted = TRUE;
	g_dwPCindex=-1;
}


void RSP_RDP_InsertMatrix(uint32 word0, uint32 word1)
{
	float fraction;

	UpdateCombinedMatrix();

	if (word0 & 0x20)
	{
		int x = (word0 & 0x1F) >> 1;
		int y = x >> 2;
		x &= 3;

		fraction = (word1>>16)/65536.0f;
		gRSPworldProject.m[y][x] = (float)(int)gRSPworldProject.m[y][x];
		gRSPworldProject.m[y][x] += fraction;

		fraction = (word1&0xFFFF)/65536.0f;
		gRSPworldProject.m[y][x+1] = (float)(int)gRSPworldProject.m[y][x+1];
		gRSPworldProject.m[y][x+1] += fraction;
	}
	else
	{
		int x = (word0 & 0x1F) >> 1;
		int y = x >> 2;
		x &= 3;

		fraction = (float)fabs(gRSPworldProject.m[y][x] - (int)gRSPworldProject.m[y][x]);
		gRSPworldProject.m[y][x] = (short)(word1>>16) + fraction;

		fraction = (float)fabs(gRSPworldProject.m[y][x+1] - (int)gRSPworldProject.m[y][x+1]);
		gRSPworldProject.m[y][x+1] = (short)(word1&0xFFFF) + fraction;
	}

	gRSP.bMatrixIsUpdated = false;
	gRSP.bCombinedMatrixIsUpdated = true;

#ifdef _DEBUG
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		pauseAtNext = false;
		debuggerPause = true;
		DebuggerAppendMsg("Pause after insert matrix: %08X, %08X", word0, word1);
	}
	else
	{
		if( pauseAtNext && logMatrix ) 
		{
			DebuggerAppendMsg("insert matrix: %08X, %08X", word0, word1);
		}
	}
#endif
}