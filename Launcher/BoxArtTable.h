#pragma once

#include "Launcher.h"

struct BoxArtEntry
{
	dword dwCrc1;
	char szPath[_MAX_PATH];

	BoxArtEntry *pNextEntry;
};

class BoxArtTable
{
public:
	BoxArtTable(void);
	~BoxArtTable(void);

	void Build();
	void Destroy();
	void Refresh();
	string GetBoxArtFilename(dword crc1);
	//string GetVideoFilename(dword crc1);

private:
	BoxArtEntry *CreateBoxArtEntry(dword crc1, const string &pathName);
	BoxArtEntry **m_entryTable;
	
	bool m_bBoxartLoaded;
	
};

extern BoxArtTable g_boxArtTable;

