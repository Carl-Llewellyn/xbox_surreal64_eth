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

Modified for use with UltraHLE by oDD. Thanks StrmnNrmn!

*/

#include "Ultra.h"
#include "Controller.h"
#include <Input.h>
#include <stdlib.h>
#include "../config.h"

#define ANALOGUE_SENS 80

#define CONT_GET_STATUS      0x00
#define CONT_READ_CONTROLLER 0x01
#define CONT_READ_MEMPACK    0x02
#define CONT_WRITE_MEMPACK   0x03
#define CONT_READ_EEPROM     0x04
#define CONT_WRITE_EEPROM    0x05
#define CONT_RESET           0xff

// Controller errors
#define CONT_NO_RESPONSE_ERROR          0x8
#define CONT_OVERRUN_ERROR              0x4

// Controller type

#define CONT_ABSOLUTE           0x0001
#define CONT_RELATIVE           0x0002
#define CONT_JOYPORT            0x0004
#define CONT_EEPROM				0x8000

// Controller status

#define CONT_CARD_ON            0x01
#define CONT_CARD_PULL          0x02
#define CONT_ADDR_CRC_ER        0x04
#define CONT_EEPROM_BUSY		0x80

OSContPad g_Pads[4];
CONTROL Controls[4];


ConfigAppLoadTemp();//OK
ConfigAppLoad();//OK
ConfigAppLoad2();//OK

//freakdave - FIXME!!
//Problem: Changing Mempak/Rumblepak/Controllers in Submenu does NOT work
//Current Solution: Change BEFORE launching rom
BOOL g_bControllerPresent[4];
BOOL g_bMemPackPresent[4];

extern char g_szPathSaves[256];

static void Cont_Eeprom(byte * pbPIRAM);
static void Cont_LoadEeprom();
static void Cont_SaveEeprom();
static void Cont_LoadMempack();
static void Cont_SaveMempack();
static dword Cont_StatusEeprom(byte * pbPIRAM, dword i, dword iError, dword ucWrite, dword ucRead);

static dword Cont_ReadEeprom(byte * pbPIRAM, dword i, dword iError, dword ucWrite, dword ucRead);
static dword Cont_WriteEeprom(byte * pbPIRAM, dword i, dword iError, dword ucWrite, dword ucRead);
static dword Cont_ReadMemPack(byte * pbPIRAM, dword i, dword iError, dword iChannel, dword ucWrite, dword ucRead);
static dword Cont_WriteMemPack(byte * pbPIRAM, dword i, dword iError, dword iChannel, dword ucWrite, dword ucRead);

static dword Cont_Command(byte * pbPIRAM, dword i, dword iError, byte ucCmd, dword iChannel, dword ucWrite, dword ucRead);
static void Cont_InitMemPack();
static byte Cont_DataCrc(byte * pBuf);
static void Cont_IDCheckSum(word * pBuf, word * pPos, word * pNot);

static BOOL g_bEepromPresent = TRUE;
static byte * g_pEepromData = NULL;
static dword g_dwEepromSize = 2048;

static byte g_MemPack[4][(0x400+1) * 32];

static BOOL s_bEepromUsed = FALSE;
static BOOL s_bMempackUsed = FALSE;

//#define DEBUG_PAKS 1 // to print out console info
// there doesn't seem to be any attempt to read/write the mempaks

HRESULT Controller_Initialise()
{
	s_bEepromUsed = FALSE;
	s_bMempackUsed = FALSE;

	//g_dwEepromSize = g_ROM.dwEepromSize;
	g_pEepromData = new byte[g_dwEepromSize];
	if (g_pEepromData == NULL)
		return E_OUTOFMEMORY;


	_INPUT_InitiateControllers(NULL, Controls);

	//Ugly Controller/Mempak detection hack - freakdave
	g_bControllerPresent[0] = EnableController1;
	g_bControllerPresent[1] = EnableController2;
	g_bControllerPresent[2] = EnableController3;
	g_bControllerPresent[3] = EnableController4;

	if(DefaultPak == PLUGIN_MEMPAK){
	g_bMemPackPresent[0] = TRUE;//0 = NoPak, 1 = MemPak, 2 = RumblePak : Default = 2
	g_bMemPackPresent[1] = TRUE;
	g_bMemPackPresent[2] = TRUE;
	g_bMemPackPresent[3] = TRUE;
	
	//s_bMempackUsed = TRUE; // force a mempak to be written with null data
#ifdef DEBUG_PAKS
	OutputDebugString("MemPaks: Enabled\n");
#endif
	}else{
	g_bMemPackPresent[0] = FALSE;
	g_bMemPackPresent[1] = FALSE;
	g_bMemPackPresent[2] = FALSE;
	g_bMemPackPresent[3] = FALSE;
#ifdef DEBUG_PAKS
	OutputDebugString("MemPaks: Disabled\n");
#endif
	}

	Cont_InitMemPack();

	return S_OK;
}

