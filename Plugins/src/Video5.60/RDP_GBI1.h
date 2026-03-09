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


// SP ucode for GBI1


void RSP_GBI1_Vtx(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);

	uint32 dwV0  = (word0 >>17 ) & 0x7f;		// ==((x>>16)&0xff)/2
	uint32 dwN   = (word0 >>10 ) & 0x3f;
	uint32 dwLength = (word0      ) & 0x3ff;

	LOG_DL("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", dwAddr, dwV0, dwN, dwLength);

	if (dwAddr > g_dwRamSize)
	{
		TRACE0("     Address out of range - ignoring load");
		return;
	}

	if ((dwV0 + dwN) > 80)
	{
		TRACE5("Warning, invalid vertex positions, N=%d, v0=%d, Addr=0x%08X, Cmd=%08X-%08X",
			dwN, dwV0, dwAddr, word0, word1);
		return;
	}

	ProcessVertexData(dwAddr, dwV0, dwN);

	status.dwNumVertices += dwN;

	DisplayVertexInfo(dwAddr, dwV0, dwN);
}

void RSP_GBI1_ModifyVtx(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_ModifyVtx);

	if( gRSP.ucode == 5 && (word0&0x00FFFFFF) == 0 && (word1&0xFF000000) == 0x80000000 )
	{
		DLParser_Bomberman2TextRect(word0, word1);
	}
	else
	{
		uint32 dwWhere = (word0 >> 16) & 0xFF;
		uint32 dwVert   = ((word0      ) & 0xFFFF) / 2;
		uint32 dwValue  = word1;

		if( dwVert > 80 )
		{
			RSP_RDP_NOIMPL("RSP_GBI1_ModifyVtx: Invalid vertex number: %d", dwVert, 0);
			return;
		}

		// Data for other commands?
		switch (dwWhere)
		{
		case RSP_MV_WORD_OFFSET_POINT_RGBA:			// Modify RGBA
		case RSP_MV_WORD_OFFSET_POINT_XYSCREEN:		// Modify X,Y
		case RSP_MV_WORD_OFFSET_POINT_ZSCREEN:		// Modify C
		case RSP_MV_WORD_OFFSET_POINT_ST:			// Texture
			ModifyVertexInfo(dwWhere, dwVert, dwValue);
			break;
		default:
			RSP_RDP_NOIMPL("RSP_GBI1_ModifyVtx: Setting unk value: 0x%02x, 0x%08x", dwWhere, dwValue);
			break;
		}
	}
}

