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

DWORD dwConkerVtxZAddr=0;

static void RDP_GFX_DumpVtxInfoDKR(DWORD dwAddress, DWORD dwV0, DWORD dwN);


// BB2k
// DKR
//00229B70: 07020010 000DEFC8 CMD G_DLINMEM  Displaylist at 800DEFC8 (stackp 1, limit 2)
//00229A58: 06000000 800DE520 CMD G_DL  Displaylist at 800DE520 (stackp 1, limit 0)
//00229B90: 07070038 00225850 CMD G_DLINMEM  Displaylist at 80225850 (stackp 1, limit 7)

/* flags to inhibit pushing of the display list (on branch) */
//#define G_DL_PUSH		0x00
//#define G_DL_NOPUSH		0x01


void RDP_GFX_DLInMem(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwLimit = (dwCmd0 >> 16) & 0xFF;
	DWORD dwPush = G_DL_PUSH; //(dwCmd0 >> 16) & 0xFF;
	DWORD dwAddr = 0x00000000 | dwCmd1; //RDPSegAddr(dwCmd1);

	DL_PF("    Address=0x%08x Push: 0x%02x", dwAddr, dwPush);
	
	switch (dwPush)
	{
	case G_DL_PUSH:
		DL_PF("    Pushing DisplayList 0x%08x", dwAddr);
		g_dwPCindex++;
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = dwLimit;

		break;
	case G_DL_NOPUSH:
		DL_PF("    Jumping to DisplayList 0x%08x", dwAddr);
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = dwLimit;
		break;
	}

	DL_PF("");
	DL_PF("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
	DL_PF("#############################################");
}

extern DaedalusMatrix dkrMatrixTransposed;
void DLParser_Mtx_DKR(DWORD dwCmd0, DWORD dwCmd1)
{	
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwCommand = (dwCmd0>>16)&0xFF;
	DWORD dwLength  = (dwCmd0)    &0xFFFF;

	//if( dwAddress == 0 || dwAddress < 0x2000)
	{
		dwAddress = dwCmd1+RDPSegAddr(gRSP.dwDKRMatrixAddr);
	}

	//gRSP.DKRCMatrixIndex = (dwCmd0>>22)&3;
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

	DL_PF("    DKR Loading Mtx: %d, command=%d", index, dwCommand);

	if (dwAddress + 64 > g_dwRamSize)
	{
		TRACE1("Mtx: Address invalid (0x%08x)", dwAddress);
		return;
	}

	// Load matrix from dwAddress
	DaedalusMatrix tempmat;
	DaedalusMatrix &mat = gRSP.DKRMatrixes[index];

	DWORD dwI;
	DWORD dwJ;
	const float fRecip = 1.0f / 65536.0f;
	
	for (dwI = 0; dwI < 4; dwI++) {
		for (dwJ = 0; dwJ < 4; dwJ++) {

			SHORT nDataHi = *(SHORT *)(g_pu8RamBase + ((dwAddress+(dwI<<3)+(dwJ<<1)     )^0x2));
			WORD  nDataLo = *(WORD  *)(g_pu8RamBase + ((dwAddress+(dwI<<3)+(dwJ<<1) + 32)^0x2));

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
		DebuggerAppendMsg("DKR Matrix: %08X-%08X\n\tMatrix:%d Address 0x%08x", dwCmd0, dwCmd1, gRSP.DKRCMatrixIndex, dwAddress);
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
		CGraphicsContext::Get()->SwapBuffer();
	}
	else
	{
		if( (pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) ) && logTriDetailsWithVertexMtx ) 
		{
			TRACE0("DKR Matrix\n");
		}
	}
#endif
}

void DLParser_Vtx_DKR(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwV0 =  gRSP.DKRVtxCount;
	DWORD dwN  = ((dwCmd0 >>19 )&0x1F)+1;

	DL_PF("    Address 0x%08x, v0: %d, Num: %d", dwAddress, dwV0, dwN);
	DEBUGGER_ONLY_IF( (pauseAtNext && (eventToPause==NEXT_VERTEX_CMD||eventToPause==NEXT_MATRIX_CMD)), {DebuggerAppendMsg("DKR Vtx: Cmd0=%08X, Cmd1=%08X", dwCmd0, dwCmd1);});

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) ) && (logTriDetails||logTriDetailsWithVertexMtx) ) 
	{
		DebuggerAppendMsg("Vtx_DKR, cmd0=%08X cmd1=%08X", dwCmd0, dwCmd1);
	}
#endif

	if (dwV0 >= 32)
		dwV0 = 31;
	
	if ((dwV0 + dwN) > 32)
	{
		TRACE0("Warning, attempting to load into invalid vertex positions");
		dwN = 32 - dwV0;
	}

	
	//if( dwAddress == 0 || dwAddress < 0x2000)
	{
		dwAddress = dwCmd1+RDPSegAddr(gRSP.dwDKRVtxAddr);
	}

	// Check that address is valid...
	if ((dwAddress + (dwN*16)) > g_dwRamSize)
	{
		TRACE1("SetNewVertexInfo: Address out of range (0x%08x)", dwAddress);
	}
	else
	{
		SetNewVertexInfoDKR(dwAddress, dwV0, dwN);

		status.dwNumVertices += dwN;

		RDP_GFX_DumpVtxInfoDKR(dwAddress, dwV0, dwN);
	}
}


void DLParser_Vtx_Gemini(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwV0 =  ((dwCmd0>>9)&0x1F);
	DWORD dwN  = ((dwCmd0 >>19 )&0x1F);

	DL_PF("    Address 0x%08x, v0: %d, Num: %d", dwAddress, dwV0, dwN);
	DEBUGGER_ONLY_IF( (pauseAtNext && (eventToPause==NEXT_VERTEX_CMD||eventToPause==NEXT_MATRIX_CMD)), {DebuggerAppendMsg("DKR Vtx: Cmd0=%08X, Cmd1=%08X", dwCmd0, dwCmd1);});

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) ) && (logTriDetails||logTriDetailsWithVertexMtx) ) 
	{
		DebuggerAppendMsg("Vtx_DKR, cmd0=%08X cmd1=%08X", dwCmd0, dwCmd1);
	}
