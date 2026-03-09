#pragma once

#include "Launcher.h"
#include "Rom.h"

enum RomListMode
{
	All,
	Favourite
};

class RomList
{
public:
	RomList(void);
	virtual ~RomList(void);

	void Load();
	void Save();
	void Refresh();

	bool IsLoaded();

	void SetRomListMode(int mode);
	int GetRomListMode();

	void AddRomToList(Rom *rom, int mode);
	void RemoveRomFromList(Rom *rom, int mode);

	int GetBaseIndex();
	void SetBaseIndex(int index);

	int GetRomListSize();

	Rom *GetRomAt(int index);
	int FindRom(Rom *rom, int mode);

	int m_iBaseIndex;
	
	bool m_bUseCache; // toggle cache file
	bool IsListEmpty(int mode); // test list before switching to it

private:
	void Build();
	void Destroy();
	
	void LoadDatFile(int mode);
	void SaveDatFile(int mode);
	void RemoveFromDatFile(Rom *rom, int mode);

public:
	vector<Rom *> m_romList;
	vector<Rom *> m_favList;

	
	bool m_bLoaded;

	int m_romListMode;
};

extern RomList g_romList;