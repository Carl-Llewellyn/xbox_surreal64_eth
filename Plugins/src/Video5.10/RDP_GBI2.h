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

// SP ucode for GBI2


/*	
cc = cmd
ff = first
ll = len
          ccffll??
802C80C0: 01004008 01001C00 CMD Zelda_LOADVTX  Vertex 00..03 at 802C7480
80317030: 01020040 08000EB0 CMD Zelda_LOADVTX  Vertex 00..31 at 802048D0
80317140: 01010020 08001470 CMD Zelda_LOADVTX  Vertex 00..15 at 80204E90
80317228: 0100F01E 08001670 CMD Zelda_LOADVTX  Vertex 00..14 at 80205090
80315FE8: 0100D01A 0600A580 CMD Zelda_LOADVTX  Vertex 00..12 at 803158D0
80316060: 0101B036 0600A650 CMD Zelda_LOADVTX  Vertex 00..26 at 803159A0

802F9C48: 0100E024 0602D9E0 CMD Zelda_LOADVTX  Vertex 04..17 at 802F7150

*/
void DLParser_GBI2_Vtx(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwVEnd    = ((dwCmd0   )&0xFFF)/2;
	DWORD dwN      = ((dwCmd0>>12)&0xFFF);

	DWORD dwV0		= dwVEnd - dwN;

	DL_PF("    Vtx: Address 0x%08x, vEnd: %d, v0: %d, Num: %d", dwAddress, dwVEnd, dwV0, dwN);

	if( dwVEnd > 64 )
	{
		DebuggerAppendMsg("Warning, attempting to load into invalid vertex positions, v0=%d, n=%d", dwV0, dwN);
		return;
	}

	// Check that address is valid...
	if ((dwAddress + (dwN*16)) > g_dwRamSize)
	{
		DebuggerAppendMsg("SetNewVertexInfo: Address out of range (0x%08x)", dwAddress);
	}
	else
	{
		SetNewVertexInfo(dwAddress, dwV0, dwN);

		status.dwNumVertices += dwN;

		RDP_DumpVtxInfo(dwAddress, dwV0, dwN);
	}

}



void DLParser_GBI2_EndDL(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_EndDL);

	RDP_GFX_PopDL();
}

void DLParser_GBI2_CullDL(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_CullDL);

#ifdef _DEBUG
	if( !debuggerEnableCullFace )
	{
		return;	//Disable Culling
	}
#endif

	DWORD i;
	//TODO - check mask 
	DWORD dwVFirst = ((dwCmd0) & 0xfff) / gRSP.vertexMult;
	DWORD dwVLast  = ((dwCmd1) & 0xfff) / gRSP.vertexMult;

	DL_PF("    Culling using verts %d to %d", dwVFirst, dwVLast);

	// Mask into range
	dwVFirst &= 0x1f;
	dwVLast &= 0x1f;

	if( dwVLast < dwVFirst )	return;
	if( !gRSP.bRejectVtx )	return;

	for (i = dwVFirst; i <= dwVLast; i++)
	{
		//if (g_dwVecFlags[i] == 0)
		if (g_clipFlag[i] == 0)
		{
			DL_PF("    Vertex %d is visible, returning", i);
			return;
		}
	}

	status.dwNumDListsCulled++;

	DL_PF("    No vertices were visible, culling");

	RDP_GFX_PopDL();
}


//0016A710: DB020000 00000018 CMD Zelda_MOVEWORD  Mem[2][00]=00000018 Lightnum=0
//001889F0: DB020000 00000030 CMD Zelda_MOVEWORD  Mem[2][00]=00000030 Lightnum=2


void DLParser_GBI2_MoveWord(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_MoveWord);

	DWORD dwType   = (dwCmd0 >> 16) & 0xFF;
	DWORD dwOffset = (dwCmd0      ) & 0xFFFF;
	DL_PF("     MoveWord: type=%08X, offset=%08X", dwType, dwOffset);

	switch (dwType)
	{
	case G_MW_NUMLIGHT:
		{
			// Lightnum
			// dwCmd1:
			// 0x18 = 24 = 0 lights
			// 0x30 = 48 = 2 lights

			//DWORD dwNumLights = (dwCmd1/12) - 2;
			DWORD dwNumLights = (dwCmd1/24);
			DL_PF("     G_MW_NUMLIGHT: %d", dwNumLights);

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
			default:
				DL_PF("     G_MW_CLIP  ?   : 0x%08x", dwCmd1);
				break;
			}

			//RDP_NOIMPL_WARN("G_MW_CLIP Not Implemented");
		}
		break;

	case G_MW_SEGMENT:
		{
			DWORD dwSeg     = dwOffset / 4;
			DWORD dwAddress = dwCmd1 & 0x00FFFFFF;			// Hack - convert to physical

			DL_PF("      G_MW_SEGMENT Segment[%d] = 0x%08x",	dwSeg, dwAddress);
			if( dwAddress > g_dwRamSize )
			{
#ifdef _DEBUG
				if( pauseAtNext )
					DebuggerAppendMsg("warning: Segment %d addr is %8X", dwSeg, dwAddress);
#endif
			}
			else
			{
				gRSP.segments[dwSeg] = dwAddress;
			}
		}
		break;
	case G_MW_FOG:
		{
			//0x7d00
			//0x8400
			WORD wMult = (WORD)((dwCmd1 >> 16) & 0xFFFF);
			WORD wOff  = (WORD)((dwCmd1      ) & 0xFFFF);

			float fMult = (float)(short)wMult;
			float fOff = (float)(short)wOff;

			float rng = 128000.0f / fMult;
			float fMin = 500.0f - (fOff*rng/256.0f);
			float fMax = rng + fMin;
			SetFogMinMax(fMin, fMax, fMult, fOff);
		}
		break;
	case G_MW_LIGHTCOL:
		{
			//ZeldaMoveWord: 0xdb0a0000 0xafafaf00
			//ZeldaMoveWord: 0xdb0a0004 0xafafaf00
			//ZeldaMoveWord: 0xdb0a0018 0x46464600
			//ZeldaMoveWord: 0xdb0a001c 0x46464600

			DWORD dwLight = dwOffset / 0x18;
			DWORD dwField = (dwOffset & 0x7);

			DL_PF("     G_MW_LIGHTCOL/0x%08x: 0x%08x", dwOffset, dwCmd1);

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
				DebuggerAppendMsg("G_MW_LIGHTCOL with unknown offset 0x%08x", dwField);
				break;
			}


		}
		break;

	case G_MW_PERSPNORM:
		DL_PF("     G_MW_PERSPNORM 0x%04x", (short)dwCmd1);
		break;

	case G_MW_POINTS:
		DL_PF("     2nd cmd of Force Matrix");
		break;

	default:
		{
			DL_PF("      Ignored!!");

		}
		break;
	}
}