#endif

	if (dwV0 >= 32)
		dwV0 = 31;

	if ((dwV0 + dwN) > 32)
	{
		TRACE0("Warning, attempting to load into invalid vertex positions");
		dwN = 32 - dwV0;
	}


	//if( dwAddress == 0 || dwAddress < 0x2000)
	{
		dwAddress = dwCmd1+RDPSegAddr(gRSP.dwDKRVtxAddr);
	}

	// Check that address is valid...
	if ((dwAddress + (dwN*16)) > g_dwRamSize)
	{
		TRACE1("SetNewVertexInfo: Address out of range (0x%08x)", dwAddress);
	}
	else
	{
		SetNewVertexInfoDKR(dwAddress, dwV0, dwN);

		status.dwNumVertices += dwN;

		RDP_GFX_DumpVtxInfoDKR(dwAddress, dwV0, dwN);
	}
}

// DKR verts are extra 4 bytes
void RDP_GFX_DumpVtxInfoDKR(DWORD dwAddress, DWORD dwV0, DWORD dwN)
{
#ifdef _DEBUG
		DWORD dwV;
		LONG i;

		s16 * psSrc = (s16 *)(g_pu8RamBase + dwAddress);

		i = 0;
		for (dwV = dwV0; dwV < dwV0 + dwN; dwV++)
		{
			float x = (float)psSrc[(i + 0) ^ 1];
			float y = (float)psSrc[(i + 1) ^ 1];
			float z = (float)psSrc[(i + 2) ^ 1];

			//WORD wFlags = CDaedalusRender::g_pRender->m_dwVecFlags[dwV]; //(WORD)psSrc[3^0x1];

			WORD wA = psSrc[(i + 3) ^ 1];
			WORD wB = psSrc[(i + 4) ^ 1];

			BYTE a = wA>>8;
			BYTE b = (BYTE)wA;
			BYTE c = wB>>8;
			BYTE d = (BYTE)wB;

			D3DXVECTOR4 & t = g_vecProjected[dwV];


			DL_PF(" #%02d Pos: {% 6f,% 6f,% 6f} Extra: %02x %02x %02x %02x (transf: {% 6f,% 6f,% 6f})",
				dwV, x, y, z, a, b, c, d, t.x, t.y, t.z );

			i+=5;
		}


		u16 * pwSrc = (u16 *)(g_pu8RamBase + dwAddress);
		i = 0;
		for (dwV = dwV0; dwV < dwV0 + dwN; dwV++)
		{
			DL_PF(" #%02d %04x %04x %04x %04x %04x",
				dwV, pwSrc[(i + 0) ^ 1],
				pwSrc[(i + 1) ^ 1],
				pwSrc[(i + 2) ^ 1],
				pwSrc[(i + 3) ^ 1],
				pwSrc[(i + 4) ^ 1]);

			i += 5;
		}

#endif // _DEBUG
}

void DLParser_Set_Addr_Ucode6(DWORD dwCmd0, DWORD dwCmd1)
{
	gRSP.dwDKRMatrixAddr = dwCmd0&0x00FFFFFF;
	gRSP.dwDKRVtxAddr = dwCmd1&0x00FFFFFF;
	gRSP.DKRVtxCount=0;
}



void DLParser_Vtx_WRUS(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwLength = (dwCmd0)&0xFFFF;

	DWORD dwN= (dwLength + 1) / 0x210;
	//DWORD dwV0 = ((dwCmd0>>16)&0x3f)/5;
	DWORD dwV0 = ((dwCmd0>>16)&0xFF)/5;

	DL_PF("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", dwAddress, dwV0, dwN, dwLength);

	if (dwV0 >= 32)
		dwV0 = 31;
	
	if ((dwV0 + dwN) > 32)
	{
		TRACE0("Warning, attempting to load into invalid vertex positions");
		dwN = 32 - dwV0;
	}

	SetNewVertexInfo(dwAddress, dwV0, dwN);

	status.dwNumVertices += dwN;

	RDP_DumpVtxInfo(dwAddress, dwV0, dwN);
}



void DLParser_DL_In_MEM_DKR(DWORD dwCmd0, DWORD dwCmd1)
{
	// This cmd is likely to execute number of ucode at the given address
	DWORD dwAddr = dwCmd1;//RDPSegAddr(dwCmd1);
	{
		g_dwPCindex++;
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = ((dwCmd0>>16)&0xFF);
	}
}


extern uObjMtxReal gObjMtxReal;
void DLParser_Bomberman2TextRect(DWORD dwCmd0, DWORD dwCmd1)
{
	// Bomberman 64 - The Second Attack! (U) [!]
	// The 0x02 cmd, list a TexRect cmd

	u32 dwAddress = RDPSegAddr(dwCmd1);
	uObjSprite *info = (uObjSprite*)(g_pu8RamBase+dwAddress);

	u32 dwTile	= gRSP.curTile;

	SetupTextures();
	
	//CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();

	uObjTxSprite drawinfo;
	memcpy( &(drawinfo.sprite), info, sizeof(uObjSprite));
	CDaedalusRender::g_pRender->DrawSpriteR(drawinfo, false, dwTile, 0, 0, drawinfo.sprite.imageW/32, drawinfo.sprite.imageH/32);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI)),
		{
			DebuggerAppendMsg("Bomberman 64 - TextRect: Addr=%08X\n", dwAddress);
			dwAddress &= (g_dwRamSize-1);
			DebuggerAppendMsg("%08X-%08X-%08X-%08X-%08X-%08X\n", RDRAM_UWORD(dwAddress), RDRAM_UWORD(dwAddress+4),
				RDRAM_UWORD(dwAddress+8), RDRAM_UWORD(dwAddress+12), RDRAM_UWORD(dwAddress+16), RDRAM_UWORD(dwAddress+20) );
		}
	);
}

