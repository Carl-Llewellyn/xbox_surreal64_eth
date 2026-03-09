/*
 * 1964 Copyright (C) 1999-2002 Joel Middendorf, <schibo@emulation64.com> This
 * program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: schibo@emulation64.com, rice1964@yahoo.com
 */

#include "../interrupt.h"
#include "../n64rcp.h"
#include "../memory.h"
#include "DLL_Rsp.h"
#include "Dll_Audio.h"
#include "Dll_Video.h"
#include "wingui.h"

uint16		RSPVersion = 0;
DWORD		RspTaskValue = 0;
HINSTANCE	hRSPHandle = NULL;
BOOL		rsp_plugin_is_loaded = FALSE;

// Ez0n3 - use iAudioPlugin instead to determine if basic audio is used
//BOOL	g_bUseBasicAudio = FALSE; 
//int g_iAudioPlugin = _AudioPluginJttl; // set default to jttl
//BOOL	g_bUseLLERspPlugin = FALSE;
BOOL g_bUseRspAudio = FALSE; // control a listing
int g_iRspPlugin = _RSPPluginHLE; // default hle

void (*_RSP_LINK_CloseDLL)(void) = NULL;
DWORD (*_RSP_LINK_DoRspCycles)(DWORD Cycles) = NULL;
void (*_RSP_LINK_InitiateRSP)(RSP_INFO Rsp_Info, DWORD *CycleCount) = NULL;
void (*_RSP_LINK_RomClosed)(void) = NULL;
void (*_RSP_LINK_GetDllInfo)(PLUGIN_INFO *PluginInfo) = NULL;
void (*_RSP_LINK_DllConfig)(HWND hWnd) = NULL;

BOOL LoadRSPPlugin(){
	if(g_iRspPlugin == _RSPPluginNone)
	{
		/*_RSP_LINK_CloseDLL		=	_RSP_NONE_CloseDLL;
		_RSP_LINK_DoRspCycles	=	_RSP_NONE_DoRspCycles;
		_RSP_LINK_InitiateRSP	=	_RSP_NONE_InitiateRSP;
		_RSP_LINK_RomClosed		=	_RSP_NONE_RomClosed;*/
	}
	else if(g_iRspPlugin == _RSPPluginLLE)
	{
		_RSP_LINK_CloseDLL		=	_RSP_CloseDLL;
		_RSP_LINK_DoRspCycles	=	_RSP_DoRspCycles;
		_RSP_LINK_InitiateRSP	=	_RSP_InitiateRSP;
		_RSP_LINK_RomClosed		=	_RSP_RomClosed;
	}
	else if(g_iRspPlugin == _RSPPluginHLE)
	{
		_RSP_LINK_CloseDLL		=	_RSP_HLE_CloseDLL;
		_RSP_LINK_DoRspCycles	=	_RSP_HLE_DoRspCycles;
		_RSP_LINK_InitiateRSP	=	_RSP_HLE_InitiateRSP;
		_RSP_LINK_RomClosed		=	_RSP_HLE_RomClosed;
	}
	else if(g_iRspPlugin == _RSPPluginM64p)
	{
		_RSP_LINK_CloseDLL		=	_RSP_M64p_CloseDLL;
		_RSP_LINK_DoRspCycles	=	_RSP_M64p_DoRspCycles;
		_RSP_LINK_InitiateRSP	=	_RSP_M64p_InitiateRSP;
		_RSP_LINK_RomClosed		=	_RSP_M64p_RomClosed;
	}

	return TRUE;
}

//
// ----------------------------------------------------------------------------
// Functions called by RSP Plugin
// ----------------------------------------------------------------------------
//
void ProcessDList(void)
{
	VIDEO_ProcessDList();
}

void ShowCFB(void)
{
	VIDEO_ShowCFB();
}

void ProcessRDPList(void)
{
	VIDEO_ProcessRDPList();
}

void ProcessAList(void)
{
	AUDIO_ProcessAList();
}

void RspCheckInterrupts(void)
{
	CheckInterrupts();
}

//
// ----------------------------------------------------------------------------
// Wrapper functions from the RSP Plugin
// ----------------------------------------------------------------------------
//
void RSPCloseDLL( void )
{
	//_RSP_CloseDLL();
	_RSP_LINK_CloseDLL();
}

