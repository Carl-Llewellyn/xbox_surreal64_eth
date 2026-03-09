#include "PathExplorer.h"
#include "Util.h"

//#define DEBUG_PATH_EXPL 1

// crude buttons - string checks are run on these
#define BUTTON_UP	"/\\     /\\     /\\     /\\"
#define BUTTON_DOWN	"\\/     \\/     \\/     \\/"
#define BUTTON_BACK ".."

#define PATH_EXPL_LIST_MAX 10 // default max

PathList g_PathList;

PathList::PathList(void)
{
	m_bLoaded = false;
	m_bIsRoot = true;
	m_bIsReady = false;
	m_iListMax = PATH_EXPL_LIST_MAX;
	m_iBaseIndex = 0;
	m_iSelected = 0;
}

PathList::~PathList(void)
{
	Destroy();
}

void PathList::Update(int iSelected)
{
	// 1st run each time will set menu defaults
	if (!m_bIsReady) {
		m_bIsReady = true;
		return;
	}

	string szSelected = GetName(iSelected);
	
	m_iSelected = 0;
	
	if (m_bIsRoot)
	{
		m_bIsRoot = false;
		m_szCurrentPath = szSelected;
	}
	else
	{
		// scroll up
		if (strcmp(szSelected.c_str(), BUTTON_UP) == 0)
		{
			m_iBaseIndex--;
		}
		// scroll down
		else if (strcmp(szSelected.c_str(), BUTTON_DOWN) == 0)
		{
			m_iBaseIndex++;
			m_iSelected = (GetMax() - 1);
		}
		// up one level
		else if (strcmp(szSelected.c_str(), BUTTON_BACK) == 0)
		{
			m_szCurrentPath = GetParent(m_szCurrentPath);
			m_iBaseIndex = 0;
			
			// start over
			if (m_szCurrentPath.length() == 0) {
				Refresh();
				return;
			}
		}
		// down one level
		else
		{
			m_szCurrentPath = FixPath(m_szCurrentPath.append(szSelected));
			m_iBaseIndex = 0;
		}

	}

	ClearList();
	
	GetFiles(m_szCurrentPath);
	
	SortList();
	
	// needs to be after generation of new list
	if (m_iBaseIndex < 0)
		m_iBaseIndex = 0;
	else if (m_iBaseIndex > (GetSize() - 1))
		m_iBaseIndex = (GetSize() - 1);

	TrimList();
}

void PathList::TrimList()
{
	if (!m_bIsRoot)
		InsertAt(0, BUTTON_BACK); //back
	
	// it's going to overflow
	if (GetSize() > GetMax()) {
		
		bool bBtnUp = false;
		bool bBtnDn = false;
		
		int iFloor = m_iBaseIndex;
		int iCeil = (m_iBaseIndex + (GetMax() - 1)); // % GetSize();
		
		if (iCeil > GetSize())
			iCeil = (GetSize() - 1);

		if (iFloor > 0) {
			bBtnUp = true;
			iFloor += 1;
		}
		
		if (iCeil < (GetSize() - 1)) {
			bBtnDn = true;
			iCeil -= 1;
		}
		
		// copy whats needed to a temp list and then re-assign
		vector<string> m_vTrimList;
		
		if (bBtnUp)
			m_vTrimList.push_back(BUTTON_UP);
		
		int iCount = 0;
		for (vector<string>::iterator i = m_vPathList.begin(); i != m_vPathList.end(); ++i)
		{
			if (iCount < iFloor) {
				iCount++;
				continue;
			}
			else if (iCount > iCeil && bBtnDn) {
				break;
			}
			else {
				m_vTrimList.push_back(*i);
			}			
			iCount++;
		}
		
		if (bBtnDn)
			m_vTrimList.push_back(BUTTON_DOWN);
		
		ClearList();
		m_vPathList.assign(m_vTrimList.begin(), m_vTrimList.end());
		
		m_vTrimList.clear();
	}
}

int PathList::GetSelected()
{
	return m_iSelected;
}

void PathList::GetFiles(const string& szPath)
{
	HANDLE 				hFind;
	WIN32_FIND_DATA		oFindData;

	char szSearchPath[256];
	sprintf(szSearchPath, "%s*.*", szPath.c_str());
	hFind = FindFirstFile(szSearchPath, &oFindData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if(oFindData.cFileName[0] == '.' || !(oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				continue;
			}
			
			Add(string(oFindData.cFileName));

		} while (FindNextFile(hFind, &oFindData));
	}
	FindClose(hFind);
}

string PathList::GetParent(const string& szPath)
{
	string szParent = szPath;
	
	// rem last slash
	if (szParent[szParent.length() - 1] == '\\')
		szParent.erase(szParent.length() - 1);
	
	// ret if left with drive:
	if (szParent.length() < 3) {
		szParent.clear();
		return szParent;
	}
	
	// rem from last slash onward
	szParent.erase(szParent.find_last_of("/\\"));
	szParent.append("\\");
	
	return szParent;
}

string PathList::GetCurrent()
{
	return m_szCurrentPath;
}

string PathList::GetName(int iIndex)
{
	string szName;
	if (iIndex < GetSize()) {
		szName = m_vPathList[iIndex];
	}
	return string(szName);
}

void PathList::AddRoots()
{
	Add("A:\\");
	Add("C:\\");
	Add("D:\\");
	Add("E:\\");
	Add("F:\\");
	Add("G:\\");	
}

void PathList::Add(const string& szName)
{
	m_vPathList.push_back(szName);
}

void PathList::InsertAt(int iIndex, const string& szName)
{
	vector<string>::iterator i;
	i = m_vPathList.begin() + iIndex;
	m_vPathList.insert(i, szName);
}

void PathList::ClearList()
{
	m_vPathList.clear();
}

void PathList::SortList()
{
	sort(m_vPathList.begin(), m_vPathList.end(), StringRLTL);
}

int PathList::GetSize()
{
	return (int)m_vPathList.size();
}

void PathList::SetMax(int iMax)
{
	m_iListMax = iMax;
}

int PathList::GetMax()
{
	return m_iListMax;
}

bool PathList::IsLoaded()
{
	return m_bLoaded;
}

void PathList::Refresh()
{
	m_szCurrentPath.clear();
	
	ClearList();
	
	AddRoots(); // start with roots
	
	SortList();
	
	m_bIsRoot = true;
	m_iBaseIndex = 0;
	m_iSelected = 0;
}

void PathList::Build()
{
	if (IsLoaded()) {
		Destroy();
	}
	
	Refresh();
	
	m_bLoaded = true;
}

void PathList::Destroy()
{
	m_bLoaded = false;
	m_bIsRoot = true;
	m_bIsReady = false;
	m_iListMax = PATH_EXPL_LIST_MAX;
	m_iBaseIndex = 0;
	m_iSelected = 0;
	
	m_szCurrentPath.clear();

	ClearList();
}