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

// Manages textures for RDP code
// Uses a HashTable (hashing on TImg) to allow quick access
//  to previously used textures

#include "stdafx.h"

static TextureEntry * g_pFirstUsedSurface = NULL;
CTextureCache gTextureCache;

BOOL g_bTHMakeTexturesBlue = FALSE;
BOOL g_bTHDumpTextures = FALSE;


// Returns the first prime greater than or equal to nFirst
inline LONG GetNextPrime(LONG nFirst)
{
	LONG nCurrent;

	LONG i;

	nCurrent = nFirst;

	// Just make sure it's odd
	if ((nCurrent % 2) == 0)
		nCurrent++;

	for (;;)
	{
		LONG nSqrtCurrent;
		BOOL bIsComposite;

		// nSqrtCurrent = nCurrent^0.5 + 1 (round up)
		nSqrtCurrent = (LONG)sqrt((double)nCurrent) + 1;


		bIsComposite = FALSE;
		
		// Test all odd numbers from 3..nSqrtCurrent
		for (i = 3; i <= nSqrtCurrent; i+=2)
		{
			if ((nCurrent % i) == 0)
			{
				bIsComposite = TRUE;
				break;
			}
		}

		if (!bIsComposite)
		{			
			return nCurrent;
		}

		// Select next odd candidate...
		nCurrent += 2;
	}

}



///////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////

CTextureCache::CTextureCache() :
	m_pFirstUsedSurface(NULL),
	m_pTextureHash(NULL),
	m_dwTextureHashSize(0)
{
	m_dwTextureHashSize = GetNextPrime(16);

	m_pTextureHash = new TextureEntry *[m_dwTextureHashSize];
	SAFE_CHECK(m_pTextureHash);

	for (u32 i = 0; i < m_dwTextureHashSize; i++)
		m_pTextureHash[i] = NULL;

	memset(&m_blackTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_PrimColorTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_EnvColorTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_LODFracTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_PrimLODFracTextureEntry, 0, sizeof(TextureEntry));
}

CTextureCache::~CTextureCache()
{
	DropTextures();
	
	while (m_pFirstUsedSurface)
	{
		TextureEntry * pVictim = m_pFirstUsedSurface;
		m_pFirstUsedSurface = pVictim->pNext;
		
		delete pVictim;
	}

	delete []m_pTextureHash;
	m_pTextureHash = NULL;	

	if( m_blackTextureEntry.pTexture )		delete m_blackTextureEntry.pTexture;
	if( m_PrimColorTextureEntry.pTexture )	delete m_PrimColorTextureEntry.pTexture;
	if( m_EnvColorTextureEntry.pTexture )	delete m_EnvColorTextureEntry.pTexture;
	if( m_LODFracTextureEntry.pTexture )	delete m_LODFracTextureEntry.pTexture;
}


//
// Called when the device-dependent objects are about to be lost.
//
HRESULT CTextureCache::InvalidateDeviceObjects()
{
	DropTextures();
	
	while (m_pFirstUsedSurface)
	{
		TextureEntry * pVictim = m_pFirstUsedSurface;
		m_pFirstUsedSurface = pVictim->pNext;
		
		delete pVictim;
	}

	return S_OK;
}

// Purge any textures whos last usage was over 5 seconds ago
void CTextureCache::PurgeOldTextures()
{
	if (m_pTextureHash == NULL)
		return;
	
	static const u32 dwFramesToKill = 5*30;			// 5 secs at 30 fps
	static const u32 dwFramesToDelete = 30*30;		// 30 secs at 30 fps
	
	for ( u32 i = 0; i < m_dwTextureHashSize; i++ )
	{
		TextureEntry * pEntry;
		TextureEntry * pNext;
		
		pEntry = m_pTextureHash[i];
		while (pEntry)
		{
			pNext = pEntry->pNext;
			
			if ( status.gRDPFrame - pEntry->FrameLastUsed > dwFramesToKill )
			{
				RemoveTextureEntry(pEntry);
			}
			pEntry = pNext;
		}
	}
	
	
	// Remove any old textures that haven't been recycled in 1 minute or so
	// Normally these would be reused
	TextureEntry * pPrev;
	TextureEntry * pCurr;
	TextureEntry * pNext;
	
	
	pPrev = NULL;
	pCurr = m_pFirstUsedSurface;
	
	while (pCurr)
	{
		pNext = pCurr->pNext;
		
		if ( status.gRDPFrame - pCurr->FrameLastUsed > dwFramesToDelete )
		{
			// Everything from this point on should be too old!
			// Remove from list
			if (pPrev != NULL) pPrev->pNext        = pCurr->pNext;
			else			   m_pFirstUsedSurface = pCurr->pNext;
			
			////DBGConsole_Msg(0, "Killing old used texture (%d x %d)", pCurr->dwWidth, pCurr->dwHeight);
			
			delete pCurr;
			
			// pPrev remains the same
			pCurr = pNext;	
		}
		else
		{
			pPrev = pCurr;
			pCurr = pNext;
		}
	}
}

void CTextureCache::DropTextures()
{
	if (m_pTextureHash == NULL)
		return;
	
	u32 dwCount = 0;
	u32 dwTotalUses = 0;
	
	for (u32 i = 0; i < m_dwTextureHashSize; i++)
	{
		while (m_pTextureHash[i])
		{
			TextureEntry *pTVictim = m_pTextureHash[i];
			m_pTextureHash[i] = pTVictim->pNext;
			
			dwTotalUses += pTVictim->dwUses;
			dwCount++;
			
			AddToRecycleList(pTVictim);
		}
	}
}


// Add to the recycle list
void CTextureCache::AddToRecycleList(TextureEntry *pEntry)
{
	delete pEntry;
		return;

	if( CDeviceBuilder::GetGeneralDeviceType() == OGL_DEVICE )
	{
		// Fix me, why I can not reuse the texture in OpenGL,
		// how can I unload texture from video card memory for OpenGL
		delete pEntry;
		return;
	}

	if (pEntry->pTexture == NULL)
	{
		// No point in saving!
		delete pEntry;
	}
	else
	{
		// Add to the list
		pEntry->pNext = m_pFirstUsedSurface;
		SAFE_DELETE(pEntry->pMirroredTexture);
		m_pFirstUsedSurface = pEntry;
	}
}