void Controller_Finalise()
{
	if (s_bEepromUsed)
	{
		// Write Eeprom to rom directory
		Cont_SaveEeprom();
	}
	if (s_bMempackUsed)
	{
#ifdef DEBUG_PAKS
		OutputDebugString("Controller_Finalise:Cont_SaveMempack\n");
#endif
		// Write Mempak to rom directory
		Cont_SaveMempack();
	}

	if (g_pEepromData != NULL)
	{
		delete [] g_pEepromData;
		g_pEepromData = NULL;
	}
}

inline dword SwapEndian(dword x)
{
	return ((x >> 24)&0x000000FF) |
		   ((x >> 8 )&0x0000FF00) |
		   ((x << 8 )&0x00FF0000) |
		   ((x << 24)&0xFF000000);
}

void Cont_LoadEeprom()
{
	int i, j;
	LONG nToDo;
	TCHAR szEepromFileName[MAX_PATH+1];
	FILE * fh;
	byte b[2048];

	//Dump_GetSaveDirectory(szEepromFileName, g_ROM.szFileName, TEXT(".sav"));

	sprintf(szEepromFileName, "%s%08X\\%08X-%08X-%02X.eep", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country);

	//MessageBox(NULL, szEepromFileName, "Loading EEPROM from", MB_OK);

	//DBGConsole_Msg(0, "Loading eeprom from [C%s]", szEepromFileName);

	fh = fopen(szEepromFileName, "rb");
	if (fh != NULL)
	{
		// Read in chunks of 2KB
		for (i = 0; i < g_dwEepromSize; )
		{
			nToDo = min(g_dwEepromSize - i, 2048);

			fread(b, nToDo, 1, fh);

			for (j = 0; j < nToDo; j++)
			{
				g_pEepromData[i+j] = b[j^0x3];
			}

			i += nToDo;
		}

		fclose(fh);
	}
}


void Cont_SaveEeprom()
{
	int i, j;
	LONG nToDo;
	TCHAR szEepromFileName[MAX_PATH+1];
	FILE * fh;
	byte b[2048];

	sprintf(szEepromFileName, "%s%08X\\%08X-%08X-%02X.eep", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country);

	//MessageBox(NULL, szEepromFileName, "Saving EEPROM to", MB_OK);

	//Dump_GetSaveDirectory(szEepromFileName, g_ROM.szFileName, TEXT(".sav"));

	//DBGConsole_Msg(0, "Saving eeprom to [C%s]", szEepromFileName);

	fh = fopen(szEepromFileName, "wb");
	if (fh != NULL)
	{
		for (i = 0; i < g_dwEepromSize; )
		{
			nToDo = min(g_dwEepromSize - i, 2048);

			for(j = 0; j < nToDo; j++)
			{
				b[j] = g_pEepromData[(i+j)^0x3];
			}

			fwrite(b, nToDo, 1, fh);

			i += nToDo;
		}


		fclose(fh);
	}
}