/*
Zelda: Reserved0(0x022516ff 0x002814ff)
Zelda: Reserved0(0x022d18ff 0x004c22ff)
Zelda: Reserved0(0x024d2bff 0x187347ff)
SetNewVertexInfo: Address out of range (0x00c0f53f)
Zelda: Reserved0(0x022f1dff 0x085031ff)
SetNewVertexInfo: Address out of range (0x009c9b1f)
SetNewVertexInfo: Address out of range (0x00cc1e3f)
Zelda: Reserved0(0x02301cff 0x052618ff)
Zelda: Reserved0(0x020d1de2 0x0f1d3fff)
Warning, attempting to load into invalid vertex positions
Warning, attempting to load into invalid vertex positions
Warning, attempting to load into invalid vertex positions
Zelda: Reserved0(0x020815ff 0x05142eff)
Exception processing 0x05050300 0x00000000*/

void RDP_S2DEX_SPObjLoadTxtr(DWORD dwCmd0, DWORD dwCmd1);

void DLParser_GBI2_Tri1(DWORD dwCmd0, DWORD dwCmd1)
{
	if( dwCmd0 == 0x05000017 && (dwCmd1>>24) == 0x80 )
	{
		// The ObjLoadTxtr / Tlut cmd for Evangelion.v64
		RDP_S2DEX_SPObjLoadTxtr(dwCmd0, dwCmd1);
		DebuggerAppendMsg("Fix me, SPObjLoadTxtr as DLParser_GBI2_Tri2");
	}
	else
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
			DWORD dwV2 = ((dwCmd0>>16)&0xFF)/gRSP.vertexMult;
			DWORD dwV1 = ((dwCmd0>>8 )&0xFF)/gRSP.vertexMult;
			DWORD dwV0 = ((dwCmd0   )&0xFF)/gRSP.vertexMult;

			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri1", dwV0, dwV1, dwV2);
				DL_PF("    ZeldaTri1: 0x%08x 0x%08x %d,%d,%d", dwCmd0, dwCmd1, dwV0, dwV1, dwV2);
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

		} while ((dwCmd0>>24) == (BYTE)G_ZELDATRI1);
		g_dwPCStack[g_dwPCindex].addr = dwPC-8;


		if (bTrisAdded)	
		{
			CDaedalusRender::g_pRender->FlushTris();
		}
	}
}


