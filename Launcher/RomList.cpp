#include "RomList.h"
#include "Util.h"
#include "config.h"

RomList g_romList;
//extern int romcounter; // use romlist func instead

bool RLessThan(Rom *elem1, Rom *elem2)
{
	return (elem1->GetProperName() < elem2->GetProperName());
}

RomList::RomList(void)
{
	m_romListMode = All;
	m_iBaseIndex = 0;
	m_bLoaded = false;
	
	m_bUseCache = false; // toggle cache file
}

RomList::~RomList(void)
{
	Destroy();
}

void RomList::Load()
{
	// try and open the cache file, if it doesnt exist, generate the rom list
	if (!m_bUseCache)
	{
		if (PathFileExists("T:\\Data\\RomlistCache.dat")) {
			if (!DeleteFile("T:\\Data\\RomlistCache.dat"))
				OutputDebugString("T:\\Data\\RomlistCache.dat Could Not Be Deleted!\n");
		}
		if (PathFileExists("T:\\Data\\RomlistState.dat")) {
			if (!DeleteFile("T:\\Data\\RomlistState.dat"))
				OutputDebugString("T:\\Data\\RomlistState.dat Could Not Be Deleted!\n");
		}
		
		Build();
		SaveDatFile(All);
	}
	else if (!PathFileExists("T:\\Data\\RomlistCache.dat"))
	{
		Build();
		SaveDatFile(All);
	}
	else
	{
		LoadDatFile(All);
	}

	LoadDatFile(Favourite);
}

void RomList::Save()
{
	SaveDatFile(All);
}

void RomList::LoadDatFile(int mode)
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(false);
	ini.SetMultiLine(false);
	ini.SetSpaces(false);
	
	char szDatFilename[64];
	if (mode == All) {
		sprintf(szDatFilename, "T:\\Data\\RomlistCache.dat");
	} else {
		sprintf(szDatFilename, "T:\\Data\\Favorites.dat");
	}
	
	OutputDebugString(szDatFilename);
	rc = ini.LoadFile(szDatFilename);
	if (rc < 0) {
		OutputDebugStringA(" Failed to Load!\n");
		return;
	}
	OutputDebugStringA(" Successfully Loaded!\n");
	
	const TCHAR *pszSection = 0;
	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);
	CSimpleIniA::TNamesDepend::const_iterator iSection = sections.begin();
	for ( ; iSection != sections.end(); ++iSection ) {
		pszSection = iSection->pItem;
		if (!*pszSection) continue;
		
		char szRomCrcs[23];
		sprintf(szRomCrcs, "%s", pszSection);
		string szRomCrcStr (Trim(szRomCrcs));
		
		dword dwCrc1		= strtoul(szRomCrcStr.substr(0, 8).c_str(), NULL, 16);
		dword dwCrc2		= strtoul(szRomCrcStr.substr(9, 8).c_str(), NULL, 16);
		byte  byCountry		= static_cast<byte>(strtoul(szRomCrcStr.substr(20, 2).c_str(), NULL, 16));
		
		if (dwCrc1 == 0 || dwCrc2 == 0 || byCountry == 0) continue;
		
		if (mode == All)
		{
			Rom *rom = new Rom();
			
			string szFileName = ini.GetValue(szRomCrcs, "FileName", "");
			string szBoxArtFilename = ini.GetValue(szRomCrcs, "BoxArtFilename", "");
			string szComments = ini.GetValue(szRomCrcs, "Comments", "");
			dword dwRomSize = ini.GetLongValue(szRomCrcs, "RomSize", 0);
			
			bool bSuccess = rom->LoadFromCache(dwCrc1, dwCrc2, byCountry,
											   szFileName, szBoxArtFilename, dwRomSize, szComments);
											   
			if (bSuccess)
				m_romList.push_back(rom);
			else
				delete rom;
		}
		else
		{
			for (int i = 0; i < static_cast<int>(m_romList.size()); i++)
			{
				if (m_romList[i]->GetCrc1() == dwCrc1
				 && m_romList[i]->GetCrc2() == dwCrc2
				 && m_romList[i]->GetCountry() == byCountry)
				{
					m_favList.push_back(m_romList[i]);
					break;
				}
			}
		}
	}
	
	if (mode == All)
		sort(m_romList.begin(), m_romList.end(), RLessThan);
	else
		sort(m_favList.begin(), m_favList.end(), RLessThan);
}

