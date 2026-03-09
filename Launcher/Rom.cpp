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

#include "Rom.h"
#include "Util.h"
#include "BoxArtTable.h"
#include "VideoTable.h"

#include "zlib/zlib.h"
#include "zlib/unzip.h"

Rom::Rom()
{
	m_bLoaded = false;
}

Rom::~Rom(void)
{
}

bool Rom::Load(const string &szFilename)
{
	if (m_bLoaded)
		return true;

	m_szFilename = szFilename;

	byte header[0x40];

	// load the rom
	if (m_szFilename.substr(m_szFilename.length() - 4) == ".zip")
	{
		int retVal;
		bool bFoundRom = false;
		unzFile	fp;
		fp = unzOpen(m_szFilename.c_str());

		retVal = unzGoToFirstFile(fp);

		if (retVal != UNZ_OK)
		{
			unzClose(fp);
			return false;
		}

		do
		{
			unz_file_info fileInfo;
			char szUnzFileName[_MAX_FNAME];
			
			retVal = unzGetCurrentFileInfo(fp, &fileInfo, szUnzFileName, _MAX_FNAME, NULL, 0, NULL, 0);

			if (retVal != UNZ_OK)
			{
				unzClose(fp);
				return false;
			}

			string szFn(szUnzFileName);

			// find a rom file in the zip file
			if (szFn.substr(szFn.length() - 4) == ".bin"
			||	szFn.substr(szFn.length() - 4) == ".v64"
			||	szFn.substr(szFn.length() - 4) == ".rom"
			||	szFn.substr(szFn.length() - 4) == ".usa"
			||	szFn.substr(szFn.length() - 4) == ".z64"
			||	szFn.substr(szFn.length() - 4) == ".j64"
			||	szFn.substr(szFn.length() - 4) == ".pal"
			||	szFn.substr(szFn.length() - 4) == ".n64")
			{
				retVal = unzOpenCurrentFile(fp);

				if (retVal != UNZ_OK)
				{
					unzClose(fp);
					return false;
				}

				// read in the header and close the file
				unzReadCurrentFile(fp, reinterpret_cast<char *>(header), 0x40);
				unzCloseCurrentFile(fp);

				m_romSize = fileInfo.uncompressed_size;

				bFoundRom = true;
				break;
			}
		} while (unzGoToNextFile(fp) == UNZ_OK);

		unzClose(fp);

		if (!bFoundRom)
		{
			return false;
		}
	}
	else
	{
		ifstream romFile;

		// load the rom file
		romFile.open(m_szFilename.c_str(), ios_base::in | ios_base::binary);

		if (!romFile.is_open())
		{
			return false;
		}

		// get the rom size
		romFile.seekg(0, ios_base::end);
		m_romSize = romFile.tellg();
		romFile.seekg(0, ios_base::beg);

		// read in the header
		romFile.read(reinterpret_cast<char *>(header), 0x40);
		romFile.close();
	}

	// byteswap and get the crc1, crc2 and country values
	{
		// get the type of byte swapping to be performed
		// we can also check here to see if the file is a valid rom
		byte byByteSwapType = GetByteSwapType(header);

		if (byByteSwapType == -1)
			return false;

		// byte swap the header
		ByteSwap(0x40, header, byByteSwapType);
		SwapRomName(header + 0x20);

		// extract the crc values and the country value from the header
		m_dwCrc1	= *(reinterpret_cast<dword *>(header + 0x10));
		m_dwCrc2	= *(reinterpret_cast<dword *>(header + 0x14));
		m_byCountry	= *(reinterpret_cast<byte *>(header + 0x3D));
	}

	// create/read the ini entry from the ini file
	{
		m_iniEntry = g_iniFile.GetRomEntry(m_dwCrc1, m_dwCrc2, m_byCountry);

		// if there is no ini entry for this rom, create one
		if (m_iniEntry == NULL)
		{
			m_iniEntry = g_iniFile.CreateRomEntry(m_dwCrc1, m_dwCrc2, m_byCountry);
		}

		if (strlen(m_iniEntry->szGameName) == 0)
		{
			// copy the game name in the rom into game name
			strncpy(m_iniEntry->szGameName, 
					reinterpret_cast<char *>(header + 0x20), 
					39);
			m_iniEntry->szGameName[39] = '\0';
		}

		if (strlen(m_iniEntry->szAltTitle) == 0)
		{
			// because we dont know the good name for this rom, lets just copy
			// the name that the rom gives itself, it better than nothing
			strncpy(m_iniEntry->szAltTitle, 
					reinterpret_cast<char *>(header + 0x20), 
					39);

			m_iniEntry->szAltTitle[39] = '\0';
		}
	}

	// get the filename for the previews
	{
		m_szBoxArtFilename = g_boxArtTable.GetBoxArtFilename(m_dwCrc1);
		m_szVideoFilename = g_VideoTable.GetVideoFilename(m_dwCrc1);
	}

	m_bLoaded = true;

	return true;
}

