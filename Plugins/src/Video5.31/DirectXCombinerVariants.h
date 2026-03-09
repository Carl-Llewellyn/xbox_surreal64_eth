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

#ifndef _DIRECTX_COMBINER_VARIANTS_H_
#define _DIRECTX_COMBINER_VARIANTS_H_

#include "DirectXCombiner.h"
#include "OGLCombinerTNT2.h"
#include "CNvTNTDirectXCombiner.h"

class CDirectXColorCombinerLowestEnd : public CDirectXColorCombiner
{
protected:
	friend class DirectXDeviceBuilder;
	CDirectXColorCombinerLowestEnd(CDaedalusRender *pRender);
	bool GenerateD3DCombineInfo(SetCombineInfo & sci);
};

class CDirectXColorCombinerLowerEnd : public CDirectXColorCombinerLowestEnd
{
protected:
	friend class DirectXDeviceBuilder;
	CDirectXColorCombinerLowerEnd(CDaedalusRender *pRender);
	bool GenerateD3DCombineInfo(SetCombineInfo & sci);
};

class CDirectXColorCombinerMiddleLevel : public CDirectXColorCombinerLowerEnd
{
protected:
	friend class DirectXDeviceBuilder;
	CDirectXColorCombinerMiddleLevel(CDaedalusRender *pRender);
	bool GenerateD3DCombineInfo(SetCombineInfo & sci);
};

class CDirectXColorCombinerHigherEnd : public CDirectXColorCombinerMiddleLevel
{
protected:
	friend class DirectXDeviceBuilder;
	CDirectXColorCombinerHigherEnd(CDaedalusRender *pRender);
	bool GenerateD3DCombineInfo(SetCombineInfo & sci);
};

class CDirectXColorCombinerNVidiaGeforcePlus : public CNvTNTDirectXCombiner
{
protected:
	friend class DirectXDeviceBuilder;
	CDirectXColorCombinerNVidiaGeforcePlus(CDaedalusRender *pRender);
};

class CDirectXColorCombinerATIDadeonPlus : public CDirectXColorCombinerHigherEnd
{
protected:
	friend class DirectXDeviceBuilder;
	CDirectXColorCombinerATIDadeonPlus(CDaedalusRender *pRender);
};

#endif
