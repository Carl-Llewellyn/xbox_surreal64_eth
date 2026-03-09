/*
Copyright (C) 2001 StrmnNrmn

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

#include "stdafx.h"

//// Reg type things for GFXTask

// Used in a few places - don't declare as static


/////////////////////////////////////////////////////////////////////////////////////

void RDP_DumpRSPCode(char * szName, DWORD dwCRC, DWORD * pBase, DWORD dwPCBase, DWORD dwLen)
{
	//TCHAR opinfo[400];
/*	TCHAR szFilePath[MAX_PATH+1];
	TCHAR szFileName[MAX_PATH+1];
	FILE * fp;

	//Dump_GetDumpDirectory(szFilePath, TEXT("rsp_dumps"));

	wsprintf(szFileName, "task_dump_%s_crc_0x%08x.txt", szName, dwCRC);

	PathAppend(szFilePath, szFileName);
	
	fp = fopen(szFilePath, "w");
	if (fp == NULL)
		return;

	DWORD dwIndex;
	for (dwIndex = 0; dwIndex < dwLen; dwIndex+=4)
	{
		DWORD dwOpCode;
		DWORD pc = dwIndex&0x0FFF;
		dwOpCode = pBase[dwIndex/4];

		//SprintRSPOpCodeInfo(opinfo, pc + dwPCBase, dwOpCode);

		//fprintf(fp, "0x%08x: <0x%08x> %s\n", pc + dwPCBase, dwOpCode, opinfo);
		//fprintf(fp, "<0x%08x>\n", dwOpCode);
	}
	
	fclose(fp);*/

}


void RDP_DumpRSPData(char * szName, DWORD dwCRC, DWORD * pBase, DWORD dwPCBase, DWORD dwLen)
{
/*	TCHAR szFilePath[MAX_PATH+1];
	TCHAR szFileName[MAX_PATH+1];
	FILE * fp;

	//Dump_GetDumpDirectory(szFilePath, TEXT("rsp_dumps"));

	wsprintf(szFileName, "task_data_dump_%s_crc_0x%08x.txt", szName, dwCRC);

	PathAppend(szFilePath, szFileName);

	fp = fopen(szFilePath, "w");
	if (fp == NULL)
		return;

	DWORD dwIndex;
	for (dwIndex = 0; dwIndex < dwLen; dwIndex+=4)
	{
		DWORD dwData;
		DWORD pc = dwIndex&0x0FFF;
		dwData = pBase[dwIndex/4];

		fprintf(fp, "0x%08x: 0x%08x\n", pc + dwPCBase, dwData);
	}
	
	fclose(fp);*/

}

extern RDPInstruction *currentUcodeMap;
extern DList g_dwPCStack[MAX_DL_STACK_SIZE];
extern int g_dwPCindex;
void SetVIScales();
extern LPCSTR g_szRDPInstrName[256];


void RDP_DLParser_Process(void)
{
	StartRDP();
	status.gRDPTime = timeGetTime();

	DWORD dwPC;
	DWORD dwCmd0;
	DWORD dwCmd1;

	DWORD start = *(g_GraphicsInfo.DPC_START_RG);
	DWORD end = *(g_GraphicsInfo.DPC_END_RG);

	g_dwPCindex=0;
	g_dwPCStack[g_dwPCindex].addr = start;
	g_dwPCStack[g_dwPCindex].limit = ~0;

	// Check if we need to purge
	if (status.gRDPTime - status.lastPurgeTimeTime > 5000)
	{
		gTextureCache.PurgeOldTextures();
		status.lastPurgeTimeTime = status.gRDPTime;
	}
	
	// Lock the graphics context here.
	CDaedalusRender::g_pRender->SetFillMode(DAEDALUS_FILLMODE_SOLID);

	SetVIScales();

	CDaedalusRender::g_pRender->RenderReset();
	CDaedalusRender::g_pRender->BeginRendering();
	CDaedalusRender::g_pRender->SetViewport(0, 0, windowSetting.uViWidth, windowSetting.uViHeight, 0x3FF);

	dwPC = g_dwPCStack[0].addr;
	while( dwPC < end )
	{
		dwCmd0 = g_pu32RamBase[(dwPC>>2)+0];
		dwCmd1 = g_pu32RamBase[(dwPC>>2)+1];
		
		g_dwPCStack[0].addr += 8;
		DL_PF("%s: %08x - %08X", g_szRDPInstrName[dwCmd0>>24], dwCmd0, dwCmd1);
		currentUcodeMap[dwCmd0>>24](dwCmd0, dwCmd1);
		dwPC = g_dwPCStack[0].addr;
	}

	CDaedalusRender::g_pRender->EndRendering();

	StopRDP();
}

void RDP_TriFill(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriFill);

	TRACE0("RDP_TriFill");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD* pCmd = (DWORD *)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;

	g_dwPCStack[g_dwPCindex].addr += 24;
}

void RDP_TriFillZ(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriFillZ);

	TRACE0("RDP_TriFillZ");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD pCmd = (DWORD)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;
	RDPZCoeff &pz = *(RDPZCoeff*)(pCmd+32);

	g_dwPCStack[g_dwPCindex].addr += 40;
}

void RDP_TriTxtr(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriTxtr);

	TRACE0("RDP_TriTxtr");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD pCmd = (DWORD)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;
	RDPTextureCoeff &ptexture = *(RDPTextureCoeff*)(pCmd+32);

	g_dwPCStack[g_dwPCindex].addr += 88;
}

void RDP_TriTxtrZ(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriTxtrZ);

	TRACE0("RDP_TriTxtrZ");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD pCmd = (DWORD)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;
	RDPTextureCoeff &ptexture = *(RDPTextureCoeff*)(pCmd+32);
	RDPZCoeff &pz = *(RDPZCoeff*)(pCmd+96);

	g_dwPCStack[g_dwPCindex].addr += 104;
}

void RDP_TriShade(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriShade);

	TRACE0("RDP_TriShade");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD pCmd = (DWORD)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;
	RDPShadeCoeff &pshade = *(RDPShadeCoeff*)(pCmd+32);

	g_dwPCStack[g_dwPCindex].addr += 88;
}

void RDP_TriShadeZ(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriShadeZ);

	TRACE0("RDP_TriShadeZ");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD pCmd = (DWORD)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;
	RDPShadeCoeff &pshade = *(RDPShadeCoeff*)(pCmd+32);
	RDPZCoeff &pz = *(RDPZCoeff*)(pCmd+96);

	g_dwPCStack[g_dwPCindex].addr += 104;
}

void RDP_TriShadeTxtr(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriShadeTxtr);

	TRACE0("RDP_TriShadeTxtr");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD pCmd = (DWORD)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;
	RDPShadeCoeff &pshade = *(RDPShadeCoeff*)(pCmd+32);
	RDPTextureCoeff &ptexture = *(RDPTextureCoeff*)(pCmd+96);

	g_dwPCStack[g_dwPCindex].addr += 152;
}

void RDP_TriShadeTxtrZ(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(RDP_TriShadeTxtrZ);

	TRACE0("RDP_TriShadeTxtrZ");
	DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction

	DWORD pCmd = (DWORD)(g_pu8RamBase + dwPC - 8);
	RDPEdgeCoeff &pedge = *(RDPEdgeCoeff*)pCmd;
	RDPShadeCoeff &pshade = *(RDPShadeCoeff*)(pCmd+32);
	RDPTextureCoeff &ptexture = *(RDPTextureCoeff*)(pCmd+96);
	RDPZCoeff &pz = *(RDPZCoeff*)(pCmd+160);

	g_dwPCStack[g_dwPCindex].addr += 168;
}