//802C80C8: 06000204 00000406 CMD Zelda_TRI2  (2,1,0)
//00284030: 07002026 00002624 CMD Zelda_TRI3  (19,16,0)
void RDP_S2DEX_SPObjLoadTxSprite(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_SPObjLoadTxRect(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_SPObjLoadTxRectR(DWORD dwCmd0, DWORD dwCmd1);

void DLParser_GBI2_Tri2(DWORD dwCmd0, DWORD dwCmd1)
{
	if( dwCmd0 == 0x0600002f && (dwCmd1>>24) == 0x80 )
	{
		// The ObjTxSprite cmd for Evangelion.v64
		RDP_S2DEX_SPObjLoadTxSprite(dwCmd0, dwCmd1);
		DebuggerAppendMsg("Fix me, SPObjLoadTxSprite as DLParser_GBI2_Tri2");
	}
	else
	{
		status.primitiveType = PRIM_TRI2;
		BOOL bTrisAdded = FALSE;

		// While the next command pair is Tri2, add vertices
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;
		bool textureIsEnabled = CDaedalusRender::g_pRender->IsTextureEnabled();

		do {
			/*
			DWORD dwV2 = ((dwCmd1>>16)&0xFF)/gRSP.vertexMult;
			DWORD dwV1 = ((dwCmd1>>8 )&0xFF)/gRSP.vertexMult;
			DWORD dwV0 = ((dwCmd1    )&0xFF)/gRSP.vertexMult;

			DWORD dwV5 = ((dwCmd0>>16)&0xFF)/gRSP.vertexMult;
			DWORD dwV4 = ((dwCmd0>>8 )&0xFF)/gRSP.vertexMult;
			DWORD dwV3 = ((dwCmd0    )&0xFF)/gRSP.vertexMult;
			*/
			DWORD dwV2 = ((dwCmd1>>17)&0x7F);
			DWORD dwV1 = ((dwCmd1>>9 )&0x7F);
			DWORD dwV0 = ((dwCmd1>>1 )&0x7F);

			DWORD dwV5 = ((dwCmd0>>17)&0x7F);
			DWORD dwV4 = ((dwCmd0>>9 )&0x7F);
			DWORD dwV3 = ((dwCmd0>>1 )&0x7F);

			DL_PF("    ZeldaTri2: 0x%08x 0x%08x", dwCmd0, dwCmd1);
			DL_PF("           V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
			DL_PF("           V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

			// Do first tri
			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri2 1/2", dwV0, dwV1, dwV2);
				if (!bTrisAdded)
				{
					if( textureIsEnabled )
				{
					SetupTextures();
					InitVertexTextureConstants();
				}
					CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();
					bTrisAdded = TRUE;
				}

				AddTri(dwV0, dwV1, dwV2);
			}

			// Do second tri
			if (TestTri(dwV3, dwV4, dwV5))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri2 2/2", dwV3, dwV4, dwV5);
				if (!bTrisAdded)
				{
					if( textureIsEnabled )
				{
					SetupTextures();
					InitVertexTextureConstants();
				}
					CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();
					bTrisAdded = TRUE;
				}

				AddTri(dwV3, dwV4, dwV5);
			}
			
			dwCmd0			= *(DWORD *)(g_pu8RamBase + dwPC+0);
			dwCmd1			= *(DWORD *)(g_pu8RamBase + dwPC+4);
			dwPC += 8;

		} while ((dwCmd0>>24) == (BYTE)G_ZELDATRI2);

		g_dwPCStack[g_dwPCindex].addr = dwPC-8;

		if (bTrisAdded)	
		{
			CDaedalusRender::g_pRender->FlushTris();
		}
	}
}

//00284030: 07002026 00002624 CMD Zelda_TRI3  (19,16,0)
void DLParser_GBI2_Line3D(DWORD dwCmd0, DWORD dwCmd1)
{
	if( dwCmd0 == 0x0700002f && (dwCmd1>>24) == 0x80 )
	{
		// The ObjTxSprite cmd for Evangelion.v64
		RDP_S2DEX_SPObjLoadTxRect(dwCmd0, dwCmd1);
	}
	else
	{
		status.primitiveType = PRIM_TRI3;

		// While the next command pair is Tri2, add vertices
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;

		BOOL bTrisAdded = FALSE;

		do {
			// Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
			DWORD dwV2 = ((dwCmd1>>16)&0xFF)/gRSP.vertexMult;
			DWORD dwV1 = ((dwCmd1>>8 )&0xFF)/gRSP.vertexMult;
			DWORD dwV0 = ((dwCmd1    )&0xFF)/gRSP.vertexMult;

			DWORD dwV5 = ((dwCmd0>>16)&0xFF)/gRSP.vertexMult;
			DWORD dwV4 = ((dwCmd0>>8 )&0xFF)/gRSP.vertexMult;
			DWORD dwV3 = ((dwCmd0    )&0xFF)/gRSP.vertexMult;

			DL_PF("    ZeldaTri3: 0x%08x 0x%08x", dwCmd0, dwCmd1);
			DL_PF("           V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
			DL_PF("           V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

			// Do first tri
			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri3 1/2", dwV0, dwV1, dwV2);
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
			if (TestTri(dwV3, dwV4, dwV5))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri3 2/2", dwV3, dwV4, dwV5);
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
				AddTri(dwV3, dwV4, dwV5);
			}
			
			dwCmd0			= *(DWORD *)(g_pu8RamBase + dwPC+0);
			dwCmd1			= *(DWORD *)(g_pu8RamBase + dwPC+4);
			dwPC += 8;

		} while ((dwCmd0>>24) == (BYTE)G_LINE3D);


		g_dwPCStack[g_dwPCindex].addr = dwPC-8;


		if (bTrisAdded)	
		{
			CDaedalusRender::g_pRender->FlushTris();
		}
	}
}



//802FA128: D7000002 FFFFFFFF CMD Zelda_TEXTURE  texture on=2 tile=0 s=1.00 t=1.00
//000F8AA8: D7000000 FFFFFFFF CMD Zelda_TEXTURE  texture on=0 tile=0 s=1.00 t=1.00

// NOT CHECKED!
void DLParser_GBI2_Texture(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_Texture);

	//DWORD dwBowTie = (dwCmd0>>16)&0xFF;		// Don't care about this
	DWORD dwLevel  = (dwCmd0>>11)&0x07;
	DWORD dwTile= (dwCmd0>>8 )&0x07;

	bool bTextureEnable  = (dwCmd0)&0x02 ? true : false;		// Important! not 0x01!?!?
	float fTextureScaleS = (float)((dwCmd1>>16)&0xFFFF) / (65535.0f * 32.0f);
	float fTextureScaleT = (float)((dwCmd1    )&0xFFFF) / (65535.0f * 32.0f);

	if( ((dwCmd1>>16)&0xFFFF) == 0xFFFF )
	{
		fTextureScaleS = 1/32.0f;
	}
	else if( ((dwCmd1>>16)&0xFFFF) == 0x8000 )
	{
		fTextureScaleS = 1/64.0f;
	}
	if( ((dwCmd1    )&0xFFFF) == 0xFFFF )
	{
		fTextureScaleT = 1/32.0f;
	}
	else if( ((dwCmd1    )&0xFFFF) == 0x8000 )
	{
		fTextureScaleT = 1/64.0f;
	}


	/*
	if( g_curRomInfo.bTextureScaleHack )
	{
	// Hack, need to verify, refer to N64 programming manual
	// that if scale = 0.5 (1/64), vtx s,t are also doubled

		if( ((dwCmd1>>16)&0xFFFF) == 0x8000 )
		{
			fTextureScaleS = 1/128.0f;
			if( ((dwCmd1)&0xFFFF) == 0xFFFF )
			{
				fTextureScaleT = 1/64.0f;
			}
		}

		if( ((dwCmd1    )&0xFFFF) == 0x8000 )
		{
			fTextureScaleT = 1/128.0f;
			if( ((dwCmd1>>16)&0xFFFF) == 0xFFFF )
			{
				fTextureScaleS = 1/64.0f;
			}
		}
	}
	*/

	CDaedalusRender::g_pRender->SetTextureEnableAndScale(dwTile, bTextureEnable, fTextureScaleS, fTextureScaleT);

	// What happens if these are 0? Interpret as 1.0f?

	DL_PF("    Level: %d Tile: %d %s", dwLevel, dwTile, bTextureEnable ? "enabled":"disabled");
	DL_PF("    ScaleS: %f, ScaleT: %f", fTextureScaleS*32.0f, fTextureScaleT*32.0f);
}



void DLParser_GBI2_PopMtx(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_PopMtx);

	BYTE nCommand = (BYTE)(dwCmd0 & 0xFF);

	DL_PF("        PopMtx: 0x%02x (%s)",
		nCommand, 
		(nCommand & G_ZMTX_PROJECTION) ? "Projection" : "ModelView");


/*	if (nCommand & G_ZMTX_PROJECTION)
	{
		CDaedalusRender::g_pRender->PopProjection();
	}
	else*/
	{
		CDaedalusRender::g_pRender->PopWorldView();
	}
#ifdef _DEBUG
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		pauseAtNext = false;
		debuggerPause = true;
		TRACE0("Pause after Pop GBI2_PopMtx:");
	}
	else
	{
		if( (pauseAtNext /*&& (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) */) && logTriDetailsWithVertexMtx ) 
		{
			TRACE0("Pause after Pop GBI2_PopMtx:");
		}
	}
#endif

}


