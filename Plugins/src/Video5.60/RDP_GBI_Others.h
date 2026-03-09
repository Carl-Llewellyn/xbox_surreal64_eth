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

// A few ucode used in DKR and Others Special games

uint32 dwConkerVtxZAddr=0;

static void RDP_GFX_DumpVtxInfoDKR(uint32 dwAddr, uint32 dwV0, uint32 dwN);

void RDP_GFX_DLInMem(uint32 word0, uint32 word1)
{
	uint32 dwLimit = (word0 >> 16) & 0xFF;
	uint32 dwPush = RSP_DLIST_PUSH; //(word0 >> 16) & 0xFF;
	uint32 dwAddr = 0x00000000 | word1; //RSPSegmentAddr(word1);

	LOG_DL("    Address=0x%08x Push: 0x%02x", dwAddr, dwPush);
	
	switch (dwPush)
	{
	case RSP_DLIST_PUSH:
		LOG_DL("    Pushing DisplayList 0x%08x", dwAddr);
		g_dwPCindex++;
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = dwLimit;

		break;
	case RSP_DLIST_NOPUSH:
		LOG_DL("    Jumping to DisplayList 0x%08x", dwAddr);
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = dwLimit;
		break;
	}

	LOG_DL("");
	LOG_DL("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
	LOG_DL("#############################################");
}

extern Matrix dkrMatrixTransposed;
void RSP_Mtx_DKR(uint32 word0, uint32 word1)
{	
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwCommand = (word0>>16)&0xFF;
	uint32 dwLength  = (word0)    &0xFFFF;

	//if( dwAddr == 0 || dwAddr < 0x2000)
	{
		dwAddr = word1+RSPSegmentAddr(gRSP.dwDKRMatrixAddr);
	}

	//gRSP.DKRCMatrixIndex = (word0>>22)&3;
	bool mul=false;
	int index;
	switch( dwCommand )
	{
	case 0xC0:	// DKR
		gRSP.DKRCMatrixIndex = index = 3;
		break;
	case 0x80:	// DKR
		gRSP.DKRCMatrixIndex = index = 2;
		break;
	case 0x40:	// DKR
		gRSP.DKRCMatrixIndex = index = 1;
		break;
	case 0x20:	// DKR
		gRSP.DKRCMatrixIndex = index = 0;
		break;
	case 0x00:
		gRSP.DKRCMatrixIndex = index = 0;
		break;
	case 0x01:
		//mul = true;
		gRSP.DKRCMatrixIndex = index = 1;
		break;
	case 0x02:
		//mul = true;
		gRSP.DKRCMatrixIndex = index = 2;
		break;
	case 0x03:
		//mul = true;
		gRSP.DKRCMatrixIndex = index = 3;
		break;
	case 0x81:
		index = 1;
		mul = true;
		break;
	case 0x82:
		index = 2;
		mul = true;
		break;
	case 0x83:
		index = 3;
		mul = true;
		break;
	default:
		DebuggerAppendMsg("Fix me, mtx DKR, cmd=%08X", dwCommand);
		break;
	}

	LOG_DL("    DKR Loading Mtx: %d, command=%d", index, dwCommand);

	if (dwAddr + 64 > g_dwRamSize)
	{
		TRACE1("Mtx: Address invalid (0x%08x)", dwAddr);
		return;
	}

	// Load matrix from dwAddr
	Matrix tempmat;
	Matrix &mat = gRSP.DKRMatrixes[index];

	uint32 dwI;
	uint32 dwJ;
	const float fRecip = 1.0f / 65536.0f;
	
	for (dwI = 0; dwI < 4; dwI++) {
		for (dwJ = 0; dwJ < 4; dwJ++) {

			short nDataHi = *(short *)(g_pRDRAMu8 + ((dwAddr+(dwI<<3)+(dwJ<<1)     )^0x2));
			WORD  nDataLo = *(WORD  *)(g_pRDRAMu8 + ((dwAddr+(dwI<<3)+(dwJ<<1) + 32)^0x2));

			tempmat.m[dwI][dwJ] = (float)(((LONG)nDataHi<<16) | (nDataLo))*fRecip;
		}
	}

	if( mul )
	{
		mat = tempmat*gRSP.DKRMatrixes[0];
	}
	else
	{
		mat = tempmat;
	}

	if( status.isSSEEnabled )
		D3DXMatrixTranspose(&dkrMatrixTransposed, &mat);

#ifdef _DEBUG
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		DebuggerAppendMsg("DKR Matrix: %08X-%08X\n\tMatrix:%d Address 0x%08x", word0, word1, gRSP.DKRCMatrixIndex, dwAddr);
		DebuggerAppendMsg(
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n",
			mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
			mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
			mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
			mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);

		pauseAtNext = false;
		debuggerPause = true;
		CGraphicsContext::Get()->UpdateFrame();
	}
	else
	{
		if( pauseAtNext && logMatrix ) 
		{
			TRACE0("DKR Matrix\n");
		}
	}
#endif
}

void RSP_Vtx_DKR(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwV0 =  gRSP.DKRVtxCount;
	uint32 dwN  = ((word0 >>19 )&0x1F)+1;

	LOG_DL("    Address 0x%08x, v0: %d, Num: %d", dwAddr, dwV0, dwN);
	DEBUGGER_ONLY_IF( (pauseAtNext && (eventToPause==NEXT_VERTEX_CMD||eventToPause==NEXT_MATRIX_CMD)), {DebuggerAppendMsg("DKR Vtx: Cmd0=%08X, Cmd1=%08X", word0, word1);});

#ifdef _DEBUG
	if( pauseAtNext && logVertex ) 
	{
		DebuggerAppendMsg("Vtx_DKR, cmd0=%08X cmd1=%08X", word0, word1);
	}
#endif

	if (dwV0 >= 32)
		dwV0 = 31;
	
	if ((dwV0 + dwN) > 32)
	{
		TRACE0("Warning, attempting to load into invalid vertex positions");
		dwN = 32 - dwV0;
	}

	
	//if( dwAddr == 0 || dwAddr < 0x2000)
	{
		dwAddr = word1+RSPSegmentAddr(gRSP.dwDKRVtxAddr);
	}

	// Check that address is valid...
	if ((dwAddr + (dwN*16)) > g_dwRamSize)
	{
		TRACE1("ProcessVertexData: Address out of range (0x%08x)", dwAddr);
	}
	else
	{
		ProcessVertexDataDKR(dwAddr, dwV0, dwN);

		status.dwNumVertices += dwN;

		RDP_GFX_DumpVtxInfoDKR(dwAddr, dwV0, dwN);
	}
}


