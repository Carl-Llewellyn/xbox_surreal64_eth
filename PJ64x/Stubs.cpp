
// NOT USED!!!

#include <xtl.h>
#include <stdio.h>
//#include "XBFont.h"
#include "main.h"
#include "IOSupport.h"
#include "stubs.h"
 
#define PAGE_SIZE		0x10000		// 64k pages
#define NO_FRAMES		16			// 64k*128 = 8mb of memory

#define PAGE_NOT_IN_MEMORY	(NO_FRAMES+1)
#define FRAME_FREE			0xFFFFFFFF
#define NEVER				0x0
#define NONE_FOUND			0xFFFFFFFF		

// Frames
// the 8meg memory block is made up of 128 frames, each able to hold 64k
// the frame table index is the same that index's memory, i wanted to keep
// memory sequential so i kept them seperate
// the frame table stores the page number of the page its storing in memory
// and the last time the page was accessed
typedef struct
{
	LARGE_INTEGER	lastUsed;
	uint32			pageNum;
} Frame;

Frame g_frameTable[NO_FRAMES];

// Memory
uint8 g_memory[PAGE_SIZE*NO_FRAMES];


#define SHIFTER1_READ	2				/* Shifts off insignificant bits from memory_read_functions array size. The
										 * significant bits are 0xFFFC0000 (14 significant bits) because Check_LW needs
										 * to check SPREG_1, which is at 0xA4040000. So we only need 14bit instead of
										 * 16bit. */
#define SHIFTER1_WRITE	0				/* Shifts off insignificant bits from memory write functions array size. Set to
										 * zero because of protected memory in 0x1000 blocks. = All bits are
										 * significant. */

#define SHIFTER2_READ	(16 + SHIFTER1_READ)
#define SHIFTER2_WRITE	(12 + SHIFTER1_WRITE)

// Page table
// is your standard page table. each entry in the array contains a number indicating
// the frame that stores its page in memory
uint8 *g_pageTable;
uint32 g_pageTableSize;

uint32 g_pageFunctionHits;
uint32 g_memFunctionHits;
uint32 g_pageHits;
uint32 g_pageMisses;
uint32 g_dynaHits;

// Rom file
// the original rom file is loaded, byte swapped and written to a temporary file pointed
// to below. the file is open for the duration of play and seeked and read when needed.

FILE	*g_temporaryRomFile = NULL;
char    g_temporaryRomPath[260];


BOOL	Is_Reading_Rom_File = FALSE;
BOOL	To_Stop_Reading_Rom_File = FALSE;

extern "C" DWORD RomFileSize;
extern "C" char RomHeader[64];
extern "C" char RomName[MAX_PATH+1];

/*
extern LPDIRECT3DDEVICE8 g_pD3DDev;
extern LPDIRECT3D8	m_pD3D;

CXBFont m_Font;			// Font object	;
*/

BOOL EnableMenuItem(HMENU hMenu, UINT uIDEnableItem, UINT uEnable)
{

	return TRUE;

}

HMENU GetSubMenu(HMENU hMenu, int nPos)
{
	return (HMENU)1;

}

BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode)
{
	ExitThread(dwExitCode);
}

LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	return 0;
}

BOOL SetMenuItemInfo(HMENU, UINT, BOOL, LPMENUITEMINFO)
{
	return TRUE;

}