#define G_ZELDA_ZBUFFER				0x00000001		// Guess
#define G_ZELDA_CULL_BACK			0x00000200
#define G_ZELDA_CULL_FRONT			0x00000400
#define G_ZELDA_FOG					0x00010000
#define G_ZELDA_LIGHTING			0x00020000
#define G_ZELDA_TEXTURE_GEN			0x00040000
#define G_ZELDA_TEXTURE_GEN_LINEAR	0x00080000
#define G_ZELDA_SHADING_SMOOTH		0x00200000

/*
00000000 00101110 00000100 00000101    002E0405  texgen(0) flat(1) cull(2) light(1)
00000000 00100011 00000100 00000101    00230405  texgen(0) flat(0) cull(2) light(1)
           n FFL  cccccccc xxxxxxxL?
00000000 00000000 00000000 00000000
*mode geometry: 002E0405  texgen(0) flat(1) cull(2) light(1)
*mode geometry: 00230405  texgen(0) flat(0) cull(2) light(1)
*mode geometry: 00220405  texgen(0) flat(0) cull(2) light(1)
*mode geometry: 00220405  texgen(0) flat(0) cull(2) light(1)
*mode geometry: 00220405  texgen(0) flat(0) cull(2) light(1)
*mode geometry: 00200404  texgen(0) flat(0) cull(2) light(0)
*mode geometry: 00200204  texgen(0) flat(0) cull(1) light(0)
*mode geometry: 00200000  texgen(0) flat(0) cull(0) light(0)
*mode geometry: 00200000  texgen(0) flat(0) cull(0) light(0)
*mode geometry: 00000000  texgen(0) flat(0) cull(0) light(0)
*/

// Seems to be AND (dwCmd0&0xFFFFFF) OR (dwCmd1&0xFFFFFF)
void DLParser_GBI2_GeometryMode(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI2_GeometryMode);

	DWORD dwAnd = (dwCmd0) & 0x00FFFFFF;
	DWORD dwOr  = (dwCmd1) & 0x00FFFFFF;