// Search for a texture of the specified dimensions to recycle
TextureEntry * CTextureCache::ReviveUsedTexture( u32 width, u32 height )
{
	TextureEntry * pPrev;
	TextureEntry * pCurr;
	
	pPrev = NULL;
	pCurr = m_pFirstUsedSurface;
	
	while (pCurr)
	{
		if (pCurr->ti.WidthToCreate == width &&
			pCurr->ti.HeightToCreate == height)
		{
			// Remove from list
			if (pPrev != NULL) pPrev->pNext        = pCurr->pNext;
			else			   m_pFirstUsedSurface = pCurr->pNext;
			
			////DBGConsole_Msg(0, "Reviving used texture (%d x %d)", dwWidth, dwHeight);
			// Initialize any fields:
			return pCurr;
		}
		
		pPrev = pCurr;
		pCurr = pCurr->pNext;
	}
	
	
	
	return NULL;
}


u32 CTextureCache::Hash(u32 dwValue)
{
	// Divide by four, because most textures will be on a 4 byte boundry, so bottom four
	// bits are null
	return (dwValue>>2) % m_dwTextureHashSize;
}



void CTextureCache::AddTextureEntry(TextureEntry *pEntry)
{	
	u32 dwKey = Hash(pEntry->ti.Address);
	
	if (m_pTextureHash == NULL)
		return;
	
	TextureEntry **p = &m_pTextureHash[dwKey];
	
	// Add to head (not tail, for speed - new textures are more likely to be accessed next)
	pEntry->pNext = m_pTextureHash[dwKey];
	m_pTextureHash[dwKey] = pEntry;
}



TextureEntry * CTextureCache::GetEntry(TextureInfo * pti)
{
	TextureEntry *pEntry;
	
	if (m_pTextureHash == NULL)
		return NULL;
	
	// See if it is already in the hash table
	u32 dwKey = Hash(pti->Address);

	for (pEntry = m_pTextureHash[dwKey]; pEntry; pEntry = pEntry->pNext)
	{
		if ( pEntry->ti == *pti )
			return pEntry;
	}

	return NULL;
}



void CTextureCache::RemoveTextureEntry(TextureEntry * pEntry)
{
	TextureEntry * pPrev;
	TextureEntry * pCurr;
	
	if (m_pTextureHash == NULL)
		return;
	
	////DBGConsole_Msg(0, "Remove Texture entry!");
	
	// See if it is already in the hash table
	u32 dwKey = Hash(pEntry->ti.Address);
	
	pPrev = NULL;
	pCurr = m_pTextureHash[dwKey];
	
	while (pCurr)
	{
		// Check that the attributes match
		if ( pCurr->ti == pEntry->ti )
		{
			if (pPrev != NULL) pPrev->pNext = pCurr->pNext;
			else			   m_pTextureHash[dwKey] = pCurr->pNext;
			break;
		}

		pPrev = pCurr;
		pCurr = pCurr->pNext;
	}
	
	if (pCurr == NULL)
	{
		//DBGConsole_Msg(0, "Entry not found!!!");
	}
	
	AddToRecycleList(pEntry);
}

TextureEntry * CTextureCache::CreateEntry(DWORD dwAddress, DWORD dwWidth, DWORD dwHeight)
{
	TextureEntry * pEntry;

	// Find a used texture
	pEntry = ReviveUsedTexture(dwWidth, dwHeight);
	if (pEntry == NULL)
	{
		// Couldn't find on - recreate!
		pEntry = new TextureEntry;
		if (pEntry == NULL)
			return NULL;

		pEntry->pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(dwWidth, dwHeight);
		if (pEntry->pTexture == NULL || pEntry->pTexture->GetTexture() == NULL)
		{
			TRACE2("Warning, unable to create %d x %d texture!", dwWidth, dwHeight);
		}
		else
		{
			pEntry->pTexture->m_bScaledS = false;
			pEntry->pTexture->m_bScaledT = false;
		}
	}
	
	// Initialize
	pEntry->ti.Address = dwAddress;
	pEntry->pNext = NULL;
	pEntry->dwUses = 0;
	pEntry->dwTimeLastUsed = status.gRDPTime;
	pEntry->dwCRC = 0;
	pEntry->FrameLastUsed = status.gRDPFrame;
	pEntry->lastEntry = NULL;

	// Add to the hash table
	AddTextureEntry(pEntry);
	return pEntry;	
}


