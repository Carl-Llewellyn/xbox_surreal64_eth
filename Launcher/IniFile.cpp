/**
 * Surreal 64 Launcher (C) 2003
 * 
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: buttza@hotmail.com, lantus@lantus-x.com
 */

#include "IniFile.h"
#include "Util.h"

// Ez0n3 - determine if the current phys ram is greater than 100MB
extern bool has128ram;

IniFile g_iniFile;

IniFile::IniFile(void)
{
	// allocate 64k for the rom ini entry 'hash' table
	m_entryTable = new RomIniEntry *[0x10000];

	for (dword i = 0; i < 0x10000; i++)
	{
		m_entryTable[i] = NULL;
	}

	// create a default ini entry, this will be used for games that
	// dont yet have an entry and for games that have an entry but dont
	// use every ini option
	memset(&m_defaultIniEntry, 0, sizeof(RomIniEntry));
	m_defaultIniEntry.pbEmuSupported[_1964]		= true;
	m_defaultIniEntry.pbEmuSupported[_Project64]	= true;
	m_defaultIniEntry.pbEmuSupported[_UltraHLE]	= false;
	m_defaultIniEntry.preferedemu = _1964;
	
	// Ez0n3 - default plugins per ini
	//m_defaultIniEntry.videoplugin = _VideoPluginMissing; // = num of video plugins + 1
	//m_defaultIniEntry.iAudioPlugin = _AudioPluginMissing; // = num of audio plugins + 1
	

	byte byDefaultConfig[76] 
		= {0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F,
		   0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F,
		   0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F,
		   0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,
		   0x04,0x05,0x06,0x07,0x0C,0x10,0x12,0x16,
		   0x0D,0x17,0x0F};

	memcpy(&m_defaultIniEntry.pbyControllerConfig, byDefaultConfig, 76);

	m_defaultIniEntry.dw1964DynaMem = 8;
	m_defaultIniEntry.dw1964PagingMem = 4;
	
	m_defaultIniEntry.dwPJ64DynaMem = 16; // 8 // it's 16 everywhere else, why buck the sys
	m_defaultIniEntry.dwPJ64PagingMem = 4;
	
	m_defaultIniEntry.dwMaxVideoMem = 5; // reinstate max video mem
	
	// ultrahle mem settings
	m_defaultIniEntry.dwUltraCodeMem = 5;
	m_defaultIniEntry.dwUltraGroupMem = 10;

	// some more defaults
	m_defaultIniEntry.iAudioPlugin = _AudioPluginJttl;
	m_defaultIniEntry.videoplugin = _VideoPluginRice560;
	m_defaultIniEntry.iRspPlugin = _RSPPluginHLE;
	m_defaultIniEntry.bUseRspAudio = false;
	
	m_defaultIniEntry.iPagingMethod = _PagingXXX;
	
	 // leave for ini
	m_defaultIniEntry.bUseLLERSP = false;
	m_defaultIniEntry.bUseBasicAudio = false;
	
	m_bIniLoaded = false;
}

IniFile::~IniFile(void)
{
	for (dword i = 0; i < 0x10000; i++)
	{
		if (m_entryTable[i] != NULL)
		{
			RomIniEntry *delEntry = m_entryTable[i];
			RomIniEntry *nextEntry = delEntry->pNextEntry;
			delete delEntry;

			while (nextEntry != NULL)
			{
				delEntry = nextEntry;
				nextEntry = delEntry->pNextEntry;
				delete delEntry;
			}
		}
	}

	delete [] m_entryTable;
	
	m_bIniLoaded = false;
}