void RSP_Vtx_Gemini(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwV0 =  ((word0>>9)&0x1F);
	uint32 dwN  = ((word0 >>19 )&0x1F);

	LOG_DL("    Address 0x%08x, v0: %d, Num: %d", dwAddr, dwV0, dwN);
	DEBUGGER_ONLY_IF( (pauseAtNext && (eventToPause==NEXT_VERTEX_CMD||eventToPause==NEXT_MATRIX_CMD)), {DebuggerAppendMsg("DKR Vtx: Cmd0=%08X, Cmd1=%08X", word0, word1);});

#ifdef _DEBUG
	if( pauseAtNext && logVertex ) 
	{
		DebuggerAppendMsg("Vtx_DKR, cmd0=%08X cmd1=%08X", word0, word1);
	}
#endif

	if (dwV0 >= 32)
		dwV0 = 31;

	if ((dwV0 + dwN) > 32)
	{
		TRACE0("Warning, attempting to load into invalid vertex positions");
		dwN = 32 - dwV0;
	}


	//if( dwAddr == 0 || dwAddr < 0x2000)
	{
		dwAddr = word1+RSPSegmentAddr(gRSP.dwDKRVtxAddr);
	}

	// Check that address is valid...
	if ((dwAddr + (dwN*16)) > g_dwRamSize)
	{
		TRACE1("ProcessVertexData: Address out of range (0x%08x)", dwAddr);
	}
	else
	{
		ProcessVertexDataDKR(dwAddr, dwV0, dwN);

		status.dwNumVertices += dwN;

		RDP_GFX_DumpVtxInfoDKR(dwAddr, dwV0, dwN);
	}
}

// DKR verts are extra 4 bytes
void RDP_GFX_DumpVtxInfoDKR(uint32 dwAddr, uint32 dwV0, uint32 dwN)
{
#ifdef _DEBUG
		uint32 dwV;
		LONG i;

		short * psSrc = (short *)(g_pRDRAMu8 + dwAddr);

		i = 0;
		for (dwV = dwV0; dwV < dwV0 + dwN; dwV++)
		{
			float x = (float)psSrc[(i + 0) ^ 1];
			float y = (float)psSrc[(i + 1) ^ 1];
			float z = (float)psSrc[(i + 2) ^ 1];

			//WORD wFlags = CRender::g_pRender->m_dwVecFlags[dwV]; //(WORD)psSrc[3^0x1];

			WORD wA = psSrc[(i + 3) ^ 1];
			WORD wB = psSrc[(i + 4) ^ 1];

			BYTE a = wA>>8;
			BYTE b = (BYTE)wA;
			BYTE c = wB>>8;
			BYTE d = (BYTE)wB;

			D3DXVECTOR4 & t = g_vecProjected[dwV];


			LOG_DL(" #%02d Pos: {% 6f,% 6f,% 6f} Extra: %02x %02x %02x %02x (transf: {% 6f,% 6f,% 6f})",
				dwV, x, y, z, a, b, c, d, t.x, t.y, t.z );

			i+=5;
		}


		uint16 * pwSrc = (uint16 *)(g_pRDRAMu8 + dwAddr);
		i = 0;
		for (dwV = dwV0; dwV < dwV0 + dwN; dwV++)
		{
			LOG_DL(" #%02d %04x %04x %04x %04x %04x",
				dwV, pwSrc[(i + 0) ^ 1],
				pwSrc[(i + 1) ^ 1],
				pwSrc[(i + 2) ^ 1],
				pwSrc[(i + 3) ^ 1],
				pwSrc[(i + 4) ^ 1]);

			i += 5;
		}

#endif // _DEBUG
}

void DLParser_Set_Addr_Ucode6(uint32 word0, uint32 word1)
{
	gRSP.dwDKRMatrixAddr = word0&0x00FFFFFF;
	gRSP.dwDKRVtxAddr = word1&0x00FFFFFF;
	gRSP.DKRVtxCount=0;
}



void RSP_Vtx_WRUS(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwLength = (word0)&0xFFFF;

	uint32 dwN= (dwLength + 1) / 0x210;
	//uint32 dwN= (dwLength >> 9);
	//uint32 dwV0 = ((word0>>16)&0x3f)/5;
	uint32 dwV0 = ((word0>>16)&0xFF)/5;

	LOG_DL("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", dwAddr, dwV0, dwN, dwLength);

	if (dwV0 >= 32)
		dwV0 = 31;
	
	if ((dwV0 + dwN) > 32)
	{
		TRACE0("Warning, attempting to load into invalid vertex positions");
		dwN = 32 - dwV0;
	}

	ProcessVertexData(dwAddr, dwV0, dwN);

	status.dwNumVertices += dwN;

	DisplayVertexInfo(dwAddr, dwV0, dwN);
}

void RSP_Vtx_ShadowOfEmpire(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwLength = (word0)&0xFFFF;

	uint32 dwN= ((word0 >> 4) & 0xFFF) / 33 + 1;
	uint32 dwV0 = 0;

	LOG_DL("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", dwAddr, dwV0, dwN, dwLength);

	if (dwV0 >= 32)
		dwV0 = 31;
	
	if ((dwV0 + dwN) > 32)
	{
		TRACE0("Warning, attempting to load into invalid vertex positions");
		dwN = 32 - dwV0;
	}

	ProcessVertexData(dwAddr, dwV0, dwN);

	status.dwNumVertices += dwN;

	DisplayVertexInfo(dwAddr, dwV0, dwN);
}


