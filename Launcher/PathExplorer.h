#pragma once

#include <xtl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

class PathList
{
public:
	PathList(void);
	virtual ~PathList(void);

	void	Update(int iSelected);
	
	void	SetMax(int iMax);
	int		GetMax();

	string	GetCurrent();
	string	GetName(int iIndex);
	string	GetParent(const string& szPath);
	
	int		GetSize();
	int		GetSelected();
	
	bool	IsLoaded();
	void	Refresh();
	void	Build();
	void	Destroy();

private:
	vector<string> m_vPathList;
	string	m_szCurrentPath;
	bool	m_bLoaded;
	bool	m_bIsRoot;
	bool	m_bIsReady;
	
	int		m_iListMax;
	int		m_iBaseIndex;
	int		m_iSelected;

	void	AddRoots();
	void	SortList();
	void	TrimList();
	void	ClearList();
	
	void	InsertAt(int iIndex, const string& szName);
	void	GetFiles(const string& szPath);
	void	Add(const string& szName);
	
};

extern PathList g_PathList;