void DLParser_OgreBatter64BG(DWORD dwCmd0, DWORD dwCmd1)
{
	u32 dwAddress = RDPSegAddr(dwCmd1);
	uObjTxSprite *ptr = (uObjTxSprite*)(g_pu8RamBase+dwAddress);
	CDaedalusRender::g_pRender->LoadObjSprite(*ptr,true);
	//SetupTextures();

	CTexture *ptexture = g_textures[0].m_pCTexture;
	TexRectToN64FrameBuffer_16b( (u32)gObjMtxReal.X, (u32)gObjMtxReal.Y, ptexture->m_dwWidth, ptexture->m_dwHeight, gRSP.curTile);

#ifdef _DEBUG
	CDaedalusRender::g_pRender->DrawSpriteR(*ptr, false);

	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((pauseAtNext && (eventToPause==NEXT_OBJ_TXT_CMD|| eventToPause==NEXT_FLUSH_TRI)),
		{
			DebuggerAppendMsg("OgreBatter 64 BG: Addr=%08X\n", dwAddress);
		}
	);
#endif
}

void DLParser_MoveWord_DKR(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_MoveWord);
	DWORD dwNumLights;

	switch (dwCmd0 & 0xFF)
	{
	case G_MW_NUMLIGHT:
		dwNumLights = dwCmd1&0x7;
		DL_PF("    G_MW_NUMLIGHT: Val:%d", dwNumLights);

		gRSP.ambientLightIndex = dwNumLights;
		SetNumLights(dwNumLights);
		if( dwCmd1&0x7 )
		{
			gRSP.DKRVtxAddBase = true;
		}
		else
		{
			gRSP.DKRVtxAddBase = false;
		}
		DL_PF("    gRSP.DKRVtxAddBase = %d", gRSP.DKRVtxAddBase);
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_MATRIX_CMD, {DebuggerAppendMsg("DKR Moveword, select gRSP.DKRVtxAddBase %s, cmd0=%08X, cmd1=%08X", gRSP.DKRVtxAddBase?"true":"false", dwCmd0, dwCmd1);});
		break;
	case G_MW_LIGHTCOL:
		gRSP.DKRCMatrixIndex = (dwCmd1>>6)&7;
		DL_PF("    gRSP.DKRCMatrixIndex = %d", gRSP.DKRCMatrixIndex);
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_MATRIX_CMD, {DebuggerAppendMsg("DKR Moveword, select matrix %d, cmd0=%08X, cmd1=%08X", gRSP.DKRCMatrixIndex, dwCmd0, dwCmd1);});
		break;
	default:
		DLParser_GBI1_MoveWord(dwCmd0, dwCmd1);
		break;
	}

}


void DLParser_DMA_Tri_DKR(DWORD dwCmd0, DWORD dwCmd1)
{
//DmaTri: 0x05710080 0x0021a2a0
//DmaTri: 0x05e100f0 0x0021a100
//DmaTri: 0x05a100b0 0x0021a1f0
//00229BE0: 05710080 001E4CB0 CMD G_DMATRI  Triangles 9 at 801E4CB0
/*
00229CB8: 059100A0 002381A0 CMD G_DMATRI  Triangles 11 at 802381A0
tri[ 0 1 2 ] (0,1,2)
tri[ 2 3 0 ] (2,3,0)
tri[ 4 0 5 ] (4,0,5)
tri[ 5 6 4 ] (5,6,4)
tri[ 6 1 4 ] (6,1,4)
tri[ 7 8 9 ] (7,8,9)
tri[ 7 9 10 ] (7,9,10)
tri[ 10 11 7 ] (10,11,7)
tri[ 11 12 13 ] (11,12,13)
tri[ 13 8 11 ] (13,8,11)
tri[ 17 16 2 ] (17,16,2)

0x0022cda0: 00000102
0x0022cda4: 00000000
0x0022cda8: 00000000
0x0022cdac: 00000000
0x0022cdb0: 00020300
0x0022cdb4: 00000000
0x0022cdb8: 00000000
0x0022cdbc: 00000000
0x0022cdc0: 00040005

*/

	BOOL bTrisAdded = FALSE;
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD flag = (dwCmd0 & 0xFF0000) >> 16;
	if (flag&1) 
		CDaedalusRender::g_pRender->SetCullMode(false,true);
	else
		CDaedalusRender::g_pRender->SetCullMode(false,false);


	DWORD dwNum = ((dwCmd0 &  0xFFF0) >>4 );
	DWORD i;
	DWORD * pData = &g_pu32RamBase[dwAddress/4];

	if( dwAddress+16*dwNum >= g_dwRamSize )
	{
		TRACE0("DMATRI invalid memory pointer");
		return;
	}

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) ) && (logTriDetails||logTriDetailsWithVertexMtx) ) 
	{
		DebuggerAppendMsg("DMATRI, addr=%08X, Cmd0=%08X\n", dwAddress, dwCmd0);
	}
