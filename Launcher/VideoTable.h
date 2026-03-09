#pragma once

#include "Launcher.h"

struct VideoEntry
{
	dword dwCrc1;
	char szPath[_MAX_PATH];

	VideoEntry *pNextEntry;
};

class VideoTable
{
public:
	VideoTable(void);
	~VideoTable(void);

	void Build();
	void Destroy();
	void Refresh();
	string GetVideoFilename(dword crc1);

private:
	VideoEntry *CreateVideoEntry(dword crc1, const string &pathName);
	VideoEntry **m_videntryTable;
	
	bool m_bVideoLoaded;
	
};

extern VideoTable g_VideoTable;