void Cont_SaveMempack()
{
	TCHAR szMempackFileName[MAX_PATH+1];
	FILE * fp;
	
	//Dump_GetSaveDirectory(szMempackFileName, g_ROM.szFileName, TEXT(".mpk"));

	//DBGConsole_Msg(0, "Saving mempack to [C%s]", szMempackFileName);

	sprintf(szMempackFileName, "%s%08X\\%08X-%08X-%02X.mpk", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country);

	fp = fopen(szMempackFileName, "wb");
	if (fp != NULL)
	{
#ifdef DEBUG_PAKS
		OutputDebugString("Cont_SaveMempack: ");
		OutputDebugString(szMempackFileName);
		OutputDebugString("\n");
#endif
		// Don't do last 32 bytes (like nemu)
		fwrite(&g_MemPack[0][0], (0x400) * 32, 1, fp);
		fclose(fp);
	}
#ifdef DEBUG_PAKS
	else
	{
		OutputDebugString("Cont_SaveMempack: Failed to save: ");
		OutputDebugString(szMempackFileName);
		OutputDebugString("\n");
	}
#endif
}

void Cont_LoadMempack()
{
	TCHAR szMempackFileName[MAX_PATH+1];
	FILE * fp;

	//Dump_GetSaveDirectory(szMempackFileName, g_ROM.szFileName, TEXT(".mpk"));

	//DBGConsole_Msg(0, "Loading mempack from [C%s]", szMempackFileName);

	sprintf(szMempackFileName, "%s%08X\\%08X-%08X-%02X.mpk", g_szPathSaves, cart.crc1, cart.crc1, cart.crc2, cart.country);

	fp = fopen(szMempackFileName, "rb");
	if (fp != NULL)
	{
#ifdef DEBUG_PAKS
		OutputDebugString("Cont_LoadMempack: ");
		OutputDebugString(szMempackFileName);
		OutputDebugString("\n");
#endif
		// Don't do last 32 bytes (like nemu)
		fread(&g_MemPack[0][0], (0x400) * 32, 1, fp);
		fclose(fp);
	}
#ifdef DEBUG_PAKS
	else
	{
		OutputDebugString("Cont_LoadMempack: Failed to load: ");
		OutputDebugString(szMempackFileName);
		OutputDebugString("\n");
	}
#endif
}

void Controller_Check(void)
{
	byte * pbPIRAM = (byte *)RPIF;
	BOOL bDone;
	dword i;
	dword iError;
	dword iChannel;
	
	iChannel = 0;
	while (iChannel < 6 && pbPIRAM[iChannel ^ 0x3] == 0)
	{
		iChannel++;
	}

	// Clear to indicate success - we might set this again in the handler code
	pbPIRAM[63 ^ 0x3] = 0x00;

	for (int j = 0; j < 4; j++)
	{
		BUTTONS btn;
		_INPUT_GetKeys(j, &btn);
		memcpy(&g_Pads[j], &btn, sizeof(DWORD));
	}

	i = iChannel;
	bDone = FALSE;
	do
	{
		byte ucCode;
		byte ucWrite;
		byte ucRead;
		byte ucCmd;
		ucCode = pbPIRAM[(i + 0) ^ 0x3];

		switch (ucCode)
		{
		// Fill/Padding?
		case 0x00: case 0xFF:
			//dwPrevious = ucCode;
			i++;
			break;

		// Done
		case 0xFE:
			bDone = TRUE;
			//DPF(DEBUG_MEMORY_PIF, "Controller: Code 0x%02x - Finished", ucCode);
			break;

		default:
			// Assume this is the Write value - code is 0x00, read is next value
			ucWrite = ucCode;
			ucRead = pbPIRAM[(i + 1) ^ 0x3];

			// Set up error pointer and skip read/write bytes of input
			iError = i + 1;
			i += 2;

			if (ucWrite < 1)
			{
				//DBGConsole_Msg(0, "Controller: 0 bytes of write input - no command!");
				bDone = TRUE;
			}
			else
			{
				// Read command
				ucCmd = pbPIRAM[(i + 0) ^ 0x3];
				i++;
				ucWrite--;
				
				//DPF(DEBUG_MEMORY_PIF, "Controller: Code 0x%02x, Write 0x%02x, Read 0x%02x", ucCode, ucWrite, ucRead);
				
				// when this is working right, ucCmd should be 2 or 3 when reading or writing to mempaks
				// perhaps that data isn't in pbPIRAM? or it's not being retrieved correctly?
				i = Cont_Command(pbPIRAM, i, iError, ucCmd, iChannel, ucWrite, ucRead);
			}
			break;
		}

	} while (i < 64 && !bDone);

	if (s_bEepromUsed)
	{
		// Write Eeprom to rom directory
		Cont_SaveEeprom();
		s_bEepromUsed = FALSE;
	}
	if (s_bMempackUsed)
	{
#ifdef DEBUG_PAKS
		// if Cont_Command was reading/writing to mempaks, this would be true, but the command is never triggered
		// if you set s_bMempackUsed to true on init (force write), this will trigger and write out all nulls
		OutputDebugString("Controller_Check:Cont_SaveMempack\n");
#endif
		// Write Eeprom to rom directory
		Cont_SaveMempack();
		s_bMempackUsed = FALSE;
	}
}

