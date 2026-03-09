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
	 
}

void CProfiler::StopTiming( ProfileItem item )
{
	 
}