u32 CTextureCache::CalculateCRC(TextureInfo * pti) const
{
	u32 dwCRC;
	u8 * pStart;
	u32 dwBytesPerLine;
	//s32 x,y;

	// If CRC checking is disabled, alwasy return 0
	if ( g_curRomInfo.bDisableTextureCRC )
		return 0;
	
	dwBytesPerLine = ((pti->WidthToLoad<<pti->Size)+1)/2;

	////DBGConsole_Msg(0, "BytesPerLine: %d", dwBytesPerLine);
	
	// A very simple crc - just summation
	dwCRC = 0;

	// The assembly below replaces and improves on this
	/*pStart = ti.pPhysicalAddress;
	pStart += (pti->TopToLoad * pti->Pitch) + (((pti->LeftToLoad<<pti->Size)+1)/2);
	for (y = 0; y < pti->HeightToLoad; y++)		// Do every nth line?
	{
		// Byte fiddling won't work, but this probably doesn't matter
		// Now process 4 bytes at a time
		for (x = 0; x < dwBytesPerLine; x+=4)
		{
			dwCRC = (dwCRC + *(u32*)&pStart[x]);
		}

		pStart += pti->Pitch;
	}

	u32 dwTestCRC = dwCRC;*/

	pStart = (BYTE*)(pti->pPhysicalAddress);
	pStart += (pti->TopToLoad * pti->Pitch) + (((pti->LeftToLoad<<pti->Size)+1)/2);

	u32 ptiHeight = pti->HeightToLoad;
	u32 ptiPitch = pti->Pitch;
	__asm
	{
		xor		eax, eax			// Clear checksum

		mov		ebx, ptiPitch
		mov		ecx, ptiHeight

		mov		esi, dword ptr [pStart]

		// Calculate initial x counter value
		mov		edx, dwBytesPerLine
		add		edx, 3				// Add 3 to round up result
		shr		edx, 2
		jz		done				// Check for BytesPerLine <= 4

		// Check which version of the code to use: optimise
		// for cases where dwBytesPerLine is a multiple of 16 bytes
		test	edx, 3
		jz		do_mult_of_4

	// The x counter is not a multiple of 4, so some extra code
	// is needed to bring it down to the boundary
	do_not_mult_of_4:
		mov		edi, edx
	x_loop_top_no4:
		test	edi, 3
		jz		do4
	//do1:
		add		eax, [esi+edi*4-4]
		dec		edi
		jnz		x_loop_top_no4
		jmp		done_x_no4
	do4:
		add		eax, [esi+edi*4-4]
		add		eax, [esi+edi*4-8]
		add		eax, [esi+edi*4-12]
		add		eax, [esi+edi*4-16]
		sub		edi, 4
		jnz		do4							// still more to do
	done_x_no4:
		add		esi, ebx					// add pitch to base pointer

		loop	do_not_mult_of_4			// dec ecx, jump to y_loop_top if not zero
		jmp		done


	// The x counter is known to be a multiple of 4, so no code
	// is needed to do the surplus bits
	do_mult_of_4:
		mov		edi, edx
	x_loop_top_4:
		add		eax, [esi+edi*4-4]
		add		eax, [esi+edi*4-8]
		add		eax, [esi+edi*4-12]
		add		eax, [esi+edi*4-16]
		sub		edi, 4
		jnz		x_loop_top_4			// still more to do
	//done_x_4:
		add		esi, ebx				// add pitch to base pointer
		loop	do_mult_of_4			// dec ecx, jump to y_loop_top if not zero
	done:
		mov		dwCRC, eax
	}

	/*if ( dwTestCRC != dwCRC )
	{
		//DBGConsole_Msg(0, "CRCs don't match");
	}*/

	if (pti->Format == G_IM_FMT_CI)
	{
		/*
		u32 bytes;
		if ( pti->Size == G_IM_SIZ_4b )		bytes = 16  * 4;
		else								bytes = 256 * 4;

		pStart = &gTextureMemory[ pti->TmemPalAddress << 3 ];
		for ( u32 x = 0; x < bytes; x+=4)
		{
			dwCRC = (dwCRC + *(u32*)&pStart[x]);
		}
		*/

		//Check PAL CRC
		DWORD dwPalSize = 16;
		DWORD dwOffset, y;
		if( pti->Size == G_IM_SIZ_8b )
		{
			dwPalSize = 256;
			dwOffset = 0;
		}
		else
		{
			dwOffset = pti->Palette << 4;
		}

		pStart = (BYTE*)pti->PalAddress+dwOffset*2;
		//pStart = ti.pPhysicalAddress;
		for (y = 0; y < dwPalSize*2; y+=4)
		{
			dwCRC = (dwCRC + *(DWORD*)&pStart[y]);
		}
	}

	return dwCRC;
}

// If already in table, return
// Otherwise, create surfaces, and load texture into 
// memory
extern int g_DlistCount;
DWORD dwAsmHeight;
DWORD dwAsmPitch;
DWORD dwAsmdwBytesPerLine;
DWORD dwAsmCRC;
BYTE* pAsmStart;

TextureEntry *g_lastTextureEntry=NULL;
bool lastEntryModified = false;

