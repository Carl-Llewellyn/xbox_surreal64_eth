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


void DLParser_GBI1_Vtx(DWORD dwCmd0, DWORD dwCmd1)
{
	DWORD dwAddress = RDPSegAddr(dwCmd1);
	//DWORD dwLength = (dwCmd0)&0xFFFF;
	//DWORD dwN= (dwLength + 1) / 0x410;
	//DWORD dwV0 = ((dwCmd0>>16)&0x3f)>>1;

	DWORD dwV0  = (dwCmd0 >>17 ) & 0x7f;		// ==((x>>16)&0xff)/2
	DWORD dwN   = (dwCmd0 >>10 ) & 0x3f;
	DWORD dwLength = (dwCmd0      ) & 0x3ff;

	DL_PF("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", dwAddress, dwV0, dwN, dwLength);

	if (dwAddress > g_dwRamSize)
	{
		TRACE0("     Address out of range - ignoring load");
		return;
	}

	if ((dwV0 + dwN) > 80)
	{
		TRACE5("Warning, invalid vertex positions, N=%d, v0=%d, Addr=0x%08X, Cmd=%08X-%08X",
			dwN, dwV0, dwAddress, dwCmd0, dwCmd1);
		return;
	}

	SetNewVertexInfo(dwAddress, dwV0, dwN);

	status.dwNumVertices += dwN;

	RDP_DumpVtxInfo(dwAddress, dwV0, dwN);
}

void DLParser_GBI1_ModifyVtx(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_ModifyVtx);

	if( gRSP.ucode == 5 && (dwCmd0&0x00FFFFFF) == 0 && (dwCmd1&0xFF000000) == 0x80000000 )
	{
		DLParser_Bomberman2TextRect(dwCmd0, dwCmd1);
	}
	else
	{
		DWORD dwWhere = (dwCmd0 >> 16) & 0xFF;
		DWORD dwVert   = ((dwCmd0      ) & 0xFFFF) / 2;
		DWORD dwValue  = dwCmd1;

		if( dwVert > 80 )
		{
			RDP_NOIMPL("DLParser_GBI1_ModifyVtx: Invalid vertex number: %d", dwVert, 0);
			return;
		}

		// Data for other commands?
		switch (dwWhere)
		{
		case G_MWO_POINT_RGBA:			// Modify RGBA
		case G_MWO_POINT_XYSCREEN:		// Modify X,Y
		case G_MWO_POINT_ZSCREEN:		// Modify C
		case G_MWO_POINT_ST:			// Texture
			ModifyVertexInfo(dwWhere, dwVert, dwValue);
			break;
		default:
			RDP_NOIMPL("DLParser_GBI1_ModifyVtx: Setting unk value: 0x%02x, 0x%08x", dwWhere, dwValue);
			break;
		}
	}
}