void RSP_DL_In_MEM_DKR(uint32 word0, uint32 word1)
{
	// This cmd is likely to execute number of ucode at the given address
	uint32 dwAddr = word1;//RSPSegmentAddr(word1);
	{
		g_dwPCindex++;
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = ((word0>>16)&0xFF);
	}
}
WORD ConvertYUVtoR5G5B5X1(int y, int u, int v)
{
	float r = y + (1.370705f * (v-128));
	float g = y - (0.698001f * (v-128)) - (0.337633f * (u-128));
	float b = y + (1.732446f * (u-128));
	r *= 0.125f;
	g *= 0.125f;
	b *= 0.125f;

	//clipping the result
	if (r > 32) r = 32;
	if (g > 32) g = 32;
	if (b > 32) b = 32;
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	WORD c = (WORD)(((WORD)(r) << 11) |
		((WORD)(g) << 6) |
		((WORD)(b) << 1) | 1);
	return c;
}

void TexRectToN64FrameBuffer_YUV_16b(DWORD x0, DWORD y0, DWORD width, DWORD height)
{
	// Convert YUV image at TImg and Copy the texture into the N64 RDRAM framebuffer memory

	DWORD n64CIaddr = g_CI.dwAddr;
	DWORD n64CIwidth = g_CI.dwWidth;

	for (int y = 0; y < (int)height; y++)
	{
		DWORD* pN64Src = (DWORD*)(g_pRDRAMu8+(g_TI.dwAddr&(g_dwRamSize-1)))+y*(g_TI.dwWidth>>1);
		WORD* pN64Dst = (WORD*)(g_pRDRAMu8+(n64CIaddr&(g_dwRamSize-1)))+(y+y0)*n64CIwidth;

		for (int x = 0; x < (int)width; x+=2)
		{
			DWORD val = *pN64Src++;
			int y0 = (BYTE)val&0xFF;
			int v  = (BYTE)(val>>8)&0xFF;
			int y1 = (BYTE)(val>>16)&0xFF;
			int u  = (BYTE)(val>>24)&0xFF;

			pN64Dst[x+x0] = ConvertYUVtoR5G5B5X1(y0,u,v);
			pN64Dst[x+x0+1] = ConvertYUVtoR5G5B5X1(y1,u,v);
		}
	}
}

extern uObjMtxReal gObjMtxReal;
void DLParser_OgreBatter64BG(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	uObjTxSprite *ptr = (uObjTxSprite*)(g_pRDRAMu8+dwAddr);
	//CRender::g_pRender->LoadObjSprite(*ptr,true);
	PrepareTextures();

	CTexture *ptexture = g_textures[0].m_pCTexture;
	TexRectToN64FrameBuffer_16b( (uint32)gObjMtxReal.X, (uint32)gObjMtxReal.Y, ptexture->m_dwWidth, ptexture->m_dwHeight, gRSP.curTile);

#ifdef _DEBUG
	CRender::g_pRender->DrawSpriteR(*ptr, false);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((pauseAtNext && (eventToPause==NEXT_OBJ_TXT_CMD|| eventToPause==NEXT_FLUSH_TRI)),
	{
		DebuggerAppendMsg("OgreBatter 64 BG: Addr=%08X\n", dwAddr);
	}
	);
#endif
}

void DLParser_Bomberman2TextRect(uint32 word0, uint32 word1)
{
	// Bomberman 64 - The Second Attack! (U) [!]
	// The 0x02 cmd, list a TexRect cmd

	if( options.enableHackForGames == HACK_FOR_OGRE_BATTLE && gRDP.tiles[7].dwFormat == TXT_FMT_YUV )
	{
		TexRectToN64FrameBuffer_YUV_16b( (uint32)gObjMtxReal.X, (uint32)gObjMtxReal.Y, 16, 16);
		//DLParser_OgreBatter64BG(word0, word1);
		return;
	}

	uint32 dwAddr = RSPSegmentAddr(word1);
	uObjSprite *info = (uObjSprite*)(g_pRDRAMu8+dwAddr);

	uint32 dwTile	= gRSP.curTile;

	PrepareTextures();
	
	//CRender::g_pRender->SetCombinerAndBlender();

	uObjTxSprite drawinfo;
	memcpy( &(drawinfo.sprite), info, sizeof(uObjSprite));
	CRender::g_pRender->DrawSpriteR(drawinfo, false, dwTile, 0, 0, drawinfo.sprite.imageW/32, drawinfo.sprite.imageH/32);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI)),
		{
			DebuggerAppendMsg("Bomberman 64 - TextRect: Addr=%08X\n", dwAddr);
			dwAddr &= (g_dwRamSize-1);
			DebuggerAppendMsg("%08X-%08X-%08X-%08X-%08X-%08X\n", RDRAM_UWORD(dwAddr), RDRAM_UWORD(dwAddr+4),
				RDRAM_UWORD(dwAddr+8), RDRAM_UWORD(dwAddr+12), RDRAM_UWORD(dwAddr+16), RDRAM_UWORD(dwAddr+20) );
		}
	);
}


void RSP_MoveWord_DKR(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI1_MoveWord);
	uint32 dwNumLights;

	switch (word0 & 0xFF)
	{
	case RSP_MOVE_WORD_NUMLIGHT:
		dwNumLights = word1&0x7;
		LOG_DL("    RSP_MOVE_WORD_NUMLIGHT: Val:%d", dwNumLights);

		gRSP.ambientLightIndex = dwNumLights;
		SetNumLights(dwNumLights);
		if( word1&0x7 )
		{
			gRSP.DKRVtxAddBase = true;
		}
		else
		{
			gRSP.DKRVtxAddBase = false;
		}
		LOG_DL("    gRSP.DKRVtxAddBase = %d", gRSP.DKRVtxAddBase);
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_MATRIX_CMD, {DebuggerAppendMsg("DKR Moveword, select gRSP.DKRVtxAddBase %s, cmd0=%08X, cmd1=%08X", gRSP.DKRVtxAddBase?"true":"false", word0, word1);});
		break;
	case RSP_MOVE_WORD_LIGHTCOL:
		gRSP.DKRCMatrixIndex = (word1>>6)&7;
		LOG_DL("    gRSP.DKRCMatrixIndex = %d", gRSP.DKRCMatrixIndex);
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_MATRIX_CMD, {DebuggerAppendMsg("DKR Moveword, select matrix %d, cmd0=%08X, cmd1=%08X", gRSP.DKRCMatrixIndex, word0, word1);});
		break;
	default:
		RSP_GBI1_MoveWord(word0, word1);
		break;
	}

}