bool IniFile::Load(const string &szIniFilename)
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(true); // only grab the 1st one if multiple are set
	ini.SetMultiLine(false); // vars should not span multiple lines
	ini.SetSpaces(false); // spaces before and after "="

	char szFilename[256];
	sprintf(szFilename, "T:\\%s", szIniFilename.c_str()); // try T first
	
	rc = ini.LoadFile(szFilename);
	if (rc < 0) 
	{
		OutputDebugString(szFilename);
		OutputDebugStringA(" Failed to Load!\n");
		//return false;
		
		sprintf(szFilename, "D:\\%s", szIniFilename.c_str()); // then try D
		rc = ini.LoadFile(szFilename);
		if (rc < 0) 
		{
			OutputDebugString(szFilename);
			OutputDebugStringA(" Failed to Load!\n");
			
			// if both fail, save a new one to T
			sprintf(szFilename, "T:\\%s", szIniFilename.c_str());
			bool bCreateNewIniFile = CreateAndSaveDefaultIniEntry(szFilename);
			return bCreateNewIniFile;
		}
	}

	OutputDebugString(szFilename);
	OutputDebugStringA(" Successfully Loaded!\n");

	string szStrBuf; // temp string
	const TCHAR *pszSection = 0;
	RomIniEntry *pCurrentEntry = NULL;
	
	//m_currentIniEntry = m_defaultIniEntry;
	
	// set setting based on phys ram
	// also set in ConfigAppLoad3
	unsigned int iMaxVideoMem = 10;
	unsigned int iMaxDynaMem = 20;
	unsigned int iMaxPagingMem = 20;
	if (has128ram) {
		iMaxDynaMem = 32;
		iMaxPagingMem = 64;
	}

	m_szRomPath = FixPath( ini.GetValue("Settings", "Rom Path", "D:\\Roms\\" ) );
	m_szMediaPath = FixPath( ini.GetValue("Settings", "Media Path", "D:\\Media\\" ) );
	
	m_szSkinPath = FixPath( ini.GetValue("Settings", "Skin Path", "D:\\Skins\\" ) );
	m_szSavePath = FixPath( ini.GetValue("Settings", "Save Path", "D:\\Saves\\" ) );
	m_szScreenshotPath = FixPath( ini.GetValue("Settings", "Screenshot Path", "D:\\Screenshots\\" ) );
	
	// default controller config
	szStrBuf = ini.GetValue("Settings", "Default Controller Config", "" ); //NULL
	if (szStrBuf.size() >= 220)
	{
		for (int i = 0; i < 76; i++)
		{
			m_currentIniEntry.pbyControllerConfig[i] = static_cast<byte>(strtoul(szStrBuf.substr((i*3), 2).c_str(), NULL, 16));
		}
	}
	szStrBuf.clear();
	
	//m_currentIniEntry.preferedemu = ini.GetLongValue("Settings", "Default Preferred Emulator", m_defaultIniEntry.preferedemu );
	m_currentIniEntry.preferedemu = ini.GetLongValue("Settings", "Default Emulator", ini.GetLongValue("Settings", "Default Preferred Emulator", m_defaultIniEntry.preferedemu ));
	if (m_currentIniEntry.preferedemu >= _None) 
		m_currentIniEntry.preferedemu = m_defaultIniEntry.preferedemu;

	m_currentIniEntry.dw1964DynaMem = ini.GetLongValue("Settings", "Default 1964 Dyna Mem", m_defaultIniEntry.dw1964DynaMem );
	if (m_currentIniEntry.dw1964DynaMem > iMaxDynaMem) 
		m_currentIniEntry.dw1964DynaMem = iMaxDynaMem;
		
	m_currentIniEntry.dw1964PagingMem = ini.GetLongValue("Settings", "Default 1964 Paging Mem", m_defaultIniEntry.dw1964PagingMem );
	if (m_currentIniEntry.dw1964PagingMem > iMaxPagingMem) 
		m_currentIniEntry.dw1964PagingMem = iMaxPagingMem;
	
	m_currentIniEntry.dwPJ64DynaMem = ini.GetLongValue("Settings", "Default PJ64 Dyna Mem", m_defaultIniEntry.dwPJ64DynaMem );
	if (m_currentIniEntry.dwPJ64DynaMem > iMaxDynaMem) 
		m_currentIniEntry.dwPJ64DynaMem = iMaxDynaMem;
		
	m_currentIniEntry.dwPJ64PagingMem = ini.GetLongValue("Settings", "Default PJ64 Paging Mem", m_defaultIniEntry.dwPJ64PagingMem );
	if (m_currentIniEntry.dwPJ64PagingMem > iMaxPagingMem) 
		m_currentIniEntry.dwPJ64PagingMem = iMaxPagingMem;
	
	// ultrahle mem settings
	m_currentIniEntry.dwUltraCodeMem = ini.GetLongValue("Settings", "Default Ultra Code Mem", m_defaultIniEntry.dwUltraCodeMem );
	if (m_currentIniEntry.dwUltraCodeMem > iMaxDynaMem) 
		m_currentIniEntry.dwUltraCodeMem = iMaxDynaMem;
		
	m_currentIniEntry.dwUltraGroupMem = ini.GetLongValue("Settings", "Default Ultra Group Mem", m_defaultIniEntry.dwUltraGroupMem );
	if (m_currentIniEntry.dwUltraGroupMem > iMaxDynaMem) 
		m_currentIniEntry.dwUltraGroupMem = iMaxDynaMem;

	// some more defaults
	m_currentIniEntry.dwMaxVideoMem = ini.GetLongValue("Settings", "Default Max Video Mem", m_defaultIniEntry.dwMaxVideoMem ); // reinstate max video mem
	if (m_currentIniEntry.dwMaxVideoMem > iMaxVideoMem) 
		m_currentIniEntry.dwMaxVideoMem = iMaxVideoMem;
	
	m_currentIniEntry.videoplugin = ini.GetLongValue("Settings", "Default Video Plugin", m_defaultIniEntry.videoplugin );
	if (m_currentIniEntry.videoplugin >= _VideoPluginMissing)
		m_currentIniEntry.videoplugin = m_defaultIniEntry.videoplugin;
	
	m_currentIniEntry.iAudioPlugin = ini.GetLongValue("Settings", "Default Audio Plugin", m_defaultIniEntry.iAudioPlugin );
	if (m_currentIniEntry.iAudioPlugin >= _AudioPluginMissing)
		m_currentIniEntry.iAudioPlugin = m_defaultIniEntry.iAudioPlugin;
	
	m_currentIniEntry.iRspPlugin = ini.GetLongValue("Settings", "Default Rsp Plugin", m_defaultIniEntry.iRspPlugin );
	if (m_currentIniEntry.iRspPlugin >= _RSPPluginMissing)
		m_currentIniEntry.iRspPlugin = m_defaultIniEntry.iRspPlugin;

		
	m_currentIniEntry.iPagingMethod = ini.GetLongValue("Settings", "Default Paging Method", m_defaultIniEntry.iPagingMethod );
	if (m_currentIniEntry.iPagingMethod >= _PagingMissing)
		m_currentIniEntry.iPagingMethod = m_defaultIniEntry.iPagingMethod;
		
	m_currentIniEntry.bUseRspAudio = ini.GetBoolValue("Settings", "Default Use Rsp Audio", m_defaultIniEntry.bUseRspAudio );
	
	
	// these two are not used anymore in the emu's, but leave them to retain user setting between versions
	m_currentIniEntry.bUseLLERSP = ini.GetBoolValue("Settings", "Default Use LLE RSP", m_defaultIniEntry.bUseLLERSP );
	m_currentIniEntry.bUseLLERSP = ini.GetBoolValue("Settings", "Default Use Basic Audio", m_defaultIniEntry.bUseBasicAudio );
	
	
	// need to parse all of the specific rom settings
	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);
	CSimpleIniA::TNamesDepend::const_iterator iSection = sections.begin();
	for ( ; iSection != sections.end(); ++iSection ) {
		pszSection = iSection->pItem;
		if (!*pszSection) continue;
		
		int iLineLength = 0;
		char szRomCrcs[23]; //25
		sprintf(szRomCrcs, "%s", pszSection);
		string szLine (Trim(szRomCrcs));
		
		/*OutputDebugStringA("INI: ");
		OutputDebugString(szLine.c_str());
		OutputDebugString("\n");*/

		iLineLength = szLine.length();
		if (iLineLength != 22 || szLine.find("-") != 8 || szLine.find("-C:") != 17) continue; // not a valid rom entry
		
		// extract the crc1/crc2 and country values from the string // [xxxxxxxx-xxxxxxxx-C:xx]
		dword dwCrc1		= strtoul(szLine.substr(0, 8).c_str(), NULL, 16);
		dword dwCrc2		= strtoul(szLine.substr(9, 8).c_str(), NULL, 16);
		byte  byCountry		= static_cast<byte>(strtoul(szLine.substr(20, 2).c_str(), NULL, 16));
		
		// create a new entry
		pCurrentEntry = CreateRomEntry(dwCrc1, dwCrc2, byCountry);

		
		strncpy(pCurrentEntry->szGameName, ini.GetValue(szRomCrcs, "Game Name", "" ), sizeof(pCurrentEntry->szGameName) - 1); //NULL
		strncpy(pCurrentEntry->szAltTitle, ini.GetValue(szRomCrcs, "Alternate Title", "" ), sizeof(pCurrentEntry->szAltTitle) - 1); //NULL
		strncpy(pCurrentEntry->szComments, ini.GetValue(szRomCrcs, "Comments", "" ), sizeof(pCurrentEntry->szComments) - 1); //NULL

		szStrBuf = ini.GetValue(szRomCrcs, "Emulators Supported", "" ); //NULL
		if (szStrBuf.size() >= 5) // theres no way that it can be shorter than 5 characters
		{
			for (int i = 0; i < 3; i++)
			{
				int val;
				stringstream conv(szStrBuf.substr((i*2), 1));
				conv >> val;

				pCurrentEntry->pbEmuSupported[i] = (val != 0);
			}
		}
		szStrBuf.clear();
		
		// controller config
		szStrBuf = ini.GetValue(szRomCrcs, "Controller Config", "" ); //NULL
		if (szStrBuf.size() >= 220) // theres no way that it can be shorter than 220 characters
		{
			for (int i = 0; i < 72; i++)
			{
				pCurrentEntry->pbyControllerConfig[i] = static_cast<byte>(strtoul(szStrBuf.substr((i*3), 2).c_str(), NULL, 16));
			}
		}
		szStrBuf.clear();
		
		
		// the rest of these are only used by Save() [below] and ConfigAppLoad3
		// so, we'll let it fill the hash even if it's not set
		// but if it's not, we'll set numeric values to -1 and non-set bools to NULL
		// that way, we can check against the hash and be sure if a value it set or not
		// before, checking a var for NULL would also be true for "0" and screw up the results
		// we'll run the checks we were running here in ConfigAppLoad3, were they will used
		
		// preferred emulator
		//pCurrentEntry->preferedemu = ini.GetLongValue(szRomCrcs, "Preferred Emulator", m_currentIniEntry.preferedemu );
		pCurrentEntry->preferedemu = ini.GetLongValue(szRomCrcs, "Emulator", ini.GetLongValue(szRomCrcs, "Preferred Emulator", m_currentIniEntry.preferedemu));

		// preferred video plugin
		//pCurrentEntry->videoplugin = ini.GetLongValue(szRomCrcs, "Preferred Video Plugin", m_currentIniEntry.videoplugin );
		pCurrentEntry->videoplugin = ini.GetLongValue(szRomCrcs, "Video Plugin", ini.GetLongValue(szRomCrcs, "Preferred Video Plugin", m_currentIniEntry.videoplugin));

		// preferred audio plugin
		//pCurrentEntry->iAudioPlugin = ini.GetLongValue(szRomCrcs, "Preferred Audio Plugin", m_currentIniEntry.iAudioPlugin );
		pCurrentEntry->iAudioPlugin = ini.GetLongValue(szRomCrcs, "Audio Plugin", ini.GetLongValue(szRomCrcs, "Preferred Audio Plugin", m_currentIniEntry.iAudioPlugin));

		// preferred rsp plugin
		//pCurrentEntry->iRspPlugin = ini.GetLongValue(szRomCrcs, "Preferred Rsp Plugin", m_currentIniEntry.iRspPlugin );
		pCurrentEntry->iRspPlugin = ini.GetLongValue(szRomCrcs, "Rsp Plugin", ini.GetLongValue(szRomCrcs, "Preferred Rsp Plugin", m_currentIniEntry.iRspPlugin));

		// paging method
		//pCurrentEntry->iPagingMethod = ini.GetLongValue(szRomCrcs, "Preferred Paging Method", m_currentIniEntry.iPagingMethod );
		pCurrentEntry->iPagingMethod = ini.GetLongValue(szRomCrcs, "Paging Method", ini.GetLongValue(szRomCrcs, "Preferred Paging Method", m_currentIniEntry.iPagingMethod));
		
		
		pCurrentEntry->dw1964DynaMem = ini.GetLongValue(szRomCrcs, "1964 Dyna Mem", m_currentIniEntry.dw1964DynaMem );
		
		pCurrentEntry->dw1964PagingMem = ini.GetLongValue(szRomCrcs, "1964 Paging Mem", m_currentIniEntry.dw1964PagingMem );
		
		pCurrentEntry->dwPJ64DynaMem = ini.GetLongValue(szRomCrcs, "PJ64 Dyna Mem", m_currentIniEntry.dwPJ64DynaMem );
		
		pCurrentEntry->dwPJ64PagingMem = ini.GetLongValue(szRomCrcs, "PJ64 Paging Mem", m_currentIniEntry.dwPJ64PagingMem );

		// ultrahle mem settings
		pCurrentEntry->dwUltraCodeMem = ini.GetLongValue(szRomCrcs, "Ultra Code Mem", m_currentIniEntry.dwUltraCodeMem );
		
		pCurrentEntry->dwUltraGroupMem = ini.GetLongValue(szRomCrcs, "Ultra Group Mem", m_currentIniEntry.dwUltraGroupMem );
		
		pCurrentEntry->dwMaxVideoMem = ini.GetLongValue(szRomCrcs, "Max Video Mem", m_currentIniEntry.dwMaxVideoMem );

		pCurrentEntry->bUseRspAudio = ini.GetBoolValue(szRomCrcs, "Use Rsp Audio", m_currentIniEntry.bUseRspAudio ); // control a listing

		// these two are not used anymore in the emu's, but leave them to retain user setting between versions
		pCurrentEntry->bUseLLERSP = ini.GetBoolValue(szRomCrcs, "Use LLE RSP", m_currentIniEntry.bUseLLERSP ); //NULL
		pCurrentEntry->bUseBasicAudio = ini.GetBoolValue(szRomCrcs, "Use Basic Audio", m_currentIniEntry.bUseLLERSP ); //NULL
	}
	
	m_bIniLoaded = true;

	return true;
}