static void Cont_PIRAMWrite4BitsHi(byte * pbPIRAM, dword i, byte val)
{
	pbPIRAM[(i + 0) ^ 0x3] &= 0x0F;
	pbPIRAM[(i + 0) ^ 0x3] |= (val<<4);
}

static void Cont_PIRAMWrite8Bits(byte * pbPIRAM, dword i, byte val)
{
	pbPIRAM[(i + 0) ^ 0x3] = val;
}

static void Cont_PIRAMWrite16Bits(byte * pbPIRAM, dword i, word val)
{
	pbPIRAM[(i + 0) ^ 0x3] = (byte)(val   );	// Lo
	pbPIRAM[(i + 1) ^ 0x3] = (byte)(val>>8);	// Hi
}
static void Cont_PIRAMWrite16Bits_Swapped(byte * pbPIRAM, dword i, word val)
{
	pbPIRAM[(i + 0) ^ 0x3] = (byte)(val>>8);	// Hi
	pbPIRAM[(i + 1) ^ 0x3] = (byte)(val   );	// Lo
}


// i points to start of command
dword Cont_Command(byte * pbPIRAM, dword i, dword iError, byte ucCmd, dword iChannel, dword ucWrite, dword ucRead)
{
	dword dwController;
	dword dwRetVal;

	// Figure this out from the current offset...hack
	dwController = i/8;

	// i Currently points to data to write to
	
#ifdef DEBUG_PAKS
	if (ucCmd != 1) { // too much console spam, 1=normal op
	// 0, 1 and 4 are ok - but where's 2 (read mempak) and 3 (write mempak)?
	char dbgcmd[256];
	sprintf(dbgcmd, "Cont_Command: i=%X, iError=%X, ucCmd=%X, iChannel=%X, ucWrite=%X, ucRead=%X\n", i, iError, ucCmd, iChannel, ucWrite, ucRead);
	OutputDebugString(dbgcmd);
	}
#endif

	switch (ucCmd)
	{
	case CONT_GET_STATUS: //0x00		// Status
		if (iChannel == 0)
		{
			//DPF(DEBUG_MEMORY_PIF, "Controller: Executing GET_STATUS");
			// This is controller status
			if (g_bControllerPresent[dwController])
			{
				if (ucRead > 3)
				{
					// Transfer error...
					Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_OVERRUN_ERROR);
				}
				else
				{

					Cont_PIRAMWrite16Bits(pbPIRAM, i, CONT_ABSOLUTE|CONT_JOYPORT);

					if (g_bMemPackPresent[dwController])
						Cont_PIRAMWrite8Bits(pbPIRAM, i+2, CONT_CARD_ON);	// Is the mempack plugged in?
					else
						Cont_PIRAMWrite8Bits(pbPIRAM, i+2, CONT_CARD_PULL);	// Is the mempack plugged in?
				}
				
			} else {
				// Not connected
				Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_NO_RESPONSE_ERROR);
			}
			i += ucWrite + ucRead;	
		}
		else if (iChannel == 4)
		{
		   // This is eeprom status?
			//DPF(DEBUG_MEMORY_PIF, "Controller: Executing GET_EEPROM_STATUS?");

			dwRetVal = Cont_StatusEeprom(pbPIRAM, i, iError, ucWrite, ucRead);
			if (dwRetVal == ~0)
				i = 63;
			else
				i = dwRetVal;

		}
		else
		{
			//DPF(DEBUG_MEMORY_PIF, "Controller: UnkStatus, Channel = %d", iChannel);
			//DBGConsole_Msg(0, "UnkStatus, Channel = %d", iChannel);
			i += ucWrite + ucRead;	
		}	
		break;


	case CONT_READ_CONTROLLER: //0x01		// Controller
		{
			//DPF(DEBUG_MEMORY_PIF, "Controller: Executing READ_CONTROLLER");
			// This is controller status
			if (g_bControllerPresent[dwController])
			{
				if (ucRead > 4)
				{
					// Transfer error...
					Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_OVERRUN_ERROR);
				}
				else
				{
					// Hack - we need to only write the number of bytes asked for!
					Cont_PIRAMWrite16Bits(pbPIRAM, i, g_Pads[dwController].button);
					Cont_PIRAMWrite8Bits(pbPIRAM, i+2, g_Pads[dwController].stick_x);
					Cont_PIRAMWrite8Bits(pbPIRAM, i+3, g_Pads[dwController].stick_y);
				}
				
			} else {
				// Not connected			
				Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_NO_RESPONSE_ERROR);
			}
		}
		i += ucWrite + ucRead;		

		break;
	case CONT_READ_MEMPACK: //0x02 - this isn't getting triggered
		{
#ifdef DEBUG_PAKS
			OutputDebugString("CONT_READ_MEMPACK\n");
#endif
			//DPF(DEBUG_MEMORY_PIF, "Controller: Command is READ_MEMPACK");
			if (g_bControllerPresent[iChannel])
			{
#ifdef DEBUG_PAKS
				char dbg[256];
				sprintf(dbg, "Cont_Command:CONT_READ_MEMPACK: (%i)\n", iChannel);
				OutputDebugString(dbg);
#endif
				dwRetVal = Cont_ReadMemPack(pbPIRAM, i, iError, iChannel, ucWrite, ucRead);
				if (dwRetVal == ~0)
					i = 63;
				else
					i = dwRetVal;
#ifdef DEBUG_PAKS
				sprintf(dbg, "Returned: (%i)\n", i);
				OutputDebugString(dbg);
#endif
			}
			else
			{
				Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_NO_RESPONSE_ERROR);
				i += ucWrite + ucRead;
			}
		}
		break;
	case CONT_WRITE_MEMPACK: //0x03 - this isn't getting triggered
		{
#ifdef DEBUG_PAKS
			OutputDebugString("CONT_WRITE_MEMPACK\n");
#endif
			//DPF(DEBUG_MEMORY_PIF, "Controller: Command is WRITE_MEMPACK");
			if (g_bControllerPresent[iChannel])
			{
#ifdef DEBUG_PAKS
				char dbg[256];
				sprintf(dbg, "Cont_Command:CONT_WRITE_MEMPACK: (%i)\n", iChannel);
				OutputDebugString(dbg);
#endif
				dwRetVal = Cont_WriteMemPack(pbPIRAM, i, iError, iChannel, ucWrite, ucRead);
				if (dwRetVal == ~0)
					i = 63;
				else
					i = dwRetVal;
#ifdef DEBUG_PAKS
				sprintf(dbg, "Returned: (%i)\n", i);
				OutputDebugString(dbg);
#endif
			}
			else
			{
				Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_NO_RESPONSE_ERROR);
				i += ucWrite + ucRead;
			}
		}
		break;
		

	case CONT_READ_EEPROM: //0x04
		dwRetVal = Cont_ReadEeprom(pbPIRAM, i, iError, ucWrite, ucRead);
		if (dwRetVal == ~0)
			i = 63;
		else
			i = dwRetVal;
		break;
	case CONT_WRITE_EEPROM: //0x05
		dwRetVal = Cont_WriteEeprom(pbPIRAM, i, iError, ucWrite, ucRead);
		if (dwRetVal == ~0)
			i = 63;
		else
			i = dwRetVal;
		break;

	case CONT_RESET: //0xff

		//DPF(DEBUG_MEMORY_PIF, "Controller: Command is RESET");
		i += ucWrite + ucRead;
		break;

	default:
		//DBGConsole_Msg(DEBUG_MEMORY_PIF, "Controller: UnkCommand is %d", ucCmd);
		//DPF(DEBUG_MEMORY_PIF, "Controller: UnkCommand is %d", ucCmd);
		//DPF(DEBUG_MEMORY_PIF, "Controller: i is now %d", i);
		//pbPIRAM[iError ^ 0x3] |= ((/*CONT_OVERRUN_ERROR|*/CONT_NO_RESPONSE_ERROR) << 4);
		{
			for (dword j = 0; j < ucRead; j++)
			{
				if (i + j < 64)
					pbPIRAM[(i + j) ^ 0x3] = 0x00;
			}
			//CPU_Halt("Controller UnkCommand");
		}
		//pbPIRAM[63 ^ 0x3] = 0x01;
		//DPF(DEBUG_MEMORY_PIF, "Controller: next byte is at %d / %d", i+ucRead, pbPIRAM[(i + ucRead) ^ 0x3]);

		i += ucWrite + ucRead;
		break;

	}

	return i;
}