void RSP_DMA_Tri_DKR(uint32 word0, uint32 word1)
{
	BOOL bTrisAdded = FALSE;
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 flag = (word0 & 0xFF0000) >> 16;
	if (flag&1) 
		CRender::g_pRender->SetCullMode(false,true);
	else
		CRender::g_pRender->SetCullMode(false,false);


	uint32 dwNum = ((word0 &  0xFFF0) >>4 );
	uint32 i;
	uint32 * pData = (uint32*)&g_pRDRAMu32[dwAddr/4];

	if( dwAddr+16*dwNum >= g_dwRamSize )
	{
		TRACE0("DMATRI invalid memory pointer");
		return;
	}

#ifdef _DEBUG
	if( pauseAtNext && logTriangles ) 
	{
		DebuggerAppendMsg("DMATRI, addr=%08X, Cmd0=%08X\n", dwAddr, word0);
	}
#endif

	status.primitiveType = PRIM_DMA_TRI;

	for (i = 0; i < dwNum; i++)
	{
		LOG_DL("    0x%08x: %08x %08x %08x %08x", dwAddr + i*16,
			pData[0], pData[1], pData[2], pData[3]);

		uint32 dwInfo = pData[0];

		uint32 dwV0 = (dwInfo >> 16) & 0x1F;
		uint32 dwV1 = (dwInfo >>  8) & 0x1F;
		uint32 dwV2 = (dwInfo      ) & 0x1F;

#ifdef _DEBUG
		if( pauseAtNext && logTriangles ) 
		{
			DebuggerAppendMsg("DMATRI: %d, %d, %d (%08X-%08X)", dwV0,dwV1,dwV2,word0,word1);
		}
#endif

		//if (TestTri(dwV0, dwV1, dwV2))
		{
			DEBUG_DUMP_VERTEXES("DmaTri", dwV0, dwV1, dwV2);
			LOG_DL("   Tri: %d,%d,%d", dwV0, dwV1, dwV2);
			if (!bTrisAdded )//&& CRender::g_pRender->IsTextureEnabled())
			{
				PrepareTextures();
				InitVertexTextureConstants();
			}

			// Generate texture coordinates
			short s0 = ((short)(pData[1]>>16));
			short t0 = ((short)(pData[1]&0xFFFF));
			short s1 = ((short)(pData[2]>>16));
			short t1 = ((short)(pData[2]&0xFFFF));
			short s2 = ((short)(pData[3]>>16));
			short t2 = ((short)(pData[3]&0xFFFF));

#ifdef _DEBUG
			if( pauseAtNext && logTriangles ) 
			{
				DebuggerAppendMsg(" (%d,%d), (%d,%d), (%d,%d)",s0,t0,s1,t1,s2,t2);
				DebuggerAppendMsg(" (%08X), (%08X), (%08X), (%08X)",pData[0],pData[1],pData[2],pData[3]);
			}
#endif
			CRender::g_pRender->SetVtxTextureCoord(dwV0, s0, t0);
			CRender::g_pRender->SetVtxTextureCoord(dwV1, s1, t1);
			CRender::g_pRender->SetVtxTextureCoord(dwV2, s2, t2);

			if( !bTrisAdded )
			{
				CRender::g_pRender->SetCombinerAndBlender();
			}

			bTrisAdded = TRUE;
			AddTri(dwV0, dwV1, dwV2);
		}

		pData += 4;

	}

	if (bTrisAdded)	
	{
		CRender::g_pRender->FlushTris();
	}
	gRSP.DKRVtxCount=0;
}

uint32 dwPDCIAddr = 0;
void ProcessVertexDataPD(uint32 dwAddr, uint32 dwV0, uint32 dwNum);
void RSP_Vtx_PD(uint32 word0, uint32 word1)
{
	SP_Timing(RSP_GBI0_Vtx);

	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwV0 =  (word0>>16)&0x0F;
	uint32 dwN  = ((word0>>20)&0x0F)+1;
	uint32 dwLength = (word0)&0xFFFF;

	LOG_DL("    Address 0x%08x, v0: %d, Num: %d", dwAddr, dwV0, dwN);

	ProcessVertexDataPD(dwAddr, dwV0, dwN);
	status.dwNumVertices += dwN;
}

void RSP_Set_Vtx_CI_PD(uint32 word0, uint32 word1)
{
	// Color index buf address
	dwPDCIAddr = RSPSegmentAddr(word1);
}

void RSP_Tri4_PD(uint32 word0, uint32 word1)
{
	status.primitiveType = PRIM_TRI2;

	// While the next command pair is Tri2, add vertices
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	do {
		uint32 dwFlag = (word0>>16)&0xFF;
		LOG_DL("    PD Tri4: 0x%08x 0x%08x Flag: 0x%02x", word0, word1, dwFlag);

		BOOL bVisible;
		for( uint32 i=0; i<4; i++)
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
}


void DLParser_Tri4_Conker(uint32 word0, uint32 word1)
{
	status.primitiveType = PRIM_TRI2;

	// While the next command pair is Tri2, add vertices
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	do {
		LOG_DL("    Conker Tri4: 0x%08x 0x%08x", word0, word1);
		uint32 idx[12];
		idx[0] = (word1   )&0x1F;
		idx[1] = (word1>> 5)&0x1F;
		idx[2] = (word1>>10)&0x1F;
		idx[3] = (word1>>15)&0x1F;
		idx[4] = (word1>>20)&0x1F;
		idx[5] = (word1>>25)&0x1F;

		idx[6] = (word0    )&0x1F;
		idx[7] = (word0>> 5)&0x1F;
		idx[8] = (word0>>10)&0x1F;

		idx[ 9] = (((word0>>15)&0x7)<<2)|(word1>>30);
		idx[10] = (word0>>18)&0x1F;
		idx[11] = (word0>>23)&0x1F;

		BOOL bVisible;
		for( uint32 i=0; i<4; i++)
		{
			uint32 v0=idx[i*3  ];
			uint32 v1=idx[i*3+1];
			uint32 v2=idx[i*3+2];
			bVisible = TestTri(v0, v1, v2);
			LOG_DL("       (%d, %d, %d) %s", v0, v1, v2, bVisible ? "": "(clipped)");
			if (bVisible)
			{
				DEBUG_DUMP_VERTEXES("Tri4 Conker:", v0, v1, v2);
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
				AddTri(v0, v1, v2);
			}
		}

		word0			= *(uint32 *)(g_pRDRAMu8 + dwPC+0);
		word1			= *(uint32 *)(g_pRDRAMu8 + dwPC+4);
		dwPC += 8;

#ifdef _DEBUG
	} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (word0>>28) == 1 );
	if(pauseAtNext && eventToPause==NEXT_TRIANGLE )
	{
		eventToPause = NEXT_FLUSH_TRI;
	}

#else
	} while ((word0>>28) == 1);
