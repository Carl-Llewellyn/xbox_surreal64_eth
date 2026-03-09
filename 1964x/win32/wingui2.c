/*$T wingui2.c GC 1.136 03/09/02 17:35:12 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    User Interface Client Dialogue Windows and Message Boxes
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


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

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include <xtl.h>
#include <dsound.h>
#define WIN32_LEAN_AND_MEAN
#include "../debug_option.h"
#include "../interrupt.h"
#include "../n64rcp.h"
#include "wingui.h"
#include "dll_audio.h"
#include "dll_video.h"
#include "dll_input.h"
#include "dll_rsp.h"
#include "../1964ini.h"
#include "../timer.h"
#include "registry.h"
#include "../emulator.h"
#include "../rompaging.h"

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void CountryCodeToCountryName_and_TVSystem(int countrycode, char *countryname, int *tvsystem)
{
	//Keep Country Name < 10 characters!
	switch(countrycode)
	{
	/* Demo */
	case 0:
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "Demo");
		break;

	case '7':
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "Beta");
		break;

	case 0x41:
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "USA/Japan");
		break;

	/* Germany */
	case 0x44:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "German");
		break;

	/* USA */
	case 0x45:
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "USA");
		break;

	/* France */
	case 0x46:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "France");
		break;

	/* Italy */
	case 'I':
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "Italy");
		break;

	/* Japan */
	case 0x4A:
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "Japan");
		break;

	/* Europe - PAL */
	case 0x50:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "Europe");
		break;

	case 'S':	/* Spain */
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "Spain");
		break;

	/* Australia */
	case 0x55:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "Australia");
		break;

	case 0x58:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "Europe");
		break;

	/* Australia */
	case 0x59:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "Australia");
		break;

	case 0x20:
	case 0x21:
	case 0x38:
	case 0x70:
		*tvsystem = TV_SYSTEM_PAL;
		sprintf(countryname, "Europe", countrycode);
		break;

	/* ??? */
	default:
		*tvsystem = TV_SYSTEM_PAL;
		sprintf(countryname, "PAL", countrycode);
		break;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void InitPluginData(void)
{
	Gfx_Info.hWnd = gui.hwnd1964main;
	Gfx_Info.hStatusBar = NULL;
	Gfx_Info.MemoryBswaped = TRUE;
	Gfx_Info.HEADER = (BYTE *) &HeaderDllPass[0];
	Gfx_Info.RDRAM = (BYTE *) &gMemoryState.RDRAM[0];
	Gfx_Info.DMEM = (BYTE *) &SP_DMEM;
	Gfx_Info.IMEM = (BYTE *) &SP_IMEM;
	Gfx_Info.MI_INTR_RG = (DWORD *)&MI_INTR_REG_R;
	Gfx_Info.DPC_START_RG = (DWORD *)&DPC_START_REG;
	Gfx_Info.DPC_END_RG = (DWORD *)&DPC_END_REG;
	Gfx_Info.DPC_CURRENT_RG = (DWORD *)&DPC_CURRENT_REG;
	Gfx_Info.DPC_STATUS_RG = (DWORD *)&DPC_STATUS_REG;
	Gfx_Info.DPC_CLOCK_RG = (DWORD *)&DPC_CLOCK_REG;
	Gfx_Info.DPC_BUFBUSY_RG = (DWORD *)&DPC_BUFBUSY_REG;
	Gfx_Info.DPC_PIPEBUSY_RG = (DWORD *)&DPC_PIPEBUSY_REG;
	Gfx_Info.DPC_TMEM_RG = (DWORD *)&DPC_TMEM_REG;

	Gfx_Info.VI_STATUS_RG = (DWORD *)&VI_STATUS_REG;
	Gfx_Info.VI_ORIGIN_RG = (DWORD *)&VI_ORIGIN_REG;
	Gfx_Info.VI_WIDTH_RG = (DWORD *)&VI_WIDTH_REG;
	Gfx_Info.VI_INTR_RG = (DWORD *)&VI_INTR_REG;
	Gfx_Info.VI_V_CURRENT_LINE_RG = (DWORD *)&VI_CURRENT_REG;
	Gfx_Info.VI_TIMING_RG = (DWORD *)&VI_BURST_REG;
	Gfx_Info.VI_V_SYNC_RG = (DWORD *)&VI_V_SYNC_REG;
	Gfx_Info.VI_H_SYNC_RG = (DWORD *)&VI_H_SYNC_REG;
	Gfx_Info.VI_LEAP_RG = (DWORD *)&VI_LEAP_REG;
	Gfx_Info.VI_H_START_RG = (DWORD *)&VI_H_START_REG;
	Gfx_Info.VI_V_START_RG = (DWORD *)&VI_V_START_REG;
	Gfx_Info.VI_V_BURST_RG = (DWORD *)&VI_V_BURST_REG;
	Gfx_Info.VI_X_SCALE_RG = (DWORD *)&VI_X_SCALE_REG;
	Gfx_Info.VI_Y_SCALE_RG = (DWORD *)&VI_Y_SCALE_REG;
	Gfx_Info.CheckInterrupts = CheckInterrupts;

	Audio_Info.hwnd = gui.hwnd1964main;
	Audio_Info.hinst = NULL;

	Audio_Info.MemoryBswaped = 1;	/* If this is set to TRUE, then the memory has been pre */

	/*
	 * bswap on a dword (32 bits) boundry £
	 * eg. the first 8 bytes are stored like this: £
	 * 4 3 2 1 8 7 6 5
	 */
	Audio_Info.HEADER = (BYTE *) &HeaderDllPass[0];
	Audio_Info.RDRAM = (BYTE *) &gMemoryState.RDRAM[0];
	Audio_Info.DMEM = (BYTE *) &SP_DMEM;
	Audio_Info.IMEM = (BYTE *) &SP_IMEM;

	Audio_Info.MI_INTR_RG = (DWORD *)&MI_INTR_REG_R;

	Audio_Info.AI_DRAM_ADDR_RG = (DWORD *)&AI_DRAM_ADDR_REG;;
	Audio_Info.AI_LEN_RG = (DWORD *)&AI_LEN_REG;
	Audio_Info.AI_CONTROL_RG = (DWORD *)&AI_CONTROL_REG;
	Audio_Info.AI_STATUS_RG = (DWORD *)&AI_STATUS_REG;
	Audio_Info.AI_DACRATE_RG = (DWORD *)&AI_DACRATE_REG;
	Audio_Info.AI_BITRATE_RG = (DWORD *)&AI_BITRATE_REG;
	Audio_Info.CheckInterrupts = CheckInterrupts;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Set_1964_Directory(void)
{
	//strcpy(directories.main_directory, "D:\\");
	strcpy(directories.main_directory, "T:\\");
}

char	critical_msg_buffer[32 * 1024]; /* 32KB */

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void __cdecl DisplayCriticalMessage(char *Message, ...)
{
//#ifdef DEBUG
	char	Msg[400];
	va_list ap;
	
	va_start(ap, Message);
	vsprintf(Msg, Message, ap);
	va_end(ap);

	OutputDebugString(Msg);
/*#else
	OutputDebugString(Message);
#endif*/
	OutputDebugString("\n");
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void __cdecl DisplayError(char *Message, ...)
{
//#ifdef DEBUG
	char	Msg[400];
	va_list ap;
	
	va_start(ap, Message);
	vsprintf(Msg, Message, ap);
	va_end(ap);

	OutputDebugString(Msg);
/*#else
	OutputDebugString(Message);
#endif*/
	OutputDebugString("\n");
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BOOL __cdecl DisplayError_AskIfContinue(char *Message, ...)
{
//#ifdef DEBUG
	char	Msg[400];
	va_list ap;
	
	va_start(ap, Message);
	vsprintf(Msg, Message, ap);
	va_end(ap);

	OutputDebugString(Msg);
/*#else
	OutputDebugString(Message);
#endif*/
	OutputDebugString("\n");

	return TRUE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void UpdateCIC(void)
{
	/*~~~~~~~~~~~~~~~~*/
	/* Math CIC */
	__int64 CIC_CRC = 0;
	int		i;
	/*~~~~~~~~~~~~~~~~*/

	// added by oDD
	// temporarily load the first meg of the rom for CIC and checksum
	FILE *tmpFile = fopen(g_temporaryRomPath, "rb");
	uint8 *tmpBuf = (uint8 *)malloc(0x00110000);

	fseek(tmpFile, 0, SEEK_SET);
	fread(tmpBuf, sizeof(uint8), 0x00110000, tmpFile);
	fclose(tmpFile);

	for(i = 0; i < 0xFC0; i++)
	{
		CIC_CRC = CIC_CRC + (uint8) tmpBuf[0x40 + i];
	}

	switch(CIC_CRC)
	{
	/* CIC-NUS-6101 (starfox) */
	case 0x33a27:
	case 0x3421e:
		/* DisplayError("Using CIC-NUS-6101\n"); */
		TRACE0("Using CIC-NUS-6101 for starfox\n");
		rominfo.CIC = (uint64) 0x3f;
		rominfo.RDRam_Size_Hack = (uint32) 0x318;
		break;

	/* CIC-NUS-6102 (mario) */
	case 0x34044:
		/* DisplayError("Using CIC-NUS-6102\n"); */
		TRACE0("Using CIC-NUS-6102 for mario\n");
		rominfo.CIC = (uint64) 0x3f;
		rominfo.RDRam_Size_Hack = (uint32) 0x318;
		ROM_CheckSumMario(tmpBuf);

		break;

	/* CIC-NUS-6103 (Banjo) */
	case 0x357d0:
		/* DisplayError("Using CIC-NUS-6103\n"); */
		TRACE0("Using CIC-NUS-6103 for Banjo\n");
		rominfo.CIC = (uint64) 0x78;
		rominfo.RDRam_Size_Hack = (uint32) 0x318;
		break;

	/* CIC-NUS-6105 (Zelda) */
	case 0x47a81:
		/* DisplayError("Using CIC-NUS-6105\n"); */
		TRACE0("Using CIC-NUS-6105 for Zelda\n");
		rominfo.CIC = 0x91;
		rominfo.RDRam_Size_Hack = (uint32) 0x3F0;
		ROM_CheckSumZelda(tmpBuf);
		break;

	/* CIC-NUS-6106 (F-Zero X) */
	case 0x371cc:
		/* DisplayError("Using CIC-NUS-6106\n"); */
		TRACE0("Using CIC-NUS-6106 for F-Zero/Yoshi Story\n");
		rominfo.CIC = (uint64) 0x85;
		rominfo.RDRam_Size_Hack = (uint32) 0x318;
		break;

	/* Using F1 World Grand Prix */
	case 0x343c9:
		/*
		 * LogDirectToFile("Using f1 World Grand Prix\n"); £
		 * DisplayError("F1 World Grand Prix ... i never saw ths BootCode before");
		 */
		TRACE0("Using Boot Code for F1 World Grand Prix\n");
		rominfo.CIC = (uint64) 0x85;
		rominfo.RDRam_Size_Hack = (uint32) 0x3F0;
		break;

	default:
		/*
		 * DisplayError("unknown CIC %08x!!!", (uint32)CIC_CRC); £
		 * SystemFailure(FILEIO_EXIT); £
		 * Use Mario for unknown boot code
		 */
		TRACE0("Unknown boot code, using Mario boot code instead");
		rominfo.CIC = (uint64) 0x3f;
		rominfo.RDRam_Size_Hack = (uint32) 0x318;
		break;
	}

	free(tmpBuf);

	rominfo.countrycode = HeaderDllPass[0x3D];

	CountryCodeToCountryName_and_TVSystem(rominfo.countrycode, game_country_name, &game_country_tvsystem);
	rominfo.TV_System = game_country_tvsystem;
	Init_VI_Counter(game_country_tvsystem);
}

//LPDIRECTSOUND lpds;
void LoadPlugins()
{
	SetDefaultOptions();

	// control RSP with multi plugin options
	if (g_iRspPlugin > _RSPPluginNone && g_iRspPlugin < _RSPPluginMissing)
	{
		rsp_plugin_is_loaded = TRUE;
		emuoptions.UsingRspPlugin = TRUE;
		InitializeRSP();
	}
	else
	{
		rsp_plugin_is_loaded = FALSE;
		emuoptions.UsingRspPlugin = FALSE;
	}

	CONTROLLER_InitiateControllers(gui.hwnd1964main, Controls);

	if(AUDIO_Initialize(Audio_Info) == TRUE)
	{
		Audio_Is_Initialized = 1;
	}
	else
	{
		Audio_Is_Initialized = 0;
	}

	InitPluginData();
	VIDEO_InitiateGFX(Gfx_Info);
}