#endif

	status.primitiveType = PRIM_DMA_TRI;

	for (i = 0; i < dwNum; i++)
	{
		DL_PF("    0x%08x: %08x %08x %08x %08x", dwAddress + i*16,
			pData[0], pData[1], pData[2], pData[3]);

		DWORD dwInfo = pData[0];

		DWORD dwV0 = (dwInfo >> 16) & 0x1F;
		DWORD dwV1 = (dwInfo >>  8) & 0x1F;
		DWORD dwV2 = (dwInfo      ) & 0x1F;

#ifdef _DEBUG
		if( (pauseAtNext && (eventToPause==NEXT_MATRIX_CMD|| eventToPause==NEXT_FLUSH_TRI || eventToPause==NEXT_VERTEX_CMD) ) && (logTriDetails||logTriDetailsWithVertexMtx) ) 
		{
			DebuggerAppendMsg("DMATRI: %d, %d, %d (%08X-%08X)", dwV0,dwV1,dwV2,dwCmd0,dwCmd1);
		}
#endif

		//if (TestTri(dwV0, dwV1, dwV2))
		{
			DEBUG_DUMP_VERTEXES("DmaTri", dwV0, dwV1, dwV2);
			DL_PF("   Tri: %d,%d,%d", dwV0, dwV1, dwV2);
			if (!bTrisAdded )//&& CDaedalusRender::g_pRender->IsTextureEnabled())
			{
				SetupTextures();
				InitVertexTextureConstants();
			}

			// Generate texture coordinates
			s16 s0 = ((s16)(pData[1]>>16));
			s16 t0 = ((s16)(pData[1]&0xFFFF));
			s16 s1 = ((s16)(pData[2]>>16));
			s16 t1 = ((s16)(pData[2]&0xFFFF));
			s16 s2 = ((s16)(pData[3]>>16));
			s16 t2 = ((s16)(pData[3]&0xFFFF));

#ifdef _DEBUG
			if( (pauseAtNext && (eventToPause==NEXT_MATRIX_CMD|| eventToPause==NEXT_FLUSH_TRI || eventToPause==NEXT_VERTEX_CMD) ) && (logTriDetails||logTriDetailsWithVertexMtx) ) 
			{
				DebuggerAppendMsg(" (%d,%d), (%d,%d), (%d,%d)",s0,t0,s1,t1,s2,t2);
				DebuggerAppendMsg(" (%08X), (%08X), (%08X), (%08X)",pData[0],pData[1],pData[2],pData[3]);
			}
#endif
			CDaedalusRender::g_pRender->SetVtxTextureCoord(dwV0, s0, t0);
			CDaedalusRender::g_pRender->SetVtxTextureCoord(dwV1, s1, t1);
			CDaedalusRender::g_pRender->SetVtxTextureCoord(dwV2, s2, t2);

			if( !bTrisAdded )
			{
				CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();
			}

			bTrisAdded = TRUE;
			AddTri(dwV0, dwV1, dwV2);
		}

		pData += 4;

	}

	if (bTrisAdded)	
	{
		CDaedalusRender::g_pRender->FlushTris();
	}
	gRSP.DKRVtxCount=0;
}

DWORD dwPDCIAddr = 0;
void SetNewVertexInfoPD(DWORD dwAddress, DWORD dwV0, DWORD dwNum);
void DLParser_Vtx_PD(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI0_Vtx);

	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwV0 =  (dwCmd0>>16)&0x0F;
	DWORD dwN  = ((dwCmd0>>20)&0x0F)+1;
	DWORD dwLength = (dwCmd0)&0xFFFF;

	DL_PF("    Address 0x%08x, v0: %d, Num: %d", dwAddress, dwV0, dwN);

	SetNewVertexInfoPD(dwAddress, dwV0, dwN);
	status.dwNumVertices += dwN;
}

void DLParser_Set_Vtx_CI_PD(DWORD dwCmd0, DWORD dwCmd1)
{
	// Color index buf address
	dwPDCIAddr = RDPSegAddr(dwCmd1);
}

void DLParser_Tri4_PD(DWORD dwCmd0, DWORD dwCmd1)
{
	status.primitiveType = PRIM_TRI2;

	// While the next command pair is Tri2, add vertices
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	do {
		DWORD dwFlag = (dwCmd0>>16)&0xFF;
		DL_PF("    PD Tri4: 0x%08x 0x%08x Flag: 0x%02x", dwCmd0, dwCmd1, dwFlag);

		BOOL bVisible;
		for( u32 i=0; i<4; i++)
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
}


void DLParser_Tri4_Conker(DWORD dwCmd0, DWORD dwCmd1)
{
	status.primitiveType = PRIM_TRI2;

	// While the next command pair is Tri2, add vertices
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;

	BOOL bTrisAdded = FALSE;

	do {
		DL_PF("    Conker Tri4: 0x%08x 0x%08x", dwCmd0, dwCmd1);
		u32 idx[12];
		idx[0] = (dwCmd1   )&0x1F;
		idx[1] = (dwCmd1>> 5)&0x1F;
		idx[2] = (dwCmd1>>10)&0x1F;
		idx[3] = (dwCmd1>>15)&0x1F;
		idx[4] = (dwCmd1>>20)&0x1F;
		idx[5] = (dwCmd1>>25)&0x1F;

		idx[6] = (dwCmd0    )&0x1F;
		idx[7] = (dwCmd0>> 5)&0x1F;
		idx[8] = (dwCmd0>>10)&0x1F;

		idx[ 9] = (((dwCmd0>>15)&0x7)<<2)|(dwCmd1>>30);
		idx[10] = (dwCmd0>>18)&0x1F;
		idx[11] = (dwCmd0>>23)&0x1F;

		BOOL bVisible;
		for( u32 i=0; i<4; i++)
		{
			u32 v0=idx[i*3  ];
			u32 v1=idx[i*3+1];
			u32 v2=idx[i*3+2];
			bVisible = TestTri(v0, v1, v2);
			DL_PF("       (%d, %d, %d) %s", v0, v1, v2, bVisible ? "": "(clipped)");
			if (bVisible)
			{
				DEBUG_DUMP_VERTEXES("Tri4 Conker:", v0, v1, v2);
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
				AddTri(v0, v1, v2);
			}
		}

		dwCmd0			= *(DWORD *)(g_pu8RamBase + dwPC+0);
		dwCmd1			= *(DWORD *)(g_pu8RamBase + dwPC+4);
		dwPC += 8;

#ifdef _DEBUG
	} while (!(pauseAtNext && eventToPause==NEXT_TRIANGLE) && (dwCmd0>>28) == 1 );
	if(pauseAtNext && eventToPause==NEXT_TRIANGLE )
	{
		eventToPause = NEXT_FLUSH_TRI;
	}

#else
	} while ((dwCmd0>>28) == 1);
#endif

	g_dwPCStack[g_dwPCindex].addr = dwPC-8;

	if (bTrisAdded)	
	{
		CDaedalusRender::g_pRender->FlushTris();
	}
}

