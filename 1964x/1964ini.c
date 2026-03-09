/*$T 1964ini.c GC 1.136 03/09/02 17:36:01 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Source for the managing the 1964.ini file.
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
#include "globals.h"
#include "1964ini.h"
#include "rompaging.h"
#include "fileio.h"
#include "win32\wingui.h"

uint32		ConvertHexCharToInt(char c);
uint32		ConvertHexStringToInt(const char *str, int nchars);
void		chopm(char *str);

/* Global variables */
char		default_save_directory[_MAX_PATH];
char		default_state_save_directory[_MAX_PATH];
char		user_set_save_directory[_MAX_PATH];
char		state_save_directory[_MAX_PATH];

float		vips_speed_limits[] = { 500.0f, 500.0f, 60.0f, 50.0f, 60.0f };
int			ini_entry_count = 0;

INI_ENTRY	defaultoptions;					/* 1964 default options */
INI_ENTRY	currentromoptions;				/* option setting for the current ROM, options are generates */

/*
 =======================================================================================================================
    from rom options and 1964 default options if the options £
    are set as DEFAULT in ROM setting
 =======================================================================================================================
 */

void ReadRomHeaderInMemory(INI_ENTRY *ini_entry)
{
	/*~~~~~~~~~~~~~~~~~~*/
	uint8	buffer[0x100];
	/*~~~~~~~~~~~~~~~~~~*/

	// added by oDD
	// replaces: memcpy(buffer, gMS_ROM_Image, 0x40);
	{
		FILE *tmpFile = fopen(g_temporaryRomPath, "rb");
		fseek(tmpFile, 0, SEEK_SET);
		fread(buffer, sizeof(uint8), 0x40, tmpFile);
		fclose(tmpFile);
	}

	strncpy(ini_entry->Game_Name, buffer + 0x20, 0x14);
	SwapRomName(ini_entry->Game_Name);

	ini_entry->crc1 = *((uint32 *) (buffer + 0x10));
	ini_entry->crc2 = *((uint32 *) (buffer + 0x14));
	ini_entry->countrycode = buffer[0x3D];
}

void SetDefaultOptions(void)
{
	defaultoptions.Code_Check = CODE_CHECK_PROTECT_MEMORY;
	defaultoptions.Comments[0] = '\0';
	defaultoptions.Alt_Title[0] = '\0';
	defaultoptions.countrycode = 0;
	defaultoptions.crc1 = 0;
	defaultoptions.crc2 = 0;
	defaultoptions.Emulator = DYNACOMPILER;
	defaultoptions.Game_Name[0] = '\0';
	defaultoptions.Max_FPS = MAXFPS_AUTO_SYNC;
	defaultoptions.RDRAM_Size = RDRAMSIZE_4MB;
	defaultoptions.Save_Type = ANYUSED_SAVETYPE;
	defaultoptions.Use_TLB = USETLB_YES;
	defaultoptions.Eeprom_size = EEPROMSIZE_4KB;
	defaultoptions.Use_Register_Caching = USEREGC_YES;
	defaultoptions.Counter_Factor = COUTERFACTOR_2;
	defaultoptions.FPU_Hack = USEFPUHACK_YES;
	defaultoptions.DMA_Segmentation = USEDMASEG_YES;
	defaultoptions.Link_4KB_Blocks = USE4KBLINKBLOCK_YES;
	defaultoptions.Advanced_Block_Analysis = USEBLOCKANALYSIS_YES;
	defaultoptions.Assume_32bit = ASSUME_32BIT_NO;
	defaultoptions.Use_HLE = USEHLE_NO;
}

/*
 =======================================================================================================================
    This function should be called everytime before playing a game, will £
    recalculate the values in the currentromoptions for emulator and CPU £
    core to use
 =======================================================================================================================
 */