#endif

	g_dwPCStack[g_dwPCindex].addr = dwPC-8;

	if (bTrisAdded)	
	{
		CRender::g_pRender->FlushTris();
	}
}

void RDP_GFX_Force_Vertex_Z_Conker(uint32 dwAddr)
{
#ifdef _DEBUG
	if( pauseAtNext && logVertex ) 
	{
		s8 * pcBase = g_pRDRAMs8 + (dwAddr&(g_dwRamSize-1));
		uint32 * pdwBase = (uint32 *)pcBase;
		LONG i;

		for (i = 0; i < 4; i++)
		{
			DebuggerAppendMsg("    %08x %08x %08x %08x", pdwBase[0], pdwBase[1], pdwBase[2], pdwBase[3]);
			pdwBase+=4;
		}
	}
#endif

	dwConkerVtxZAddr = dwAddr;
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at RDP_GFX_Force_Matrix_Conker Cmd");});
}



void DLParser_MoveMem_Conker(uint32 word0, uint32 word1)
{
	uint32 dwType    = (word0     ) & 0xFE;
	uint32 dwAddr = RSPSegmentAddr(word1);
	if( dwType == RSP_GBI2_MV_MEM__MATRIX )
	{
		LOG_DL("    DLParser_MoveMem_Conker");
		RDP_GFX_Force_Vertex_Z_Conker(dwAddr);
	}
	else if( dwType == RSP_GBI2_MV_MEM__LIGHT )
	{
		LOG_DL("    MoveMem Light Conker");
		uint32 dwOffset2 = (word0 >> 5) & 0x3FFF;
		uint32 dwLight=0xFF;
		if( dwOffset2 >= 0x30 )
		{
			dwLight = (dwOffset2 - 0x30)/0x30;
			LOG_DL("    Light %d:", dwLight);
			RSP_MoveMemLight(dwLight, dwAddr);
		}
		else
		{
			// fix me
			//TRACE0("Check me in DLParser_MoveMem_Conker - MoveMem Light");
		}
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_SET_LIGHT, 
		{
			DebuggerAppendMsg("RSP_MoveMemLight: %d, Addr=%08X, cmd0=%08X", dwLight, dwAddr, word0);
			TRACE0("Pause after MoveMemLight");
		});
	}
	else
	{
		RSP_GBI2_MoveMem(word0,word1);
	}
}

extern void ProcessVertexDataConker(uint32 dwAddr, uint32 dwV0, uint32 dwNum);
void RSP_Vtx_Conker(uint32 word0, uint32 word1)
{
	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwVEnd    = ((word0   )&0xFFF)/2;
	uint32 dwN      = ((word0>>12)&0xFFF);
	uint32 dwV0		= dwVEnd - dwN;

	LOG_DL("    Vtx: Address 0x%08x, vEnd: %d, v0: %d, Num: %d", dwAddr, dwVEnd, dwV0, dwN);

	ProcessVertexDataConker(dwAddr, dwV0, dwN);
	status.dwNumVertices += dwN;
	DisplayVertexInfo(dwAddr, dwV0, dwN);
}


void DLParser_MoveWord_Conker(uint32 word0, uint32 word1)
{
	uint32 dwType   = (word0 >> 16) & 0xFF;
	if( dwType != RSP_MOVE_WORD_NUMLIGHT )
	{
		RSP_GBI2_MoveWord(word0, word1);
	}
	else
	{
		uint32 dwNumLights = (word1/48);
		LOG_DL("Conker RSP_MOVE_WORD_NUMLIGHT: %d", dwNumLights);
		gRSP.ambientLightIndex = dwNumLights+1;
		SetNumLights(dwNumLights);
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_SET_LIGHT, 
		{
			DebuggerAppendMsg("SetNumLights: %d", dwNumLights);
			TRACE0("Pause after SetNumLights");
		});
	}
}

void RSP_Mtx_SiliconValley(uint32 word0, uint32 word1)
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
		if( nLoadCommand == RENDER_MUL_MATRIX && bPush == FALSE )
		{
			//nLoadCommand = RENDER_LOAD_MATRIX;
			//TRACE0("Matrix mul and nopush ==> load");
		}
		CRender::g_pRender->SetWorldView(mat, bPush, nLoadCommand);
	}