void RDP_GFX_Force_Vertex_Z_Conker(DWORD dwAddress)
{
#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
	{
		s8 * pcBase = g_ps8RamBase + (dwAddress&(g_dwRamSize-1));
		DWORD * pdwBase = (DWORD *)pcBase;
		LONG i;

		for (i = 0; i < 4; i++)
		{
			DebuggerAppendMsg("    %08x %08x %08x %08x", pdwBase[0], pdwBase[1], pdwBase[2], pdwBase[3]);
			pdwBase+=4;
		}
	}
#endif

	dwConkerVtxZAddr = dwAddress;
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at RDP_GFX_Force_Matrix_Conker Cmd");});
}



void DLParser_MoveMem_Conker(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwType    = (dwCmd0     ) & 0xFE;
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	if( dwType == F3DEX_GBI_2X_G_MV_MATRIX )
	{
		DL_PF("    DLParser_MoveMem_Conker");
		RDP_GFX_Force_Vertex_Z_Conker(dwAddress);
	}
	else if( dwType == F3DEX_GBI_2X_G_MV_LIGHT )
	{
		DL_PF("    MoveMem Light Conker");
		DWORD dwOffset2 = (dwCmd0 >> 5) & 0x3FFF;
		DWORD dwLight=0xFF;
		if( dwOffset2 >= 0x30 )
		{
			dwLight = (dwOffset2 - 0x30)/0x30;
			DL_PF("    Light %d:", dwLight);
			RDP_MoveMemLight(dwLight, dwAddress);
		}
		else
		{
			// fix me
			//TRACE0("Check me in DLParser_MoveMem_Conker - MoveMem Light");
		}
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_SET_LIGHT, 
		{
			DebuggerAppendMsg("RDP_MoveMemLight: %d, Addr=%08X, cmd0=%08X", dwLight, dwAddress, dwCmd0);
			TRACE0("Pause after MoveMemLight");
		});
	}
	else
	{
		DLParser_GBI2_MoveMem(dwCmd0,dwCmd1);
	}
}

extern void SetNewVertexInfoConker(DWORD dwAddress, DWORD dwV0, DWORD dwNum);
void DLParser_Vtx_Conker(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	DWORD dwVEnd    = ((dwCmd0   )&0xFFF)/2;
	DWORD dwN      = ((dwCmd0>>12)&0xFFF);
	DWORD dwV0		= dwVEnd - dwN;

	DL_PF("    Vtx: Address 0x%08x, vEnd: %d, v0: %d, Num: %d", dwAddress, dwVEnd, dwV0, dwN);

	SetNewVertexInfoConker(dwAddress, dwV0, dwN);
	status.dwNumVertices += dwN;
	RDP_DumpVtxInfo(dwAddress, dwV0, dwN);
}


void DLParser_MoveWord_Conker(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwType   = (dwCmd0 >> 16) & 0xFF;
	if( dwType != G_MW_NUMLIGHT )
	{
		DLParser_GBI2_MoveWord(dwCmd0, dwCmd1);
	}
	else
	{
		DWORD dwNumLights = (dwCmd1/48);
		DL_PF("Conker G_MW_NUMLIGHT: %d", dwNumLights);
		gRSP.ambientLightIndex = dwNumLights+1;
		SetNumLights(dwNumLights);
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_SET_LIGHT, 
		{
			DebuggerAppendMsg("SetNumLights: %d", dwNumLights);
			TRACE0("Pause after SetNumLights");
		});
	}
}