void RSPDllAbout( HWND hWnd )
{
}

void RSPDllConfig( HWND hWnd )
{
}

void RSPRomClosed( void )
{
	//_RSP_RomClosed();
	_RSP_LINK_RomClosed();
}

DWORD DoRspCycles( DWORD cycles )
{
	DWORD retval;
	DWORD sp_pc_save = SP_PC_REG;
	SP_PC_REG &= 0xFFC;
	retval = _RSP_LINK_DoRspCycles(cycles);
	SP_PC_REG = sp_pc_save;
	return retval;
}

void InitiateRSP( RSP_INFO rspinfo, DWORD * cycles)
{
	//_RSP_InitiateRSP(rspinfo, cycles);
	_RSP_LINK_InitiateRSP(rspinfo, cycles);
}

//
// ----------------------------------------------------------------------------
// Supporting functions For the RSP Plugin
// ----------------------------------------------------------------------------
//

void InitializeRSP (void) 
{
	RSP_INFO RspInfo;

	RspInfo.CheckInterrupts = RspCheckInterrupts;
	RspInfo.ProcessDList = ProcessDList;
	
	// going to use rsp audio bool here instead of the use lle rsp bool
	// this was always NULL before
	if (g_bUseRspAudio) // g_bUseLLERspPlugin // g_iAudioPlugin == _AudioPluginLleRsp
	{
		RspInfo.ProcessAList = NULL;
	}
	else
	{
		RspInfo.ProcessAList = ProcessAList;
	}	
	
	RspInfo.ProcessRdpList = ProcessRDPList;
	RspInfo.ShowCFB = ShowCFB;

	RspInfo.hInst = gui.hInst;
	RspInfo.RDRAM = gMS_RDRAM;
	RspInfo.DMEM = (uint8*)&SP_DMEM;
	RspInfo.IMEM = (uint8*)&SP_IMEM;
	RspInfo.MemoryBswaped = FALSE;

	RspInfo.MI_INTR_RG = &MI_INTR_REG_R;
		
	RspInfo.SP_MEM_ADDR_RG = &SP_MEM_ADDR_REG;
	RspInfo.SP_DRAM_ADDR_RG = &SP_DRAM_ADDR_REG;
	RspInfo.SP_RD_LEN_RG = &SP_RD_LEN_REG;
	RspInfo.SP_WR_LEN_RG = &SP_WR_LEN_REG;
	RspInfo.SP_STATUS_RG = &SP_STATUS_REG;
	RspInfo.SP_DMA_FULL_RG = &SP_DMA_FULL_REG;
	RspInfo.SP_DMA_BUSY_RG = &SP_DMA_BUSY_REG;
	RspInfo.SP_PC_RG = &SP_PC_REG;
	RspInfo.SP_SEMAPHORE_RG = &SP_SEMAPHORE_REG;
		
	RspInfo.DPC_START_RG = &DPC_START_REG;
	RspInfo.DPC_END_RG = &DPC_END_REG;
	RspInfo.DPC_CURRENT_RG = &DPC_CURRENT_REG;
	RspInfo.DPC_STATUS_RG = &DPC_STATUS_REG;
	RspInfo.DPC_CLOCK_RG = &DPC_CLOCK_REG;
	RspInfo.DPC_BUFBUSY_RG = &DPC_BUFBUSY_REG;
	RspInfo.DPC_PIPEBUSY_RG = &DPC_PIPEBUSY_REG;
	RspInfo.DPC_TMEM_RG = &DPC_TMEM_REG;
	
	InitiateRSP(RspInfo, &RspTaskValue); 

#ifndef _DEBUG
	//InitiateInternalRSP(RspInfo11, &RspTaskValue);
#endif
}

/*BOOL LoadRSPPlugin(char * libname) 
{
	PLUGIN_INFO pi;
	//_RSP_GetDllInfo(&pi);
	_RSP_LINK_GetDllInfo(&pi);
	return TRUE;
}

void CloseRSPPlugin (void) 
{
	//_RSP_CloseDLL();
	_RSP_LINK_CloseDLL(); 
}*/