TextureEntry * CTextureCache::GetTexture(TextureInfo * pgti, bool doCRCCheck, bool AutoExtendTexture)
{
	TextureEntry *pEntry;

	gRDP.texturesAreReloaded = true;

	dwAsmCRC = 0;
	DWORD dwPalCRC = 0;

	// Exit if alread cached
	pEntry = GetEntry(pgti);

	/*
	if( currentRomOptions.dwFastTextureCRC==2) && pEntry && g_lastTextureEntry != NULL && g_lastTextureEntry->dwTimeLastUsed == status.gRDPTime
		&& (pEntry->lastEntry == g_lastTextureEntry || pEntry == g_lastTextureEntry ) //&& lastEntryModified == false 
		&& pEntry->ti.Format != G_IM_FMT_CI )
	{
		// Fast check
		if( pEntry != g_lastTextureEntry )
			g_lastTextureEntry = pEntry;
		return pEntry;
	}
	*/

	if (pEntry && pEntry->dwTimeLastUsed == status.gRDPTime && g_DlistCount != 0 )		// This is not good, Palatte may changes
	{
		// We've already calculated a CRC this frame!
		dwAsmCRC = pEntry->dwCRC;
	}
	else 
	if (g_bCRCCheck && doCRCCheck)
	{
#define FAST_CRC_CHECKING_INC_X	13
#define FAST_CRC_CHECKING_INC_Y	11
#define FAST_CRC_MIN_Y_INC		2
#define FAST_CRC_MIN_X_INC		2
#define FAST_CRC_MAX_X_INC		7
#define FAST_CRC_MAX_Y_INC		3

		dwAsmdwBytesPerLine = ((pgti->WidthToLoad<<pgti->Size)+1)/2;
		if( currentRomOptions.bFastTexCRC && (pgti->HeightToLoad>=32 || (dwAsmdwBytesPerLine>>2)>=16))
		{
			DWORD realWidthInDWORD = dwAsmdwBytesPerLine>>2;
			DWORD xinc = realWidthInDWORD / FAST_CRC_CHECKING_INC_X;	
			if( xinc < FAST_CRC_MIN_X_INC )
			{
				xinc = min(FAST_CRC_MIN_X_INC, pgti->WidthToLoad);
			}
			if( xinc > FAST_CRC_MAX_X_INC )
			{
				xinc = FAST_CRC_MAX_X_INC;
			}
			
			DWORD yinc = pgti->HeightToLoad / FAST_CRC_CHECKING_INC_Y;	
			if( yinc < FAST_CRC_MIN_Y_INC ) 
			{
				yinc = min(FAST_CRC_MIN_Y_INC, pgti->HeightToLoad);
			}
			if( yinc > FAST_CRC_MAX_Y_INC )
			{
				yinc = FAST_CRC_MAX_Y_INC;
			}

			DWORD pitch = pgti->Pitch>>2;
			register DWORD *pStart = (DWORD*)(pgti->pPhysicalAddress);
			pStart += (pgti->TopToLoad * pitch) + (((pgti->LeftToLoad<<pgti->Size)+1)>>3);

			/*
			DWORD x,y;
			for (y = 0; y < pgti->HeightToLoad; y+=yinc)		// Do every nth line?
			{
				for (x = 0; x < realWidthInDWORD; x+=xinc)
				{
					dwAsmCRC += *(pStart+x);
					dwAsmCRC ^= x;
				}
				pStart += pitch;
				dwAsmCRC ^= y;
			}
			*/


			DWORD height = pgti->HeightToLoad;

			__asm
			{
				push	esi;
				mov		esi, DWORD PTR [xinc]; 
				mov		ebx, DWORD PTR [pStart];
				mov		eax,0;	// EAX = the CRC
				mov		edx,0x0;
loop1:
				cmp		edx, height;
				jae		endloop1;
				mov		ecx, 0x0;
loop2:
				add		eax, ecx;
				cmp		ecx, DWORD PTR [realWidthInDWORD]
				jae		endloop2;

				add		eax, DWORD PTR [ebx][ecx*4];

				add		ecx, esi;
				jmp		loop2;
endloop2:
				xor		eax, edx;
				add		edx, DWORD PTR [yinc];
				add		ebx, DWORD PTR [pitch];
				jmp		loop1;
endloop1:
				mov		DWORD PTR [dwAsmCRC], eax;
				pop		esi;
			}
		}
		else
		{
			try{
				dwAsmdwBytesPerLine = ((pgti->WidthToLoad<<pgti->Size)+1)/2;

				// A very simple crc - just summation
				dwAsmCRC = 0;
				pAsmStart = (BYTE*)(pgti->pPhysicalAddress);
				pAsmStart += (pgti->TopToLoad * pgti->Pitch) + (((pgti->LeftToLoad<<pgti->Size)+1)>>1);
				
				dwAsmHeight = pgti->HeightToLoad - 1;
				dwAsmPitch = pgti->Pitch - 1;


				__asm 
				{
					push eax
					push ebx
					push ecx
					push edx
					push esi

					mov  ecx, pAsmStart;	// = pStart
					mov  edx, 0			// The CRC
					mov  eax, dwAsmHeight	// = y
				l2:	mov	 ebx, dwAsmdwBytesPerLine	// = x
				l1:	mov esi, [ecx+ebx]
					xor esi, ebx
					xor esi, eax
					add edx, esi
					sub	ebx, 4
					jge l1
					add ecx, dwAsmPitch
					dec eax
					jge l2

					mov	dwAsmCRC, edx

					pop esi
					pop edx
					pop ecx
					pop ebx
					pop	eax
				}
				
				/*
				DWORD x,y;
				for (y = 0; y < pgti->HeightToLoad; y++)		// Do every nth line?
				{
					// Byte fiddling won't work, but this probably doesn't matter
					// Now process 4 bytes at a time
					for (x = 0; x < dwBytesPerLine; x+=4)
					{
						dwCRC += (((*(DWORD*)(pStart+x))^y)^x);
						//dwCRC += x;
					}
					pStart += pgti->Pitch;
				}
				*/
			}
			catch(...)
			{
				TRACE0("Exception in texture CRC calculation");
				//ErrorMsg("Exception in texture CRC calculation");
				g_lastTextureEntry = NULL;
				return NULL;
				//throw;
			}
		}
	}

	if (pgti->Format == G_IM_FMT_CI)
	{
		//Check PAL CRC
		BYTE * pStart;
		DWORD dwPalSize = 16;
		DWORD dwOffset, y;
		if( pgti->Size == G_IM_SIZ_8b )
		{
			dwPalSize = 256;
			dwOffset = 0;
		}
		else
		{
			dwOffset = pgti->Palette << 4;
		}

		pStart = (BYTE*)pgti->PalAddress+dwOffset*2;
		//pStart = ti.pPhysicalAddress;
		for (y = 0; y < dwPalSize*2; y+=4)
		{
			dwPalCRC = (dwPalCRC + *(DWORD*)&pStart[y]);
		}
	}
	
	if (pEntry && doCRCCheck )
	{
		if(pEntry->dwCRC == dwAsmCRC && pEntry->dwPalCRC == dwPalCRC )
		{
			// Tile is ok, return
			pEntry->dwUses++;
			pEntry->dwTimeLastUsed = status.gRDPTime;
			pEntry->FrameLastUsed = status.gRDPFrame;
#ifdef _DEBUG
			if( logTextureDetails )
			{
				TRACE0("   Use current texture:\n");
			}
#endif
			pEntry->lastEntry = g_lastTextureEntry;
			g_lastTextureEntry = pEntry;
			lastEntryModified = false;
			return pEntry;
		}
		else
		{
			; //Do something
		}
	}

	if (pEntry == NULL)
	{
		// We need to create a new entry, and add it
		//  to the hash table.
		pEntry = CreateEntry(pgti->Address, pgti->WidthToCreate, pgti->HeightToCreate);

		if (pEntry == NULL)
		{
			g_lastTextureEntry = pEntry;
			return NULL;
		}
	}

	pEntry->ti = *pgti;
	pEntry->dwCRC = dwAsmCRC;
	pEntry->dwPalCRC = dwPalCRC;

	if( pEntry->pTexture->m_dwCreatedTextureWidth < pgti->WidthToCreate )
	{
		// Can not create texture large enough with this video card such as voodoo3
		// skip this texture
		//SAFE_DELETE(pEntry->pTexture);
		//pEntry->pTexture = NULL;
		//pgti->WidthToCreate = pEntry->ti.WidthToLoad = pEntry->ti.WidthToCreate 
		//	= pEntry->pTexture->m_dwWidth = pEntry->pTexture->m_dwCreatedTextureWidth;
		pEntry->ti.WidthToLoad = pEntry->pTexture->m_dwCreatedTextureWidth;
		pEntry->pTexture->m_bScaledS = false;
		pEntry->pTexture->m_bScaledT = false;
	}
	if( pEntry->pTexture->m_dwCreatedTextureHeight < pgti->HeightToCreate )
	{
		//pgti->HeightToCreate = pEntry->ti.HeightToLoad = pEntry->ti.HeightToCreate 
		//	= pEntry->pTexture->m_dwHeight = pEntry->pTexture->m_dwCreatedTextureHeight;
		pEntry->ti.HeightToLoad = pEntry->pTexture->m_dwCreatedTextureHeight;
		pEntry->pTexture->m_bScaledT = false;
		pEntry->pTexture->m_bScaledS = false;
	}

	try 
	{
		if (pEntry->pTexture != NULL)
		{
			DaedalusTexFmt dwType = pEntry->pTexture->GetSurfaceFormat();
			if (dwType != TEXTURE_FMT_UNKNOWN)
			{
				DEBUGGER_IF_DUMP( logTextureDetails, {DebuggerAppendMsg("   Load new texture from RDRAM:\n");});
				if (dwType == TEXTURE_FMT_A8R8G8B8)
					DecompressTexture(pEntry);		
				else
					DecompressTexture_16(pEntry);
			}

			pEntry->ti.WidthToLoad = pgti->WidthToLoad;
			pEntry->ti.HeightToLoad = pgti->HeightToLoad;
			
			if( AutoExtendTexture )
			{
				ExpandTextureS(pEntry);
				ExpandTextureT(pEntry);
			}

			SAFE_DELETE(pEntry->pMirroredTexture);
			pEntry->dwEnhancementFlag = TEXTURE_NO_ENHANCEMENT;

#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_NEW_TEXTURE )
			{
				CDaedalusRender::g_pRender->SetCurrentTexture( 0, pEntry->pTexture, pEntry->ti.WidthToCreate, pEntry->ti.HeightToCreate);
				CDaedalusRender::g_pRender->DrawTexture(0);
				debuggerPause = true;
				TRACE0("Pause after loading a new texture");
				if( pEntry->ti.Format == G_IM_FMT_YUV )
				{
					TRACE0("This is YUV texture");
				}
				DebuggerAppendMsg("W:%d, H:%d, RealW:%d, RealH:%d, D3DW:%d, D3DH: %d", pEntry->ti.WidthToCreate, pEntry->ti.HeightToCreate,
					pEntry->ti.WidthToLoad, pEntry->ti.HeightToLoad, pEntry->pTexture->m_dwCreatedTextureWidth, pEntry->pTexture->m_dwCreatedTextureHeight);
				DebuggerAppendMsg("ScaledS:%s, ScaledT:%s, CRC=%08X", pEntry->pTexture->m_bScaledS?"T":"F", pEntry->pTexture->m_bScaledT?"T":"F", pEntry->dwCRC);
				DebuggerPause();
				CDaedalusRender::g_pRender->SetCurrentTexture( 0, NULL, 64, 64);
			}
#endif
		}
	}
	catch (...)
	{
		TRACE0("Exception in texture decompression");
		//ErrorMsg("Exception in texture decompression");
		g_lastTextureEntry = NULL;
		return NULL;
		//throw;
	}

	pEntry->lastEntry = g_lastTextureEntry;
	g_lastTextureEntry = pEntry;
	lastEntryModified = true;
	return pEntry;
}