void DLParser_Mtx_SiliconValley(DWORD dwCmd0, DWORD dwCmd1)
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
		if( nLoadCommand == RENDER_MUL_MATRIX && bPush == FALSE )
		{
			//nLoadCommand = RENDER_LOAD_MATRIX;
			//TRACE0("Matrix mul and nopush ==> load");
		}
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
		if( !(dwCommand & G_MTX_PROJECTION ) && nLoadCommand == RENDER_MUL_MATRIX && bPush == FALSE )
		{
			DebuggerAppendMsg("Mul&Nopush: %08X, %08X", dwCmd0, dwCmd1 );
		}
		pauseAtNext = false;
		debuggerPause = true;
		if (dwCommand & G_MTX_PROJECTION)
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
		if( (pauseAtNext && (eventToPause==NEXT_TRIANGLE|| eventToPause==NEXT_FLUSH_TRI) ) && logTriDetailsWithVertexMtx ) 
		{
			if (dwCommand & G_MTX_PROJECTION)
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

void DLParser_Ucode8_0x0(DWORD dwCmd0, DWORD dwCmd1)
{
	/*
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC);

	if( (dwCmd2&0xFF000000) == 0x00000000 )
	{
		DL_PF("ucode 0x00 at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);
		RDP_GFX_PopDL();
	}
	*/
}


DWORD Rogue_Squadron_Vtx_XYZ_Cmd;
DWORD Rogue_Squadron_Vtx_XYZ_Addr;
DWORD Rogue_Squadron_Vtx_Color_Cmd;
DWORD Rogue_Squadron_Vtx_Color_Addr;
DWORD GSBlkAddrSaves[100][2];

void SetNewVertexInfo_Rogue_Squadron(DWORD dwXYZAddr, DWORD dwColorAddr, DWORD dwXYZCmd, DWORD dwColorCmd);

void DLParser_RS_Color_Buffer(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DWORD dwAddr = RDPSegAddr(dwCmd1);

	if( dwAddr > g_dwRamSize )
	{
		TRACE0("DL, addr is wrong");
		dwAddr = dwCmd1&(g_dwRamSize-1);
	}

	Rogue_Squadron_Vtx_Color_Cmd = dwCmd0;
	Rogue_Squadron_Vtx_Color_Addr = dwAddr;

	DL_PF("Vtx_Color at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);
#ifdef _DEBUG
	if( pauseAtNext && (eventToPause == NEXT_VERTEX_CMD ) )
	{
		DebuggerAppendMsg("Vtx_Color at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);
		if( dwAddr < g_dwRamSize )
		{
			DumpHex(dwAddr, min(64, g_dwRamSize-dwAddr));
		}
	}
#endif

	SetNewVertexInfo_Rogue_Squadron(Rogue_Squadron_Vtx_XYZ_Addr, Rogue_Squadron_Vtx_Color_Addr, Rogue_Squadron_Vtx_XYZ_Cmd, Rogue_Squadron_Vtx_Color_Cmd);

}


void DLParser_RS_Vtx_Buffer(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DWORD dwAddr = RDPSegAddr(dwCmd1);
	if( dwAddr > g_dwRamSize )
	{
		TRACE0("DL, addr is wrong");
		dwAddr = dwCmd1&(g_dwRamSize-1);
	}

	DL_PF("Vtx_XYZ at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);
	Rogue_Squadron_Vtx_XYZ_Cmd = dwCmd0;
	Rogue_Squadron_Vtx_XYZ_Addr = dwAddr;

#ifdef _DEBUG
	if( pauseAtNext && (eventToPause == NEXT_VERTEX_CMD ) )
	{
		DebuggerAppendMsg("Vtx_XYZ at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);
		if( dwAddr < g_dwRamSize )
		{
			DumpHex(dwAddr, min(64, g_dwRamSize-dwAddr));
		}
	}
#endif
}


void DLParser_RS_Block(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode 0x80 at PC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
}

void DLParser_RS_MoveMem(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;
	DWORD cmd1 = ((dwPC)&0x00FFFFFF)|0x80000000;
	DLParser_GBI1_MoveMem(dwCmd0, cmd1);
	/*
	DL_PF("RS_MoveMem", (dwCmd0>>24));
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd2, dwCmd3);
	dwPC+=8;
	DWORD dwCmd4 = *(DWORD *)(g_pu8RamBase + dwPC);
	DWORD dwCmd5 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd4, dwCmd5);
	*/
	g_dwPCStack[g_dwPCindex].addr += 16;

	//DEBUGGER_PAUSE_AND_DUMP(NEXT_SET_MODE_CMD, {
	//	DebuggerAppendMsg("Pause after RS_MoveMem at: %08X\n", dwPC-8);
	//});

}

void DLParser_RS_0xbe(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode %02X, skip 1", (dwCmd0>>24));
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd2, dwCmd3);
	g_dwPCStack[g_dwPCindex].addr += 8;

	DEBUGGER_PAUSE_AND_DUMP(NEXT_SET_MODE_CMD, {
		DebuggerAppendMsg("Pause after RS_0xbe at: %08X\n", dwPC-8);
		DebuggerAppendMsg("\t0x%08x 0x%08x", dwCmd0, dwCmd1);
		DebuggerAppendMsg("\t0x%08x 0x%08x", dwCmd2, dwCmd3);
	});

}


void DLParser_Ucode8_EndDL(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	RDP_GFX_PopDL();
	DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, DebuggerAppendMsg("PC=%08X: EndDL, return to %08X\n\n", dwPC, g_dwPCStack[g_dwPCindex].addr));
}

void DLParser_Ucode8_DL(DWORD dwCmd0, DWORD dwCmd1)	// DL Function Call
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;

	DWORD dwAddr = RDPSegAddr(dwCmd1);
	DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwAddr);
	DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwAddr+4);

	if( dwAddr > g_dwRamSize )
	{
		TRACE0("DL, addr is wrong");
		dwAddr = dwCmd1&(g_dwRamSize-1);
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

void DLParser_Ucode8_JUMP(DWORD dwCmd0, DWORD dwCmd1)	// DL Function Call
{
	if( (dwCmd0&0x00FFFFFF) == 0 )
	{
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DWORD dwAddr = RDPSegAddr(dwCmd1);

		if( dwAddr > g_dwRamSize )
		{
			TRACE0("DL, addr is wrong");
			dwAddr = dwCmd1&(g_dwRamSize-1);
		}

		DWORD dwCmd2 = *(DWORD *)(g_pu8RamBase + dwAddr);
		DWORD dwCmd3 = *(DWORD *)(g_pu8RamBase + dwAddr+4);

		g_dwPCStack[g_dwPCindex].addr = dwAddr+8;		// Jump to new address
		DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
			DebuggerAppendMsg("\nPC=%08X: Jump to Address %08X - %08X, %08X\n\n", dwPC, dwAddr, dwCmd2, dwCmd3)
		);
	}
	else
	{
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DL_PF("ucode 0x07 at PC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
	}
}



void DLParser_Ucode8_Unknown(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode %02X at PC=%08X: 0x%08x 0x%08x\n", (dwCmd0>>24), dwPC, dwCmd0, dwCmd1);
}

void DLParser_Unknown_Skip1(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode %02X, skip 1", (dwCmd0>>24));
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
	g_dwPCStack[g_dwPCindex].addr += 8;
}

void DLParser_Unknown_Skip2(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode %02X, skip 2", (dwCmd0>>24));
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
	g_dwPCStack[g_dwPCindex].addr += 16;
}

void DLParser_Unknown_Skip3(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode %02X, skip 3", (dwCmd0>>24));
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
	g_dwPCStack[g_dwPCindex].addr += 24;
}

void DLParser_Unknown_Skip4(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode %02X, skip 4", (dwCmd0>>24));
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x", dwPC, dwCmd0, dwCmd1);
	dwPC+=8;
	dwCmd0 = *(DWORD *)(g_pu8RamBase + dwPC);
	dwCmd1 = *(DWORD *)(g_pu8RamBase + dwPC+4);
	DL_PF("\tPC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
	g_dwPCStack[g_dwPCindex].addr += 32;
}

void DLParser_Ucode8_0x05(DWORD dwCmd0, DWORD dwCmd1)
{
	// Be careful, 0x05 is variable length ucode
	/*
	0028E4E0: 05020088, 04D0000F - Reserved1
	0028E4E8: 6BDC0306, 00000000 - G_NOTHING
	0028E4F0: 05010130, 01B0000F - Reserved1
	0028E4F8: 918A01CA, 1EC5FF3B - G_NOTHING
	0028E500: 05088C68, F5021809 - Reserved1
	0028E508: 04000405, 00000000 - G_VTX
	0028E510: 102ECE60, 202F2AA0 - G_NOTHING
	0028E518: 05088C90, F5021609 - Reserved1
	0028E520: 04050405, F0F0F0F0 - G_VTX
	0028E528: 102ED0C0, 202F2D00 - G_NOTHING
	0028E530: B5000000, 00000000 - G_LINE3D
	0028E538: 8028E640, 8028E430 - G_NOTHING
	0028E540: 00000000, 00000000 - G_SPNOOP
	*/

	if( dwCmd1 == 0 )
	{
		return;
	}
	else
	{
		DLParser_Unknown_Skip4(dwCmd0, dwCmd1);
	}
}

void DLParser_Ucode8_0xb4(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;

	if( (dwCmd0&0xFF) == 0x06 )
		DLParser_Unknown_Skip3(dwCmd0, dwCmd1);
	else if( (dwCmd0&0xFF) == 0x04 )
		DLParser_Unknown_Skip1(dwCmd0, dwCmd1);
	else if( (dwCmd0&0xFFF) == 0x600 )
		DLParser_Unknown_Skip3(dwCmd0, dwCmd1);
	else
	{
#ifdef _DEBUG
		if( pauseAtNext )
			DebuggerAppendMsg("ucode 0xb4 at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);
#endif
		DLParser_Unknown_Skip3(dwCmd0, dwCmd1);
	}
}

void DLParser_Ucode8_0xb5(DWORD dwCmd0, DWORD dwCmd1)
{
	DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
	return;


	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
	DL_PF("ucode 0xB5 at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);

	DWORD dwCmd2, dwCmd3;
	dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC+8);
	dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+12);
	DL_PF("		: 0x%08x 0x%08x\n", dwCmd2, dwCmd3);


	if( GSBlkAddrSaves[g_dwPCindex][0] == 0 || GSBlkAddrSaves[g_dwPCindex][1] == 0 )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, no next blk\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
		return;
	}

	if( ((dwCmd2>>24)!=0x80 && (dwCmd2>>24)!=0x00 ) || ((dwCmd3>>24)!=0x80 && (dwCmd3>>24)!=0x00 ) )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, Unknown\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
		return;
	}

	if( (dwCmd2>>24)!= (dwCmd3>>24) )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, Unknown\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
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
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
			}
#endif
			DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
			return;
		}

		DWORD dwCmd4 = *(DWORD *)(g_pu8RamBase + (dwCmd2&0x00FFFFFF));
		DWORD dwCmd5 = *(DWORD *)(g_pu8RamBase + (dwCmd2&0x00FFFFFF)+4);
		DWORD dwCmd6 = *(DWORD *)(g_pu8RamBase + (dwCmd3&0x00FFFFFF));
		DWORD dwCmd7 = *(DWORD *)(g_pu8RamBase + (dwCmd3&0x00FFFFFF)+4);
		if( (dwCmd4>>24) != 0x80 || (dwCmd5>>24) != 0x80 || (dwCmd6>>24) != 0x80 || (dwCmd7>>24) != 0x80 || dwCmd4 < dwCmd5 || dwCmd6 < dwCmd7 )
		{
			// All right, the next block is not ucode, but data
#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_DLIST)
			{
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
				DebuggerAppendMsg("%08X, %08X     %08X,%08X\n", dwCmd4, dwCmd5, dwCmd6, dwCmd7);
			}
#endif
			DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
			return;
		}

		g_dwPCStack[g_dwPCindex].addr += 8;
		DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, continue\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
			);
		return;
	}
	else if( (dwCmd2>>24)==0x00 && (dwCmd3>>24)==0x00 )
	{
#ifdef _DEBUG
		if( pauseAtNext && eventToPause == NEXT_DLIST)
		{
			DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
		}
#endif
		DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
		return;
	}
	else if( (dwCmd2>>24)==0x00 && (dwCmd3>>24)==0x00 )
	{
		dwCmd2 = *(DWORD *)(g_pu8RamBase + dwPC+16);
		dwCmd3 = *(DWORD *)(g_pu8RamBase + dwPC+20);
		if( (dwCmd2>>24)==0x80 && (dwCmd3>>24)==0x80 && dwCmd2 < dwCmd3 )
		{
			// All right, the next block is not ucode, but data
#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_DLIST)
			{
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, EndDL, next blk is data\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
			}
#endif
			DLParser_Ucode8_EndDL(dwCmd0, dwCmd1);	// Check me
			return;
		}
		else
		{
			g_dwPCStack[g_dwPCindex].addr += 8;
			DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
				DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, continue\n", dwPC, dwCmd1, dwCmd2, dwCmd3)
				);
			return;
		}
	}

	DWORD dwAddr1 = RDPSegAddr(dwCmd2);
	DWORD dwAddr2 = RDPSegAddr(dwCmd3);