// i points to start of command
dword Cont_StatusEeprom(byte *pbPIRAM, dword i, dword iError, dword ucWrite, dword ucRead)
{

	//DPF(DEBUG_MEMORY_PIF, "Controller: GetStatusEEPROM");

	if (ucWrite != 0 || ucRead > 4)
	{
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_OVERRUN_ERROR);
		//DBGConsole_Msg(0, "GetEepromStatus Overflow");
		return ~0;
	}

	if (g_bEepromPresent)
	{
		Cont_PIRAMWrite16Bits(pbPIRAM, i, CONT_EEPROM);
		Cont_PIRAMWrite8Bits(pbPIRAM, i+2, 0x00);

		i += 3;
		ucRead -= 3;
	}
	else
	{
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_NO_RESPONSE_ERROR);
	}

	//if (ucWrite > 0 || ucRead > 0)
	//{
		//DBGConsole_Msg(0, "GetEepromStatus Read / Write bytes remaining");
	//}

	i += ucWrite + ucRead;
	return i;

}

// Returns FALSE if we should stop 
dword Cont_ReadEeprom(byte * pbPIRAM, dword i, dword iError, dword ucWrite, dword ucRead)
{
	byte block;

	//DPF(DEBUG_MEMORY_PIF, "Controller: ReadEEPROM");

	if (!s_bEepromUsed)
	{
		Cont_LoadEeprom();
		s_bEepromUsed = TRUE;
	}

	if (ucWrite != 1 || ucRead > 8)
	{
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_OVERRUN_ERROR);
		//DBGConsole_Msg(0, "ReadEeprom Overflow");
		return ~0;
	}

	// Read the block 
	block = pbPIRAM[(i + 0) ^ 0x3];
	i++;
	ucWrite--;

	// TODO limit block to g_dwEepromSize / 8
	
	if (g_bEepromPresent)
	{
		byte j;

		j = 0;
		while (ucRead)
		{
			pbPIRAM[i ^ 0x3] = g_pEepromData[(block*8 + j) ^ 0x3];

			i++;
			j++;
			ucRead--;
		}
	}
	else
	{
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_NO_RESPONSE_ERROR);
	}

	/*if (ucWrite > 0 || ucRead > 0)
	{
		DBGConsole_Msg(0, "ReadEeprom Read / Write bytes remaining");
	}*/

	i += ucWrite + ucRead;
	return i;
}



