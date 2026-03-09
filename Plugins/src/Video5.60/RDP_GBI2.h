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

void RSP_GBI2_Vtx(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwVEnd    = ((word0   )&0xFFF)/2;
	uint32 dwN      = ((word0>>12)&0xFFF);

	uint32 dwV0		= dwVEnd - dwN;

	LOG_DL("    Vtx: Address 0x%08x, vEnd: %d, v0: %d, Num: %d", dwAddr, dwVEnd, dwV0, dwN);

	if( dwVEnd > 64 )
	{
		DebuggerAppendMsg("Warning, attempting to load into invalid vertex positions, v0=%d, n=%d", dwV0, dwN);
		return;
	}

	// Check that address is valid...
	if ((dwAddr + (dwN*16)) > g_dwRamSize)
	{
		DebuggerAppendMsg("ProcessVertexData: Address out of range (0x%08x)", dwAddr);
	}
	else
	{
		ProcessVertexData(dwAddr, dwV0, dwN);

		status.dwNumVertices += dwN;

		DisplayVertexInfo(dwAddr, dwV0, dwN);
	}

}



void RSP_GBI2_EndDL(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_EndDL);

	RDP_GFX_PopDL();
}

void RSP_GBI2_CullDL(uint32 word0, uint32 word1)
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
	uint32 dwVFirst = ((word0) & 0xfff) / gRSP.vertexMult;
	uint32 dwVLast  = ((word1) & 0xfff) / gRSP.vertexMult;

	LOG_DL("    Culling using verts %d to %d", dwVFirst, dwVLast);

	// Mask into range
	dwVFirst &= 0x1f;
	dwVLast &= 0x1f;

	if( dwVLast < dwVFirst )	return;
	if( !gRSP.bRejectVtx )	return;

	for (i = dwVFirst; i <= dwVLast; i++)
	{
		//if (g_dwVtxFlags[i] == 0)
		if (g_clipFlag[i] == 0)
		{
			LOG_DL("    Vertex %d is visible, returning", i);
			return;
		}
	}

	status.dwNumDListsCulled++;

	LOG_DL("    No vertices were visible, culling");

	RDP_GFX_PopDL();
}

void RSP_GBI2_MoveWord(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_MoveWord);

	uint32 dwType   = (word0 >> 16) & 0xFF;
	uint32 dwOffset = (word0      ) & 0xFFFF;
	LOG_DL("     MoveWord: type=%08X, offset=%08X", dwType, dwOffset);

	switch (dwType)
	{
	case RSP_MOVE_WORD_MATRIX:
		RSP_RDP_InsertMatrix(word0, word1);
		break;
	case RSP_MOVE_WORD_NUMLIGHT:
		{
			uint32 dwNumLights = (word1/24);
			LOG_DL("     RSP_MOVE_WORD_NUMLIGHT: %d", dwNumLights);

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
			default:
				LOG_DL("     RSP_MOVE_WORD_CLIP  ?   : 0x%08x", word1);
				break;
			}
		}
		break;

	case RSP_MOVE_WORD_SEGMENT:
		{
			uint32 dwSeg     = dwOffset / 4;
			uint32 dwAddr = word1 & 0x00FFFFFF;			// Hack - convert to physical

			LOG_DL("      RSP_MOVE_WORD_SEGMENT Segment[%d] = 0x%08x",	dwSeg, dwAddr);
			if( dwAddr > g_dwRamSize )
			{
				gRSP.segments[dwSeg] = dwAddr;
#ifdef _DEBUG
				if( pauseAtNext )
					DebuggerAppendMsg("warning: Segment %d addr is %8X", dwSeg, dwAddr);
#endif
			}
			else
			{
				gRSP.segments[dwSeg] = dwAddr;
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

			SetFogMinMax(fMin, fMax, fMult, fOff);
		}
		break;
	case RSP_MOVE_WORD_LIGHTCOL:
		{
			uint32 dwLight = dwOffset / 0x18;
			uint32 dwField = (dwOffset & 0x7);

			LOG_DL("     RSP_MOVE_WORD_LIGHTCOL/0x%08x: 0x%08x", dwOffset, word1);

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
				DebuggerAppendMsg("RSP_MOVE_WORD_LIGHTCOL with unknown offset 0x%08x", dwField);
				break;
			}


		}
		break;

	case RSP_MOVE_WORD_PERSPNORM:
		LOG_DL("     RSP_MOVE_WORD_PERSPNORM 0x%04x", (short)word1);
		break;

	case RSP_MOVE_WORD_POINTS:
		LOG_DL("     2nd cmd of Force Matrix");
		break;

	default:
		{
			LOG_DL("      Ignored!!");

		}
		break;
	}
}

