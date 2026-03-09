/*
 * RSP Compiler plug in for Project 64 (A Nintendo 64 emulator).
 *
 * (c) Copyright 2001 jabo (jabo@emulation64.com) and
 * zilmar (zilmar@emulation64.com)
 *
 * pj64 homepage: www.pj64.net
 * 
 * Permission to use, copy, modify and distribute Project64 in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Project64 is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Project64 or software derived from Project64.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so if they want them.
 *
 */
#pragma once
#include "Common.h"
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct {
	HINSTANCE hInst;
	BOOL MemoryBswaped;    /* If this is set to TRUE, then the memory has been pre
	                          bswap on a dword (32 bits) boundry */
	BYTE * RDRAM;
	BYTE * DMEM;
	BYTE * IMEM;
	DWORD * MI_INTR_REG;

	DWORD * SP_MEM_ADDR_REG;
	DWORD * SP_DRAM_ADDR_REG;
	DWORD * SP_RD_LEN_REG;
	DWORD * SP_WR_LEN_REG;
	DWORD * SP_STATUS_REG;
	DWORD * SP_DMA_FULL_REG;
	DWORD * SP_DMA_BUSY_REG;
	DWORD * SP_PC_REG;
	DWORD * SP_SEMAPHORE_REG;

	DWORD * DPC_START_REG;
	DWORD * DPC_END_REG;
	DWORD * DPC_CURRENT_REG;
	DWORD * DPC_STATUS_REG;
	DWORD * DPC_CLOCK_REG;
	DWORD * DPC_BUFBUSY_REG;
	DWORD * DPC_PIPEBUSY_REG;
	DWORD * DPC_TMEM_REG;
	void (*CheckInterrupts)( void );
	void (*ProcessDList)( void );
	void (*ProcessAList)( void );
	void (*ProcessRdpList)( void );
	void (*ShowCFB)( void );
} RSP_INFO;

void  _RSP_M64p_CloseDLL (void);
void  _RSP_M64p_DllAbout ( HWND hParent );
DWORD _RSP_M64p_DoRspCycles ( DWORD Cycles );
void  _RSP_M64p_GetDllInfo ( PLUGIN_INFO * PluginInfo );
void  _RSP_M64p_InitiateRSP ( RSP_INFO Rsp_Info, DWORD * CycleCount);
void  _RSP_M64p_RomClosed (void);
void  _RSP_M64p_DllConfig (HWND hWnd);
#if defined(__cplusplus)
}
#endif