// Returns FALSE if we should stop 
dword Cont_WriteEeprom(byte * pbPIRAM, dword i, dword iError, dword ucWrite, dword ucRead)
{
	dword j;
	byte block;

	//DPF(DEBUG_MEMORY_PIF, "Controller: WriteEEPROM");

	s_bEepromUsed = TRUE;

	// 9 bytes of input remaining - 8 bytes data + block
	if (ucWrite != 9 /*|| ucRead != 1*/)
	{
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_OVERRUN_ERROR);
		//DBGConsole_Msg(0, "WriteEeprom Overflow");
		return ~0;
	}

	// Read the block 
	block = pbPIRAM[(i + 0) ^ 0x3];
	i++;
	ucWrite--;

	// TODO limit block to g_dwEepromSize / 8

	if (g_bEepromPresent)
	{
		j = 0;
		while (ucWrite)
		{
			g_pEepromData[(block*8 + j) ^ 0x3] = pbPIRAM[i ^ 0x3];

			i++;
			j++;
			ucWrite--;

		}
	}
	else
	{
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_NO_RESPONSE_ERROR);
	}

	/*if (ucWrite > 0 || ucRead > 0)
	{
		DBGConsole_Msg(0, "WriteEeprom Read / Write bytes remaining");
	}*/

	i += ucWrite + ucRead;
	return i;
}


