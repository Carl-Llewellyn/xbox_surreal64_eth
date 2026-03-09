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

#pragma once

#include "Launcher.h"
#include "IniFile.h"

class Rom
{
public:
	Rom();
	~Rom();

	bool Load(const string &szFilename);
	bool LoadFromCache(dword crc1, dword crc2, byte country, const string &szFilename, 
					   const string &szBoxArtFilename, dword romSize, const string &szComments);

	dword GetCrc1();
	dword GetCrc2();
	byte  GetCountry();

	string GetFileName();
	string GetRomName();
	string GetProperName();
	string GetBoxArtFilename();
	string GetVideoFilename();
	string GetComments();
	dword GetRomSize();
	
	string GetCleanName(); // clean name
	string GetCleanTruncName(int iMaxChars); // clean trunc name

	RomIniEntry *GetIniEntry();

	
	string m_szFilename;
	string m_szBoxArtFilename;
	string m_szVideoFilename;

	bool m_bLoaded;

	dword m_dwCrc1;
	dword m_dwCrc2;
	dword m_byCountry;

	dword m_romSize;

	RomIniEntry *m_iniEntry;

};