void RSP_S2DEX_SPObjLoadTxtr(uint32 word0, uint32 word1);

void RSP_GBI2_Tri1(uint32 word0, uint32 word1)
{
	if( word0 == 0x05000017 && (word1>>24) == 0x80 )
	{
		// The ObjLoadTxtr / Tlut cmd for Evangelion.v64
		RSP_S2DEX_SPObjLoadTxtr(word0, word1);
		DebuggerAppendMsg("Fix me, SPObjLoadTxtr as RSP_GBI2_Tri2");
	}
	else
	{
		status.primitiveType = PRIM_TRI1;
		bool bTrisAdded = false;
		bool textureIsEnabled = CRender::g_pRender->IsTextureEnabled();

		// While the next command pair is Tri1, add vertices
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;
		uint32 * pCmdBase = (uint32 *)(g_pRDRAMu8 + dwPC);

		do
		{
			uint32 dwV2 = ((word0>>16)&0xFF)/gRSP.vertexMult;
			uint32 dwV1 = ((word0>>8 )&0xFF)/gRSP.vertexMult;
			uint32 dwV0 = ((word0   )&0xFF)/gRSP.vertexMult;

			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri1", dwV0, dwV1, dwV2);
				LOG_DL("    ZeldaTri1: 0x%08x 0x%08x %d,%d,%d", word0, word1, dwV0, dwV1, dwV2);
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
		} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (word0>>24) == (BYTE)RSP_ZELDATRI1);
		if(pauseAtNext && eventToPause==NEXT_TRIANGLE )
		{
			eventToPause = NEXT_FLUSH_TRI;
		}

#else
		} while ((word0>>24) == (BYTE)RSP_ZELDATRI1);
#endif

		g_dwPCStack[g_dwPCindex].addr = dwPC-8;


		if (bTrisAdded)	
		{
			CRender::g_pRender->FlushTris();
		}
	}
}

void RSP_S2DEX_SPObjLoadTxSprite(uint32 word0, uint32 word1);
void RSP_S2DEX_SPObjLoadTxRect(uint32 word0, uint32 word1);
void RSP_S2DEX_SPObjLoadTxRectR(uint32 word0, uint32 word1);

void RSP_GBI2_Tri2(uint32 word0, uint32 word1)
{
	if( word0 == 0x0600002f && (word1>>24) == 0x80 )
	{
		// The ObjTxSprite cmd for Evangelion.v64
		RSP_S2DEX_SPObjLoadTxSprite(word0, word1);
		DebuggerAppendMsg("Fix me, SPObjLoadTxSprite as RSP_GBI2_Tri2");
	}
	else
	{
		status.primitiveType = PRIM_TRI2;
		BOOL bTrisAdded = FALSE;

		// While the next command pair is Tri2, add vertices
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;
		bool textureIsEnabled = CRender::g_pRender->IsTextureEnabled();

		do {
			uint32 dwV2 = ((word1>>17)&0x7F);
			uint32 dwV1 = ((word1>>9 )&0x7F);
			uint32 dwV0 = ((word1>>1 )&0x7F);

			uint32 dwV5 = ((word0>>17)&0x7F);
			uint32 dwV4 = ((word0>>9 )&0x7F);
			uint32 dwV3 = ((word0>>1 )&0x7F);

			LOG_DL("    ZeldaTri2: 0x%08x 0x%08x", word0, word1);
			LOG_DL("           V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
			LOG_DL("           V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

			// Do first tri
			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri2 1/2", dwV0, dwV1, dwV2);
				if (!bTrisAdded)
				{
					if( textureIsEnabled )
				{
					PrepareTextures();
					InitVertexTextureConstants();
				}
					CRender::g_pRender->SetCombinerAndBlender();
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
					PrepareTextures();
					InitVertexTextureConstants();
				}
					CRender::g_pRender->SetCombinerAndBlender();
					bTrisAdded = TRUE;
				}

				AddTri(dwV3, dwV4, dwV5);
			}
			
			word0			= *(uint32 *)(g_pRDRAMu8 + dwPC+0);
			word1			= *(uint32 *)(g_pRDRAMu8 + dwPC+4);
			dwPC += 8;

#ifdef _DEBUG
		} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (word0>>24) == (BYTE)RSP_ZELDATRI2);
		if(pauseAtNext && eventToPause==NEXT_TRIANGLE )
		{
			eventToPause = NEXT_FLUSH_TRI;
		}

#else
		} while ((word0>>24) == (BYTE)RSP_ZELDATRI2 );//&& status.dwNumTrisRendered < 50);