// Returns new position to continue reading
// i is the address of the first write info (after command itself)
dword Cont_ReadMemPack(byte * pbPIRAM, dword i, dword iError, dword iChannel, dword ucWrite, dword ucRead)
{
	dword j;
	dword dwAddressCrc;
	dword dwAddress;
	dword dwCRC;
	byte ucDataCRC;
	byte * pBuf;

#ifdef DEBUG_PAKS
	char dbg[256];
	sprintf(dbg, "Cont_ReadMemPack: (%i)\n", iChannel);
	OutputDebugString(dbg);
#endif
	
	if (!s_bMempackUsed)
	{
#ifdef DEBUG_PAKS
		OutputDebugString("Loading...\n");
#endif
		Cont_LoadMempack();
		s_bMempackUsed = TRUE;
	}
	
	// There must be exactly 2 bytes to write, and 33 bytes to read
	if (ucWrite != 2 || ucRead != 33)
	{
		// TRANSFER ERROR!!!!
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_OVERRUN_ERROR);
		return ~0;
	}

	//DPF(DEBUG_MEMORY_PIF, "ReadMemPack: Channel %d, i is %d", iChannel, i);

	// Get address..
	dwAddressCrc = (pbPIRAM[(i + 0) ^ 0x3]<< 8) |
				   (pbPIRAM[(i + 1) ^ 0x3]);

	dwAddress = (dwAddressCrc >> 5);
	dwCRC = (dwAddressCrc & 0x1f);
	i += 2;	
	ucWrite -= 2;

	if (dwAddress > 0x400)
	{
		//DBGConsole_Msg(0, "Attempting to read from non-existing block 0x%08x", dwAddress);
		// SOME OTHER ERROR!
		//DBGConsole_Msg(0, "ReadMemPack: Address out of range: 0x%08x", dwAddress);
		return ~0;
	}

	pBuf = &g_MemPack[iChannel][dwAddress * 32];


	//DPF(DEBUG_MEMORY_PIF, "Controller: Reading from block 0x%04x (crc: 0x%02x)", dwAddress, dwCRC);
	
	for (j = 0; j < 32; j++)
	{
		if (i < 64)
		{
			// Here we would really read from the "mempack"
			pbPIRAM[i ^ 0x3] = pBuf[j];
		}
		i++;
		ucRead--;
	}

	// We would really generate a CRC on the mempack data
	ucDataCRC = Cont_DataCrc(pBuf);
	
	//DPF(DEBUG_MEMORY_PIF, "Controller: data crc is 0x%02x", ucDataCRC);

	// Write the crc value:
	pbPIRAM[i ^ 0x3] = ucDataCRC;
	i++;
	ucRead--;
	
	//DPF(DEBUG_MEMORY_PIF, "Returning, setting i to %d", i + 1);

	// With wetrix, there is still a padding byte?
	//if (ucWrite > 0 || ucRead > 0)
	//{
	//	DBGConsole_Msg(0, "ReadMemPack / Write bytes remaining");
	//}


	return i;
}