BOOL PeekMessage(PMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{

	return TRUE;

}

BOOL MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{

	return 0;

}

 

int GetWindowText(HWND hWnd, LPTSTR lpString, int nMaxCount)
{
	return 1;
}

HMENU GetMenu(HWND hWnd)
{

	return 0;

}

BOOL SetWindowText(HWND hWnd, LPCTSTR lpString)
{
	return TRUE;
}

void GenerateTimerResults (void)
{


}

void MountDrives(void)
{
	 CIoSupport pIOSupport;
	 pIOSupport.Mount("A:","cdrom0");
	 pIOSupport.Mount("F:","Harddisk0\\Partition6");
	 pIOSupport.Mount("E:","Harddisk0\\Partition1");

}

void UnMountDrives(void)
{
	CIoSupport pIOSupport;
	pIOSupport.Unmount("A:");
	pIOSupport.Unmount("F:");
	pIOSupport.Unmount("E:");

}
 


BOOL ByteSwap2(uint32 Size, uint8 *Image, uint8 bswaptype)
{
	/*~~~~~~*/
	uint32	k;
	/*~~~~~~*/

	switch (bswaptype)
	{
		case 1:
		{
			for(k = 0; k < Size; k += 4)
			{
				/* Converts 40123780 to 0x80371240 */
				_asm
				{
					mov edx, Image
					add edx, k
					mov eax, dword ptr[edx]
					bswap eax
					mov dword ptr[edx], eax
				}
			}

			return TRUE;
		}
		case 2:
		{
			/* Converts 0x12408037 to 0x80371240 */
			k = Size;
			_asm
			{
				mov edx, Image
				mov ebx, dword ptr[k]
				jmp _LABEL3
			}

			_LABEL2:
			_asm
			{
				sub ebx, 8
				test ebx, ebx
				jz _LABELEXIT
			}

			_LABEL3:
			_asm
			{
				/* Yup i copied this asm routine twice..to cut down on the looping by 50% */
				mov eax, dword ptr[edx]
				mov ecx, eax
				shr ecx, 0x10
				shl eax, 0x10
				or ecx, eax
				mov dword ptr[edx], ecx
				add edx, 4

				mov eax, dword ptr[edx]
				mov ecx, eax
				shr ecx, 0x10
				shl eax, 0x10
				or ecx, eax
				mov dword ptr[edx], ecx
				add edx, 4
				jmp _LABEL2
			}

			_LABELEXIT: ;
			return TRUE;
		}
		case 0:
			return TRUE;
		default:
			return FALSE;
	}
}


BOOL InitVirtualRomData(char *rompath)
{
	FILE *originalFile = NULL;
	FILE *temporaryFile = NULL;
	char tempPath[_MAX_PATH];

	// check to see if the rom is zipped
	//if(stricmp(&rompath[strlen(rompath) - 4], ".zip") == 0)
	//{
	//	return InitVirtualZippedRomData(rompath);
	//}

	// open the rom
	originalFile = fopen(rompath, "rb");

	if(originalFile == NULL)
	{
		 
		return FALSE;
	}

	strcpy(tempPath, "T:\\PJTemporaryRom.dat");

	// open the temporary file for writing
	temporaryFile = fopen(tempPath, "wb");

	if(temporaryFile == NULL)
	{
 
		return FALSE;
	}

	// get the size of the original rom file
	rewind(originalFile);
	fseek(originalFile, 0, SEEK_END);
	RomFileSize = ftell(originalFile);
	//gAllocationLength = g_romSize;

	// point back to the start of the file
	fseek(originalFile, 0, SEEK_SET);

	// start reading
	{
		uint32  i;
		uint8	*bswapBuf;
		uint8	bswapType = 0;
 
		Is_Reading_Rom_File = TRUE;
		To_Stop_Reading_Rom_File = FALSE;

		// allocate 1 meg for bswapping and storing
		bswapBuf = (uint8 *)malloc(0x100000);

		// load the rom 1 meg at a time, byteswapping that 1meg and then storing it
		// in our temporary file
		for(i = 0; i < RomFileSize && To_Stop_Reading_Rom_File == FALSE; i += 0x100000)
		{
			 

			if (To_Stop_Reading_Rom_File == TRUE)
			{
				CloseROM();
				To_Stop_Reading_Rom_File = FALSE;
				Is_Reading_Rom_File = FALSE;

				free(bswapBuf);

				fclose(originalFile);
				fclose(temporaryFile);
			
				return FALSE;
			}

			// seek to the required position and read in 1 meg
			fseek(originalFile, i, SEEK_SET);
			fread(bswapBuf, sizeof(uint8), 0x100000, originalFile);

			// check how the bytes in the rom are organised so we know what
			// to use to byte swap each meg
			if (i == 0)
			{
				if(bswapBuf[0] == 0x80 && bswapBuf[1] == 0x37)
				{
					bswapType = 1;
				}
				else if (bswapBuf[0] == 0x37 && bswapBuf[1] == 0x80)
				{
					bswapType = 2;
				}
				else if(bswapBuf[2] == 0x37 && bswapBuf[3] == 0x80)
				{
					bswapType = 0;
				}
				else
				{

					CloseROM();
					To_Stop_Reading_Rom_File = FALSE;
					Is_Reading_Rom_File = FALSE;

					free(bswapBuf);

					fclose(originalFile);
					fclose(temporaryFile);

					return FALSE;
				}
			}

			// byte swap the meg
			ByteSwap2(0x100000, bswapBuf, bswapType);

			// write it out to the temporary file
			fwrite(bswapBuf, sizeof(uint8), 0x100000, temporaryFile);
		}

		free(bswapBuf);

		Is_Reading_Rom_File = FALSE;
	}

	fclose(originalFile);
	fclose(temporaryFile);

	// open the temporary file again for reading
	temporaryFile = fopen(tempPath, "rb");
	
	// read in the rom header
	fseek(temporaryFile, 0, SEEK_SET);
	fread((uint8 *) &RomHeader, sizeof(uint8), 0x40, temporaryFile);
	//SwapRomHeader((uint8 *) &RomHeader);

	// read in boot code to SP_DMEM
	//fseek(temporaryFile, 0, SEEK_SET);
	//fread((uint8 *) &SP_DMEM, sizeof(uint8), 0x1000, temporaryFile);
	
	// read in rom name
	fseek(temporaryFile, 0x20, SEEK_SET);
	fread(RomName, sizeof(uint8), 20, temporaryFile);
	//SwapRomName((unsigned char*)RomName);

	fclose(temporaryFile);

	strcpy(g_temporaryRomPath, tempPath);
	
	return TRUE;
}
 

void CloseROM(void)
{

}


void SwapRomName(uint8 *name)
{
	/*~~~~~~*/
	int		i;
	uint8	c;
	/*~~~~~~*/

	for(i = 0; i < 20; i += 4)
	{
		c = name[i];
		name[i] = name[i + 3];
		name[i + 3] = c;

		c = name[i + 2];
		name[i + 2] = name[i + 1];
		name[i + 1] = c;
	}

	for(i = 19; i >= 0; i--)
	{
		if(name[i] != ' ') break;
	}

	name[i + 1] = '\0';

	/*
	 * for( ; i>=0; i-- ) £
	 * { £
	 * if( name[i] == ':' ) £
	 * name[i] = '-'; £
	 * }
	 */
}

void SwapRomHeader(uint8 *romheader)
{
	/*~~~~~~*/
	int		i;
	uint8	c;
	/*~~~~~~*/

	for(i = 0; i < 0x40; i += 4)
	{
		c = romheader[i];
		romheader[i] = romheader[i + 3];
		romheader[i + 3] = c;

		c = romheader[i + 2];
		romheader[i + 2] = romheader[i + 1];
		romheader[i + 1] = c;
	}
}


void LoadVirtualRomData()
{
	g_temporaryRomFile = fopen(g_temporaryRomPath, "rb");
}

void CloseVirtualRomData()
{
	if (g_temporaryRomFile != NULL)
	{
		fclose(g_temporaryRomFile);
		g_temporaryRomFile = NULL;

		free(g_pageTable);
	}
}

void InitPageAndFrameTables()
{
	uint32 i;

	// get the size of the page table
	g_pageTableSize = RomFileSize / PAGE_SIZE;

	// allocate memory for the page table
	g_pageTable = (uint8 *)malloc(g_pageTableSize);

	// initialize it
	for (i = 0; i < g_pageTableSize; i++)
	{
		g_pageTable[i] = PAGE_NOT_IN_MEMORY;
	}

	// initialize the frame table
	for (i = 0; i < NO_FRAMES; i++)
	{
		g_frameTable[i].pageNum				= FRAME_FREE;
		g_frameTable[i].lastUsed.QuadPart	= NEVER;
	}
}

uint32 ReadUWORDFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > RomFileSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint32)))
		{
			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			return *(uint32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
}

uint16 ReadUHALFFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > RomFileSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint16)))
		{
			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			return *(uint16 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
}

uint8 ReadUBYTEFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > RomFileSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint8)))
		{
			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			return *(uint8 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
}

__int32 ReadSWORDFromROM(uint32 location)
{
	uint32 i;
	uint32 pageNumberOfLocation;
	uint32 offsetFromPage;
	uint32 retVal;

	g_pageFunctionHits++;

	location &= 0x7ffffff;

	if (location > RomFileSize)
		return 0;

	// calculate what page the location is in and how far from the start of
	// the page the location is
	pageNumberOfLocation = location / PAGE_SIZE;
	offsetFromPage = location - (pageNumberOfLocation*PAGE_SIZE);

	// if the page required is in memory, use it
	if (g_pageTable[pageNumberOfLocation] != PAGE_NOT_IN_MEMORY)
	{
		g_pageHits++;

		if (offsetFromPage <= (PAGE_SIZE-sizeof(uint16)))
		{
			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
		else // if the location isn't a multiple of 4 this may happen? hopefully it doesn't ever
		{
			g_memFunctionHits++;
			// fix me
			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}
	else
	{
		uint32  freeFrame		 = NONE_FOUND;
		__int64 oldestFrameTime	 = 0x7FFFFFFFFFFFFFFF;
		uint32  oldestFrameIndex = NONE_FOUND;

		g_pageMisses++;

		// find an empty frame, or the oldest frame
		for (i = 0; i < NO_FRAMES; i++)
		{
			if (g_frameTable[i].pageNum == FRAME_FREE)
			{
				freeFrame = i;
				break;
			}
			else if (g_frameTable[i].lastUsed.QuadPart < oldestFrameTime)
			{
				oldestFrameTime  = g_frameTable[i].lastUsed.QuadPart;
				oldestFrameIndex = i;
			}
		}

		{
			uint32 frameToUse = 0;
			LARGE_INTEGER currentTime;
		
			if (freeFrame == NONE_FOUND)
			{
				frameToUse = oldestFrameIndex;
				
				// set the old page as now not being in memory
				g_pageTable[g_frameTable[frameToUse].pageNum] = PAGE_NOT_IN_MEMORY;
			}
			else
			{
				frameToUse = freeFrame;
			}

			QueryPerformanceCounter(&currentTime);

			// update the frame table
			g_frameTable[frameToUse].pageNum = pageNumberOfLocation;
			g_frameTable[frameToUse].lastUsed = currentTime;

			// seek to the start of the page in the temporary file
			fseek(g_temporaryRomFile, pageNumberOfLocation*PAGE_SIZE, SEEK_SET);

			// read in the page
			fread(&g_memory[frameToUse*PAGE_SIZE], sizeof(char), PAGE_SIZE, g_temporaryRomFile);

			// update the page table
			g_pageTable[pageNumberOfLocation] = frameToUse;

			retVal = *(__int32 *)(g_memory + (g_pageTable[pageNumberOfLocation]*PAGE_SIZE) + offsetFromPage);
		}
	}

	return retVal;
}

BOOL CheckIfInRom(uint32 location)
{
	//return (((addr)&0x1FFFFFFF) >= MEMORY_START_ROM_IMAGE && ((addr)&0x1FFFFFFF) <= 0x14000000);
	register uint32 locShifted = location >> SHIFTER2_READ;

	return ((locShifted >= 0x2400 && locShifted <= 0x24FF) \
		 || (locShifted >= 0x2C00 && locShifted <= 0x2CFF));
}


 __int32 ReadSWORDFromROM_EAXOnly(uint32 location)
{
	{
		__int32 temp;
		__asm
		{
			pushad
			mov ecx, location
			call ReadSWORDFromROM
			mov temp, eax
			popad
			mov eax, temp
		}
	}
}

/*
void InitMessageFont(void)
{
	D3DPRESENT_PARAMETERS m_d3dpp;

	m_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
	
	// Set up the presentation parameters
    ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
    m_d3dpp.Windowed               = FALSE;
    m_d3dpp.BackBufferCount        = 1;
    m_d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY;
	m_d3dpp.EnableAutoDepthStencil = TRUE;  
	m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    m_d3dpp.hDeviceWindow          = NULL;
	m_d3dpp.FullScreen_RefreshRateInHz = 60;	 
	m_d3dpp.BackBufferWidth = 640;
	m_d3dpp.BackBufferHeight = 480;
    m_d3dpp.BackBufferFormat = D3DFMT_X1R5G5B5 ;
	m_d3dpp.MultiSampleType        = D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX;
 
    // Create the device
   m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
						 NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp,
						 &g_pD3DDev );


   m_Font.Create("d:\\media\\Font.xpr");

	g_pD3DDev->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0x00000000, 1.0f, 0L );

}

void KillMessageFont(void)
{ 
	m_Font.Destroy();
	m_Font.Release();
}

void OutputTextToScreen(char *szText, int xpos, int ypos)
{
	 
	WCHAR buff[4096];
	swprintf(buff,L"%S",szText);
 
	if (g_pD3DDev)
	{
		m_Font.Begin();
		m_Font.DrawText(xpos, ypos, 0xFFAADDFF, buff, XBFONT_LEFT);
		m_Font.End();
		g_pD3DDev->Present( NULL, NULL, NULL, NULL );
		
		 
	}

} */