#ifdef _DEBUG
	char *loadstr = nLoadCommand==RENDER_LOAD_MATRIX?"Load":"Mul";
	char *pushstr = bPush?"Push":"Nopush";
	int projlevel = CRender::g_pRender->GetProjectMatrixLevel();
	int worldlevel = CRender::g_pRender->GetWorldViewMatrixLevel();
	if( pauseAtNext && eventToPause == NEXT_MATRIX_CMD )
	{
		if( !(dwCommand & RSP_MATRIX_PROJECTION ) && nLoadCommand == RENDER_MUL_MATRIX && bPush == FALSE )
		{
			DebuggerAppendMsg("Mul&Nopush: %08X, %08X", word0, word1 );
		}
		pauseAtNext = false;
		debuggerPause = true;
		if (dwCommand & RSP_MATRIX_PROJECTION)
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
			if (dwCommand & RSP_MATRIX_PROJECTION)
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

void DLParser_Ucode8_0x0(uint32 word0, uint32 word1)
{
	/*
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;
	uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC);

	if( (dwCmd2&0xFF000000) == 0x00000000 )
	{
		LOG_DL("ucode 0x00 at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);
		RDP_GFX_PopDL();
	}
	*/
}


uint32 Rogue_Squadron_Vtx_XYZ_Cmd;
uint32 Rogue_Squadron_Vtx_XYZ_Addr;
uint32 Rogue_Squadron_Vtx_Color_Cmd;
uint32 Rogue_Squadron_Vtx_Color_Addr;
uint32 GSBlkAddrSaves[100][2];

void ProcessVertexData_Rogue_Squadron(uint32 dwXYZAddr, uint32 dwColorAddr, uint32 dwXYZCmd, uint32 dwColorCmd);

void DLParser_RS_Color_Buffer(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	uint32 dwAddr = RSPSegmentAddr(word1);

	if( dwAddr > g_dwRamSize )
	{
		TRACE0("DL, addr is wrong");
		dwAddr = word1&(g_dwRamSize-1);
	}

	Rogue_Squadron_Vtx_Color_Cmd = word0;
	Rogue_Squadron_Vtx_Color_Addr = dwAddr;

	LOG_DL("Vtx_Color at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);
#ifdef _DEBUG
	if( pauseAtNext && (eventToPause == NEXT_VERTEX_CMD ) )
	{
		DebuggerAppendMsg("Vtx_Color at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);
		if( dwAddr < g_dwRamSize )
		{
			DumpHex(dwAddr, min(64, g_dwRamSize-dwAddr));
		}
	}
#endif

	ProcessVertexData_Rogue_Squadron(Rogue_Squadron_Vtx_XYZ_Addr, Rogue_Squadron_Vtx_Color_Addr, Rogue_Squadron_Vtx_XYZ_Cmd, Rogue_Squadron_Vtx_Color_Cmd);

}


void DLParser_RS_Vtx_Buffer(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	uint32 dwAddr = RSPSegmentAddr(word1);
	if( dwAddr > g_dwRamSize )
	{
		TRACE0("DL, addr is wrong");
		dwAddr = word1&(g_dwRamSize-1);
	}

	LOG_DL("Vtx_XYZ at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);
	Rogue_Squadron_Vtx_XYZ_Cmd = word0;
	Rogue_Squadron_Vtx_XYZ_Addr = dwAddr;

#ifdef _DEBUG
	if( pauseAtNext && (eventToPause == NEXT_VERTEX_CMD ) )
	{
		DebuggerAppendMsg("Vtx_XYZ at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);
		if( dwAddr < g_dwRamSize )
		{
			DumpHex(dwAddr, min(64, g_dwRamSize-dwAddr));
		}
	}
#endif
}


void DLParser_RS_Block(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode 0x80 at PC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
}

void DLParser_RS_MoveMem(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;
	uint32 cmd1 = ((dwPC)&0x00FFFFFF)|0x80000000;
	RSP_GBI1_MoveMem(word0, cmd1);
	/*
	LOG_DL("RS_MoveMem", (word0>>24));
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	uint32 dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd2, dwCmd3);
	dwPC+=8;
	uint32 dwCmd4 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	uint32 dwCmd5 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd4, dwCmd5);
	*/
	g_dwPCStack[g_dwPCindex].addr += 16;

	//DEBUGGER_PAUSE_AND_DUMP(NEXT_SET_MODE_CMD, {
	//	DebuggerAppendMsg("Pause after RS_MoveMem at: %08X\n", dwPC-8);
	//});

}

void DLParser_RS_0xbe(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode %02X, skip 1", (word0>>24));
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	uint32 dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd2, dwCmd3);
	g_dwPCStack[g_dwPCindex].addr += 8;

	DEBUGGER_PAUSE_AND_DUMP(NEXT_SET_MODE_CMD, {
		DebuggerAppendMsg("Pause after RS_0xbe at: %08X\n", dwPC-8);
		DebuggerAppendMsg("\t0x%08x 0x%08x", word0, word1);
		DebuggerAppendMsg("\t0x%08x 0x%08x", dwCmd2, dwCmd3);
	});

}


void DLParser_Ucode8_EndDL(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	RDP_GFX_PopDL();
	DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, DebuggerAppendMsg("PC=%08X: EndDL, return to %08X\n\n", dwPC, g_dwPCStack[g_dwPCindex].addr));
}

void DLParser_Ucode8_DL(uint32 word0, uint32 word1)	// DL Function Call
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;

	uint32 dwAddr = RSPSegmentAddr(word1);
	uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwAddr);
	uint32 dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwAddr+4);

	if( dwAddr > g_dwRamSize )
	{
		TRACE0("DL, addr is wrong");
		dwAddr = word1&(g_dwRamSize-1);
	}

	// Detect looping
	if(g_dwPCindex>0 )
	{
		for( int i=0; i<g_dwPCindex; i++ )
		{
			if( g_dwPCStack[i].addr == dwAddr+8 )
			{
				TRACE1("Detected DL looping, PC=%08X", dwPC );
				DLParser_Ucode8_EndDL(0,0);
				return;
			}
		}
	}

	g_dwPCindex++;
	g_dwPCStack[g_dwPCindex].addr = dwAddr+8;
	g_dwPCStack[g_dwPCindex].limit = ~0;

	GSBlkAddrSaves[g_dwPCindex][0]=GSBlkAddrSaves[g_dwPCindex][1]=0;
	if( (dwCmd2>>24) == 0x80 )
	{
		GSBlkAddrSaves[g_dwPCindex][0] = dwCmd2;
		GSBlkAddrSaves[g_dwPCindex][1] = dwCmd3;
	}

	DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
		DebuggerAppendMsg("\nPC=%08X: Call DL at Address %08X - %08X, %08X\n\n", dwPC, dwAddr, dwCmd2, dwCmd3)
	);
}

void DLParser_Ucode8_JUMP(uint32 word0, uint32 word1)	// DL Function Call
{
	if( (word0&0x00FFFFFF) == 0 )
	{
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		uint32 dwAddr = RSPSegmentAddr(word1);

		if( dwAddr > g_dwRamSize )
		{
			TRACE0("DL, addr is wrong");
			dwAddr = word1&(g_dwRamSize-1);
		}

		uint32 dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwAddr);
		uint32 dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwAddr+4);

		g_dwPCStack[g_dwPCindex].addr = dwAddr+8;		// Jump to new address
		DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
			DebuggerAppendMsg("\nPC=%08X: Jump to Address %08X - %08X, %08X\n\n", dwPC, dwAddr, dwCmd2, dwCmd3)
		);
	}
	else
	{
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		LOG_DL("ucode 0x07 at PC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
	}
}