#ifdef _DEBUG
	if( dwCmd1 != 0 )
	{
		DebuggerAppendMsg("!!!! PC=%08X: 0xB5 - %08X : %08X, %08X\n", dwPC, dwCmd1, dwCmd2, dwCmd3);
	}
#endif

	DEBUGGER_PAUSE_AND_DUMP(NEXT_DLIST, 
		DebuggerAppendMsg("PC=%08X: 0xB5 - %08X : %08X, %08X, continue\n", dwPC, dwCmd1, dwAddr1, dwAddr2)
		);

	return;
}

void DLParser_Ucode8_0xbc(DWORD dwCmd0, DWORD dwCmd1)
{
	if( (dwCmd0&0xFFF) == 0x58C )
	{
		DLParser_Ucode8_DL(dwCmd0, dwCmd1);
	}
	else
	{
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DL_PF("ucode 0xBC at PC=%08X: 0x%08x 0x%08x\n", dwPC, dwCmd0, dwCmd1);
	}
}

void DLParser_Ucode8_0xbd(DWORD dwCmd0, DWORD dwCmd1)
{
	/*
	00359A68: BD000000, DB5B0077 - G_POPMTX
	00359A70: C8C0A000, 00240024 - RDP_TriFill
	00359A78: 01000100, 00000000 - G_MTX
	00359A80: BD000501, DB5B0077 - G_POPMTX
	00359A88: C8C0A000, 00240024 - RDP_TriFill
	00359A90: 01000100, 00000000 - G_MTX
	00359A98: BD000A02, DB5B0077 - G_POPMTX
	00359AA0: C8C0A000, 00240024 - RDP_TriFill
	00359AA8: 01000100, 00000000 - G_MTX
	00359AB0: BD000F04, EB6F0087 - G_POPMTX
	00359AB8: C8C0A000, 00280028 - RDP_TriFill
	00359AC0: 01000100, 00000000 - G_MTX
	00359AC8: BD001403, DB5B0077 - G_POPMTX
	00359AD0: C8C0A000, 00240024 - RDP_TriFill
	00359AD8: 01000100, 00000000 - G_MTX
	00359AE0: B5000000, 00000000 - G_LINE3D
	00359AE8: 1A000000, 16000200 - G_NOTHING
	 */

	if( dwCmd1 != 0 )
	{
		DLParser_Unknown_Skip2(dwCmd0, dwCmd1);
		return;
	}

	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;
	DL_PF("ucode 0xbd at PC=%08X: 0x%08x 0x%08x\n", dwPC-8, dwCmd0, dwCmd1);
}