// Returns new position to continue reading
// i is the address of the first write info (after command itself)
dword Cont_WriteMemPack(byte * pbPIRAM, dword i, dword iError,  dword iChannel, dword ucWrite, dword ucRead)
{
	dword j;
	dword dwAddressCrc;
	dword dwAddress;
	dword dwCRC;
	byte ucDataCRC;
	byte * pBuf;
	
#ifdef DEBUG_PAKS
	char dbg[256];
	sprintf(dbg, "Cont_WriteMemPack: (%i)\n", iChannel);
	OutputDebugString(dbg);
#endif

	if (!s_bMempackUsed)
	{
#ifdef DEBUG_PAKS
		OutputDebugString("Loading...\n");
#endif
		Cont_LoadMempack();
		s_bMempackUsed = TRUE;
	}
	
	// There must be exactly 32+2 bytes to read

	if (ucWrite != 34 || ucRead != 1)
	{
		Cont_PIRAMWrite4BitsHi(pbPIRAM, iError, CONT_OVERRUN_ERROR);
		return ~0;
	}

	//DPF(DEBUG_MEMORY_PIF, "WriteMemPack: Channel %d, i is %d", iChannel, i);

	// Get address..
	dwAddressCrc = (pbPIRAM[(i + 0) ^ 0x3]<< 8) |
		           (pbPIRAM[(i + 1) ^ 0x3]);

	dwAddress = (dwAddressCrc >>5);
	dwCRC = (dwAddressCrc & 0x1f);
	i += 2;	
	ucWrite -= 2;
	
	if (dwAddress > 0x400)
	{
		// Starfox does this
		//DBGConsole_Msg(0, "Attempting to write to non-existing block 0x%08x", dwAddress);
		return ~0;
	}

	pBuf = &g_MemPack[iChannel][dwAddress * 32];

	//DPF(DEBUG_MEMORY_PIF, "Controller: Writing block 0x%04x (crc: 0x%02x)", dwAddress, dwCRC);
	

	for (j = 0; j < 32; j++)
	{
		if (i < 64)
		{
			// Here we would really write to the "mempack"
			pBuf[j] = pbPIRAM[i ^ 0x3];
		}
		i++;
		ucWrite--;
	}
	
	// We would really generate a CRC on the mempack data
	ucDataCRC = Cont_DataCrc(pBuf);
	
	//DPF(DEBUG_MEMORY_PIF, "Controller: data crc is 0x%02x", ucDataCRC);

	// Write the crc value:
	pbPIRAM[i ^ 0x3] = ucDataCRC;
	i++;
	ucRead--;
	
	// With wetrix, there is still a padding byte?
	//if (ucWrite > 0 || ucRead > 0)
	//{
		//DBGConsole_Msg(0, "WriteMemPack / Write bytes remaining");
	//}
	return i;
}

byte Cont_DataCrc(byte * pBuf)
{
	byte c;
	byte x,s;
	byte i;
	char z;

	c = 0;
	for (i = 0; i < 33; i++)
	{
		s = pBuf[i];

		for (z = 7; z >= 0; z--)
		{		
			if (c & 0x80)
				x = 0x85;
			else
				x = 0;

			c <<= 1;

			if (i < 32)
			{
				if (s & (1<<z))
					c |= 1;
			}

			c = c ^ x;
		}
	}

	return c;
}

void Cont_IDCheckSum(word * pBuf, word * pPos, word * pNot)
{
	word wPos = 0;
	word wNot = 0;

	for (dword i = 0; i < 14; i++)
	{
		wPos += pBuf[i];
		wNot += (~pBuf[i]);
	}

	*pPos = wPos;
	*pNot = wNot;
}

void Cont_InitMemPack()
{

	dword dwAddress;
	dword iChannel;

	for (iChannel = 0; iChannel < 4; iChannel++)
	{
		for (dwAddress = 0; dwAddress < 0x0400; dwAddress++)
		{
			byte * pBuf = &g_MemPack[iChannel][dwAddress * 32];

			// Clear values
			memset(pBuf, 0, 32);

			// Generate checksum if necessary
			if (dwAddress == 3 || dwAddress == 4 || dwAddress == 6)
			{
				word wPos, wNot;
				word * pwBuf = (word *)pBuf;

				Cont_IDCheckSum(pwBuf, &wPos, &wNot);

				pwBuf[14] = (wPos >> 8) | (wPos << 8);
				pwBuf[15] = (wNot >> 8) | (wNot << 8);

				//DPF(DEBUG_MEMORY_PIF, "Hacking ID Values: 0x%04x, 0x%04x", wPos, wNot);	
			}
		}
	}

}



