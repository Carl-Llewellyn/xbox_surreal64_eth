/*
Copyright (C) 2003 Rice1964

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
#include "../../../config.h"

CTextureManager gTextureManager;
extern RecentCIInfo* g_uRecentCIInfoPtrs[];

//9 would be dwFreeMem but this doesn't work atm - freakdave
//static const DWORD MEM_KEEP_FREE = (6*1024*1024);//freakdave
#ifndef OLDTXTCACHE
static const DWORD MEM_KEEP_FREE = (2*1024*1024); // keep 2MB free
#endif
//static bool g_bUseSetTextureMem = true;		   // we pretty much always wanna use a set texture mem

// Ez0n3 - lets revert until freakdave finishes this ( "Default Max Video Mem" would change "g_maxTextureMemUsage", but it's static instead - unsure why)
// Games seem to be smoother with the old way
bool g_bUseSetTextureMem = true;
DWORD g_maxTextureMemUsage = (5*1024*1024);
DWORD g_amountToFree = (512*1024);

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

CTextureManager::CTextureManager() :
	m_pFirstUsedSurface(NULL),
	m_pTextureHash(NULL),
	m_dwTextureHashSize(0)
{
	m_dwTextureHashSize = GetNextPrime(800);

	// Ez0n3 - old way
	m_currentTextureMemUsage	= 0;
	
	m_pYoungestTexture			= NULL;
	m_pOldestTexture			= NULL;

	m_pTextureHash = new TextureEntry *[m_dwTextureHashSize];
	SAFE_CHECK(m_pTextureHash);

	for (DWORD i = 0; i < m_dwTextureHashSize; i++)
		m_pTextureHash[i] = NULL;

	memset(&m_blackTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_PrimColorTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_EnvColorTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_LODFracTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_PrimLODFracTextureEntry, 0, sizeof(TextureEntry));
}

CTextureManager::~CTextureManager()
{
	DropTextures();
	
#ifdef OLDTXTCACHE
	if (!g_bUseSetTextureMem)
	{
		while (m_pFirstUsedSurface)
		{
			TextureEntry * pVictim = m_pFirstUsedSurface;
			m_pFirstUsedSurface = pVictim->pNext;
			
			delete pVictim;
		}
	}
#endif

	delete []m_pTextureHash;
	m_pTextureHash = NULL;	

	if( m_blackTextureEntry.pTexture )		delete m_blackTextureEntry.pTexture;
	if( m_PrimColorTextureEntry.pTexture )	delete m_PrimColorTextureEntry.pTexture;
	if( m_EnvColorTextureEntry.pTexture )	delete m_EnvColorTextureEntry.pTexture;
	if( m_LODFracTextureEntry.pTexture )	delete m_LODFracTextureEntry.pTexture;
	if( m_PrimLODFracTextureEntry.pTexture )	delete m_PrimLODFracTextureEntry.pTexture;
}


//
// Called when the device-dependent objects are about to be lost.
//
bool CTextureManager::CleanUp()
{
	DropTextures();

	if( m_blackTextureEntry.pTexture )		delete m_blackTextureEntry.pTexture;	
	if( m_PrimColorTextureEntry.pTexture )	delete m_PrimColorTextureEntry.pTexture;
	if( m_EnvColorTextureEntry.pTexture )	delete m_EnvColorTextureEntry.pTexture;
	if( m_LODFracTextureEntry.pTexture )	delete m_LODFracTextureEntry.pTexture;
	if( m_PrimLODFracTextureEntry.pTexture )	delete m_PrimLODFracTextureEntry.pTexture;
	memset(&m_blackTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_PrimColorTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_EnvColorTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_LODFracTextureEntry, 0, sizeof(TextureEntry));
	memset(&m_PrimLODFracTextureEntry, 0, sizeof(TextureEntry));

#ifdef OLDTXTCACHE
	if (!g_bUseSetTextureMem)
	{
		while (m_pFirstUsedSurface)
		{
			TextureEntry * pVictim = m_pFirstUsedSurface;
			m_pFirstUsedSurface = pVictim->pNext;
			
			delete pVictim;
		}
	}
#endif

	return true;
}

// Purge any textures whos last usage was over 5 seconds ago
void CTextureManager::PurgeOldTextures()
{
#ifdef OLDTXTCACHE
	if (m_pTextureHash == NULL)
		return;

	if (g_bUseSetTextureMem)
		return;
	
	static const DWORD dwFramesToKill = 5*30;			// 5 secs at 30 fps
	static const DWORD dwFramesToDelete = 30*30;		// 30 secs at 30 fps
	
	for ( DWORD i = 0; i < m_dwTextureHashSize; i++ )
	{
		TextureEntry * pEntry;
		TextureEntry * pNext;
		
		pEntry = m_pTextureHash[i];
		while (pEntry)
		{
			pNext = pEntry->pNext;
			
			if ( status.gDlistCount - pEntry->FrameLastUsed > dwFramesToKill )
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
		
		if ( status.gDlistCount - pCurr->FrameLastUsed > dwFramesToDelete )
		{
			// Everything from this point on should be too old!
			// Remove from list
			if (pPrev != NULL) pPrev->pNext        = pCurr->pNext;
			else			   m_pFirstUsedSurface = pCurr->pNext;
			
			delete pCurr;
			pCurr = pNext;	
		}
		else
		{
			pPrev = pCurr;
			pCurr = pNext;
		}
	}
#else
	return;
#endif
}

void CTextureManager::DropTextures()
{
	if (m_pTextureHash == NULL)
		return;
	
	DWORD dwCount = 0;
	DWORD dwTotalUses = 0;

	m_pYoungestTexture			= NULL;
	m_pOldestTexture			= NULL;
	
	for (DWORD i = 0; i < m_dwTextureHashSize; i++)
	{
		while (m_pTextureHash[i])
		{
			TextureEntry *pTVictim = m_pTextureHash[i];
			m_pTextureHash[i] = pTVictim->pNext;
			
			dwTotalUses += pTVictim->dwUses;
			dwCount++;
			
#ifdef OLDTXTCACHE
			if (g_bUseSetTextureMem)
				delete pTVictim;
			else
				AddToRecycleList(pTVictim);
#else
				delete pTVictim;
#endif
		}
	}
}


// Add to the recycle list
void CTextureManager::AddToRecycleList(TextureEntry *pEntry)
{
#ifdef OLDTXTCACHE
	if (g_bUseSetTextureMem)
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
		SAFE_DELETE(pEntry->pEnhancedTexture);
		m_pFirstUsedSurface = pEntry;
	}
#else
	return;
#endif
}

// Search for a texture of the specified dimensions to recycle
TextureEntry * CTextureManager::ReviveUsedTexture( DWORD width, DWORD height )
{
#ifdef OLDTXTCACHE
	if (g_bUseSetTextureMem)
		return NULL;

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
			
			return pCurr;
		}
		
		pPrev = pCurr;
		pCurr = pCurr->pNext;
	}
#endif

	return NULL;
}


DWORD CTextureManager::Hash(DWORD dwValue)
{
	// Divide by four, because most textures will be on a 4 byte boundry, so bottom four
	// bits are null
	return (dwValue>>2) % m_dwTextureHashSize;
}

void CTextureManager::MakeTextureYoungest(TextureEntry *pEntry)
{
#ifdef OLDTXTCACHE
	if (!g_bUseSetTextureMem)
		return;
#endif

	if (pEntry == m_pYoungestTexture)
		return;

	// if its the oldest, then change the oldest pointer
	if (pEntry == m_pOldestTexture)
	{
		m_pOldestTexture = pEntry->pNextYoungest;
	}

	// if its a not a new texture, close the gap in the age list
	// where pEntry use to reside
	if (pEntry->pNextYoungest != NULL || pEntry->pLastYoungest != NULL)
	{
		if (pEntry->pNextYoungest != NULL)
		{
			pEntry->pNextYoungest->pLastYoungest = pEntry->pLastYoungest;
		}
		if (pEntry->pLastYoungest != NULL)
		{
			pEntry->pLastYoungest->pNextYoungest = pEntry->pNextYoungest;
		}
	}

	// this texture is now the youngest, so place it on the end of the list
	if (m_pYoungestTexture != NULL)
	{
		m_pYoungestTexture->pNextYoungest = pEntry;
	}

	pEntry->pNextYoungest = NULL;
	pEntry->pLastYoungest = m_pYoungestTexture;
	m_pYoungestTexture = pEntry;
	 
	// if this is the first texture in memory then its also the oldest
	if (m_pOldestTexture == NULL)
	{
		m_pOldestTexture = pEntry;
	}
}

void CTextureManager::AddTextureEntry(TextureEntry *pEntry)
{	
	DWORD dwKey = Hash(pEntry->ti.Address);
	
	if (m_pTextureHash == NULL)
		return;
	
	TextureEntry **p = &m_pTextureHash[dwKey];
	
	// Add to head (not tail, for speed - new textures are more likely to be accessed next)
	pEntry->pNext = m_pTextureHash[dwKey];
	m_pTextureHash[dwKey] = pEntry;

	// Move the texture to the top of the age list
	MakeTextureYoungest(pEntry);
}



TextureEntry * CTextureManager::GetEntry(TextureInfo * pti)
{
	TextureEntry *pEntry;
	
	if (m_pTextureHash == NULL)
		return NULL;
	
	// See if it is already in the hash table
	DWORD dwKey = Hash(pti->Address);

	for (pEntry = m_pTextureHash[dwKey]; pEntry; pEntry = pEntry->pNext)
	{
		if (pEntry->ti == *pti)
		{
			MakeTextureYoungest(pEntry);
			return pEntry;
		}
	}

	return NULL;
}



void CTextureManager::RemoveTextureEntry(TextureEntry * pEntry)
{
	TextureEntry * pPrev;
	TextureEntry * pCurr;
	
	if (m_pTextureHash == NULL)
		return;
	
	// See if it is already in the hash table
	DWORD dwKey = Hash(pEntry->ti.Address);
	
	pPrev = NULL;
	pCurr = m_pTextureHash[dwKey];
	
	while (pCurr)
	{
		// Check that the attributes match
		if ( pCurr->ti == pEntry->ti )
		{
			if (pPrev != NULL) 
				pPrev->pNext = pCurr->pNext;
			else			   
				m_pTextureHash[dwKey] = pCurr->pNext;

#ifdef OLDTXTCACHE
			if (g_bUseSetTextureMem)
			{
#endif
				// remove the texture from the age list
				if (pEntry->pNextYoungest != NULL)
				{
					pEntry->pNextYoungest->pLastYoungest = pEntry->pLastYoungest;
				}
				if (pEntry->pLastYoungest != NULL)
				{
					pEntry->pLastYoungest->pNextYoungest = pEntry->pNextYoungest;
				}

				// decrease the mem usage counter
				m_currentTextureMemUsage -= (pEntry->pTexture->m_dwWidth * pEntry->pTexture->m_dwHeight * 4);
			
				delete pEntry;
#ifdef OLDTXTCACHE
			}
			else
			{
				AddToRecycleList(pEntry);
			}
#endif
			break;
		}

		pPrev = pCurr;
		pCurr = pCurr->pNext;
	}
	
	if (pCurr == NULL)
	{
	}
}

#ifndef OLDTXTCACHE
bool bFreeingTextures = false;
void CTextureManager::FreeTextures()
{
	if(bFreeingTextures)
		return;
		
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);

	// keep freeing textures till enough memory is free
	while (ms.dwAvailPhys < MEM_KEEP_FREE && m_pOldestTexture != NULL)
	{
		if (!bFreeingTextures) bFreeingTextures = true;
	
		TextureEntry *nextYoungest = m_pOldestTexture->pNextYoungest;

		RemoveTextureEntry(m_pOldestTexture);

		m_pOldestTexture = nextYoungest;
		
		//OutputDebugString("Freeing Texture\n");

		GlobalMemoryStatus(&ms);
	}
	
	bFreeingTextures = false;
}
#endif

TextureEntry * CTextureManager::CreateEntry(DWORD dwAddr, DWORD dwWidth, DWORD dwHeight)
{
	TextureEntry * pEntry;


	// Ez0n3 - we'll use the old way until freakdave finishes this
	if (g_bUseSetTextureMem)
	{
		DWORD freeUpSize = (dwWidth * dwHeight * 4) + g_amountToFree;

		// make sure there is enough room for the new texture by deleting old textures
		while ((m_currentTextureMemUsage + freeUpSize) > g_maxTextureMemUsage && m_pOldestTexture != NULL)
		{
			TextureEntry *nextYoungest = m_pOldestTexture->pNextYoungest;

			RemoveTextureEntry(m_pOldestTexture);

			m_pOldestTexture = nextYoungest;

			//OutputDebugString("Freeing Texture\n");
		}

		//m_currentTextureMemUsage += dwWidth * dwHeight * 4;
	}
	else
	{
#ifdef OLDTXTCACHE
		// Find a used texture
		pEntry = ReviveUsedTexture(dwWidth, dwHeight);
#else
		FreeTextures();
#endif
	}
	m_currentTextureMemUsage += (dwWidth * dwHeight * 4);
	
#ifdef OLDTXTCACHE
	if (pEntry == NULL || g_bUseSetTextureMem)
	{
#endif
		// Create a new texture
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
#ifdef OLDTXTCACHE
	}
#endif
	
	// Initialize
	pEntry->ti.Address = dwAddr;
	pEntry->pNext = NULL;
	pEntry->pNextYoungest = NULL;
	pEntry->pLastYoungest = NULL;
	pEntry->dwUses = 0;
	pEntry->dwTimeLastUsed = status.gRDPTime;
	pEntry->dwCRC = 0;
	pEntry->FrameLastUsed = status.gDlistCount;
	pEntry->FrameLastUpdated = 0;
	pEntry->lastEntry = NULL;

	// Add to the hash table
	AddTextureEntry(pEntry);
	return pEntry;
}

// If already in table, return
// Otherwise, create surfaces, and load texture into memory
DWORD dwAsmHeight;
DWORD dwAsmPitch;
DWORD dwAsmdwBytesPerLine;
DWORD dwAsmCRC;
BYTE* pAsmStart;

TextureEntry *g_lastTextureEntry=NULL;
bool lastEntryModified = false;

extern int CheckAndSaveBackBuffer(DWORD addr, DWORD memsize, bool copyToRDRAM = false);
extern DWORD CalculateRDRAMCRC(void *pAddr, DWORD left, DWORD top, DWORD width, DWORD height, DWORD size, DWORD pitchInBytes );

TextureEntry * CTextureManager::GetTexture(TextureInfo * pgti, bool fromTMEM, bool doCRCCheck, bool AutoExtendTexture)
{
	TextureEntry *pEntry;

	if( g_curRomInfo.bDisableTextureCRC )
		doCRCCheck = false;

	gRDP.texturesAreReloaded = true;

	dwAsmCRC = 0;
	DWORD dwPalCRC = 0;

	pEntry = GetEntry(pgti);
	bool loadFromTextureBuffer=false;
	int txtBufIdxToLoadFrom = -1;
	if( (frameBufferOptions.bCheckTxtBufs&&!frameBufferOptions.bWriteBackBufToRDRAM) || (frameBufferOptions.bCheckBackBufs&&!frameBufferOptions.bWriteBackBufToRDRAM) )
	{
		txtBufIdxToLoadFrom = CGraphicsContext::g_pGraphicsContext->CheckAddrInTxtrBufs(*pgti);
		if( txtBufIdxToLoadFrom >= 0 )
		{
			loadFromTextureBuffer = true;
			// Check if it is the same size,
			TextureBufferInfo &info = gTextureBufferInfos[txtBufIdxToLoadFrom];
			//if( info.pTxtBuffer && info.CI_Info.dwAddr == pgti->Address && info.CI_Info.dwFormat == pgti->Format 
			if( info.pTxtBuffer && info.CI_Info.dwFormat == pgti->Format 
				&& info.CI_Info.dwSize == pgti->Size )
			{
				info.txtEntry.ti = *pgti;
				return &info.txtEntry;
			}
		}
	}

	bool loadFromBackBuffer=false;
	if( frameBufferOptions.bCheckBackBufs && CheckAndSaveBackBuffer(pgti->Address, pgti->HeightToLoad*pgti->Pitch) >= 0 )
	{
		if( !frameBufferOptions.bWriteBackBufToRDRAM )
		{
			// Load the texture from recent back buffer
			loadFromBackBuffer = true;
			txtBufIdxToLoadFrom = CGraphicsContext::g_pGraphicsContext->CheckAddrInTxtrBufs(*pgti);
			if( txtBufIdxToLoadFrom >= 0 )
			{
				loadFromTextureBuffer = true;
				// Check if it is the same size,
				TextureBufferInfo &info = gTextureBufferInfos[txtBufIdxToLoadFrom];
				//if( info.pTxtBuffer && info.CI_Info.dwAddr == pgti->Address && info.CI_Info.dwFormat == pgti->Format 
				if( info.pTxtBuffer && info.CI_Info.dwFormat == pgti->Format 
					&& info.CI_Info.dwSize == pgti->Size )
				{
					info.txtEntry.ti = *pgti;
					return &info.txtEntry;
				}
			}
		}
	}

	if (pEntry && pEntry->dwTimeLastUsed == status.gRDPTime && status.gDlistCount != 0 && !status.bN64FrameBufferIsUsed )		// This is not good, Palatte may changes
	{
		// We've already calculated a CRC this frame!
		dwAsmCRC = pEntry->dwCRC;
	}
	else
	{
		if ( doCRCCheck && loadFromTextureBuffer )
		{
			dwAsmCRC = gTextureBufferInfos[txtBufIdxToLoadFrom].crcInRDRAM;
		}
		else if ( doCRCCheck)
		{
			CalculateRDRAMCRC(pgti->pPhysicalAddress, pgti->LeftToLoad, pgti->TopToLoad, pgti->WidthToLoad, pgti->HeightToLoad, pgti->Size, pgti->Pitch);
		}
	}

	if ( doCRCCheck && (pgti->Format == TXT_FMT_CI || (pgti->Format == TXT_FMT_RGBA && pgti->Size <= TXT_SIZE_8b )))
	{
		//Check PAL CRC
		BYTE * pStart;
		DWORD dwPalSize = 16;
		DWORD dwOffset, y;
		if( pgti->Size == TXT_SIZE_8b )
		{
			dwPalSize = 256;
			dwOffset = 0;
		}
		else
		{
			dwOffset = pgti->Palette << 4;
		}

		pStart = (BYTE*)pgti->PalAddress+dwOffset*2;
		for (y = 0; y < dwPalSize*2; y+=4)
		{
			dwPalCRC = (dwPalCRC + *(DWORD*)&pStart[y]);
		}
	}

	if (pEntry && doCRCCheck )
	{
		if(pEntry->dwCRC == dwAsmCRC && pEntry->dwPalCRC == dwPalCRC &&
			(!loadFromTextureBuffer || gTextureBufferInfos[txtBufIdxToLoadFrom].updateAtFrame < pEntry->FrameLastUsed ) )
		{
			// Tile is ok, return
			pEntry->dwUses++;
			pEntry->dwTimeLastUsed = status.gRDPTime;
			pEntry->FrameLastUsed = status.gDlistCount;
#ifdef _DEBUG
			if( logTextures )
			{
				TRACE0("   Use current texture:\n");
			}
#endif
			pEntry->lastEntry = g_lastTextureEntry;
			g_lastTextureEntry = pEntry;
			lastEntryModified = false;

			DEBUGGER_IF_DUMP((pauseAtNext && loadFromTextureBuffer) ,
			{DebuggerAppendMsg("Load cached texture from texture buffer");}
			);

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
		pEntry->ti.WidthToLoad = pEntry->pTexture->m_dwCreatedTextureWidth;
		pEntry->pTexture->m_bScaledS = false;
		pEntry->pTexture->m_bScaledT = false;
	}
	if( pEntry->pTexture->m_dwCreatedTextureHeight < pgti->HeightToCreate )
	{
		pEntry->ti.HeightToLoad = pEntry->pTexture->m_dwCreatedTextureHeight;
		pEntry->pTexture->m_bScaledT = false;
		pEntry->pTexture->m_bScaledS = false;
	}

	try 
	{
		if (pEntry->pTexture != NULL)
		{
			TextureFmt dwType = pEntry->pTexture->GetSurfaceFormat();
			SAFE_DELETE(pEntry->pEnhancedTexture);
			pEntry->dwEnhancementFlag = TEXTURE_NO_ENHANCEMENT;

			if (dwType != TEXTURE_FMT_UNKNOWN)
			{
				if( loadFromTextureBuffer )
				{
					CGraphicsContext::g_pGraphicsContext->LoadTextureFromTextureBuffer(pEntry, txtBufIdxToLoadFrom);
					DEBUGGER_IF_DUMP((pauseAtNext && loadFromTextureBuffer) ,
					{DebuggerAppendMsg("Load texture from texture buffer %d", txtBufIdxToLoadFrom);}
					);

					extern void ConvertTextureRGBAtoI(TextureEntry* pEntry, bool alpha);
					if( g_pTextureBufferInfo->CI_Info.dwFormat == TXT_FMT_I )
					{
						// Convert texture from RGBA to I
						ConvertTextureRGBAtoI(pEntry,false);
					}
					else if( g_pTextureBufferInfo->CI_Info.dwFormat == TXT_FMT_IA )
					{
						// Convert texture from RGBA to IA
						ConvertTextureRGBAtoI(pEntry,true);
					}
				}
				else
				{
					DEBUGGER_IF_DUMP( logTextures, {DebuggerAppendMsg("   Load new texture from RDRAM:\n");});
					if (dwType == TEXTURE_FMT_A8R8G8B8)
						DecompressTexture(pEntry, fromTMEM);		
					else
						DecompressTexture_16(pEntry, fromTMEM);
					pEntry->FrameLastUpdated = status.gDlistCount;

				}
			}

			pEntry->ti.WidthToLoad = pgti->WidthToLoad;
			pEntry->ti.HeightToLoad = pgti->HeightToLoad;
			
			if( AutoExtendTexture )
			{
				ExpandTextureS(pEntry);
				ExpandTextureT(pEntry);
			}


#ifdef _DEBUG
			if( pauseAtNext && eventToPause == NEXT_NEW_TEXTURE )
			{
				CRender::g_pRender->SetCurrentTexture( 0, pEntry->pTexture, pEntry->ti.WidthToCreate, pEntry->ti.HeightToCreate, pEntry);
				CRender::g_pRender->DrawTexture(0);
				debuggerPause = true;
				TRACE0("Pause after loading a new texture");
				if( pEntry->ti.Format == TXT_FMT_YUV )
				{
					TRACE0("This is YUV texture");
				}
				DebuggerAppendMsg("W:%d, H:%d, RealW:%d, RealH:%d, D3DW:%d, D3DH: %d", pEntry->ti.WidthToCreate, pEntry->ti.HeightToCreate,
					pEntry->ti.WidthToLoad, pEntry->ti.HeightToLoad, pEntry->pTexture->m_dwCreatedTextureWidth, pEntry->pTexture->m_dwCreatedTextureHeight);
				DebuggerAppendMsg("ScaledS:%s, ScaledT:%s, CRC=%08X", pEntry->pTexture->m_bScaledS?"T":"F", pEntry->pTexture->m_bScaledT?"T":"F", pEntry->dwCRC);
				DebuggerPause();
				CRender::g_pRender->SetCurrentTexture( 0, NULL, 64, 64, NULL);
			}
#endif
		}
	}
	catch (...)
	{
		TRACE0("Exception in texture decompression");
		g_lastTextureEntry = NULL;
		return NULL;
	}

	pEntry->lastEntry = g_lastTextureEntry;
	g_lastTextureEntry = pEntry;
	lastEntryModified = true;
	return pEntry;
}




char *pszImgFormat[8] = {"RGBA", "YUV", "CI", "IA", "I", "?1", "?2", "?3"};
BYTE pnImgSize[4]   = {4, 8, 16, 32};
char *textlutname[4] = {"RGB16", "I16?", "RGBA16", "IA16"};

extern WORD g_wRDPTlut[];
extern ConvertFunction	gConvertFunctions_FullTMEM[ 8 ][ 4 ];
extern ConvertFunction	gConvertFunctions[ 8 ][ 4 ];
extern ConvertFunction	gConvertTlutFunctions[ 8 ][ 4 ];
extern ConvertFunction	gConvertFunctions_16[ 8 ][ 4 ];
extern ConvertFunction	gConvertFunctions_16_FullTMEM[ 8 ][ 4 ];
extern ConvertFunction	gConvertTlutFunctions_16[ 8 ][ 4 ];
void CTextureManager::DecompressTexture(TextureEntry * pEntry, bool fromTMEM)
{
	static DWORD dwCount = 0;
	
	ConvertFunction pF;
	if( options.bUseFullTMEM && fromTMEM && status.bAllowLoadFromTMEM )
	{
		pF = gConvertFunctions_FullTMEM[ pEntry->ti.Format ][ pEntry->ti.Size ];
	}
	else
	{
		if( gRDP.tiles[7].dwFormat == TXT_FMT_YUV )
		{
			if( gRDP.otherMode.text_tlut>=2 )
				pF = gConvertTlutFunctions[ TXT_FMT_YUV ][ pEntry->ti.Size ];
			else
				pF = gConvertFunctions[ TXT_FMT_YUV ][ pEntry->ti.Size ];
		}
		else
		{
			if( gRDP.otherMode.text_tlut>=2 )
				pF = gConvertTlutFunctions[ pEntry->ti.Format ][ pEntry->ti.Size ];
			else
				pF = gConvertFunctions[ pEntry->ti.Format ][ pEntry->ti.Size ];
		}
	}

	if( pF )
	{
		pF( pEntry->pTexture, pEntry->ti );
	
#ifdef _DEBUG
		if( logTextures )
		{
			DebuggerAppendMsg("Decompress 32bit Texture:\n\tFormat: %s\n\tImage Size:%d\n", 
				pszImgFormat[pEntry->ti.Format], pnImgSize[pEntry->ti.Size]);
			DebuggerAppendMsg("Palette Format: %s (%d)\n", textlutname[pEntry->ti.TLutFmt>>RSP_SETOTHERMODE_SHIFT_TEXTLUT], pEntry->ti.TLutFmt>>RSP_SETOTHERMODE_SHIFT_TEXTLUT);
		}
#endif
	}
	else
	{
		TRACE2("DecompressTexture: Unable to decompress %s/%dbpp", pszImgFormat[pEntry->ti.Format], pnImgSize[pEntry->ti.Size]);
	}

	dwCount++;
}

void CTextureManager::DecompressTexture_16(TextureEntry * pEntry, bool fromTMEM)
{
	static DWORD dwCount = 0;
	
	ConvertFunction pF;

	if( options.bUseFullTMEM && fromTMEM && status.bAllowLoadFromTMEM )
	{
		pF = gConvertFunctions_16_FullTMEM[ pEntry->ti.Format ][ pEntry->ti.Size ];
	}
	else
	{
		if( gRDP.otherMode.text_tlut>=2 )
			pF = gConvertTlutFunctions_16[ pEntry->ti.Format ][ pEntry->ti.Size ];
		else
			pF = gConvertFunctions_16[ pEntry->ti.Format ][ pEntry->ti.Size ];
	}

	if( pF )
	{
		pF( pEntry->pTexture, pEntry->ti );

#ifdef _DEBUG
		if( logTextures )
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
void CTextureManager::ExpandTexture(TextureEntry * pEntry, DWORD sizeToLoad, DWORD sizeToCreate, DWORD sizeCreated,
	int arrayWidth, int flag, int mask, int mirror, int clamp, DWORD otherSize)
{
	if( sizeToLoad >= sizeCreated )	return;

	DWORD maskWidth = (1<<mask);
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


	//if( mask == 0 || clamp )
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
		DWORD tempwidth = clamp ? sizeToCreate : sizeCreated;
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
		Wrap(di.lpSurface, sizeToLoad, mask, sizeCreated, arrayWidth, otherSize, flag, size );

		pEntry->pTexture->EndUpdate(&di);
		return;
	}

	if( sizeToLoad == sizeToCreate && sizeToCreate < maskWidth )
	{
#ifdef _DEBUG
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

void CTextureManager::ExpandTextureS(TextureEntry * pEntry)
{
	TextureInfo &ti =  pEntry->ti;
	DWORD textureWidth = pEntry->pTexture->m_dwCreatedTextureWidth;
	ExpandTexture(pEntry, ti.WidthToLoad, ti.WidthToCreate, textureWidth, 
		textureWidth, S_FLAG, ti.maskS, ti.mirrorS, ti.clampS, ti.HeightToLoad);
}

void CTextureManager::ExpandTextureT(TextureEntry * pEntry)
{
	TextureInfo &ti =  pEntry->ti;
	DWORD textureHeight = pEntry->pTexture->m_dwCreatedTextureHeight;
	DWORD textureWidth = pEntry->pTexture->m_dwCreatedTextureWidth;
	ExpandTexture(pEntry, ti.HeightToLoad, ti.HeightToCreate, textureHeight,
		textureWidth, T_FLAG, ti.maskT, ti.mirrorT, ti.clampT, ti.WidthToLoad);
}


void CTextureManager::ClampS32(DWORD *array, DWORD width, DWORD towidth, DWORD arrayWidth, DWORD rows)
{
	for( DWORD y = 0; y<rows; y++ )
	{
		DWORD* line = array+y*arrayWidth;
		DWORD val = line[width-1];
		for( DWORD x=width; x<towidth; x++ )
		{
			line[x] = val;
		}
	}
}

void CTextureManager::ClampS16(WORD *array, DWORD width, DWORD towidth, DWORD arrayWidth, DWORD rows)
{
	for( DWORD y = 0; y<rows; y++ )
	{
		WORD* line = array+y*arrayWidth;
		WORD val = line[width-1];
		for( DWORD x=width; x<towidth; x++ )
		{
			line[x] = val;
		}
	}
}

void CTextureManager::ClampT32(DWORD *array, DWORD height, DWORD toheight, DWORD arrayWidth, DWORD cols)
{
	DWORD* linesrc = array+arrayWidth*(height-1);
	for( DWORD y = height; y<toheight; y++ )
	{
		DWORD* linedst = array+arrayWidth*y;
		for( DWORD x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureManager::ClampT16(WORD *array, DWORD height, DWORD toheight, DWORD arrayWidth, DWORD cols)
{
	WORD* linesrc = array+arrayWidth*(height-1);
	for( DWORD y = height; y<toheight; y++ )
	{
		WORD* linedst = array+arrayWidth*y;
		for( DWORD x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureManager::MirrorS32(DWORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows)
{
	DWORD maskval1 = (1<<mask)-1;
	DWORD maskval2 = (1<<(mask+1))-1;

	for( DWORD y = 0; y<rows; y++ )
	{
		DWORD* line = array+y*arrayWidth;
		for( DWORD x=width; x<towidth; x++ )
		{
			line[x] = (x&maskval2)<=maskval1 ? line[x&maskval1] : line[maskval2-(x&maskval2)];
		}
	}
}

void CTextureManager::MirrorS16(WORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows)
{
	DWORD maskval1 = (1<<mask)-1;
	DWORD maskval2 = (1<<(mask+1))-1;

	for( DWORD y = 0; y<rows; y++ )
	{
		WORD* line = array+y*arrayWidth;
		for( DWORD x=width; x<towidth; x++ )
		{
			line[x] = (x&maskval2)<=maskval1 ? line[x&maskval1] : line[maskval2-(x&maskval2)];
		}
	}
}

void CTextureManager::MirrorT32(DWORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols)
{
	DWORD maskval1 = (1<<mask)-1;
	DWORD maskval2 = (1<<(mask+1))-1;

	for( DWORD y = height; y<toheight; y++ )
	{
		DWORD srcy = (y&maskval2)<=maskval1 ? y&maskval1 : maskval2-(y&maskval2);
		DWORD* linesrc = array+arrayWidth*srcy;
		DWORD* linedst = array+arrayWidth*y;;
		for( DWORD x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureManager::MirrorT16(WORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols)
{
	DWORD maskval1 = (1<<mask)-1;
	DWORD maskval2 = (1<<(mask+1))-1;

	for( DWORD y = height; y<toheight; y++ )
	{
		DWORD srcy = (y&maskval2)<=maskval1 ? y&maskval1 : maskval2-(y&maskval2);
		WORD* linesrc = array+arrayWidth*srcy;
		WORD* linedst = array+arrayWidth*y;;
		for( DWORD x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureManager::WrapS32(DWORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows)
{
	DWORD maskval = (1<<mask)-1;

	for( DWORD y = 0; y<rows; y++ )
	{
		DWORD* line = array+y*arrayWidth;
		for( DWORD x=width; x<towidth; x++ )
		{
			line[x] = line[(x&maskval)<width?(x&maskval):towidth-(x&maskval)];
		}
	}
}

void CTextureManager::WrapS16(WORD *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows)
{
	DWORD maskval = (1<<mask)-1;

	for( DWORD y = 0; y<rows; y++ )
	{
		WORD* line = array+y*arrayWidth;
		for( DWORD x=width; x<towidth; x++ )
		{
			line[x] = line[(x&maskval)<width?(x&maskval):towidth-(x&maskval)];
		}
	}
}

void CTextureManager::WrapT32(DWORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols)
{
	DWORD maskval = (1<<mask)-1;
	for( DWORD y = height; y<toheight; y++ )
	{
		DWORD* linesrc = array+arrayWidth*(y>maskval?y&maskval:y-height);
		DWORD* linedst = array+arrayWidth*y;;
		for( DWORD x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureManager::WrapT16(WORD *array, DWORD height, DWORD mask, DWORD toheight, DWORD arrayWidth, DWORD cols)
{
	DWORD maskval = (1<<mask)-1;
	for( DWORD y = height; y<toheight; y++ )
	{
		WORD* linesrc = array+arrayWidth*(y>maskval?y&maskval:y-height);
		WORD* linedst = array+arrayWidth*y;;
		for( DWORD x=0; x<arrayWidth; x++ )
		{
			linedst[x] = linesrc[x];
		}
	}
}

void CTextureManager::Clamp(void *array, DWORD width, DWORD towidth, DWORD arrayWidth, DWORD rows, int flag, int size )
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
void CTextureManager::Wrap(void *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows, int flag, int size )
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
void CTextureManager::Mirror(void *array, DWORD width, DWORD mask, DWORD towidth, DWORD arrayWidth, DWORD rows, int flag, int size )
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
TextureEntry * CTextureManager::GetCachedTexture(DWORD tex)
{
	DWORD size = 0;
	for( DWORD i=0; i<m_dwTextureHashSize; i++ )
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
DWORD CTextureManager::GetNumOfCachedTexture()
{
	DWORD size = 0;
	for( DWORD i=0; i<m_dwTextureHashSize; i++ )
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


TextureEntry * CTextureManager::GetBlackTexture(void)
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
TextureEntry * CTextureManager::GetPrimColorTexture(DWORD color)
{
	static DWORD mcolor = 0;
	if( m_PrimColorTextureEntry.pTexture == NULL )
	{
		m_PrimColorTextureEntry.pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(4, 4);
		m_PrimColorTextureEntry.ti.WidthToCreate = 4;
		m_PrimColorTextureEntry.ti.HeightToCreate = 4;
		updateColorTexture(m_PrimColorTextureEntry.pTexture,color);
		gRDP.texturesAreReloaded = true;
	}
	else if( mcolor != color )
	{
		updateColorTexture(m_PrimColorTextureEntry.pTexture,color);
		gRDP.texturesAreReloaded = true;
	}

	mcolor = color;
	return &m_PrimColorTextureEntry;
}
TextureEntry * CTextureManager::GetEnvColorTexture(DWORD color)
{
	static DWORD mcolor = 0;
	if( m_EnvColorTextureEntry.pTexture == NULL )
	{
		m_EnvColorTextureEntry.pTexture = CDeviceBuilder::GetBuilder()->CreateTexture(4, 4);
		m_EnvColorTextureEntry.ti.WidthToCreate = 4;
		m_EnvColorTextureEntry.ti.HeightToCreate = 4;
		gRDP.texturesAreReloaded = true;

		updateColorTexture(m_EnvColorTextureEntry.pTexture,color);
	}
	else if( mcolor != color )
	{
		updateColorTexture(m_EnvColorTextureEntry.pTexture,color);
		gRDP.texturesAreReloaded = true;
	}

	mcolor = color;
	return &m_EnvColorTextureEntry;
}
TextureEntry * CTextureManager::GetLODFracTexture(BYTE fac)
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
		gRDP.texturesAreReloaded = true;
	}
	else if( mfac != fac )
	{
		DWORD factor = fac;
		DWORD color = fac;
		color |= factor << 8;
		color |= color << 16;
		updateColorTexture(m_LODFracTextureEntry.pTexture,color);
		gRDP.texturesAreReloaded = true;
	}

	mfac = fac;
	return &m_LODFracTextureEntry;
}

TextureEntry * CTextureManager::GetPrimLODFracTexture(BYTE fac)
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
		gRDP.texturesAreReloaded = true;
	}
	else if( mfac != fac )
	{
		DWORD factor = fac;
		DWORD color = fac;
		color |= factor << 8;
		color |= color << 16;
		updateColorTexture(m_PrimLODFracTextureEntry.pTexture,color);
		gRDP.texturesAreReloaded = true;
	}

	mfac = fac;
	return &m_PrimLODFracTextureEntry;
}

TextureEntry * CTextureManager::GetConstantColorTexture(DWORD constant)
{
	switch( constant )
	{
	case MUX_PRIM:
		return GetPrimColorTexture(gRDP.primitiveColor);
		break;
	case MUX_ENV:
		return GetEnvColorTexture(gRDP.envColor);
		break;
	case MUX_LODFRAC:
		return GetLODFracTexture((BYTE)gRDP.LODFrac);
		break;
	default:	// MUX_PRIMLODFRAC
		return GetPrimLODFracTexture((BYTE)gRDP.primLODFrac);
		break;
	}
}

void CTextureManager::updateColorTexture(CTexture *ptexture, DWORD color)
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
			WORD color16= (WORD)((color>>4)&0xF);
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