void DLParser_Ucode8_0xbf(DWORD dwCmd0, DWORD dwCmd1)
{
	if( (dwCmd0&0xFF) == 0x02 )
		DLParser_Unknown_Skip3(dwCmd0, dwCmd1);
	else
		DLParser_Unknown_Skip1(dwCmd0, dwCmd1);
}

void DLParser_RDPHalf_1_0xb4_GoldenEye(DWORD dwCmd0, DWORD dwCmd1)		
{
	SP_Timing(DLParser_GBI1_RDPHalf_1);
	if( (dwCmd1>>24) == 0xce )
	{
		SetupTextures();
		CDaedalusRender::g_pRender->InitCombinerAndBlenderMode();

		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

		DWORD dw1 = *(DWORD *)(g_pu8RamBase + dwPC+8*0+4);
		DWORD dw2 = *(DWORD *)(g_pu8RamBase + dwPC+8*1+4);
		DWORD dw3 = *(DWORD *)(g_pu8RamBase + dwPC+8*2+4);
		DWORD dw4 = *(DWORD *)(g_pu8RamBase + dwPC+8*3+4);
		DWORD dw5 = *(DWORD *)(g_pu8RamBase + dwPC+8*4+4);
		DWORD dw6 = *(DWORD *)(g_pu8RamBase + dwPC+8*5+4);
		DWORD dw7 = *(DWORD *)(g_pu8RamBase + dwPC+8*6+4);
		DWORD dw8 = *(DWORD *)(g_pu8RamBase + dwPC+8*7+4);
		DWORD dw9 = *(DWORD *)(g_pu8RamBase + dwPC+8*8+4);

		DWORD r = (dw8>>16)&0xFF;
		DWORD g = (dw8    )&0xFF;
		DWORD b = (dw9>>16)&0xFF;
		DWORD a = (dw9    )&0xFF;
		DWORD color = DAEDALUS_COLOR_RGBA(r, g, b, a);

		int x0 = 0;
		int x1 = gRDP.scissor.right;
		int y0 = int(dw1&0xFFFF)/4;
		int y1 = int(dw1>>16)/4;

		float xscale = g_textures[0].m_pCTexture->m_dwWidth / (float)(x1-x0);
		float yscale = g_textures[0].m_pCTexture->m_dwHeight / (float)(y1-y0);
		float fs0 = (short)(dw3&0xFFFF)/32768.0f*g_textures[0].m_pCTexture->m_dwWidth;
		float ft0 = (short)(dw3>>16)/32768.0f*256;
		CDaedalusRender::g_pRender->TexRect(x0,y0,x1,y1,0,0,xscale,yscale,true,color);

		g_dwPCStack[g_dwPCindex].addr += 312;

#ifdef _DEBUG
		if( dlistDeassemble)
		{
			DL_PF("GoldenEye Sky at PC=%08X: 0x%08x 0x%08x", dwPC-8, dwCmd0, dwCmd1);
			DWORD *ptr = (DWORD *)(g_pu8RamBase + dwPC);
			for( int i=0; i<20; i++, dwPC+=16,ptr+=4 )
			{
				DL_PF("%08X: %08X %08X %08X %08X", dwPC, ptr[0], ptr[1], ptr[2], ptr[3]);
			}
		}
#endif

		DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_FLUSH_TRI, {
			TRACE0("Pause after Golden Sky Drawing\n");
		});
	}
}