bool Rom::LoadFromCache(dword crc1, dword crc2, byte country, const string &szFilename, const string &szBoxArtFilename, dword romSize, const string &szComments)
{
	m_szFilename = szFilename;
	m_szBoxArtFilename = szBoxArtFilename;

	m_dwCrc1 = crc1;
	m_dwCrc2 = crc2;
	m_byCountry = country;

	m_romSize = romSize;	

	// create/read the ini entry from the ini file
	{
		m_iniEntry = g_iniFile.GetRomEntry(m_dwCrc1, m_dwCrc2, m_byCountry);

		// if there is no ini entry for this rom, create one
		if (m_iniEntry == NULL)
		{
			m_iniEntry = g_iniFile.CreateRomEntry(m_dwCrc1, m_dwCrc2, m_byCountry);
		}
	}

	m_bLoaded = true;

	return true;
}

dword Rom::GetCrc1()
{
	return m_dwCrc1;
}

dword Rom::GetCrc2()
{
	return m_dwCrc2;
}

byte Rom::GetCountry()
{
	return m_byCountry;
}

string Rom::GetFileName()
{
	return m_szFilename;
}

string Rom::GetRomName()
{
	return string(m_iniEntry->szGameName);
}

string Rom::GetProperName()
{
	return string(m_iniEntry->szAltTitle);
}


// clean the roms name
string Rom::GetCleanName()
{
	char szRomName[120];
	sprintf(szRomName, "%s", GetProperName().c_str());

	bool caps = true;
	bool end = false;
	for (int j=0;j<120;j++){
		if (j==0){
			if (!(szRomName[j]>=97) && (szRomName[j]<=122))
				if (!(szRomName[j]>=65) && (szRomName[j]<=90))
					if (!(szRomName[j]>=48) && (szRomName[j]<=57))
			{
				sprintf(szRomName,"Unknown");
				break;
			}
		}
		if (caps) {
			if ((szRomName[j]>=97) && (szRomName[j]<=122)) {
			szRomName[j] = szRomName[j]-32;
			}
			caps = false;
		}
		else {
			if ((szRomName[j]>=65) && (szRomName[j]<=90)) {
			szRomName[j] = szRomName[j]+32;
			}
		}
		if (szRomName[j]==' ' || szRomName[j]=='-') caps=true;
		if ((szRomName[j]=='(')|| (szRomName[j]=='[')) end=true;
		if (end) 
		{szRomName[j]='\0';break;}
	}

	return Trim(string(szRomName));
}

// trunc the roms name
string Rom::GetCleanTruncName(int iMaxChars)
{
	/*string szRomName (GetCleanName().c_str(), 0, iMaxChars); // 120 chars
	return Trim(szRomName);*/
	return Truncate(GetCleanName(), iMaxChars, false);
}


string Rom::GetBoxArtFilename()
{
	return m_szBoxArtFilename;
}

string Rom::GetVideoFilename()
{
	return m_szVideoFilename;
}

string Rom::GetComments()
{
	return string(m_iniEntry->szComments);
}

RomIniEntry *Rom::GetIniEntry()
{
	return m_iniEntry;
}

dword Rom::GetRomSize()
{
	return m_romSize;
}