void DLParser_Ucode8_Unknown(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode %02X at PC=%08X: 0x%08x 0x%08x\n", (word0>>24), dwPC, word0, word1);
}

void DLParser_Unknown_Skip1(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode %02X, skip 1", (word0>>24));
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
	g_dwPCStack[g_dwPCindex].addr += 8;
}

void DLParser_Unknown_Skip2(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode %02X, skip 2", (word0>>24));
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
	g_dwPCStack[g_dwPCindex].addr += 16;
}

void DLParser_Unknown_Skip3(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode %02X, skip 3", (word0>>24));
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
	g_dwPCStack[g_dwPCindex].addr += 24;
}

void DLParser_Unknown_Skip4(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode %02X, skip 4", (word0>>24));
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x", dwPC, word0, word1);
	dwPC+=8;
	word0 = *(uint32 *)(g_pRDRAMu8 + dwPC);
	word1 = *(uint32 *)(g_pRDRAMu8 + dwPC+4);
	LOG_DL("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
	g_dwPCStack[g_dwPCindex].addr += 32;
}

void DLParser_Ucode8_0x05(uint32 word0, uint32 word1)
{
	// Be careful, 0x05 is variable length ucode
	/*
	0028E4E0: 05020088, 04D0000F - Reserved1
	0028E4E8: 6BDC0306, 00000000 - G_NOTHING
	0028E4F0: 05010130, 01B0000F - Reserved1
	0028E4F8: 918A01CA, 1EC5FF3B - G_NOTHING
	0028E500: 05088C68, F5021809 - Reserved1
	0028E508: 04000405, 00000000 - RSP_VTX
	0028E510: 102ECE60, 202F2AA0 - G_NOTHING
	0028E518: 05088C90, F5021609 - Reserved1
	0028E520: 04050405, F0F0F0F0 - RSP_VTX
	0028E528: 102ED0C0, 202F2D00 - G_NOTHING
	0028E530: B5000000, 00000000 - RSP_LINE3D
	0028E538: 8028E640, 8028E430 - G_NOTHING
	0028E540: 00000000, 00000000 - RSP_SPNOOP
	*/

	if( word1 == 0 )
	{
		return;
	}
	else
	{
		DLParser_Unknown_Skip4(word0, word1);
	}
}

void DLParser_Ucode8_0xb4(uint32 word0, uint32 word1)
{
	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;

	if( (word0&0xFF) == 0x06 )
		DLParser_Unknown_Skip3(word0, word1);
	else if( (word0&0xFF) == 0x04 )
		DLParser_Unknown_Skip1(word0, word1);
	else if( (word0&0xFFF) == 0x600 )
		DLParser_Unknown_Skip3(word0, word1);
	else
	{
#ifdef _DEBUG
		if( pauseAtNext )
			DebuggerAppendMsg("ucode 0xb4 at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);
#endif
		DLParser_Unknown_Skip3(word0, word1);
	}
}

void DLParser_Ucode8_0xb5(uint32 word0, uint32 word1)
{
	DLParser_Ucode8_EndDL(word0, word1);	// Check me
	return;


	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	LOG_DL("ucode 0xB5 at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);

	uint32 dwCmd2, dwCmd3;
	dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC+8);
	dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwPC+12);
	LOG_DL("		: 0x%08x 0x%08x\n", dwCmd2, dwCmd3);


	if( GSBlkAddrSaves[g_dwPCindex][0] == 0 || GSBlkAddrSaves[g_dwPCindex][1] == 0 )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, no next blk\n", dwPC, word1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(word0, word1);	// Check me
		return;
	}

	if( ((dwCmd2>>24)!=0x80 && (dwCmd2>>24)!=0x00 ) || ((dwCmd3>>24)!=0x80 && (dwCmd3>>24)!=0x00 ) )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, Unknown\n", dwPC, word1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(word0, word1);	// Check me
		return;
	}

	if( (dwCmd2>>24)!= (dwCmd3>>24) )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, Unknown\n", dwPC, word1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(word0, word1);	// Check me
		return;
	}


	if( (dwCmd2>>24)==0x80 && (dwCmd3>>24)==0x80 )
	{
		if( dwCmd2 < dwCmd3  )
		{
			// All right, the next block is not ucode, but data
#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_DLIST)
			{
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, word1, dwCmd2, dwCmd3);
			}
#endif
			DLParser_Ucode8_EndDL(word0, word1);	// Check me
			return;
		}

		uint32 dwCmd4 = *(uint32 *)(g_pRDRAMu8 + (dwCmd2&0x00FFFFFF));
		uint32 dwCmd5 = *(uint32 *)(g_pRDRAMu8 + (dwCmd2&0x00FFFFFF)+4);
		uint32 dwCmd6 = *(uint32 *)(g_pRDRAMu8 + (dwCmd3&0x00FFFFFF));
		uint32 dwCmd7 = *(uint32 *)(g_pRDRAMu8 + (dwCmd3&0x00FFFFFF)+4);
		if( (dwCmd4>>24) != 0x80 || (dwCmd5>>24) != 0x80 || (dwCmd6>>24) != 0x80 || (dwCmd7>>24) != 0x80 || dwCmd4 < dwCmd5 || dwCmd6 < dwCmd7 )
		{
			// All right, the next block is not ucode, but data
#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_DLIST)
			{
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, word1, dwCmd2, dwCmd3);
				DebuggerAppendMsg("%08X, %08X     %08X,%08X\n", dwCmd4, dwCmd5, dwCmd6, dwCmd7);
			}