char *pszImgFormat[8] = {"RGBA", "YUV", "CI", "IA", "I", "?1", "?2", "?3"};
BYTE pnImgSize[4]   = {4, 8, 16, 32};
char *textlutname[4] = {"RGB16", "I16?", "RGBA16", "IA16"};

extern WORD g_wRDPPal[];
extern ConvertFunction	gConvertFunctions[ 8 ][ 4 ];
extern ConvertFunction	gConvertFunctions_16[ 8 ][ 4 ];

void CTextureCache::DecompressTexture(TextureEntry * pEntry)
{
	static DWORD dwCount = 0;
	
	ConvertFunction pF = gConvertFunctions[ pEntry->ti.Format ][ pEntry->ti.Size ];

	if( pF )
	{
		pF( pEntry->pTexture, pEntry->ti );
	
#ifdef _DEBUG
		if( logTextureDetails )
		{
			DebuggerAppendMsg("Decompress 32bit Texture:\n\tFormat: %s\n\tImage Size:%d\n", 
				pszImgFormat[pEntry->ti.Format], pnImgSize[pEntry->ti.Size]);
			DebuggerAppendMsg("Palette Format: %s (%d)\n", textlutname[pEntry->ti.TLutFmt>>G_MDSFT_TEXTLUT], pEntry->ti.TLutFmt>>G_MDSFT_TEXTLUT);
		}
#endif
	}
	else
	{
		TRACE2("DecompressTexture: Unable to decompress %s/%dbpp", pszImgFormat[pEntry->ti.Format], pnImgSize[pEntry->ti.Size]);
	}

	dwCount++;
}

void CTextureCache::DecompressTexture_16(TextureEntry * pEntry)
{
	static DWORD dwCount = 0;
	
	ConvertFunction pF = gConvertFunctions_16[ pEntry->ti.Format ][ pEntry->ti.Size ];
	
	if( pF )
	{
		pF( pEntry->pTexture, pEntry->ti );

#ifdef _DEBUG
		if( logTextureDetails )
		{
			DebuggerAppendMsg("Decompress 16bit Texture:\n\tFormat: %s\n\tImage Size:%d\n", 
				pszImgFormat[pEntry->ti.Format], pnImgSize[pEntry->ti.Size]);
		}
#endif
	}
	else
	{
		TRACE2("DecompressTexture: Unable to decompress %s/%dbpp", pszImgFormat[pEntry->ti.Format], pnImgSize[pEntry->ti.Size]);
	}

	dwCount++;
}