int GenerateCurrentRomOptions(void)
{
	char		iniFilePath[_MAX_PATH];
	FILE		*iniFile;
	CHAR		line[256];
	uint32		crc1;
	uint32		crc2;
	uint8		countrycode;
	BOOL		foundMatchingEntry = FALSE;

	// initialise the current rom options to the default values
	currentromoptions.Code_Check				= defaultoptions.Code_Check;
	currentromoptions.Comments[0]				= '\0';
	currentromoptions.Alt_Title[0]				= '\0';
	currentromoptions.countrycode				= 0;
	currentromoptions.crc1						= 0;
	currentromoptions.crc2						= 0;
	currentromoptions.Emulator					= defaultoptions.Emulator;
	currentromoptions.Game_Name[0]				= '\0';
	currentromoptions.Max_FPS					= defaultoptions.Max_FPS;
	currentromoptions.RDRAM_Size				= defaultoptions.RDRAM_Size;
	currentromoptions.Save_Type					= defaultoptions.Save_Type;
	currentromoptions.Use_TLB					= defaultoptions.Use_TLB;
	currentromoptions.Eeprom_size				= defaultoptions.Eeprom_size;
	currentromoptions.Counter_Factor			= defaultoptions.Counter_Factor;
	currentromoptions.Use_Register_Caching		= defaultoptions.Use_Register_Caching;
	currentromoptions.FPU_Hack					= defaultoptions.FPU_Hack;
	currentromoptions.DMA_Segmentation			= defaultoptions.DMA_Segmentation;
	currentromoptions.Link_4KB_Blocks			= defaultoptions.Link_4KB_Blocks;
	currentromoptions.Advanced_Block_Analysis	= defaultoptions.Advanced_Block_Analysis;
	currentromoptions.Assume_32bit				= defaultoptions.Assume_32bit;
	currentromoptions.Use_HLE					= defaultoptions.Use_HLE;

	ReadRomHeaderInMemory(&currentromoptions);

	strcpy(iniFilePath, directories.main_directory);
	strcat(iniFilePath, "1964.ini");
	
	if(!PathFileExists(iniFilePath)) {
		OutputDebugString(iniFilePath);
		OutputDebugStringA(" Failed to Load!\n");
		sprintf(iniFilePath, "D:\\1964.ini");
		if(!PathFileExists(iniFilePath)) {
			OutputDebugString(iniFilePath);
			OutputDebugStringA(" Failed to Load!\n");
			// FIXME!
			//sprintf(iniFilePath, "T:\\1964.ini");
			// WRITE INI FILE
		} else {
			OutputDebugString(iniFilePath);
			OutputDebugStringA(" Successfully Loaded!\n");
		}
	} else {
		OutputDebugString(iniFilePath);
		OutputDebugStringA(" Successfully Loaded!\n");
	}
	
	iniFile = fopen(iniFilePath, "rt");
	if(iniFile == NULL)
	{
		return FALSE;
	}

	while (TRUE)
	{
		do
		{
			// get a line from the ini
			ZeroMemory(line, 256);
			fgets(line, 255, iniFile);
			chopm(line);

			if (foundMatchingEntry && strlen(line) < 2)
				return TRUE;

			if (feof(iniFile))
			{
				fclose(iniFile);
				return FALSE;
			}
		} 
		while(strlen(line) < 2 || (line[0] == '/' && line[1] == '/'));

		if(strlen(line) > 23)
		{
			if(line[0] == '[' && line[23] == ']')
			{
				crc1		= ConvertHexStringToInt(line + 1, 8);
				crc2		= ConvertHexStringToInt(line + 10, 8);
				countrycode = (uint8) ConvertHexStringToInt(line + 21, 2);

				// see if the rom entry found in the ini matches the current rom
				if (crc1 == currentromoptions.crc1
				&& crc2 == currentromoptions.crc2
				&& countrycode == currentromoptions.countrycode)
				{
					foundMatchingEntry = TRUE;
				}

				continue;
			}
		}

		if(strncmp(line, "Game Name=", 10) == 0 && foundMatchingEntry)
		{
			strcpy(currentromoptions.Game_Name, line + 10);
		}
		else if(strncmp(line, "Comments=", 9) == 0 && foundMatchingEntry)
		{
			strcpy(currentromoptions.Comments, line + 9);
		}
		else if(strncmp(line, "Alternate Title=", 16) == 0 && foundMatchingEntry)
		{
			strcpy(currentromoptions.Alt_Title, line + 16);
		}
		else if(strncmp(line, "RDRAM Size=", 11) == 0 && foundMatchingEntry)
		{
			currentromoptions.RDRAM_Size = atoi(line + 11);
		}
		else if(strncmp(line, "Save Type=", 10) == 0 && foundMatchingEntry)
		{
			currentromoptions.Save_Type = atoi(line + 10);
		}
		else if(strncmp(line, "Emulator=", 9) == 0 && foundMatchingEntry)
		{
			currentromoptions.Emulator = atoi(line + 9);
		}
		else if(strncmp(line, "Check Self-modifying Code=", 26) == 0 && foundMatchingEntry)
		{
			currentromoptions.Code_Check = atoi(line + 26);
			if(currentromoptions.Code_Check > CODE_CHECK_PROTECT_MEMORY) currentromoptions.Code_Check = CODE_CHECK_MEMORY_QWORD;
		}
		else if(strncmp(line, "Max FPS=", 8) == 0 && foundMatchingEntry)
		{
			currentromoptions.Max_FPS = atoi(line + 8);
		}
		else if(strncmp(line, "TLB=", 4) == 0 && foundMatchingEntry)
		{
			currentromoptions.Use_TLB = atoi(line + 4);
		}
		else if(strncmp(line, "EEPROM Size=", 12) == 0 && foundMatchingEntry)
		{
			currentromoptions.Eeprom_size = atoi(line + 12);
		}
		else if(strncmp(line, "Use Register Caching=", 21) == 0 && foundMatchingEntry)
		{
			currentromoptions.Use_Register_Caching = atoi(line + 21);
			if(currentromoptions.Use_Register_Caching > 2) currentromoptions.Use_Register_Caching = 0;
		}
		else if(strncmp(line, "Counter Factor=", 15) == 0 && foundMatchingEntry)
		{
			currentromoptions.Counter_Factor = atoi(line + 15);
			if(currentromoptions.Counter_Factor > 8) currentromoptions.Counter_Factor = 0;
		}
		else if(strncmp(line, "FPU Hack=", 9) == 0 && foundMatchingEntry)
		{
			currentromoptions.FPU_Hack = atoi(line + 9);
			if(currentromoptions.FPU_Hack > 2) currentromoptions.FPU_Hack = 0;
		}
		else if(strncmp(line, "DMA=", 4) == 0 && foundMatchingEntry)
		{
			currentromoptions.DMA_Segmentation = atoi(line + 4);
			if(currentromoptions.DMA_Segmentation > 2) currentromoptions.DMA_Segmentation = 0;
		}
		else if(strncmp(line, "Link 4KB Blocks=", 16) == 0 && foundMatchingEntry)
		{
			currentromoptions.Link_4KB_Blocks = atoi(line + 16);
			if(currentromoptions.Link_4KB_Blocks > 2) currentromoptions.Link_4KB_Blocks = 0;
		}
		else if(strncmp(line, "Advanced Block Analysis=", 24) == 0 && foundMatchingEntry)
		{
			currentromoptions.Advanced_Block_Analysis = atoi(line + 24);
			if(currentromoptions.Advanced_Block_Analysis > 2) currentromoptions.Advanced_Block_Analysis = 0;
		}
		else if(strncmp(line, "Assume 32bit=", 13) == 0 && foundMatchingEntry)
		{
			currentromoptions.Assume_32bit = atoi(line + 13);
			if(currentromoptions.Assume_32bit > 2) currentromoptions.Assume_32bit = 0;
		}
		else if(strncmp(line, "HLE=", 4) == 0 && foundMatchingEntry)
		{
			currentromoptions.Use_HLE = atoi(line + 4);
			if(currentromoptions.Use_HLE > 2) currentromoptions.Use_HLE = 0;
		}
		else
		{
			// we've found our matching entry and set its values
			// as current, now we can exit.
			if (foundMatchingEntry)
			{
				fclose(iniFile);
				return TRUE;
			}
		}
	}

	return FALSE;
}

/*
 =======================================================================================================================
    I should not need to write such a stupid function to convert String to Int £
    However, the sscanf() function does not work for me to input hex number from £
    input string. I spent some time to debug it, no use, so I wrote £
    this function to do the converting myself. £
    Someone could help me to elimiate this funciton
 =======================================================================================================================
 */
uint32 ConvertHexCharToInt(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	else if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
uint32 ConvertHexStringToInt(const char *str, int nchars)
{
	/*~~~~~~~~~~~~~~~*/
	int		i;
	uint32	result = 0;
	/*~~~~~~~~~~~~~~~*/

	for(i = 0; i < nchars; i++) result = result * 16 + ConvertHexCharToInt(str[i]);

	return result;
}

/*
 =======================================================================================================================
    Delete the ctrl-m, ctrl-r characters at the end of string
 =======================================================================================================================
 */
void chopm(char *str)
{
	/*~~*/
	int i;
	/*~~*/

	if (strlen(str) == 0)
		return;

	i = strlen(str);
	while(str[i] < ' ') /* is a ctrl character */
	{
		str[i] = '\0';

		if (i == 0)
			return;

		i--;
	}
}