#endif
			DLParser_Ucode8_EndDL(word0, word1);	// Check me
			return;
		}

		g_dwPCStack[g_dwPCindex].addr += 8;
		DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, continue\n", dwPC, word1, dwCmd2, dwCmd3);
			);
		return;
	}
	else if( (dwCmd2>>24)==0x00 && (dwCmd3>>24)==0x00 )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, word1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(word0, word1);	// Check me
		return;
	}
	else if( (dwCmd2>>24)==0x00 && (dwCmd3>>24)==0x00 )
	{
		dwCmd2 = *(uint32 *)(g_pRDRAMu8 + dwPC+16);
		dwCmd3 = *(uint32 *)(g_pRDRAMu8 + dwPC+20);
		if( (dwCmd2>>24)==0x80 && (dwCmd3>>24)==0x80 && dwCmd2 < dwCmd3 )
		{
			// All right, the next block is not ucode, but data
#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_DLIST)
			{
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, word1, dwCmd2, dwCmd3);
			}
#endif
			DLParser_Ucode8_EndDL(word0, word1);	// Check me
			return;
		}
		else
		{
			g_dwPCStack[g_dwPCindex].addr += 8;
			DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, continue\n", dwPC, word1, dwCmd2, dwCmd3)
				);
			return;
		}
	}

	uint32 dwAddr1 = RSPSegmentAddr(dwCmd2);
	uint32 dwAddr2 = RSPSegmentAddr(dwCmd3);

#ifdef _DEBUG
	if( word1 != 0 )
	{
		DebuggerAppendMsg("!!!! PC=%08X: 0xB5 - %08X : %08X, %08X\n", dwPC, word1, dwCmd2, dwCmd3);
	}
#endif

	DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
		DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, continue\n", dwPC, word1, dwAddr1, dwAddr2)
		);

	return;
}

void DLParser_Ucode8_0xbc(uint32 word0, uint32 word1)
{
	if( (word0&0xFFF) == 0x58C )
	{
		DLParser_Ucode8_DL(word0, word1);
	}
	else
	{
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		LOG_DL("ucode 0xBC at PC=%08X: 0x%08x 0x%08x\n", dwPC, word0, word1);
	}
}

void DLParser_Ucode8_0xbd(uint32 word0, uint32 word1)
{
	/*
	00359A68: BD000000, DB5B0077 - RSP_POPMTX
	00359A70: C8C0A000, 00240024 - RDP_TriFill
	00359A78: 01000100, 00000000 - RSP_MTX
	00359A80: BD000501, DB5B0077 - RSP_POPMTX
	00359A88: C8C0A000, 00240024 - RDP_TriFill
	00359A90: 01000100, 00000000 - RSP_MTX
	00359A98: BD000A02, DB5B0077 - RSP_POPMTX
	00359AA0: C8C0A000, 00240024 - RDP_TriFill
	00359AA8: 01000100, 00000000 - RSP_MTX
	00359AB0: BD000F04, EB6F0087 - RSP_POPMTX
	00359AB8: C8C0A000, 00280028 - RDP_TriFill
	00359AC0: 01000100, 00000000 - RSP_MTX
	00359AC8: BD001403, DB5B0077 - RSP_POPMTX
	00359AD0: C8C0A000, 00240024 - RDP_TriFill
	00359AD8: 01000100, 00000000 - RSP_MTX
	00359AE0: B5000000, 00000000 - RSP_LINE3D
	00359AE8: 1A000000, 16000200 - G_NOTHING
	 */

	if( word1 != 0 )
	{
		DLParser_Unknown_Skip2(word0, word1);
		return;
	}

	uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;
	LOG_DL("ucode 0xbd at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, word0, word1);
}

void DLParser_Ucode8_0xbf(uint32 word0, uint32 word1)
{
	if( (word0&0xFF) == 0x02 )
		DLParser_Unknown_Skip3(word0, word1);
	else
		DLParser_Unknown_Skip1(word0, word1);
}

void DLParser_RDPHalf_1_0xb4_GoldenEye(uint32 word0, uint32 word1)		
{
	SP_Timing(RSP_GBI1_RDPHalf_1);
	if( (word1>>24) == 0xce )
	{
		PrepareTextures();
		CRender::g_pRender->SetCombinerAndBlender();

		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

		uint32 dw1 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*0+4);
		uint32 dw2 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*1+4);
		uint32 dw3 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*2+4);
		uint32 dw4 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*3+4);
		uint32 dw5 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*4+4);
		uint32 dw6 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*5+4);
		uint32 dw7 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*6+4);
		uint32 dw8 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*7+4);
		uint32 dw9 = *(uint32 *)(g_pRDRAMu8 + dwPC+8*8+4);

		uint32 r = (dw8>>16)&0xFF;
		uint32 g = (dw8    )&0xFF;
		uint32 b = (dw9>>16)&0xFF;
		uint32 a = (dw9    )&0xFF;
		uint32 color = COLOR_RGBA(r, g, b, a);

		//int x0 = 0;
		//int x1 = gRDP.scissor.right;
		int x0 = gRSP.nVPLeftN;
		int x1 = gRSP.nVPRightN;
		int y0 = int(dw1&0xFFFF)/4;
		int y1 = int(dw1>>16)/4;

		float xscale = g_textures[0].m_pCTexture->m_dwWidth / (float)(x1-x0);
		float yscale = g_textures[0].m_pCTexture->m_dwHeight / (float)(y1-y0);
		float fs0 = (short)(dw3&0xFFFF)/32768.0f*g_textures[0].m_pCTexture->m_dwWidth;
		float ft0 = (short)(dw3>>16)/32768.0f*256;
		CRender::g_pRender->TexRect(x0,y0,x1,y1,0,0,xscale,yscale,true,color);

		g_dwPCStack[g_dwPCindex].addr += 312;

#ifdef _DEBUG
		if( logUcodes)
		{
			LOG_DL("GoldenEye Sky at PC=%08X: 0x%08x 0x%08x", dwPC-8, word0, word1);
			uint32 *ptr = (uint32 *)(g_pRDRAMu8 + dwPC);
			for( int i=0; i<20; i++, dwPC+=16,ptr+=4 )
			{
				LOG_DL("%08X: %08X %08X %08X %08X", dwPC, ptr[0], ptr[1], ptr[2], ptr[3]);
			}
		}
#endif

		DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_FLUSH_TRI, {
			TRACE0("Pause after Golden Sky Drawing\n");
		});
	}
}