void RSP_GBI1_Tri2(uint32 word0, uint32 word1)
{
	status.primitiveType = PRIM_TRI2;
	bool bTrisAdded = false;
	bool textureIsEnabled = CRender::g_pRender->IsTextureEnabled();

	// While the next command pair is Tri2, add vertices
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;

	do {
		// Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
		uint32 dwV0 = ((word1>>16)&0xFF)/gRSP.vertexMult;
		uint32 dwV1 = ((word1>>8 )&0xFF)/gRSP.vertexMult;
		uint32 dwV2 = ((word1    )&0xFF)/gRSP.vertexMult;

		uint32 dwV3 = ((word0>>16)&0xFF)/gRSP.vertexMult;
		uint32 dwV4 = ((word0>>8 )&0xFF)/gRSP.vertexMult;
		uint32 dwV5 = ((word0    )&0xFF)/gRSP.vertexMult;

		LOG_DL("    Tri2: 0x%08x 0x%08x", word0, word1);
		LOG_DL("      V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
		LOG_DL("      V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

		// Do first tri
		if (TestTri(dwV0, dwV1, dwV2))
		{
			DEBUG_DUMP_VERTEXES("Tri2 1/2", dwV0, dwV1, dwV2);
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

		// Do second tri
		if (TestTri(dwV3, dwV4, dwV5))
		{
			DEBUG_DUMP_VERTEXES("Tri2 2/2", dwV3, dwV4, dwV5);
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
			AddTri(dwV3, dwV4, dwV5);
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
}

extern D3DXVECTOR4 g_vtxNonTransformed[MAX_VERTS];
extern uint32 g_Half1Val;

void RSP_GBI1_BranchZ(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_BranchZ);

	uint32 vtx = (word0&0xFFF)>1;
	float vtxdepth = g_vecProjected[vtx].z/g_vecProjected[vtx].w;

#ifdef _DEBUG
	if( debuggerEnableZBuffer==FALSE || vtxdepth <= (s32)word1 || g_curRomInfo.bForceDepthBuffer )
#else
	if( vtxdepth <= (s32)word1 || g_curRomInfo.bForceDepthBuffer )
#endif
	{
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
		uint32 dwDL = *(uint32 *)(g_pRDRAMu8 + dwPC-12);
		uint32 dwAddr = RSPSegmentAddr(dwDL);

		dwAddr = RSPSegmentAddr(dwDL);;

		LOG_DL("BranchZ to DisplayList 0x%08x", dwAddr);
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = ~0;
	}
}

#ifdef _DEBUG
void DumpUcodeInfo(UcodeInfo &info)
{
	DebuggerAppendMsg("Loading Unknown Ucode:\n%08X-%08X-%08X-%08X, Size=0x%X, CRC=0x%08X\nCode:\n",
		info.ucDWORD1, info.ucDWORD2, info.ucDWORD3, info.ucDWORD4, 
		info.ucSize, info.ucCRC);
	DumpHex(info.ucStart,20);
	TRACE0("Data:\n");
	DumpHex(info.ucDStart,20);
}
#endif

void RSP_GBI1_LoadUCode(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_LoadUCode);

	//TRACE0("Load ucode");
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;
	uint32 dwUcStart = RSPSegmentAddr(word1);
	uint32 dwSize = (word0&0xFFFF)+1;
	uint32 dwUcDStart = RSPSegmentAddr(*(uint32 *)(g_pRDRAMu8 + dwPC-12));

	uint32 ucode = DLParser_CheckUcode(dwUcStart, dwUcDStart, dwSize, 8);
	RSP_SetUcode(ucode, dwUcStart, dwUcDStart, dwSize);

	DEBUGGER_PAUSE_AND_DUMP(NEXT_SWITCH_UCODE,{DebuggerAppendMsg("Pause at loading ucode");});
}


void RSP_GFX_Force_Matrix(uint32 dwAddr)
{
	if (dwAddr + 64 > g_dwRamSize)
	{
		DebuggerAppendMsg("ForceMtx: Address invalid (0x%08x)", dwAddr);
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


	CRender::g_pRender->SetWorldProjectMatrix(mat);

	DEBUGGER_PAUSE_AND_DUMP(NEXT_MATRIX_CMD,{TRACE0("Paused at ModMatrix Cmd");});
}



void DisplayVertexInfo(uint32 dwAddr, uint32 dwV0, uint32 dwN)
{
#ifdef _DEBUG
		s8 *pcSrc = (s8 *)(g_pRDRAMu8 + dwAddr);
		short *psSrc = (short *)(g_pRDRAMu8 + dwAddr);

		for (uint32 dwV = dwV0; dwV < dwV0 + dwN; dwV++)
		{
			float x = (float)psSrc[0^0x1];
			float y = (float)psSrc[1^0x1];
			float z = (float)psSrc[2^0x1];

			uint32 wFlags = g_dwVtxFlags[dwV]; //(WORD)psSrc[3^0x1];

			BYTE a = pcSrc[12^0x3];
			BYTE b = pcSrc[13^0x3];
			BYTE c = pcSrc[14^0x3];
			BYTE d = pcSrc[15^0x3];
			
			//LONG nTU = (LONG)(short)(psSrc[4^0x1]<<4);
			//LONG nTV = (LONG)(short)(psSrc[5^0x1]<<4);

			//float tu = (float)(nTU>>4);
			//float tv = (float)(nTV>>4);
			float tu = (float)(short)(psSrc[4^0x1]);
			float tv = (float)(short)(psSrc[5^0x1]);

			D3DXVECTOR4 & t = g_vecProjected[dwV];

			psSrc += 8;			// Increase by 16 bytes
			pcSrc += 16;

			LOG_DL(" #%02d Flags: 0x%04x Pos: {% 6f,% 6f,% 6f} Tex: {%+7.2f,%+7.2f}, Extra: %02x %02x %02x %02x (transf: {% 6f,% 6f,% 6f})",
				dwV, wFlags, x, y, z, tu, tv, a, b, c, d, t.x, t.y, t.z );
		}
#endif
}



void RSP_MoveMemLight(uint32 dwLight, uint32 dwAddr)
{
	if( dwLight >= 16 )
	{
		DebuggerAppendMsg("Warning: invalid light # = %d", dwLight);
		return;
	}
	s8 * pcBase = g_pRDRAMs8 + dwAddr;
	uint32 * pdwBase = (uint32 *)pcBase;

	gRSPn64lights[dwLight].dwRGBA		= pdwBase[0];
	gRSPn64lights[dwLight].dwRGBACopy	= pdwBase[1];
	gRSPn64lights[dwLight].x			= pcBase[8 ^ 0x3];
	gRSPn64lights[dwLight].y			= pcBase[9 ^ 0x3];
	gRSPn64lights[dwLight].z			= pcBase[10 ^ 0x3];

	/*
	{
		// Normalize light
		double sum = (double)gRSPn64lights[dwLight].x * gRSPn64lights[dwLight].x;
		sum += (double)gRSPn64lights[dwLight].y * gRSPn64lights[dwLight].y;
		sum += (double)gRSPn64lights[dwLight].z * gRSPn64lights[dwLight].z;

		sum = sqrt(sum);
		sum = sum/128.0;

		gRSPn64lights[dwLight].x /= sum;
		gRSPn64lights[dwLight].y /= sum;
		gRSPn64lights[dwLight].z /= sum;
	}
	*/
					
	LOG_DL("       RGBA: 0x%08x, RGBACopy: 0x%08x, x: %d, y: %d, z: %d", 
		gRSPn64lights[dwLight].dwRGBA,
		gRSPn64lights[dwLight].dwRGBACopy,
		gRSPn64lights[dwLight].x,
		gRSPn64lights[dwLight].y,
		gRSPn64lights[dwLight].z);

	if (dwLight == gRSP.ambientLightIndex)
	{
		LOG_DL("      (Ambient Light)");

		uint32 dwCol = COLOR_RGBA( (gRSPn64lights[dwLight].dwRGBA >> 24)&0xFF,
					  (gRSPn64lights[dwLight].dwRGBA >> 16)&0xFF,
					  (gRSPn64lights[dwLight].dwRGBA >>  8)&0xFF, 0xff);

		SetAmbientLight( dwCol );
	}
	else
	{
		
		LOG_DL("      (Normal Light)");

		SetLightCol(dwLight, gRSPn64lights[dwLight].dwRGBA);
		if (pdwBase[2] == 0)	// Direction is 0!
		{
			LOG_DL("      Light is invalid");
		}
		SetLightDirection(dwLight, 
			gRSPn64lights[dwLight].x,
			gRSPn64lights[dwLight].y,
			gRSPn64lights[dwLight].z);
	}
}

void RSP_MoveMemViewport(uint32 dwAddr)
{
	if( dwAddr+16 >= g_dwRamSize )
	{
		TRACE0("MoveMem Viewport, invalid memory");
		return;
	}

	short scale[4];
	short trans[4];

	// dwAddr is offset into RD_RAM of 8 x 16bits of data...
	scale[0] = *(short *)(g_pRDRAMu8 + ((dwAddr+(0*2))^0x2));
	scale[1] = *(short *)(g_pRDRAMu8 + ((dwAddr+(1*2))^0x2));
	scale[2] = *(short *)(g_pRDRAMu8 + ((dwAddr+(2*2))^0x2));
	scale[3] = *(short *)(g_pRDRAMu8 + ((dwAddr+(3*2))^0x2));

	trans[0] = *(short *)(g_pRDRAMu8 + ((dwAddr+(4*2))^0x2));
	trans[1] = *(short *)(g_pRDRAMu8 + ((dwAddr+(5*2))^0x2));
	trans[2] = *(short *)(g_pRDRAMu8 + ((dwAddr+(6*2))^0x2));
	trans[3] = *(short *)(g_pRDRAMu8 + ((dwAddr+(7*2))^0x2));


	int nCenterX = trans[0]/4;
	int nCenterY = trans[1]/4;
	int nWidth   = scale[0]/4;
	int nHeight  = scale[1]/4;

	// Check for some strange games
	if( nWidth < 0 )	nWidth = -nWidth;
	if( nHeight < 0 )	nHeight = -nHeight;

	int nLeft = nCenterX - nWidth;
	int nTop  = nCenterY - nHeight;
	int nRight= nCenterX + nWidth;
	int nBottom= nCenterY + nHeight;

	//LONG maxZ = scale[2];
	int maxZ = 0x3FF;

	CRender::g_pRender->SetViewport(nLeft, nTop, nRight, nBottom, maxZ);


	LOG_DL("        Scale: %d %d %d %d = %d,%d", scale[0], scale[1], scale[2], scale[3], nWidth, nHeight);
	LOG_DL("        Trans: %d %d %d %d = %d,%d", trans[0], trans[1], trans[2], trans[3], nCenterX, nCenterY);
}


// S2DEX uses this - 0xc1
void RSP_S2DEX_SPObjLoadTxtr_Ucode1(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_S2DEX_SPObjLoadTxtr_Ucode1);

	// Add S2DEX ucode supporting to F3DEX, see game DT and others
	status.bUseModifiedUcodeMap = true;
	RSP_SetUcode(1);
	memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
	
	//LoadedUcodeMap[S2DEX_OBJ_RECTANGLE] = &RSP_S2DEX_OBJ_RECTANGLE;
	LoadedUcodeMap[S2DEX_OBJ_MOVEMEM] = &RSP_S2DEX_OBJ_MOVEMEM;
	LoadedUcodeMap[S2DEX_OBJ_LOADTXTR] = &RSP_S2DEX_SPObjLoadTxtr;
	LoadedUcodeMap[S2DEX_OBJ_LDTX_SPRITE] = &RSP_S2DEX_SPObjLoadTxSprite;
	LoadedUcodeMap[S2DEX_OBJ_LDTX_RECT] = &RSP_S2DEX_SPObjLoadTxRect;
	LoadedUcodeMap[S2DEX_OBJ_LDTX_RECT_R] = &RSP_S2DEX_SPObjLoadTxRectR;

	TRACE0("Adding S2DEX ucodes to ucode 1");

	RSP_S2DEX_SPObjLoadTxtr(word0, word1);
}