#ifdef _DEBUG
		DL_PF("    0x%08x 0x%08x =(x & 0x%08x) | 0x%08x", dwCmd0, dwCmd1, dwAnd, dwOr);

		if ((~dwAnd) & G_ZELDA_ZBUFFER)					DL_PF("  Disabling ZBuffer");
		//	if ((~dwAnd) & G_ZELDA_TEXTURE_ENABLE)			DL_PF("  Disabling Texture");
		//	if ((~dwAnd) & G_ZELDA_SHADE)					DL_PF("  Disabling Shade");
		if ((~dwAnd) & G_ZELDA_SHADING_SMOOTH)			DL_PF("  Disabling Flat Shading");
		if ((~dwAnd) & G_ZELDA_CULL_FRONT)				DL_PF("  Disabling Front Culling");
		if ((~dwAnd) & G_ZELDA_CULL_BACK)				DL_PF("  Disabling Back Culling");
		if ((~dwAnd) & G_ZELDA_FOG)						DL_PF("  Disabling Fog");
		if ((~dwAnd) & G_ZELDA_LIGHTING)				DL_PF("  Disabling Lighting");
		if ((~dwAnd) & G_ZELDA_TEXTURE_GEN)				DL_PF("  Disabling Texture Gen");
		if ((~dwAnd) & G_ZELDA_TEXTURE_GEN_LINEAR)		DL_PF("  Disabling Texture Gen Linear");
		//	if ((~dwAnd) & G_ZELDA_LOD)						DL_PF("  Disabling LOD (no impl)");

		if (dwOr & G_ZELDA_ZBUFFER)						DL_PF("  Enabling ZBuffer");
		//	if (dwOr & G_ZELDA_TEXTURE_ENABLE)				DL_PF("  Enabling Texture");
		//	if (dwOr & G_ZELDA_SHADE)						DL_PF("  Enabling Shade");
		if (dwOr & G_ZELDA_SHADING_SMOOTH)				DL_PF("  Enabling Flat Shading");
		if (dwOr & G_ZELDA_CULL_FRONT)					DL_PF("  Enabling Front Culling");
		if (dwOr & G_ZELDA_CULL_BACK)					DL_PF("  Enabling Back Culling");
		if (dwOr & G_ZELDA_FOG)							DL_PF("  Enabling Fog");
		if (dwOr & G_ZELDA_LIGHTING)					DL_PF("  Enabling Lighting");
		if (dwOr & G_ZELDA_TEXTURE_GEN)					DL_PF("  Enabling Texture Gen");
		if (dwOr & G_ZELDA_TEXTURE_GEN_LINEAR)			DL_PF("  Enabling Texture Gen Linear");
		//	if (dwOr & G_ZELDA_LOD)							DL_PF("  Enabling LOD (no impl)");
#endif // _DEBUG

		gRDP.geometryMode &= dwAnd;
	gRDP.geometryMode |= dwOr;


	bool bCullFront		= (gRDP.geometryMode & G_ZELDA_CULL_FRONT) ? true : false;
	bool bCullBack		= (gRDP.geometryMode & G_ZELDA_CULL_BACK) ? true : false;
	
	BOOL bShade			= (gRDP.geometryMode & G_SHADE) ? TRUE : FALSE;
	//BOOL bFlatShade		= (gRDP.geometryMode & G_ZELDA_SHADING_SMOOTH) ? TRUE : FALSE;
	BOOL bFlatShade		= (gRDP.geometryMode & G_ZELDA_TEXTURE_GEN_LINEAR) ? TRUE : FALSE;
	
	BOOL bFog			= (gRDP.geometryMode & G_ZELDA_FOG) ? TRUE : FALSE;
	bool bTextureGen	= (gRDP.geometryMode & G_ZELDA_TEXTURE_GEN) ? true : false;

	bool bLighting      = (gRDP.geometryMode & G_ZELDA_LIGHTING) ? true : false;
	BOOL bZBuffer		= (gRDP.geometryMode & G_ZELDA_ZBUFFER)	? TRUE : FALSE;	

	CDaedalusRender::g_pRender->SetCullMode(bCullFront, bCullBack);
	
	//if (bFlatShade||!bShade)	CDaedalusRender::g_pRender->SetShadeMode( SHADE_FLAT );
	if (bFlatShade)	CDaedalusRender::g_pRender->SetShadeMode( SHADE_FLAT );
	else			CDaedalusRender::g_pRender->SetShadeMode( SHADE_SMOOTH );
	
	CDaedalusRender::g_pRender->SetFogEnable( bFog );
	SetTextureGen(bTextureGen);

	SetLighting( bLighting );
	CDaedalusRender::g_pRender->ZBufferEnable( bZBuffer );

	//RDP_GFX_InitGeometryMode();
}


/*
0016A748: DA380007 801764C8 CMD Zelda_LOADMTX  {Matrix} at 001764C8  Load:Prj 
Load matrix(1):
0016A750: DA380005 80176488 CMD Zelda_LOADMTX  {Matrix} at 00176488  Mul:Prj 
Mul matrix(1):
0016A760: DA380003 80176448 CMD Zelda_LOADMTX  {Matrix} at 00176448  Load:Mod 
Load matrix(0):
0016AA70: DA380000 80175F88 CMD Zelda_LOADMTX  {Matrix} at 00175F88  Mul:Mod Push 
Mul matrix(0): (push)


#define	G_MTX_MODELVIEW		0x00
#define	G_MTX_PROJECTION	0x01
#define	G_MTX_MUL			0x00
#define	G_MTX_LOAD			0x02
#define G_MTX_NOPUSH		0x00
#define G_MTX_PUSH			0x04
*/