/* Mariokart etc*/
void DLParser_GBI1_Tri2(DWORD dwCmd0, DWORD dwCmd1)
{
	status.primitiveType = PRIM_TRI2;
	bool bTrisAdded = false;
	bool textureIsEnabled = CDaedalusRender::g_pRender->IsTextureEnabled();

	// While the next command pair is Tri2, add vertices
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;

	do {
		// Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
		DWORD dwV0 = ((dwCmd1>>16)&0xFF)/gRSP.vertexMult;
		DWORD dwV1 = ((dwCmd1>>8 )&0xFF)/gRSP.vertexMult;
		DWORD dwV2 = ((dwCmd1    )&0xFF)/gRSP.vertexMult;

		DWORD dwV3 = ((dwCmd0>>16)&0xFF)/gRSP.vertexMult;
		DWORD dwV4 = ((dwCmd0>>8 )&0xFF)/gRSP.vertexMult;
		DWORD dwV5 = ((dwCmd0    )&0xFF)/gRSP.vertexMult;

		DL_PF("    Tri2: 0x%08x 0x%08x", dwCmd0, dwCmd1);
		DL_PF("      V0: %d, V1: %d, V2: %d", dwV0, dwV1, dwV2);
		DL_PF("      V3: %d, V4: %d, V5: %d", dwV3, dwV4, dwV5);

		// Do first tri
		if (TestTri(dwV0, dwV1, dwV2))
		{
			DEBUG_DUMP_VERTEXES("Tri2 1/2", dwV0, dwV1, dwV2);
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

		// Do second tri
		if (TestTri(dwV3, dwV4, dwV5))
		{
			DEBUG_DUMP_VERTEXES("Tri2 2/2", dwV3, dwV4, dwV5);
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
			AddTri(dwV3, dwV4, dwV5);
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


void DLParser_GBI1_BranchZ(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_BranchZ);

	DWORD vtx = (dwCmd0&0xFFF)>1;

#ifdef _DEBUG
	if( debuggerEnableZBuffer==FALSE || (g_pVtxBase != NULL && g_pVtxBase[vtx].z <= (short)dwCmd1 ) )
#else
	if( g_pVtxBase != NULL && g_pVtxBase[vtx].z <= (short)dwCmd1 )
#endif
	{
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
		DWORD dwDL = *(DWORD *)(g_pu8RamBase + dwPC-12);
		DWORD dwAddr = RDPSegAddr(dwDL);

		DL_PF("BranchZ to DisplayList 0x%08x", dwAddr);
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

void DLParser_GBI1_LoadUCode(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DLParser_GBI1_LoadUCode);

	//TRACE0("Load ucode");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;
	DWORD dwUcStart = RDPSegAddr(dwCmd1);
	DWORD dwSize = (dwCmd0&0xFFFF)+1;
	DWORD dwUcDStart = RDPSegAddr(*(DWORD *)(g_pu8RamBase + dwPC-12));

	u32 ucode = DLParser_CheckUcode(dwUcStart, dwUcDStart, dwSize, 8);
	RDP_SetUcode(ucode, dwUcStart, dwUcDStart, dwSize);

	DEBUGGER_PAUSE_AND_DUMP(NEXT_SWITCH_UCODE,{DebuggerAppendMsg("Pause at loading ucode");});
}


void RDP_GFX_Force_Matrix(DWORD dwAddress)
{
	if (dwAddress + 64 > g_dwRamSize)
	{
		DebuggerAppendMsg("ForceMtx: Address invalid (0x%08x)", dwAddress);
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


#ifdef USING_INT_MATRIX
	N64IntMatrix intmtx((DWORD)(g_pu8RamBase+dwAddress));
#endif

#ifdef USING_INT_MATRIX
		CDaedalusRender::g_pRender->SetWorldProjectMatrix(mat,intmtx);
#else
		CDaedalusRender::g_pRender->SetWorldProjectMatrix(mat);
#endif

	DEBUGGER_PAUSE_AND_DUMP(NEXT_MATRIX_CMD,{TRACE0("Paused at ModMatrix Cmd");});
}



void RDP_DumpVtxInfo(DWORD dwAddress, DWORD dwV0, DWORD dwN)
{
#ifdef _DEBUG
		s8 *pcSrc = (s8 *)(g_pu8RamBase + dwAddress);
		s16 *psSrc = (s16 *)(g_pu8RamBase + dwAddress);

		for (DWORD dwV = dwV0; dwV < dwV0 + dwN; dwV++)
		{
			float x = (float)psSrc[0^0x1];
			float y = (float)psSrc[1^0x1];
			float z = (float)psSrc[2^0x1];

			DWORD wFlags = g_dwVecFlags[dwV]; //(WORD)psSrc[3^0x1];

			BYTE a = pcSrc[12^0x3];
			BYTE b = pcSrc[13^0x3];
			BYTE c = pcSrc[14^0x3];
			BYTE d = pcSrc[15^0x3];
			
			LONG nTU = (LONG)(SHORT)(psSrc[4^0x1]<<4);
			LONG nTV = (LONG)(SHORT)(psSrc[5^0x1]<<4);


			float tu = (float)(nTU>>4);
			float tv = (float)(nTV>>4);

			D3DXVECTOR4 & t = g_vecProjected[dwV];

			psSrc += 8;			// Increase by 16 bytes
			pcSrc += 16;

			DL_PF(" #%02d Flags: 0x%04x Pos: {% 6f,% 6f,% 6f} Tex: {%+7.2f,%+7.2f}, Extra: %02x %02x %02x %02x (transf: {% 6f,% 6f,% 6f})",
				dwV, wFlags, x, y, z, tu, tv, a, b, c, d, t.x, t.y, t.z );
		}
#endif
}



void RDP_MoveMemLight(DWORD dwLight, DWORD dwAddress)
{
	if( dwLight >= 16 )
	{
		DebuggerAppendMsg("Warning: invalid light # = %d", dwLight);
		return;
	}
	s8 * pcBase = g_ps8RamBase + dwAddress;
	DWORD * pdwBase = (DWORD *)pcBase;

	gRSPn64lights[dwLight].dwRGBA		= pdwBase[0];
	gRSPn64lights[dwLight].dwRGBACopy	= pdwBase[1];
	gRSPn64lights[dwLight].x			= pcBase[8 ^ 0x3];
	gRSPn64lights[dwLight].y			= pcBase[9 ^ 0x3];
	gRSPn64lights[dwLight].z			= pcBase[10 ^ 0x3];
					
	DL_PF("       RGBA: 0x%08x, RGBACopy: 0x%08x, x: %d, y: %d, z: %d", 
		gRSPn64lights[dwLight].dwRGBA,
		gRSPn64lights[dwLight].dwRGBACopy,
		gRSPn64lights[dwLight].x,
		gRSPn64lights[dwLight].y,
		gRSPn64lights[dwLight].z);

	if (dwLight == gRSP.ambientLightIndex)
	{
		DL_PF("      (Ambient Light)");

		DWORD dwCol = DAEDALUS_COLOR_RGBA( (gRSPn64lights[dwLight].dwRGBA >> 24)&0xFF,
					  (gRSPn64lights[dwLight].dwRGBA >> 16)&0xFF,
					  (gRSPn64lights[dwLight].dwRGBA >>  8)&0xFF, 0xff);

		SetAmbientLight( dwCol );
	}
	else
	{
		
		DL_PF("      (Normal Light)");

		SetLightCol(dwLight, gRSPn64lights[dwLight].dwRGBA);
		if (pdwBase[2] == 0)	// Direction is 0!
		{
			DL_PF("      Light is invalid");
		}
		else
		{
			SetLightDirection(dwLight, 
				gRSPn64lights[dwLight].x,
				gRSPn64lights[dwLight].y,
				gRSPn64lights[dwLight].z);
		}
	}
}

void RDP_MoveMemViewport(DWORD dwAddress)
{
	if( dwAddress+16 >= g_dwRamSize )
	{
		TRACE0("MoveMem Viewport, invalid memory");
		return;
	}

	s16 scale[4];
	s16 trans[4];

	// dwAddress is offset into RD_RAM of 8 x 16bits of data...
	scale[0] = *(s16 *)(g_pu8RamBase + ((dwAddress+(0*2))^0x2));
	scale[1] = *(s16 *)(g_pu8RamBase + ((dwAddress+(1*2))^0x2));
	scale[2] = *(s16 *)(g_pu8RamBase + ((dwAddress+(2*2))^0x2));
	scale[3] = *(s16 *)(g_pu8RamBase + ((dwAddress+(3*2))^0x2));

	trans[0] = *(s16 *)(g_pu8RamBase + ((dwAddress+(4*2))^0x2));
	trans[1] = *(s16 *)(g_pu8RamBase + ((dwAddress+(5*2))^0x2));
	trans[2] = *(s16 *)(g_pu8RamBase + ((dwAddress+(6*2))^0x2));
	trans[3] = *(s16 *)(g_pu8RamBase + ((dwAddress+(7*2))^0x2));


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

	// With D3D we had to ensure that the vp coords are positive, so
	// we truncated them to 0. This happens a lot, as things
	// seem to specify the scale as the screen w/2 h/2

	CDaedalusRender::g_pRender->SetViewport(nLeft, nTop, nRight, nBottom, maxZ);


	DL_PF("        Scale: %d %d %d %d = %d,%d", scale[0], scale[1], scale[2], scale[3], nWidth, nHeight);
	DL_PF("        Trans: %d %d %d %d = %d,%d", trans[0], trans[1], trans[2], trans[3], nCenterX, nCenterY);
}


// S2DEX uses this - 0xc1
void RDP_S2DEX_SPObjLoadTxtr_Ucode1(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(RDP_S2DEX_SPObjLoadTxtr_Ucode1);

	// Add S2DEX ucode supporting to F3DEX, see game DT and others
	status.useModifiedMap = true;
	RDP_SetUcode(1);
	memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
	
	//LoadedUcodeMap[G_OBJ_RECTANGLE] = &RDP_S2DEX_OBJ_RECTANGLE;
	LoadedUcodeMap[G_OBJ_MOVEMEM] = &RDP_S2DEX_OBJ_MOVEMEM;
	LoadedUcodeMap[G_OBJ_LOADTXTR] = &RDP_S2DEX_SPObjLoadTxtr;
	LoadedUcodeMap[G_OBJ_LDTX_SPRITE] = &RDP_S2DEX_SPObjLoadTxSprite;
	LoadedUcodeMap[G_OBJ_LDTX_RECT] = &RDP_S2DEX_SPObjLoadTxRect;
	LoadedUcodeMap[G_OBJ_LDTX_RECT_R] = &RDP_S2DEX_SPObjLoadTxRectR;

	TRACE0("Adding S2DEX ucodes to ucode 1");

	RDP_S2DEX_SPObjLoadTxtr(dwCmd0, dwCmd1);
}
