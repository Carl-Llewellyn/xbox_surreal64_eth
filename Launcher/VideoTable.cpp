#include "VideoTable.h"
#include "IniFile.h"

extern int VideoType;
extern char szPathMedia[256];

VideoTable g_VideoTable;

VideoTable::VideoTable(void)
{
	m_videntryTable = new VideoEntry *[0x10000];

	for (dword i = 0; i < 0x10000; i++)
	{
		m_videntryTable[i] = NULL;
	}
	
	m_bVideoLoaded = false;
}

VideoTable::~VideoTable(void)
{
	for (dword i = 0; i < 0x10000; i++)
	{
		if (m_videntryTable[i] != NULL)
		{
			VideoEntry *delEntry = m_videntryTable[i];
			VideoEntry *nextEntry = delEntry->pNextEntry;
			delete delEntry;

			while (nextEntry != NULL)
			{
				delEntry = nextEntry;
				nextEntry = delEntry->pNextEntry;
				delete delEntry;
			}
		}
	}

	delete [] m_videntryTable;
	
	m_bVideoLoaded = false;
}

void VideoTable::Build()
{
	WIN32_FIND_DATA fd;

	char pathmedia[256];
	sprintf(pathmedia, "%sMovies\\*.xmv", szPathMedia);
	HANDLE hFF = FindFirstFile(pathmedia, &fd);
	
	do
	{
		string szFilename(fd.cFileName);

		for (dword i = 0; i < (szFilename.length() - 4); i += 9)
		{
			dword crc1 = strtoul(szFilename.substr(i, 8).c_str(), NULL, 16);
			CreateVideoEntry(crc1, "Movies\\" + szFilename);		
			
		}
	}
	while (FindNextFile(hFF, &fd));
	
	m_bVideoLoaded = true;
}

void VideoTable::Destroy()
{
	for (dword i = 0; i < 0x10000; i++)
	{
		if (m_videntryTable[i] != NULL)
		{
			VideoEntry *delEntry = m_videntryTable[i];
			VideoEntry *nextEntry = delEntry->pNextEntry;
			delete delEntry;

			while (nextEntry != NULL)
			{
				delEntry = nextEntry;
				nextEntry = delEntry->pNextEntry;
				delete delEntry;
			}
			
			m_videntryTable[i] = NULL;
		}
	}

	m_bVideoLoaded = false;
}

void VideoTable::Refresh()
{
	Destroy();
	Build();
}

VideoEntry *VideoTable::CreateVideoEntry(dword crc1, const string &pathName)
{
	// create a new box art entry struct for this entry 
	// and fill it with 0's
	VideoEntry *newEntry = new VideoEntry;
	memset(newEntry, 0, sizeof(VideoEntry));
	
	newEntry->dwCrc1 = crc1;
	strcpy(newEntry->szPath, pathName.c_str());
	newEntry->pNextEntry = NULL;

	// place the new entry in the table according to the first 16
	// bits of crc1. if the entry is already used, iterate
	// until the end of the chain and place the new entry there.
	word tableLocation = static_cast<word>(crc1 >> 16);

	if (m_videntryTable[tableLocation] == NULL)
	{
		m_videntryTable[tableLocation] = newEntry;
	}
	else
	{
		VideoEntry *tmpEntry = m_videntryTable[tableLocation];

		// keep looping till we reach the end of the chain
		while (tmpEntry->pNextEntry != NULL)
		{					
			tmpEntry = tmpEntry->pNextEntry;
		}
		
		tmpEntry->pNextEntry = newEntry;
	}

	return newEntry;
}

string VideoTable::GetVideoFilename(dword crc1)
{
	
	
	// look up the table using the first 16 bits of crc1
	word tableLocation = static_cast<word>(crc1 >> 16);

	VideoEntry *curEntry = m_videntryTable[tableLocation];

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
	
		return string("Movies\\static.xmv");
	}