bool IniFile::Save(const string &szIniFilename)
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
    ini.SetMultiKey(true);
    ini.SetMultiLine(false);
	ini.SetSpaces(false); // spaces before and after =

	string szStrBuf; // temp string
	
	//SETTINGS
	ini.SetValue("Settings", "Rom Path", m_szRomPath.c_str());
	ini.SetValue("Settings", "Media Path", m_szMediaPath.c_str());
	
	ini.SetValue("Settings", "Skin Path", m_szSkinPath.c_str());
	ini.SetValue("Settings", "Save Path", m_szSavePath.c_str());
	ini.SetValue("Settings", "Screenshot Path", m_szScreenshotPath.c_str());

	// default controller config
	{
		for (int i = 0; i < 72; i++)
		{
			char buf[3];
			sprintf(buf, "%02X", m_currentIniEntry.pbyControllerConfig[i]);
			szStrBuf.append(buf);

			if (i < 71) szStrBuf.append(",");
		}
		ini.SetValue("Settings", "Default Controller Config", szStrBuf.c_str());
		szStrBuf.clear();
	}
	
	// other rom defaults
	ini.SetLongValue("Settings", "Default 1964 Dyna Mem", m_currentIniEntry.dw1964DynaMem);
	ini.SetLongValue("Settings", "Default 1964 Paging Mem", m_currentIniEntry.dw1964PagingMem);
	ini.SetLongValue("Settings", "Default PJ64 Dyna Mem", m_currentIniEntry.dwPJ64DynaMem);
	ini.SetLongValue("Settings", "Default PJ64 Paging Mem", m_currentIniEntry.dwPJ64PagingMem);
	
	// ultrahle mem settings
	ini.SetLongValue("Settings", "Default Ultra Code Mem", m_currentIniEntry.dwUltraCodeMem);
	ini.SetLongValue("Settings", "Default Ultra Group Mem", m_currentIniEntry.dwUltraGroupMem);
	
	// leave for ini
	ini.SetBoolValue("Settings", "Default Use LLE RSP", m_currentIniEntry.bUseLLERSP);
	ini.SetBoolValue("Settings", "Default Use Basic Audio", m_currentIniEntry.bUseBasicAudio);
	
	// reinstate max video mem
	ini.SetLongValue("Settings", "Default Max Video Mem", m_currentIniEntry.dwMaxVideoMem);
	
	// some more defaults
	ini.SetLongValue("Settings", "Default Video Plugin", m_currentIniEntry.videoplugin);
	ini.SetLongValue("Settings", "Default Audio Plugin", m_currentIniEntry.iAudioPlugin);
	ini.SetLongValue("Settings", "Default Rsp Plugin", m_currentIniEntry.iRspPlugin);
	ini.SetBoolValue("Settings", "Default Use Rsp Audio", m_currentIniEntry.bUseRspAudio); // control a listing
	
	ini.SetLongValue("Settings", "Default Paging Method", m_currentIniEntry.iPagingMethod);
	
	
	// write out all the rom settings
	for (dword i = 0; i < 0x10000; i++)
	{
		if (m_entryTable[i] != NULL)
		{
			RomIniEntry *pCurrentEntry = m_entryTable[i];
			
			do
			{
				// output the rom's crc and country values
				char szRomCrcs[23]; //25
				sprintf(szRomCrcs, "%08X-%08X-C:%02X", pCurrentEntry->dwCrc1, pCurrentEntry->dwCrc2, pCurrentEntry->byCountry);
				
				// always output the game name, alt title and comments
				ini.SetValue(szRomCrcs, "Game Name", pCurrentEntry->szGameName);
				ini.SetValue(szRomCrcs, "Alternate Title", pCurrentEntry->szAltTitle);
				ini.SetValue(szRomCrcs, "Comments", pCurrentEntry->szComments);
				
				// from now on, only output values that differ from the current default values
				
				// emulators supported
				if (pCurrentEntry->pbEmuSupported[_1964] != m_currentIniEntry.pbEmuSupported[_1964] || 
					pCurrentEntry->pbEmuSupported[_Project64] != m_currentIniEntry.pbEmuSupported[_Project64] ||
					pCurrentEntry->pbEmuSupported[_UltraHLE] != m_currentIniEntry.pbEmuSupported[_UltraHLE])
				{
				
					szStrBuf.append(pCurrentEntry->pbEmuSupported[_1964] ? "1," : "0,");
					szStrBuf.append(pCurrentEntry->pbEmuSupported[_1964] ? "1," : "0,");
					szStrBuf.append(pCurrentEntry->pbEmuSupported[_UltraHLE] ? "1" : "0");
					
					ini.SetValue(szRomCrcs, "Emulators Supported", szStrBuf.c_str());
					szStrBuf.clear();
				}
				
				// controller config
				if (memcmp(pCurrentEntry->pbyControllerConfig, m_currentIniEntry.pbyControllerConfig, sizeof(m_currentIniEntry.pbyControllerConfig)) != 0)
				{
					for (int i = 0; i < 76; i++)
					{
						char buf[3];
						sprintf(buf, "%02X", pCurrentEntry->pbyControllerConfig[i]);
						szStrBuf.append(buf);

						if (i < 75) szStrBuf.append(",");
					}
					ini.SetValue(szRomCrcs, "Controller Config", szStrBuf.c_str());
					szStrBuf.clear();
				}
				
				// preferred options

				// changing the way values are loaded, so also need to change this a bit
				// this only gets used if there is no surreal.ini to load
				// it will save out an ini with the defaults and the 00000000 rom entry
				// although, the 00000000 rom entry won't use any of these
				// just for kicks i guess :P
				
				// save using shorthand
				
				// preferred emulator
				if (pCurrentEntry->preferedemu != m_currentIniEntry.preferedemu)
					//ini.SetLongValue(szRomCrcs, "Preferred Emulator", pCurrentEntry->preferedemu);
					ini.SetLongValue(szRomCrcs, "Emulator", pCurrentEntry->preferedemu);
				
				// preferred video plugin
				if (pCurrentEntry->videoplugin != m_currentIniEntry.videoplugin)
					//ini.SetLongValue(szRomCrcs, "Preferred Video Plugin", pCurrentEntry->videoplugin);
					ini.SetLongValue(szRomCrcs, "Video Plugin", pCurrentEntry->videoplugin);
				
				// preferred audio plugin
				if (pCurrentEntry->iAudioPlugin != m_currentIniEntry.iAudioPlugin)
					//ini.SetLongValue(szRomCrcs, "Preferred Audio Plugin", pCurrentEntry->iAudioPlugin);
					ini.SetLongValue(szRomCrcs, "Audio Plugin", pCurrentEntry->iAudioPlugin);
				
				// preferred rsp plugin
				if (pCurrentEntry->iRspPlugin != m_currentIniEntry.iRspPlugin)
					//ini.SetLongValue(szRomCrcs, "Preferred Rsp Plugin", pCurrentEntry->iRspPlugin);
					ini.SetLongValue(szRomCrcs, "Rsp Plugin", pCurrentEntry->iRspPlugin);

				// preferred paging method
				if (pCurrentEntry->iPagingMethod != m_currentIniEntry.iPagingMethod)
					//ini.SetLongValue(szRomCrcs, "Preferred Paging Method", pCurrentEntry->iPagingMethod);
					ini.SetLongValue(szRomCrcs, "Paging Method", pCurrentEntry->iPagingMethod);
				
				// memory options

				if (pCurrentEntry->dw1964DynaMem != m_currentIniEntry.dw1964DynaMem)
					ini.SetLongValue(szRomCrcs, "1964 Dyna Mem", pCurrentEntry->dw1964DynaMem);
				
				if (pCurrentEntry->dw1964PagingMem != m_currentIniEntry.dw1964PagingMem)
					ini.SetLongValue(szRomCrcs, "1964 Paging Mem", pCurrentEntry->dw1964PagingMem);
				
				if (pCurrentEntry->dwPJ64DynaMem != m_currentIniEntry.dwPJ64DynaMem)
					ini.SetLongValue(szRomCrcs, "PJ64 Dyna Mem", pCurrentEntry->dwPJ64DynaMem);
				
				if (pCurrentEntry->dwPJ64PagingMem != m_currentIniEntry.dwPJ64PagingMem)
					ini.SetLongValue(szRomCrcs, "PJ64 Paging Mem", pCurrentEntry->dwPJ64PagingMem);

				// ultrahle mem settings
				if (pCurrentEntry->dwUltraCodeMem != m_currentIniEntry.dwUltraCodeMem)
					ini.SetLongValue(szRomCrcs, "Ultra Code Mem", pCurrentEntry->dwUltraCodeMem);
				
				if (pCurrentEntry->dwUltraGroupMem != m_currentIniEntry.dwUltraGroupMem)
					ini.SetLongValue(szRomCrcs, "Ultra Group Mem", pCurrentEntry->dwUltraGroupMem);

				if (pCurrentEntry->dwMaxVideoMem != m_currentIniEntry.dwMaxVideoMem) // reinstate max video mem
					ini.SetLongValue(szRomCrcs, "Max Video Mem", pCurrentEntry->dwMaxVideoMem);
					
				if (pCurrentEntry->bUseRspAudio != m_currentIniEntry.bUseRspAudio)
					ini.SetBoolValue(szRomCrcs, "Use Rsp Audio", pCurrentEntry->bUseRspAudio);
					
				
				// not used anymore but leave for ini
				if (pCurrentEntry->bUseLLERSP != m_currentIniEntry.bUseLLERSP)
					ini.SetBoolValue(szRomCrcs, "Use LLE RSP", pCurrentEntry->bUseLLERSP);

				if (pCurrentEntry->bUseBasicAudio != m_currentIniEntry.bUseBasicAudio)
					ini.SetBoolValue(szRomCrcs, "Use Basic Audio", pCurrentEntry->bUseBasicAudio);

				pCurrentEntry = pCurrentEntry->pNextEntry;
			}
			while (pCurrentEntry != NULL);
		}
	}

	OutputDebugString(szIniFilename.c_str());
	rc = ini.SaveFile(szIniFilename.c_str());
    if (rc < 0) 
	{
		OutputDebugStringA(" Failed to Save!\n");
		return false;
	}
	OutputDebugStringA(" Saved Successfully!\n");
	
	return true;
}