void RomList::SaveDatFile(int mode)
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
    ini.SetMultiKey(false);
    ini.SetMultiLine(false);
	ini.SetSpaces(false);
	
	char szRomCrcs[23];
	char szDatFilename[64];
	vector<Rom *>::iterator i;
	
	if (mode == All)
	{
		sprintf(szDatFilename, "T:\\Data\\RomlistCache.dat");
		
		for (i = m_romList.begin(); i != m_romList.end(); i++)
		{
			Rom *rom = *i;

			sprintf(szRomCrcs, "%08X-%08X-C:%02X", rom->GetCrc1(), rom->GetCrc2(), static_cast<dword>(rom->GetCountry()));
			ini.SetValue(szRomCrcs, "FileName", rom->GetFileName().c_str());
			ini.SetValue(szRomCrcs, "BoxArtFilename", rom->GetBoxArtFilename().c_str());
			ini.SetValue(szRomCrcs, "Comments", rom->GetComments().c_str());
			ini.SetLongValue(szRomCrcs, "RomSize", rom->GetRomSize());
		}
	}
	else
	{
		sprintf(szDatFilename, "T:\\Data\\Favorites.dat");
		
		for (i = m_favList.begin(); i != m_favList.end(); i++)
		{
			Rom *rom = *i;

			sprintf(szRomCrcs, "%08X-%08X-C:%02X", rom->GetCrc1(), rom->GetCrc2(), static_cast<dword>(rom->GetCountry()));
			// don't really need the name, just need at least one key=val
			// so, using name in header to make the fav easier to identify
			ini.SetValue(szRomCrcs, "RomName", rom->GetRomName().c_str());
		}
	}
	
	OutputDebugString(szDatFilename);
	rc = ini.SaveFile(szDatFilename);
    if (rc < 0) {
		OutputDebugStringA(" Failed to Save!\n");
		return;
	}
	OutputDebugStringA(" Successfully Saved!\n");
}

void RomList::RemoveFromDatFile(Rom *rom, int mode)
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(false);
	ini.SetMultiLine(false);
	ini.SetSpaces(false);
	
	char szDatFilename[64];
	if (mode == All) {
		sprintf(szDatFilename, "T:\\Data\\RomlistCache.dat");
	} else {
		sprintf(szDatFilename, "T:\\Data\\Favorites.dat");
	}
	
	OutputDebugString(szDatFilename);
	rc = ini.LoadFile(szDatFilename);
	if (rc < 0) {
		OutputDebugStringA(" Failed to Load!\n");
		return;
	}
	OutputDebugStringA(" Successfully Loaded!\n");
	
	char szRomCrcs[23];
	sprintf(szRomCrcs, "%08X-%08X-C:%02X", rom->GetCrc1(), rom->GetCrc2(), static_cast<dword>(rom->GetCountry()));
	
	ini.Delete(szRomCrcs, NULL); // rem section and all assoc keys

	OutputDebugString(szDatFilename);
	rc = ini.SaveFile(szDatFilename);
    if (rc < 0) {
		OutputDebugStringA(" Failed to Save!\n");
		return;
	}
	OutputDebugStringA(" Successfully Saved!\n");
}

void RomList::Refresh()
{
	Destroy();
	
	if (PathFileExists("T:\\Data\\RomlistCache.dat")) {
		if (!DeleteFile("T:\\Data\\RomlistCache.dat"))
			OutputDebugString("T:\\Data\\RomlistCache.dat Could Not Be Deleted!\n");
	}
	if (PathFileExists("T:\\Data\\RomlistState.dat")) {
		if (!DeleteFile("T:\\Data\\RomlistState.dat"))
			OutputDebugString("T:\\Data\\RomlistState.dat Could Not Be Deleted!\n");
	}

	Load();
}

bool RomList::IsLoaded()
{
	return m_bLoaded;
}

void RomList::SetRomListMode(int mode)
{
	m_iBaseIndex = 0;
	m_romListMode = mode;
}

int RomList::GetRomListMode()
{
	return m_romListMode;	
}

