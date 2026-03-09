#include "BoxArtTable.h"
#include "IniFile.h"

extern int BoxartType;
extern char BoxartName[32];
extern char szPathMedia[256];

BoxArtTable g_boxArtTable;

BoxArtTable::BoxArtTable(void)
{
	m_entryTable = new BoxArtEntry *[0x10000];

	for (dword i = 0; i < 0x10000; i++)
	{
		m_entryTable[i] = NULL;
	}
	
	m_bBoxartLoaded = false;
}

BoxArtTable::~BoxArtTable(void)
{
	for (dword i = 0; i < 0x10000; i++)
	{
		if (m_entryTable[i] != NULL)
		{
			BoxArtEntry *delEntry = m_entryTable[i];
			BoxArtEntry *nextEntry = delEntry->pNextEntry;
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
	
	m_bBoxartLoaded = false;
}

void BoxArtTable::Build()
{
	WIN32_FIND_DATA fd;
	
	char BoxPath[256];
	char pathmedia[256];
	sprintf(pathmedia, "%s%s\\*.png", szPathMedia, BoxartName);
	
	HANDLE hFF = FindFirstFile(pathmedia, &fd);
	do
	{
		string szFilename(fd.cFileName);

		for (dword i = 0; i < (szFilename.length() - 4); i += 9)
		{
			dword crc1 = strtoul(szFilename.substr(i, 8).c_str(), NULL, 16);

			sprintf(BoxPath, "%s\\%s", BoxartName, szFilename.c_str());
			CreateBoxArtEntry(crc1, BoxPath);
		}
	}
	while (FindNextFile(hFF, &fd));
	
	m_bBoxartLoaded = true;
}

void BoxArtTable::Destroy()
{
	for (dword i = 0; i < 0x10000; i++)
	{
		if (m_entryTable[i] != NULL)
		{
			BoxArtEntry *delEntry = m_entryTable[i];
			BoxArtEntry *nextEntry = delEntry->pNextEntry;
			delete delEntry;

			while (nextEntry != NULL)
			{
				delEntry = nextEntry;
				nextEntry = delEntry->pNextEntry;
				delete delEntry;
			}
			
			m_entryTable[i] = NULL;
		}
	}

	m_bBoxartLoaded = false;
}

void BoxArtTable::Refresh()
{
	Destroy();
	Build();
}

BoxArtEntry *BoxArtTable::CreateBoxArtEntry(dword crc1, const string &pathName)
{
	// create a new box art entry struct for this entry 
	// and fill it with 0's
	BoxArtEntry *newEntry = new BoxArtEntry;
	memset(newEntry, 0, sizeof(BoxArtEntry));
	
	newEntry->dwCrc1 = crc1;
	strcpy(newEntry->szPath, pathName.c_str());
	newEntry->pNextEntry = NULL;

	// place the new entry in the table according to the first 16
	// bits of crc1. if the entry is already used, iterate
	// until the end of the chain and place the new entry there.
	word tableLocation = static_cast<word>(crc1 >> 16);

	if (m_entryTable[tableLocation] == NULL)
	{
		m_entryTable[tableLocation] = newEntry;
	}
	else
	{
		BoxArtEntry *tmpEntry = m_entryTable[tableLocation];

		// keep looping till we reach the end of the chain
		while (tmpEntry->pNextEntry != NULL)
		{					
			tmpEntry = tmpEntry->pNextEntry;
		}
		
		tmpEntry->pNextEntry = newEntry;
	}

	return newEntry;
}


string BoxArtTable::GetBoxArtFilename(dword crc1)
{
	
	// look up the table using the first 16 bits of crc1
	word tableLocation = static_cast<word>(crc1 >> 16);

	BoxArtEntry *curEntry = m_entryTable[tableLocation];

	// search through the chain
	while (curEntry != NULL)
	{
		if (curEntry->dwCrc1 == crc1)
		{
			// a match was found
			return string(curEntry->szPath);
		}

		curEntry = curEntry->pNextEntry;
	}

	// no entry was found that matches
	
		//return string("%s\\default.png", BoxartName);
		return string(string(BoxartName) + "\\default.png");

}