#endif


		g_dwPCStack[g_dwPCindex].addr = dwPC-8;

		if (bTrisAdded)	
		{
			CRender::g_pRender->FlushTris();
		}
	}
}

void RSP_GBI2_Line3D(uint32 word0, uint32 word1)
{
	if( word0 == 0x0700002f && (word1>>24) == 0x80 )
	{
		// The ObjTxSprite cmd for Evangelion.v64
		RSP_S2DEX_SPObjLoadTxRect(word0, word1);
	}
	else
	{
		status.primitiveType = PRIM_TRI3;

		// While the next command pair is Tri2, add vertices
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;

		BOOL bTrisAdded = FALSE;

		do {
			// Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
			uint32 dwV2 = ((word1>>16)&0xFF)/gRSP.vertexMult;
			uint32 dwV1 = ((word1>>8 )&0xFF)/gRSP.vertexMult;
			uint32 dwV0 = ((word1    )&0xFF)/gRSP.vertexMult;

			uint32 dwV5 = ((word0>>16)&0xFF)/gRSP.vertexMult;
			uint32 dwV4 = ((word0>>8 )&0xFF)/gRSP.vertexMult;
			uint32 dwV3 = ((word0    )&0xFF)/gRSP.vertexMult;

			LOG_DL("    ZeldaTri3: 0x%08x 0x%08x", word0, word1);
			LOG_DL("           V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
			LOG_DL("           V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

			// Do first tri
			if (TestTri(dwV0, dwV1, dwV2))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri3 1/2", dwV0, dwV1, dwV2);
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
			if (TestTri(dwV3, dwV4, dwV5))
			{
				DEBUG_DUMP_VERTEXES("ZeldaTri3 2/2", dwV3, dwV4, dwV5);
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
				AddTri(dwV3, dwV4, dwV5);
			}
			
			word0			= *(uint32 *)(g_pRDRAMu8 + dwPC+0);
			word1			= *(uint32 *)(g_pRDRAMu8 + dwPC+4);
			dwPC += 8;

#ifdef _DEBUG
		} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (word0>>24) == (BYTE)RSP_LINE3D);
		if(pauseAtNext && eventToPause==NEXT_TRIANGLE )
		{
			eventToPause = NEXT_FLUSH_TRI;
		}

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

void RSP_GBI2_Texture(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_Texture);

	uint32 dwLevel  = (word0>>11)&0x07;
	uint32 dwTile= (word0>>8 )&0x07;

	bool bTextureEnable  = (word0)&0x02 ? true : false;		// Important! not 0x01!?!?
	float fTextureScaleS = (float)((word1>>16)&0xFFFF) / (65535.0f * 32.0f);
	float fTextureScaleT = (float)((word1    )&0xFFFF) / (65535.0f * 32.0f);

	if( ((word1>>16)&0xFFFF) == 0xFFFF )
	{
		fTextureScaleS = 1/32.0f;
	}
	else if( ((word1>>16)&0xFFFF) == 0x8000 )
	{
		fTextureScaleS = 1/64.0f;
	}
	if( ((word1    )&0xFFFF) == 0xFFFF )
	{
		fTextureScaleT = 1/32.0f;
	}
	else if( ((word1    )&0xFFFF) == 0x8000 )
	{
		fTextureScaleT = 1/64.0f;
	}


	/*
	if( g_curRomInfo.bTextureScaleHack )
	{
	// Hack, need to verify, refer to N64 programming manual
	// that if scale = 0.5 (1/64), vtx s,t are also doubled

		if( ((word1>>16)&0xFFFF) == 0x8000 )
		{
			fTextureScaleS = 1/128.0f;
			if( ((word1)&0xFFFF) == 0xFFFF )
			{
				fTextureScaleT = 1/64.0f;
			}
		}

		if( ((word1    )&0xFFFF) == 0x8000 )
		{
			fTextureScaleT = 1/128.0f;
			if( ((word1>>16)&0xFFFF) == 0xFFFF )
			{
				fTextureScaleS = 1/64.0f;
			}
		}
	}
	*/

	CRender::g_pRender->SetTextureEnableAndScale(dwTile, bTextureEnable, fTextureScaleS, fTextureScaleT);

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



void RSP_GBI2_PopMtx(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_PopMtx);

	BYTE nCommand = (BYTE)(word0 & 0xFF);

	LOG_DL("        PopMtx: 0x%02x (%s)",
		nCommand, 
		(nCommand & RSP_ZELDA_MTX_PROJECTION) ? "Projection" : "ModelView");


/*	if (nCommand & RSP_ZELDA_MTX_PROJECTION)
	{
		CRender::g_pRender->PopProjection();
	}
	else*/
	{
		CRender::g_pRender->PopWorldView();
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
		if( pauseAtNext && logMatrix ) 
		{
			TRACE0("Pause after Pop GBI2_PopMtx:");
		}
	}
#endif

}


#define RSP_ZELDA_ZBUFFER				0x00000001		// Guess
#define RSP_ZELDA_CULL_BACK			0x00000200
#define RSP_ZELDA_CULL_FRONT			0x00000400
#define RSP_ZELDA_FOG					0x00010000
#define RSP_ZELDA_LIGHTING			0x00020000
#define RSP_ZELDA_TEXTURE_GEN			0x00040000
#define RSP_ZELDA_TEXTURE_GEN_LINEAR	0x00080000
#define RSP_ZELDA_SHADING_SMOOTH		0x00200000

void RSP_GBI2_GeometryMode(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI2_GeometryMode);

	uint32 dwAnd = (word0) & 0x00FFFFFF;
	uint32 dwOr  = (word1) & 0x00FFFFFF;

#ifdef _DEBUG
		LOG_DL("    0x%08x 0x%08x =(x & 0x%08x) | 0x%08x", word0, word1, dwAnd, dwOr);

		if ((~dwAnd) & RSP_ZELDA_ZBUFFER)					LOG_DL("  Disabling ZBuffer");
		//	if ((~dwAnd) & RSP_ZELDA_TEXTURE_ENABLE)			LOG_DL("  Disabling Texture");
		//	if ((~dwAnd) & RSP_ZELDA_SHADE)					LOG_DL("  Disabling Shade");
		if ((~dwAnd) & RSP_ZELDA_SHADING_SMOOTH)			LOG_DL("  Disabling Flat Shading");
		if ((~dwAnd) & RSP_ZELDA_CULL_FRONT)				LOG_DL("  Disabling Front Culling");
		if ((~dwAnd) & RSP_ZELDA_CULL_BACK)				LOG_DL("  Disabling Back Culling");
		if ((~dwAnd) & RSP_ZELDA_FOG)						LOG_DL("  Disabling Fog");
		if ((~dwAnd) & RSP_ZELDA_LIGHTING)				LOG_DL("  Disabling Lighting");
		if ((~dwAnd) & RSP_ZELDA_TEXTURE_GEN)				LOG_DL("  Disabling Texture Gen");
		if ((~dwAnd) & RSP_ZELDA_TEXTURE_GEN_LINEAR)		LOG_DL("  Disabling Texture Gen Linear");
		//	if ((~dwAnd) & RSP_ZELDA_LOD)						LOG_DL("  Disabling LOD (no impl)");

		if (dwOr & RSP_ZELDA_ZBUFFER)						LOG_DL("  Enabling ZBuffer");
		//	if (dwOr & RSP_ZELDA_TEXTURE_ENABLE)				LOG_DL("  Enabling Texture");
		//	if (dwOr & RSP_ZELDA_SHADE)						LOG_DL("  Enabling Shade");
		if (dwOr & RSP_ZELDA_SHADING_SMOOTH)				LOG_DL("  Enabling Flat Shading");
		if (dwOr & RSP_ZELDA_CULL_FRONT)					LOG_DL("  Enabling Front Culling");
		if (dwOr & RSP_ZELDA_CULL_BACK)					LOG_DL("  Enabling Back Culling");
		if (dwOr & RSP_ZELDA_FOG)							LOG_DL("  Enabling Fog");
		if (dwOr & RSP_ZELDA_LIGHTING)					LOG_DL("  Enabling Lighting");
		if (dwOr & RSP_ZELDA_TEXTURE_GEN)					LOG_DL("  Enabling Texture Gen");
		if (dwOr & RSP_ZELDA_TEXTURE_GEN_LINEAR)			LOG_DL("  Enabling Texture Gen Linear");
		//	if (dwOr & RSP_ZELDA_LOD)							LOG_DL("  Enabling LOD (no impl)");
#endif // _DEBUG

		gRDP.geometryMode &= dwAnd;
	gRDP.geometryMode |= dwOr;


	bool bCullFront		= (gRDP.geometryMode & RSP_ZELDA_CULL_FRONT) ? true : false;
	bool bCullBack		= (gRDP.geometryMode & RSP_ZELDA_CULL_BACK) ? true : false;
	
	BOOL bShade			= (gRDP.geometryMode & G_SHADE) ? TRUE : FALSE;
	//BOOL bFlatShade		= (gRDP.geometryMode & RSP_ZELDA_SHADING_SMOOTH) ? TRUE : FALSE;
	BOOL bFlatShade		= (gRDP.geometryMode & RSP_ZELDA_TEXTURE_GEN_LINEAR) ? TRUE : FALSE;
	if( options.enableHackForGames == HACK_FOR_TIGER_HONEY_HUNT )
		bFlatShade		= FALSE;
	
	BOOL bFog			= (gRDP.geometryMode & RSP_ZELDA_FOG) ? TRUE : FALSE;
	bool bTextureGen	= (gRDP.geometryMode & RSP_ZELDA_TEXTURE_GEN) ? true : false;

	bool bLighting      = (gRDP.geometryMode & RSP_ZELDA_LIGHTING) ? true : false;
	BOOL bZBuffer		= (gRDP.geometryMode & RSP_ZELDA_ZBUFFER)	? TRUE : FALSE;	

	CRender::g_pRender->SetCullMode(bCullFront, bCullBack);
	
	//if (bFlatShade||!bShade)	CRender::g_pRender->SetShadeMode( SHADE_FLAT );
	if (bFlatShade)	CRender::g_pRender->SetShadeMode( SHADE_FLAT );
	else			CRender::g_pRender->SetShadeMode( SHADE_SMOOTH );
	
	SetTextureGen(bTextureGen);

	SetLighting( bLighting );
	CRender::g_pRender->ZBufferEnable( bZBuffer );
	CRender::g_pRender->SetFogEnable( bFog );

	//RSP_GFX_InitGeometryMode();
}


int dlistMtxCount=0;
extern uint32 dwConkerVtxZAddr;
void RSP_GBI2_Mtx(uint32 word0, uint32 word1)
{	
	SP_Timing(RSP_GBI0_Mtx);
	dwConkerVtxZAddr = 0;	// For Conker BFD

	Matrix mat;
	uint32 dwI;
	uint32 dwJ;
	const float fRecip = 1.0f / 65536.0f;
	uint32 dwAddr = RSPSegmentAddr(word1);

	uint32 dwCommand = (word0)&0xFF;
	uint32 dwLength  = (word0 >> 8)&0xFFFF;

	if( dwCommand == 0 && dwLength == 0 )
	{
		DLParser_Bomberman2TextRect(word0, word1);
		return;
	}

	LOG_DL("    Mtx: %s %s %s Length %d Address 0x%08x",
		(dwCommand & RSP_ZELDA_MTX_PROJECTION) ? "Projection" : "ModelView",
		(dwCommand & RSP_ZELDA_MTX_LOAD) ? "Load" : "Mul",	
		(dwCommand & RSP_ZELDA_MTX_NOPUSH) ? "NoPush" : "Push",
		dwLength, dwAddr);

	if (dwAddr + 64 > g_dwRamSize)
	{
		DebuggerAppendMsg("ZeldaMtx: Address invalid (0x%08x)", dwAddr);
		return;
	}

	for (dwI = 0; dwI < 4; dwI++)
	{
		for (dwJ = 0; dwJ < 4; dwJ++) 
		{
			short nDataHi = *(short *)(g_pRDRAMu8 + ((dwAddr+(dwI<<3)+(dwJ<<1)     )^0x2));
			WORD  nDataLo = *(WORD  *)(g_pRDRAMu8 + ((dwAddr+(dwI<<3)+(dwJ<<1) + 32)^0x2));

			mat.m[dwI][dwJ] = (float)(((LONG)nDataHi<<16) | (nDataLo))*fRecip;
		}
	}

#ifdef _DEBUG
	LOG_DL(
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n"
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n"
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n"
		" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\r\n",
		mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
		mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
		mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
		mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);
#endif // _DEBUG

	LONG nLoadCommand = (dwCommand & RSP_ZELDA_MTX_LOAD) ? RENDER_LOAD_MATRIX : RENDER_MUL_MATRIX;
	BOOL bPush = (dwCommand & RSP_ZELDA_MTX_NOPUSH) ? FALSE : TRUE;


	if (dwCommand & RSP_ZELDA_MTX_PROJECTION)
	{
		// So far only Extreme-G seems to Push/Pop projection matrices	
		CRender::g_pRender->SetProjection(mat, bPush, nLoadCommand);
	}
	else
	{
		CRender::g_pRender->SetWorldView(mat, bPush, nLoadCommand);

		if( options.enableHackForGames == HACK_FOR_SOUTH_PARK_RALLY )
		{
			dlistMtxCount++;
			if( dlistMtxCount == 2 )
			{
				CRender::g_pRender->ClearZBuffer(1.0f);
			}
		}
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
		if (dwCommand & RSP_ZELDA_MTX_PROJECTION)
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
		if( pauseAtNext && logMatrix ) 
		{
			if (dwCommand & RSP_ZELDA_MTX_PROJECTION)
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

void RSP_S2DEX_OBJ_MOVEMEM(uint32 word0, uint32 word1);
void RSP_GBI2_MoveMem(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_MoveMem);


	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwOffset = (word0 >> 8) & 0xFFFF;
	uint32 dwType    = (word0     ) & 0xFE;

	uint32 dwLen = (word0 >> 16) & 0xFF;
	uint32 dwOffset2 = (word0 >> 5) & 0x3FFF;

	LOG_DL("    MoveMem: Type: %02x Len: %02x Off: %04x", dwType, dwLen, dwOffset2);

	BOOL bHandled = FALSE;

	switch (dwType)
	{
	case RSP_GBI2_MV_MEM__VIEWPORT:
		{
			RSP_MoveMemViewport(dwAddr);
			bHandled = TRUE;
		}
		break;
	case RSP_GBI2_MV_MEM__LIGHT:
		{
		switch (dwOffset2)
		{
		case 0x00:
			{
				s8 * pcBase = g_pRDRAMs8 + dwAddr;
				LOG_DL("    RSP_GBI1_MV_MEM_LOOKATX %f %f %f",
					(float)pcBase[8 ^ 0x3],
					(float)pcBase[9 ^ 0x3],
					(float)pcBase[10 ^ 0x3]);

			}
			break;
		case 0x18:
			{
				s8 * pcBase = g_pRDRAMs8 + dwAddr;
				LOG_DL("    RSP_GBI1_MV_MEM_LOOKATY %f %f %f",
					(float)pcBase[8 ^ 0x3],
					(float)pcBase[9 ^ 0x3],
					(float)pcBase[10 ^ 0x3]);
			}
			break;
		default:		//0x30/48/60
			{
				uint32 dwLight = (dwOffset2 - 0x30)/0x18;
				LOG_DL("    Light %d:", dwLight);
				RSP_MoveMemLight(dwLight, dwAddr);
				bHandled = TRUE;
			}
			break;
		}
		break;

		}
	case RSP_GBI2_MV_MEM__MATRIX:
		LOG_DL("Force Matrix: addr=%08X", dwAddr);
		RSP_GFX_Force_Matrix(dwAddr);
		break;
	case RSP_GBI2_MV_MEM_O_L0:
	case RSP_GBI2_MV_MEM_O_L1:
	case RSP_GBI2_MV_MEM_O_L2:
	case RSP_GBI2_MV_MEM_O_L3:
	case RSP_GBI2_MV_MEM_O_L4:
	case RSP_GBI2_MV_MEM_O_L5:
	case RSP_GBI2_MV_MEM_O_L6:
	case RSP_GBI2_MV_MEM_O_L7:
		LOG_DL("Zelda Move Light");
		RDP_NOIMPL_WARN("Zelda Move Light");
		break;

	case RSP_GBI2_MV_MEM__POINT:
		LOG_DL("Zelda Move Point");
		RDP_NOIMPL_WARN("Zelda Move Point");
		break;

	case RSP_GBI2_MV_MEM_O_LOOKATX:
		if( word0 == 0xDC170000 && (word1&0xFF000000) == 0x80000000 )
		{
			// Ucode for Evangelion.v64, the ObjMatrix cmd
			RSP_S2DEX_OBJ_MOVEMEM(word0, word1);
		}
		else
		{
			RSP_RDP_NOIMPL("Not implemented ZeldaMoveMem LOOKATX, Cmd0=0x%08X, Cmd1=0x%08X", word0, word1);
		}
		break;
	case RSP_GBI2_MV_MEM_O_LOOKATY:
		RSP_RDP_NOIMPL("Not implemented ZeldaMoveMem LOOKATY, Cmd0=0x%08X, Cmd1=0x%08X", word0, word1);
		break;
	case 0x02:
		if( word0 == 0xDC070002 && (word1&0xFF000000) == 0x80000000 )
		{
			RSP_S2DEX_OBJ_MOVEMEM(word0, word1);
			break;
		}
	default:
		LOG_DL("ZeldaMoveMem Type: Unknown");
		RSP_RDP_NOIMPL("Unknown ZeldaMoveMem Type, type=0x%X, Addr=%08X", dwType, dwAddr);
		break;
	}

	if (!bHandled)
	{
		s8 * pcBase = g_pRDRAMs8 + (dwAddr&(g_dwRamSize-1));
		uint32 * pdwBase = (uint32 *)pcBase;
		LONG i;
		
		for (i = 0; i < 4; i++)
		{
			LOG_DL("    %08x %08x %08x %08x", pdwBase[0], pdwBase[1], pdwBase[2], pdwBase[3]);
			pdwBase+=4;
		}
	}

}



void RSP_GBI2_DL(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI0_DL);


	uint32 dwPush = (word0 >> 16) & 0xFF;
	uint32 dwAddr = RSPSegmentAddr(word1);

	if( dwAddr > g_dwRamSize )
	{
		RSP_RDP_NOIMPL("Error: DL addr = %08X out of range, PC=%08X", dwAddr, g_dwPCStack[g_dwPCindex].addr );
		dwAddr &= (g_dwRamSize-1);
		DebuggerPauseCountN( NEXT_DLIST );
	}

	LOG_DL("    DL: Push:0x%02x Addr: 0x%08x", dwPush, dwAddr);
	
	switch (dwPush)
	{
	case RSP_DLIST_PUSH:
		LOG_DL("    Pushing ZeldaDisplayList 0x%08x", dwAddr);
		g_dwPCindex++;
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = ~0;

		break;
	case RSP_DLIST_NOPUSH:
		LOG_DL("    Jumping to ZeldaDisplayList 0x%08x", dwAddr);
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

	LOG_DL("");
	LOG_DL("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
	LOG_DL("#############################################");


}



void RSP_GBI2_SetOtherModeL(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_SetOtherModeL);

	uint32 dwShift = (word0>>8)&0xFF;
	uint32 dwLength= (word0   )&0xFF;
	uint32 dwData  = word1;

	// Mask is constructed slightly differently
	uint32 dwMask = (uint32)((s32)(0x80000000)>>dwLength)>>dwShift;
	dwData &= dwMask;

	uint32 modeL = gRDP.otherModeL;
	modeL = (modeL&(~dwMask)) | dwData;

	DLParser_RDPSetOtherMode(gRDP.otherModeH, modeL );
}



void RSP_GBI2_SetOtherModeH(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_SetOtherModeH);

	uint32 dwLength= ((word0)&0xFF)+1;
	uint32 dwShift = 32 - ((word0>>8)&0xFF) - dwLength;
	uint32 dwData  = word1;

	uint32 dwMask2 = ((1<<dwLength)-1)<<dwShift;
	uint32 dwModeH = gRDP.otherModeH;
	dwModeH = (dwModeH&(~dwMask2)) | dwData;

	DLParser_RDPSetOtherMode(dwModeH, gRDP.otherModeL );
}


void RSP_GBI2_SubModule(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI2_SubModule);

	RSP_RDP_NOIMPL("RDP: RSP_GBI2_SubModule (0x%08x 0x%08x)", word0, word1);
}