extern DWORD dwConkerVtxZAddr;
void DLParser_GBI2_Mtx(DWORD dwCmd0, DWORD dwCmd1)
{	
	SP_Timing(DLParser_GBI0_Mtx);
	dwConkerVtxZAddr = 0;	// For Conker BFD

	DaedalusMatrix mat;
	DWORD dwI;
	DWORD dwJ;
	const float fRecip = 1.0f / 65536.0f;
	DWORD dwAddress = RDPSegAddr(dwCmd1);

	// THESE ARE SWAPPED OVER FROM NORMAL MTX
	// Not sure if this is right!!!!
	DWORD dwCommand = (dwCmd0)&0xFF;
	DWORD dwLength  = (dwCmd0 >> 8)&0xFFFF;

	if( dwCommand == 0 && dwLength == 0 )
	{
		DLParser_Bomberman2TextRect(dwCmd0, dwCmd1);
		return;
	}

	DL_PF("    Mtx: %s %s %s Length %d Address 0x%08x",
		(dwCommand & G_ZMTX_PROJECTION) ? "Projection" : "ModelView",
		(dwCommand & G_ZMTX_LOAD) ? "Load" : "Mul",	
		(dwCommand & G_ZMTX_NOPUSH) ? "NoPush" : "Push",
		dwLength, dwAddress);

	if (dwAddress + 64 > g_dwRamSize)
	{
		DebuggerAppendMsg("ZeldaMtx: Address invalid (0x%08x)", dwAddress);
		return;
	}

	// Load matrix from dwAddress	
	for (dwI = 0; dwI < 4; dwI++)
	{
		for (dwJ = 0; dwJ < 4; dwJ++) 
		{
			SHORT nDataHi = *(SHORT *)(g_pu8RamBase + ((dwAddress+(dwI<<3)+(dwJ<<1)     )^0x2));
			WORD  nDataLo = *(WORD  *)(g_pu8RamBase + ((dwAddress+(dwI<<3)+(dwJ<<1) + 32)^0x2));

			mat.m[dwI][dwJ] = (float)(((LONG)nDataHi<<16) | (nDataLo))*fRecip;
		}
	}

#ifdef _DEBUG
	DL_PF(
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n"
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n"
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n"
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n",
		mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
		mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
		mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
		mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);
#endif // _DEBUG

	LONG nLoadCommand = (dwCommand & G_ZMTX_LOAD) ? RENDER_LOAD_MATRIX : RENDER_MUL_MATRIX;
	BOOL bPush = (dwCommand & G_ZMTX_NOPUSH) ? FALSE : TRUE;


#ifdef USING_INT_MATRIX
	N64IntMatrix intmtx((DWORD)(g_pu8RamBase+dwAddress));
#endif

	if (dwCommand & G_ZMTX_PROJECTION)
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

	//DEBUGGER_PAUSE_AND_DUMP(NEXT_MATRIX_CMD, {DebuggerAppendMsg("GBI2 Mtx: Cmd0=%08X, Cmd1=%08X", dwCmd0, dwCmd1);});
#ifdef _DEBUG
	char *loadstr = nLoadCommand==RENDER_LOAD_MATRIX?"Load":"Mul";
	char *pushstr = bPush?"Push":"Nopush";
	int projlevel = CDaedalusRender::g_pRender->GetProjectMatrixLevel();
	int worldlevel = CDaedalusRender::g_pRender->GetWorldViewMatrixLevel();
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		pauseAtNext = false;
		debuggerPause = true;
		if (dwCommand & G_ZMTX_PROJECTION)
		{
			DebuggerAppendMsg("Pause after %s and %s Matrix: Projection, level=%d\n", loadstr, pushstr, projlevel );
		}
		else
		{
			DebuggerAppendMsg("Pause after %s and %s Matrix: WorldView level=%d\n", loadstr, pushstr, worldlevel);
		}
	}
	else
	{
		if( (pauseAtNext /*&& (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) */) && logTriDetailsWithVertexMtx ) 
		{
			if (dwCommand & G_ZMTX_PROJECTION)
			{
				DebuggerAppendMsg("Matrix: %s and %s Projection level=%d\n", loadstr, pushstr, projlevel);
			}
			else
			{
				DebuggerAppendMsg("Matrix: %s and %s WorldView\n level=%d", loadstr, pushstr, worldlevel);
			}
		}
	}
#endif
}