// new
bool IniFile::CreateAndSaveDefaultIniEntry(const string &szIniFileName)
{
	m_szRomPath = "D:\\Roms\\";
	m_szMediaPath = "D:\\Media\\";
	
	m_szSkinPath = "D:\\Skins\\";
	m_szSavePath = "D:\\Saves\\";
	m_szScreenshotPath = "D:\\Screenshots\\";

	// our current ini is now the default ini
	//memset(&m_currentIniEntry, 0, sizeof(RomIniEntry));
	m_currentIniEntry = m_defaultIniEntry;
	
	CreateRomEntry(00000000, 00000000, 00); // make a dummy entry

	// save the default ini
	bool bSaveIniFile = Save(szIniFileName.c_str());
	
	return bSaveIniFile;
}

// new - needed?
bool IniFile::CheckForIniEntry(const string &szIniFileName)
{
	// try to load our ini file
	if(!Load(szIniFileName.c_str()))
	{
		// create a new one, if it doesn't exist
		CreateAndSaveDefaultIniEntry(szIniFileName.c_str());
	}
	
	return true;
}

bool IniFile::IsLoaded()
{
	return m_bIniLoaded;
}

RomIniEntry *IniFile::CreateRomEntry(dword crc1, dword crc2, byte country)
{
	// create a new ini entry struct for this entry 
	// and copy the default ini entry into it
	RomIniEntry *newEntry = new RomIniEntry;
	memcpy(newEntry, &m_currentIniEntry, sizeof(RomIniEntry));

	newEntry->dwCrc1 = crc1;
	newEntry->dwCrc2 = crc2;
	newEntry->byCountry = country;
	newEntry->pNextEntry = NULL;

	// place the new entry in the table according to the first 16
	// bits of its crc1. if the entry is already used, iterate
	// until the end of the chain and place the new entry there.
	word tableLocation = static_cast<word>(crc1 >> 16);

	if (m_entryTable[tableLocation] == NULL)
	{
		m_entryTable[tableLocation] = newEntry;
	}
	else
	{
		RomIniEntry *tmpEntry = m_entryTable[tableLocation];

		// keep looping till we reach the end of the chain
		while (tmpEntry->pNextEntry != NULL)
		{					
			tmpEntry = tmpEntry->pNextEntry;
		}
		
		tmpEntry->pNextEntry = newEntry;
	}

	return newEntry;
}

