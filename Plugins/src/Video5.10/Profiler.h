// Profiler.h: interface for the CProfiler class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PROFILER_H__
#define __PROFILER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Unique.h"

#define START_PROFILE( item )  
#define STOP_PROFILE( item )  

enum ProfileItem
{
	PROFILE_RENDER,
	PROFILE_COMBINER,
	PROFILE_LIGHTING,
	PROFILE_TRANSFORMATING,
	PROFILE_DRAWING,
	PROFILE_COMBINER_PARSER,
	PROFILE_COMBINER_GENERATOR,
	PROFILE_TEXTURE,
	PROFILE_UCODE_PARSER,
	PROFILE_ALL,

	NUM_PROFILE_ITEMS
};

class CProfiler : public CUnique< CProfiler >  
{
	protected:
		friend class CUnique< CProfiler >;
		CProfiler()
		{
			m_nCurrent = 0;
			QueryPerformanceFrequency( &m_liFrequency );

		}


		enum { NUM_HISTORY_ITEMS = 5 };

	public:
		virtual ~CProfiler() {}

		// Update s_ItemNames when adding to this

		// Reset all times. Probably do this once per frame
		void Reset()
		{
			 
		}

		// Start stop profiling for an item
		void StartTiming( ProfileItem item )
		{
			 
		}

		void StopTiming( ProfileItem item );

		// Return the time spent on this item since resetting
		float GetTime( u32 history, ProfileItem item )
		{
			 
		}

		void Display();

		u32 GetNumHistoryItems() const { return NUM_HISTORY_ITEMS; }
		u32 GetNumProfileItems() const { return NUM_PROFILE_ITEMS; }

		const CHAR * GetProfileItemName( ProfileItem item )
		{
			return s_ItemInfo[ (u32) item ].name;
		}

	protected:

		typedef struct 
		{
			ProfileItem item;
			ProfileItem parent;
			const CHAR * name;

		} ProfileInfo;

		u32				m_nCurrent;


		LARGE_INTEGER	m_liStartTimes[ NUM_PROFILE_ITEMS ];

		float			m_fTimes[NUM_HISTORY_ITEMS ][ NUM_PROFILE_ITEMS ];

		LARGE_INTEGER	m_liFrequency;

		static const ProfileInfo s_ItemInfo[ NUM_PROFILE_ITEMS ];

		bool			m_bStarted;
		ProfileItem		m_currentItem;
};

#endif // __PROFILER_H__
