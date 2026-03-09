// Profiler.cpp: implementation of the CProfiler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



HRESULT CUnique< CProfiler >::Create()
{
	_ASSERTE(m_pInstance == NULL);
	
	m_pInstance = new CProfiler();
	SAFE_CHECK(m_pInstance);
	
	return S_OK;
};

const CProfiler::ProfileInfo CProfiler::s_ItemInfo[ NUM_PROFILE_ITEMS ] =
{
	{PROFILE_RENDER,				PROFILE_RENDER,			"Render" },
	{PROFILE_COMBINER,				PROFILE_COMBINER,		"Combiner" },
	{PROFILE_LIGHTING,				PROFILE_RENDER,			"Render: Lighter" },
	{PROFILE_TRANSFORMATING,		PROFILE_RENDER,			"Render: T" },
	{PROFILE_DRAWING,				PROFILE_RENDER,			"Render: Drawing" },
	{PROFILE_COMBINER_PARSER,		PROFILE_COMBINER,		"Com: Parser" },
	{PROFILE_COMBINER_GENERATOR,	PROFILE_COMBINER,		"Com: Generator" },
	{PROFILE_TEXTURE,				PROFILE_TEXTURE,		"Texture" },
	{PROFILE_UCODE_PARSER,			PROFILE_UCODE_PARSER,	"Ucode Parser" },
	{PROFILE_ALL,					PROFILE_ALL,			"All" }

};

void CProfiler::Display()
{
	char buf[3000];
	char buf2[200];
	buf[0]=0;

	for( int i=0; i<NUM_PROFILE_ITEMS; i++ )
	{
		sprintf(buf2, "%s: %f (s)\n", s_ItemInfo[i].name, m_fTimes[1][i]);
		strcat(buf, buf2);
	}

	MsgInfo(buf);
}

void CProfiler::StopTiming( ProfileItem item )
{
	LARGE_INTEGER liEnd;
	QueryPerformanceCounter( &liEnd );

	//m_bStarted = false;
	m_fTimes[ 1 ][ (u32)item ] += (float)(liEnd.QuadPart - m_liStartTimes[ (u32)item ].QuadPart) / m_liFrequency.QuadPart;
}