#define absi(x)		((x)>=0?(x):(-x))
#define S_FLAG	0
#define T_FLAG	1
void CTextureCache::ExpandTexture(TextureEntry * pEntry, u32 sizeToLoad, u32 sizeToCreate, u32 sizeCreated,
	int arrayWidth, int flag, int mask, int mirror, int clamp, u32 otherSize)
{
	if( sizeToLoad >= sizeCreated )	return;

	u32 maskWidth = (1<<mask);
	int size = pEntry->pTexture->GetPixelSize();

#ifdef _DEBUG
	// Some checks
	if( sizeToLoad > sizeToCreate || sizeToCreate > sizeCreated )	
		TRACE0("Something is wrong, check me here in ExpandTextureS");
#endif

	// Doing Mirror And/Or Wrap in S direction
	// Image has been loaded with width=WidthToLoad, we need to enlarge the image
	// to width = pEntry->ti.WidthToCreate by doing mirroring or wrapping

	DrawInfo di;
	if( !(pEntry->pTexture->StartUpdate(&di)) )
	{
		TRACE0("Cann't update the texture");
		return;
	}


	if( mask == 0 )
	{
		// Clamp
		Clamp(di.lpSurface, sizeToLoad, sizeCreated, arrayWidth, otherSize, 
			flag, size);
		pEntry->pTexture->EndUpdate(&di);
		return;
	}

#ifdef _DEBUG
	if( sizeToLoad > maskWidth )
	{
		TRACE0("Something is wrong, check me here in ExpandTextureS");
		pEntry->pTexture->EndUpdate(&di);
		return;
	}
	if( sizeToLoad == maskWidth && maskWidth == sizeToCreate && sizeToCreate != sizeCreated )
	{
		TRACE0("Something is wrong, check me here in ExpandTextureS");
		pEntry->pTexture->EndUpdate(&di);
		return;
	}
#endif

	if( sizeToLoad == maskWidth )
	{
		u32 tempwidth = clamp ? sizeToCreate : sizeCreated;
		if( mirror )
		{
			Mirror(di.lpSurface, sizeToLoad, mask, tempwidth,
				arrayWidth, otherSize, flag, size );
		}
		else
		{
			Wrap(di.lpSurface, sizeToLoad, mask, tempwidth,
				arrayWidth, otherSize, flag, size );
		}

		if( tempwidth < sizeCreated )
		{
			Clamp(di.lpSurface, tempwidth, sizeCreated, arrayWidth, otherSize, 
				flag, size );
		}

		pEntry->pTexture->EndUpdate(&di);
		return;
	}


	if( sizeToLoad < sizeToCreate && sizeToCreate == maskWidth && maskWidth == sizeCreated )
	{
		// widthToLoad < widthToCreate = maskWidth
		//DebuggerAppendMsg("load=%d, other=%d, created=%d, %08X, flag=%d", sizeToLoad, otherSize, sizeToCreate,  di.lpSurface, flag);
		//ErrorMsg("load=%d, other=%d, created=%d, %08X, flag=%d", sizeToLoad, otherSize, sizeToCreate,  di.lpSurface, flag);
		Clamp(di.lpSurface, sizeToLoad, sizeCreated, arrayWidth, otherSize, flag, size );

		pEntry->pTexture->EndUpdate(&di);
		return;
	}

	if( sizeToLoad == sizeToCreate && sizeToCreate < maskWidth )
	{
#ifdef _DEBUG
		//if( clamp )	TRACE0("Incorrect condition, check me");
		if( maskWidth < sizeToCreate )	TRACE0("Incorrect condition, check me");
#endif
		Clamp(di.lpSurface, sizeToLoad, sizeCreated, arrayWidth, otherSize, flag, size );

		pEntry->pTexture->EndUpdate(&di);
		return;
	}

	if( sizeToLoad < sizeToCreate && sizeToCreate < maskWidth )
	{
#ifdef _DEBUG
		if( clamp )	TRACE0("Incorrect condition, check me");
		if( maskWidth < sizeCreated )	TRACE0("Incorrect condition, check me");
#endif
		Clamp(di.lpSurface, sizeToLoad, sizeCreated, arrayWidth, otherSize, flag, size );
		pEntry->pTexture->EndUpdate(&di);
		return;
	}

	TRACE0("Check me, should not get here");
	pEntry->pTexture->EndUpdate(&di);
}

void CTextureCache::ExpandTextureS(TextureEntry * pEntry)
{
	TextureInfo &ti =  pEntry->ti;
	u32 textureWidth = pEntry->pTexture->m_dwCreatedTextureWidth;
	ExpandTexture(pEntry, ti.WidthToLoad, ti.WidthToCreate, textureWidth, 
		textureWidth, S_FLAG, ti.maskS, ti.mirrorS, ti.clampS, ti.HeightToLoad);
}

void CTextureCache::ExpandTextureT(TextureEntry * pEntry)
{
	TextureInfo &ti =  pEntry->ti;
	u32 textureHeight = pEntry->pTexture->m_dwCreatedTextureHeight;
	u32 textureWidth = pEntry->pTexture->m_dwCreatedTextureWidth;
	ExpandTexture(pEntry, ti.HeightToLoad, ti.HeightToCreate, textureHeight,
		textureWidth, T_FLAG, ti.maskT, ti.mirrorT, ti.clampT, ti.WidthToLoad);
}


void CTextureCache::ClampS32(DWORD *array, u32 width, u32 towidth, u32 arrayWidth, u32 rows)
{
	for( u32 y = 0; y<rows; y++ )
	{
		DWORD* line = array+y*arrayWidth;
		DWORD val = line[width-1];
		for( u32 x=width; x<towidth; x++ )
		{
			line[x] = val;
		}
	}
}

void CTextureCache::ClampS16(WORD *array, u32 width, u32 towidth, u32 arrayWidth, u32 rows)
{
	for( u32 y = 0; y<rows; y++ )
	{
		WORD* line = array+y*arrayWidth;
		WORD val = line[width-1];
		for( u32 x=width; x<towidth; x++ )
		{
			line[x] = val;
		}
	}
}