RomIniEntry *IniFile::GetRomEntry(dword crc1, dword crc2, byte country)
{
	// look up the table using the first 16 bits of crc1
	word tableLocation = static_cast<word>(crc1 >> 16);

	RomIniEntry *curEntry = m_entryTable[tableLocation];

	// search through the chain
	while (curEntry != NULL)
	{
		if (curEntry->dwCrc1 == crc1 &&
			curEntry->dwCrc2 == crc2 &&
			curEntry->byCountry == country)
		{
			// a match was found
			return curEntry;
		}

		curEntry = curEntry->pNextEntry;
	}

	// no entry was found that matches
	return NULL;
}

//needed?
RomIniEntry *IniFile::GetDefaultRomEntry()
{
	return &m_currentIniEntry;
}

string IniFile::GetRomPath()
{
	return m_szRomPath;
}

// needed?
void IniFile::SetRomPath(const string &romPath)
{
	m_szRomPath = romPath;
}

string IniFile::GetMediaPath()
{
	return m_szMediaPath;
}

//needed?
void IniFile::SetMediaPath(const string &mediaPath)
{
	m_szMediaPath = mediaPath;
}


string IniFile::GetSkinPath()
{
	return m_szSkinPath;
}

string IniFile::GetSavePath()
{
	return m_szSavePath;
}

string IniFile::GetScreenshotPath()
{
	return m_szScreenshotPath;
}