void RomList::AddRomToList(Rom *rom, int mode)
{
	vector<Rom *> *pList;

	switch (mode)
	{
		case All:
			pList = &m_romList;
			break;
		case Favourite:
			pList = &m_favList;
			break;
		default:
			return;
	}

	// look to see if the rom is already in the list, we dont want duplicates
	for (int i = 0; i < static_cast<int>(pList->size()); i++)
	{
		if (rom == (*pList)[i])
			return;
	}

	pList->push_back(rom);
	sort(pList->begin(), pList->end(), RLessThan);
	
	SaveDatFile(mode);
}

void RomList::RemoveRomFromList(Rom *rom, int mode)
{
	vector<Rom *> *pList;

	switch (mode)
	{
		case All:
			pList = &m_romList;
			break;
		case Favourite:
			pList = &m_favList;
			break;
		default:
			return;
	}

	vector<Rom *>::iterator i;

	// look to see if the rom is already in the list, we dont want duplicates
	for (i = pList->begin(); i != pList->end(); i++)
	{
		if (rom == *i)
		{
			RemoveFromDatFile(rom, mode);
		
			pList->erase(i);
			return;
		}
	}
}

int RomList::GetBaseIndex()
{
	if (m_iBaseIndex > GetRomListSize() - 1)
		m_iBaseIndex = GetRomListSize() - 1;
	if (m_iBaseIndex < 0)
		m_iBaseIndex = 0;

	return m_iBaseIndex;
}

void RomList::SetBaseIndex(int index)
{
	if (index > GetRomListSize() - 1)
		index = GetRomListSize() - 1;
	if (index < 0)
		index = 0;

	m_iBaseIndex = index;
}

// test list before switching to it
bool RomList::IsListEmpty(int mode)
{
	int iSize;
	switch (mode)
	{
		case All:
			iSize = m_romList.size();
			break;
		case Favourite:
			iSize = m_favList.size();
			break;
		default:
			return NULL;
	}
	
	if (iSize > 0)
		return false;
	else
		return true;
}

int RomList::GetRomListSize()
{
	switch (m_romListMode)
	{
		case All:
			return m_romList.size();
		case Favourite:
			return m_favList.size();
	}

	return 0;
}

Rom *RomList::GetRomAt(int index)
{
	switch (m_romListMode)
	{
		case All:
			return m_romList[index];
		case Favourite:
			return m_favList[index];
	}

	return 0;
}

int RomList::FindRom(Rom *rom, int mode)
{
	vector<Rom *> *pList;

	switch (mode)
	{
		case All:
			pList = &m_romList;
			break;
		case Favourite:
			pList = &m_favList;
			break;
		default:
			return -1;
	}

	for (int i = 0; i < static_cast<int>(pList->size()); i++)
	{
		if (rom == (*pList)[i])
			return i;
	}
	
	return -1;
}

void RomList::Build()
{
	//romcounter=0;
	WIN32_FIND_DATA fd;

	char pathroms[256];
	sprintf(pathroms, "%s*.*", szPathRoms);
	HANDLE hFF = FindFirstFile(pathroms, &fd);

	do
	{
		if(fd.cFileName[0] == '.' ) continue;
	
		char ext[_MAX_EXT];

		// get the filename extension
		char pathrom[256];
		sprintf(pathrom, "%s%s", szPathRoms, fd.cFileName); //.c_str()
		_splitpath(pathrom, NULL, NULL, NULL, ext);

		if (
			stricmp(ext, ".rom") == 0
		||	stricmp(ext, ".v64") == 0
		||	stricmp(ext, ".z64") == 0
		||	stricmp(ext, ".usa") == 0
		||	stricmp(ext, ".n64") == 0
		||	stricmp(ext, ".bin") == 0
		||	stricmp(ext, ".zip") == 0
		||	stricmp(ext, ".j64") == 0
		||	stricmp(ext, ".pal") == 0
			)
		{
			Rom *rom = new Rom();
			bool bSuccess = rom->Load(pathrom);

			if (bSuccess){
				m_romList.push_back(rom);
				//romcounter++;
			}
			else
				delete rom;
		}
	} while (FindNextFile(hFF, &fd));

	sort(m_romList.begin(), m_romList.end(), RLessThan);
    //ConfigAppSave(); // for romcounter

	m_bLoaded = true;
}

void RomList::Destroy()
{
	m_bLoaded = false;
	m_iBaseIndex = 0;

	vector<Rom *>::iterator i;

	for (i = m_romList.begin(); i != m_romList.end(); i++)
	{
		delete *i;
	}

	m_romList.clear();
	m_favList.clear();
}