void CTextureCache::ClampT32(DWORD *array, u32 height, u32 toheight, u32 arrayWidth, u32 cols)
{
	DWORD* linesrc = array+arrayWidth*(height-1);
	for( u32 y = height; y<toheight; y++ )
	{
		DWORD* linedst = array+arrayWidth*y;
		for( u32 x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureCache::ClampT16(WORD *array, u32 height, u32 toheight, u32 arrayWidth, u32 cols)
{
	WORD* linesrc = array+arrayWidth*(height-1);
	for( u32 y = height; y<toheight; y++ )
	{
		WORD* linedst = array+arrayWidth*y;
		for( u32 x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureCache::MirrorS32(DWORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows)
{
	u32 maskval1 = (1<<mask)-1;
	u32 maskval2 = (1<<(mask+1))-1;

	for( u32 y = 0; y<rows; y++ )
	{
		DWORD* line = array+y*arrayWidth;
		for( u32 x=width; x<towidth; x++ )
		{
			line[x] = (x&maskval2)<=maskval1 ? line[x&maskval1] : line[maskval2-(x&maskval2)];
		}
	}
}

void CTextureCache::MirrorS16(WORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows)
{
	u32 maskval1 = (1<<mask)-1;
	u32 maskval2 = (1<<(mask+1))-1;

	for( u32 y = 0; y<rows; y++ )
	{
		WORD* line = array+y*arrayWidth;
		for( u32 x=width; x<towidth; x++ )
		{
			line[x] = (x&maskval2)<=maskval1 ? line[x&maskval1] : line[maskval2-(x&maskval2)];
		}
	}
}

void CTextureCache::MirrorT32(DWORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols)
{
	u32 maskval1 = (1<<mask)-1;
	u32 maskval2 = (1<<(mask+1))-1;

	for( u32 y = height; y<toheight; y++ )
	{
		u32 srcy = (y&maskval2)<=maskval1 ? y&maskval1 : maskval2-(y&maskval2);
		DWORD* linesrc = array+arrayWidth*srcy;
		DWORD* linedst = array+arrayWidth*y;;
		for( u32 x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureCache::MirrorT16(WORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols)
{
	u32 maskval1 = (1<<mask)-1;
	u32 maskval2 = (1<<(mask+1))-1;

	for( u32 y = height; y<toheight; y++ )
	{
		u32 srcy = (y&maskval2)<=maskval1 ? y&maskval1 : maskval2-(y&maskval2);
		WORD* linesrc = array+arrayWidth*srcy;
		WORD* linedst = array+arrayWidth*y;;
		for( u32 x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureCache::WrapS32(DWORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows)
{
	u32 maskval = (1<<mask)-1;

	for( u32 y = 0; y<rows; y++ )
	{
		DWORD* line = array+y*arrayWidth;
		for( u32 x=width; x<towidth; x++ )
		{
			line[x] = line[x&maskval];
		}
	}
}

void CTextureCache::WrapS16(WORD *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows)
{
	u32 maskval = (1<<mask)-1;

	for( u32 y = 0; y<rows; y++ )
	{
		WORD* line = array+y*arrayWidth;
		for( u32 x=width; x<towidth; x++ )
		{
			line[x] = line[x&maskval];
		}
	}
}

void CTextureCache::WrapT32(DWORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols)
{
	u32 maskval = (1<<mask)-1;
	for( u32 y = height; y<toheight; y++ )
	{
		DWORD* linesrc = array+arrayWidth*(y&maskval);
		DWORD* linedst = array+arrayWidth*y;;
		for( u32 x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureCache::WrapT16(WORD *array, u32 height, u32 mask, u32 toheight, u32 arrayWidth, u32 cols)
{
	u32 maskval = (1<<mask)-1;
	for( u32 y = height; y<toheight; y++ )
	{
		WORD* linesrc = array+arrayWidth*(y&maskval);
		WORD* linedst = array+arrayWidth*y;;
		for( u32 x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureCache::Clamp(void *array, u32 width, u32 towidth, u32 arrayWidth, u32 rows, int flag, int size )
{
	if( flag == S_FLAG )	// s
	{
		if( size == 4 )	// 32 bit
		{
			ClampS32((DWORD*)array, width, towidth, arrayWidth, rows);
		}
		else	// 16 bits
		{
			ClampS16((WORD*)array, width, towidth, arrayWidth, rows);
		}
	}
	else	// t
	{
		if( size == 4 )	// 32 bit
		{
			ClampT32((DWORD*)array, width, towidth, arrayWidth, rows);
		}
		else	// 16 bits
		{
			ClampT16((WORD*)array, width, towidth, arrayWidth, rows);
		}
	}
}
void CTextureCache::Wrap(void *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows, int flag, int size )
{
	if( flag == S_FLAG )	// s
	{
		if( size == 4 )	// 32 bit
		{
			WrapS32((DWORD*)array, width, mask, towidth, arrayWidth, rows);
		}
		else	// 16 bits
		{
			WrapS16((WORD*)array, width, mask, towidth, arrayWidth, rows);
		}
	}
	else	// t
	{
		if( size == 4 )	// 32 bit
		{
			WrapT32((DWORD*)array, width, mask, towidth, arrayWidth, rows);
		}
		else	// 16 bits
		{
			WrapT16((WORD*)array, width, mask, towidth, arrayWidth, rows);
		}
	}
}
void CTextureCache::Mirror(void *array, u32 width, u32 mask, u32 towidth, u32 arrayWidth, u32 rows, int flag, int size )
{
	if( flag == S_FLAG )	// s
	{
		if( size == 4 )	// 32 bit
		{
			MirrorS32((DWORD*)array, width, mask, towidth, arrayWidth, rows);
		}
		else	// 16 bits
		{
			MirrorS16((WORD*)array, width, mask, towidth, arrayWidth, rows);
		}
	}
	else	// t
	{
		if( size == 4 )	// 32 bit
		{
			MirrorT32((DWORD*)array, width, mask, towidth, arrayWidth, rows);
		}
		else	// 16 bits
		{
			MirrorT16((WORD*)array, width, mask, towidth, arrayWidth, rows);
		}
	}

}


#ifdef _DEBUG
TextureEntry * CTextureCache::GetCachedTexture(u32 tex)
{
	u32 size = 0;
	for( u32 i=0; i<m_dwTextureHashSize; i++ )
	{
		if( m_pTextureHash[i] == NULL )
			continue;
		else
		{
			TextureEntry *pEntry;

			for (pEntry = m_pTextureHash[i]; pEntry; pEntry = pEntry->pNext)
			{
				if( size == tex )
					return pEntry;
				else
					size++;
			}
		}
	}
	return NULL;
}
u32 CTextureCache::GetNumOfCachedTexture()
{
	u32 size = 0;
	for( u32 i=0; i<m_dwTextureHashSize; i++ )
	{
		if( m_pTextureHash[i] == NULL )
			continue;
		else
		{
			TextureEntry *pEntry;

			for (pEntry = m_pTextureHash[i]; pEntry; pEntry = pEntry->pNext)
			{
				size++;
			}
		}
	}
	TRACE1("Totally %d texture cached", size);
	return size;
}
#endif


TextureEntry * CTextureCache::GetBlackTexture(void)
{
	if( m_blackTextureEntry.pTexture == NULL )
	{
		m_blackTextureEntry.pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(4, 4);
		m_blackTextureEntry.ti.WidthToCreate = 4;
		m_blackTextureEntry.ti.HeightToCreate = 4;
		updateColorTexture(m_blackTextureEntry.pTexture,0x00000000);
	}
	return &m_blackTextureEntry;
}
TextureEntry * CTextureCache::GetPrimColorTexture(DWORD color)
{
	static DWORD mcolor = 0;
	if( m_PrimColorTextureEntry.pTexture == NULL )
	{
		m_PrimColorTextureEntry.pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(4, 4);
		m_PrimColorTextureEntry.ti.WidthToCreate = 4;
		m_PrimColorTextureEntry.ti.HeightToCreate = 4;
		updateColorTexture(m_PrimColorTextureEntry.pTexture,color);
	}
	else if( mcolor != color )
	{
		updateColorTexture(m_PrimColorTextureEntry.pTexture,color);
	}

	mcolor = color;
	return &m_PrimColorTextureEntry;
}
TextureEntry * CTextureCache::GetEnvColorTexture(DWORD color)
{
	static DWORD mcolor = 0;
	if( m_EnvColorTextureEntry.pTexture == NULL )
	{
		m_EnvColorTextureEntry.pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(4, 4);
		m_EnvColorTextureEntry.ti.WidthToCreate = 4;
		m_EnvColorTextureEntry.ti.HeightToCreate = 4;

		updateColorTexture(m_EnvColorTextureEntry.pTexture,color);
	}
	else if( mcolor != color )
	{
		updateColorTexture(m_EnvColorTextureEntry.pTexture,color);
	}

	mcolor = color;
	return &m_EnvColorTextureEntry;
}
TextureEntry * CTextureCache::GetLODFracTexture(BYTE fac)
{
	static BYTE mfac = 0;
	if( m_LODFracTextureEntry.pTexture == NULL )
	{
		m_LODFracTextureEntry.pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(4, 4);
		m_LODFracTextureEntry.ti.WidthToCreate = 4;
		m_LODFracTextureEntry.ti.HeightToCreate = 4;
		DWORD factor = fac;
		DWORD color = fac;
		color |= factor << 8;
		color |= color << 16;
		updateColorTexture(m_LODFracTextureEntry.pTexture,color);
	}
	else if( mfac != fac )
	{
		DWORD factor = fac;
		DWORD color = fac;
		color |= factor << 8;
		color |= color << 16;
		updateColorTexture(m_LODFracTextureEntry.pTexture,color);
	}

	mfac = fac;
	return &m_LODFracTextureEntry;
}

TextureEntry * CTextureCache::GetPrimLODFracTexture(BYTE fac)
{
	static BYTE mfac = 0;
	if( m_PrimLODFracTextureEntry.pTexture == NULL )
	{
		m_PrimLODFracTextureEntry.pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(4, 4);
		m_PrimLODFracTextureEntry.ti.WidthToCreate = 4;
		m_PrimLODFracTextureEntry.ti.HeightToCreate = 4;
		DWORD factor = fac;
		DWORD color = fac;
		color |= factor << 8;
		color |= color << 16;
		updateColorTexture(m_PrimLODFracTextureEntry.pTexture,color);
	}
	else if( mfac != fac )
	{
		DWORD factor = fac;
		DWORD color = fac;
		color |= factor << 8;
		color |= color << 16;
		updateColorTexture(m_PrimLODFracTextureEntry.pTexture,color);
	}

	mfac = fac;
	return &m_PrimLODFracTextureEntry;
}

TextureEntry * CTextureCache::GetConstantColorTexture(DWORD constant)
{
	switch( constant )
	{
	case MUX_PRIM:
		return GetPrimColorTexture(gRDP.primitiveColor);
		break;
	case MUX_ENV:
		return GetPrimColorTexture(gRDP.envColor);
		break;
	case MUX_LODFRAC:
	default:	// MUX_PRIMLODFRAC
		return GetPrimColorTexture((BYTE)gRDP.primLODFrac);
		break;
	}
}

void CTextureCache::updateColorTexture(CTexture *ptexture, DWORD color)
{
	DrawInfo di;
	if( !(ptexture->StartUpdate(&di)) )
	{
		TRACE0("Cann't update the texture");
		return;
	}

	int size = ptexture->GetPixelSize();
	switch( size )
	{
	case 2:	// 16 bits
		{
			WORD *buf = (WORD*)di.lpSurface;
			WORD color16=(color>>4)&0xF;
			color16 |= ((color>>12)&0xF)<<4;
			color16 |= ((color>>20)&0xF)<<8;
			color16 |= ((color>>28)&0xF)<<12;
			for( int i=0; i<16; i++ )
			{
				buf[i] = color16;
			}
		}
		break;
	case 4: // 32 bits
		{
			DWORD *buf = (DWORD*)di.lpSurface;
			for( int i=0; i<16; i++ )
			{
				buf[i] = color;
			}
		}
		break;
	}

	ptexture->EndUpdate(&di);
}