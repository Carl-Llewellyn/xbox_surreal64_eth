/*$T registry.c GC 1.136 03/09/02 17:30:37 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    System registry keys and fields. Search your registry for 1964emu
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

#include <xtl.h>
#include "registry.h"
#include "../1964ini.h"
#include "../debug_option.h"
#include "wingui.h"
#include "../emulator.h"


void	InitAll1964Options(void);

/*
 =======================================================================================================================
    This function is called only when 1964 starts £
 =======================================================================================================================
 */
void ReadConfiguration(void)
{
	InitAll1964Options();
	return;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void InitAll1964Options(void)
{
	//strcat(default_save_directory, "T:\\");
	strcpy(default_save_directory, g_szPathSaves);
	
	strcpy(user_set_save_directory, default_save_directory);
	strcpy(state_save_directory, default_save_directory);
	strcpy(directories.save_directory_to_use, default_save_directory);

	emuoptions.auto_run_rom = TRUE;
	emuoptions.auto_full_screen = FALSE;
	emuoptions.auto_apply_cheat_code = FALSE;
	emuoptions.UsingRspPlugin = TRUE;
	emuoptions.dma_in_segments = FALSE;
	emuoptions.SyncVI = TRUE;

	defaultoptions.Eeprom_size = EEPROMSIZE_4KB;
	defaultoptions.RDRAM_Size = RDRAMSIZE_4MB;
	defaultoptions.Emulator = DYNACOMPILER;
	defaultoptions.Save_Type = ANYUSED_SAVETYPE;

	defaultoptions.Code_Check = CODE_CHECK_MEMORY_QWORD;
	defaultoptions.Max_FPS = MAXFPS_AUTO_SYNC;
	defaultoptions.Use_TLB = USETLB_YES;
	defaultoptions.FPU_Hack = USEFPUHACK_DEFAULT;
	defaultoptions.DMA_Segmentation = USEDMASEG_YES;
	defaultoptions.Link_4KB_Blocks = USE4KBLINKBLOCK_YES;
	defaultoptions.Advanced_Block_Analysis = USEBLOCKANALYSIS_YES;
	defaultoptions.Assume_32bit = ASSUME_32BIT_NO;
	defaultoptions.Use_HLE = USEHLE_NO;
	defaultoptions.Counter_Factor = COUTERFACTOR_2;
	defaultoptions.Use_Register_Caching = USEREGC_YES;

	// Default 480
	guistatus.clientwidth = 640;
	guistatus.clientheight = 480;

	//Change window size for HD
	/*if(XGetAVPack() == XC_AV_PACK_HDTV)
	  {		
		  if((XGetVideoFlags() & XC_VIDEO_FLAGS_HDTV_720p)){
			guistatus.clientwidth = 1280;
			guistatus.clientheight = 720;
		  }
	  }
	  */
	guistatus.window_position.top = 100;
	guistatus.window_position.left = 100;
	guistatus.WindowIsMaximized = FALSE;
}