/*

001889F8: DC08060A 80188708 CMD Zelda_MOVEMEM  Movemem[0806] <- 80188708
!light 0 color 0.12 0.16 0.35 dir 0.01 0.00 0.00 0.00 (2 lights) [ 1E285A00 1E285A00 01000000 00000000 ]
data(00188708): 1E285A00 1E285A00 01000000 00000000 
00188A00: DC08090A 80188718 CMD Zelda_MOVEMEM  Movemem[0809] <- 80188718
!light 1 color 0.23 0.25 0.30 dir 0.01 0.00 0.00 0.00 (2 lights) [ 3C404E00 3C404E00 01000000 00000000 ]
data(00188718): 3C404E00 3C404E00 01000000 00000000 
00188A08: DC080C0A 80188700 CMD Zelda_MOVEMEM  Movemem[080C] <- 80188700
!light 2 color 0.17 0.16 0.26 dir 0.23 0.31 0.70 0.00 (2 lights) [ 2C294300 2C294300 1E285A00 1E285A00 ]
*/
/*
ZeldaMoveMem: 0xdc080008 0x801984d8
SetScissor: x0=416 y0=72 x1=563 y1=312 mode=0
// Mtx
ZeldaMoveWord:0xdb0e0000 0x00000041 Ignored
ZeldaMoveMem: 0xdc08000a 0x80198538
ZeldaMoveMem: 0xdc08030a 0x80198548

ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198518
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198528
ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198538
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198548
ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198518
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198528
ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198538
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198548


0xa4001120: <0x0c000487> JAL       0x121c        Seg2Addr(t8)				dram
0xa4001124: <0x332100fe> ANDI      at = t9 & 0x00fe
0xa4001128: <0x937309c1> LBU       s3 <- 0x09c1(k1)							len
0xa400112c: <0x943402f0> LHU       s4 <- 0x02f0(at)							dmem
0xa4001130: <0x00191142> SRL       v0 = t9 >> 0x0005
0xa4001134: <0x959f0336> LHU       ra <- 0x0336(t4)
0xa4001138: <0x080007f6> J         0x1fd8        SpMemXfer
0xa400113c: <0x0282a020> ADD       s4 = s4 + v0								dmem

ZeldaMoveMem: 0xdc08000a 0x8010e830 Type: 0a Len: 08 Off: 4000
ZeldaMoveMem: 0xdc08030a 0x8010e840 Type: 0a Len: 08 Off: 4018
// Light
ZeldaMoveMem: 0xdc08060a 0x800ff368 Type: 0a Len: 08 Off: 4030
ZeldaMoveMem: 0xdc08090a 0x800ff360 Type: 0a Len: 08 Off: 4048
//VP
ZeldaMoveMem: 0xdc080008 0x8010e3c0 Type: 08 Len: 08 Off: 4000

*/
void RDP_S2DEX_OBJ_MOVEMEM(DWORD dwCmd0, DWORD dwCmd1);

void DLParser_GBI2_MoveMem(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_MoveMem);


	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwOffset = (dwCmd0 >> 8) & 0xFFFF;
	DWORD dwType    = (dwCmd0     ) & 0xFE;

	DWORD dwLen = (dwCmd0 >> 16) & 0xFF;
	DWORD dwOffset2 = (dwCmd0 >> 5) & 0x3FFF;

	DL_PF("    MoveMem: Type: %02x Len: %02x Off: %04x", dwType, dwLen, dwOffset2);

	BOOL bHandled = FALSE;

	switch (dwType)
	{
	case F3DEX_GBI_2X_G_MV_VIEWPORT:
		{
			RDP_MoveMemViewport(dwAddress);
			bHandled = TRUE;
		}
		break;
	case F3DEX_GBI_2X_G_MV_LIGHT:
		{
		switch (dwOffset2)
		{

			/*
					{	{{ {{0,0,0},0,{0,0,0},0,{rightx,righty,rightz},0}}, \
			{ {{0,0x80,0},0,{0,0x80,0},0,{upx,upy,upz},0}}}   }
*/
		case 0x00:
			{
				s8 * pcBase = g_ps8RamBase + dwAddress;
				DL_PF("    G_MV_LOOKATX %f %f %f",
					(float)pcBase[8 ^ 0x3],
					(float)pcBase[9 ^ 0x3],
					(float)pcBase[10 ^ 0x3]);

			}
			break;
		case 0x18:
			{
				s8 * pcBase = g_ps8RamBase + dwAddress;
				DL_PF("    G_MV_LOOKATY %f %f %f",
					(float)pcBase[8 ^ 0x3],
					(float)pcBase[9 ^ 0x3],
					(float)pcBase[10 ^ 0x3]);
			}
			break;
		default:		//0x30/48/60
			{
				DWORD dwLight = (dwOffset2 - 0x30)/0x18;
				DL_PF("    Light %d:", dwLight);
				RDP_MoveMemLight(dwLight, dwAddress);
				bHandled = TRUE;
			}
			break;
		}
		break;

		}
	case F3DEX_GBI_2X_G_MV_MATRIX:
		/*
		gsDma2p(G_MOVEMEM,(mptr),sizeof(Mtx),G_MV_MATRIX,0),		\
		gsMoveWd(G_MW_FORCEMTX,0,0x00010000)

		#define	gsDma2p(c, adrs, len, idx, ofs)					\
		{									\
		(_SHIFTL((c),24,8)|_SHIFTL((ofs)/8,16,8)|			\
		_SHIFTL(((len)-1)/8,8,8)|_SHIFTL((idx),0,8)),			\
		(unsigned int)(adrs)						\
		}

		#define gsMoveWd(    index, offset, data)				\
		gsDma1p(      G_MOVEWORD, data, offset, index)

		#define	gDma1p(pkt, c, s, l, p)						\
		{									\
		Gfx *_g = (Gfx *)(pkt);						\
		\
		_g->words.w0 = (_SHIFTL((c), 24, 8) | _SHIFTL((p), 16, 8) |	\
		_SHIFTL((l), 0, 16));				\
		_g->words.w1 = (unsigned int)(s);				\
		}
		 */
		DL_PF("Force Matrix: addr=%08X", dwAddress);
		RDP_GFX_Force_Matrix(dwAddress);
		break;
	case F3DEX_GBI_2X_G_MVO_L0:
	case F3DEX_GBI_2X_G_MVO_L1:
	case F3DEX_GBI_2X_G_MVO_L2:
	case F3DEX_GBI_2X_G_MVO_L3:
	case F3DEX_GBI_2X_G_MVO_L4:
	case F3DEX_GBI_2X_G_MVO_L5:
	case F3DEX_GBI_2X_G_MVO_L6:
	case F3DEX_GBI_2X_G_MVO_L7:
		DL_PF("Zelda Move Light");
		RDP_NOIMPL_WARN("Zelda Move Light");
		break;

	case F3DEX_GBI_2X_G_MV_POINT:
		DL_PF("Zelda Move Point");
		RDP_NOIMPL_WARN("Zelda Move Point");
		break;

	case F3DEX_GBI_2X_G_MVO_LOOKATX:
		if( dwCmd0 == 0xDC170000 && (dwCmd1&0xFF000000) == 0x80000000 )
		{
			// Ucode for Evangelion.v64, the ObjMatrix cmd
			RDP_S2DEX_OBJ_MOVEMEM(dwCmd0, dwCmd1);
		}
		else
		{
			RDP_NOIMPL("Not implemented ZeldaMoveMem LOOKATX, Cmd0=0x%08X, Cmd1=0x%08X", dwCmd0, dwCmd1);
		}
		break;
	case F3DEX_GBI_2X_G_MVO_LOOKATY:
		RDP_NOIMPL("Not implemented ZeldaMoveMem LOOKATY, Cmd0=0x%08X, Cmd1=0x%08X", dwCmd0, dwCmd1);
		break;
	case 0x02:
		if( dwCmd0 == 0xDC070002 && (dwCmd1&0xFF000000) == 0x80000000 )
		{
			RDP_S2DEX_OBJ_MOVEMEM(dwCmd0, dwCmd1);
			break;
		}
	default:
		DL_PF("ZeldaMoveMem Type: Unknown");
		RDP_NOIMPL("Unknown ZeldaMoveMem Type, type=0x%X, Addr=%08X", dwType, dwAddress);
		break;
	}

