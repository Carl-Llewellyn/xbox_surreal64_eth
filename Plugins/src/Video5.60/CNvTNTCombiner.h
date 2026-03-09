/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _NVIDIA_TNT_COMBINER_H_
#define _NVIDIA_TNT_COMBINER_H_

#include "Combiner.h"

typedef struct
{
	BYTE	arg0;
	BYTE	arg1;
	BYTE	arg2;
	BYTE	arg3;
} TNT2CombType;

typedef struct {
	union {
		struct {
			unsigned int	rgbOp;
			unsigned int	alphaOp;
		};
		unsigned int ops[2];
	};

	union {
		struct {
			BYTE	rgbArg0;
			BYTE	rgbArg1;
			BYTE	rgbArg2;
			BYTE	rgbArg3;
			BYTE	alphaArg0;
			BYTE	alphaArg1;
			BYTE	alphaArg2;
			BYTE	alphaArg3;
		};
		TNT2CombType Combs[2];
		BYTE args[2][4];
	};

	int constant;
} TNT2CombinerType;

typedef struct {
	DWORD	dwMux0;
	DWORD	dwMux1;
	union {
		struct {
			TNT2CombinerType	unit1;
			TNT2CombinerType	unit2;
		};
		TNT2CombinerType units[2];
	};
	int		numOfUnits;
} TNT2CombinerSaveType;

class CNvTNTCombiner
{
protected:
	CNvTNTCombiner();

	int FindCompiledMux();
	int ParseDecodedMux();				// Compile the decodedMux into NV register combiner setting
	virtual void ParseDecodedMuxForConstants(TNT2CombinerSaveType &res);
	int SaveParserResult(TNT2CombinerSaveType &result);
	
#ifdef _DEBUG
	void DisplaySimpleMuxString();
#endif
	std::vector<TNT2CombinerSaveType>	m_vCompiledTNTSettings;
	int m_lastIndexTNT;
	DecodedMux **m_ppDecodedMux;
};

#endif