/*
0x00112440: dc08000a 8010e830 G_ZMOVEMEM
    Type: 0a Len: 08 Off: 4000
    00000000 00000000 5a00a600 00000000*/
/*
0x000ff418: dc08060a 800ff368 G_ZMOVEMEM
    Type: 0a Len: 08 Off: 4030
    Light 0:
       RGBA: 0x69696900, RGBACopy: 0x69696900, x: 0.000000, y: -127.000000, z: 0.000000
      (Ambient Light)
0x000ff420: dc08090a 800ff360 G_ZMOVEMEM
    Type: 0a Len: 08 Off: 4048
    Light 1:
       RGBA: 0x46464600, RGBACopy: 0x46464600, x: 105.000000, y: 105.000000, z: 105.000000
      (Normal Light)
*/
	if (!bHandled)
	{
		s8 * pcBase = g_ps8RamBase + (dwAddress&(g_dwRamSize-1));
		DWORD * pdwBase = (DWORD *)pcBase;
		LONG i;
		
		for (i = 0; i < 4; i++)
		{
			DL_PF("    %08x %08x %08x %08x", pdwBase[0], pdwBase[1], pdwBase[2], pdwBase[3]);
			pdwBase+=4;
		}
	}

}



void DLParser_GBI2_DL(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI0_DL);


	DWORD dwPush = (dwCmd0 >> 16) & 0xFF;
	DWORD dwAddr = RDPSegAddr(dwCmd1);

	DL_PF("    DL: Push:0x%02x Addr: 0x%08x", dwPush, dwAddr);
	
	switch (dwPush)
	{
	case G_DL_PUSH:
		DL_PF("    Pushing ZeldaDisplayList 0x%08x", dwAddr);
		g_dwPCindex++;
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = ~0;

		break;
	case G_DL_NOPUSH:
		DL_PF("    Jumping to ZeldaDisplayList 0x%08x", dwAddr);
		if( g_dwPCStack[g_dwPCindex].addr == dwAddr+8 )	//Is this a loop
		{
			//Hack for Gauntlet Legends
			g_dwPCStack[g_dwPCindex].addr = dwAddr+8;
		}
		else
			g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = ~0;
		break;
	}

	DL_PF("");
	DL_PF("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
	DL_PF("#############################################");


}



void DLParser_GBI2_SetOtherModeL(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_SetOtherModeL);

	DWORD dwShift = (dwCmd0>>8)&0xFF;
	DWORD dwLength= (dwCmd0   )&0xFF;
	DWORD dwData  = dwCmd1;

	// Mask is constructed slightly differently
	DWORD dwMask = (u32)((s32)(0x80000000)>>dwLength)>>dwShift;
	dwData &= dwMask;

	DWORD modeL = gRDP.otherModeL;
	modeL = (modeL&(~dwMask)) | dwData;

	DLParser_RDPSetOtherMode(gRDP.otherModeH, modeL );
}



void DLParser_GBI2_SetOtherModeH(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_SetOtherModeH);

	DWORD dwLength= ((dwCmd0)&0xFF)+1;
	DWORD dwShift = 32 - ((dwCmd0>>8)&0xFF) - dwLength;
	DWORD dwData  = dwCmd1;

	DWORD dwMask2 = ((1<<dwLength)-1)<<dwShift;
	DWORD dwModeH = gRDP.otherModeH;
	dwModeH = (dwModeH&(~dwMask2)) | dwData;

	DLParser_RDPSetOtherMode(dwModeH, gRDP.otherModeL );
}


void DLParser_GBI2_SubModule(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI2_SubModule);

	RDP_NOIMPL("RDP: DLParser_GBI2_SubModule (0x%08x 0x%08x)", dwCmd0, dwCmd1);
}
