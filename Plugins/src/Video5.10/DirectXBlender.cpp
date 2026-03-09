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

#include "stdafx.h"

CDirectXBlender::CDirectXBlender(CDaedalusRender *pRender) 
	: CBlender(pRender),m_pD3DRender((D3DRender*)pRender) 
{
}

void CDirectXBlender::NormalAlphaBlender(void)
{
	m_pD3DRender->SetD3DRSSrcBlend( D3DBLEND_SRCALPHA);
	m_pD3DRender->SetD3DRSDestBlend( D3DBLEND_INVSRCALPHA);
	m_pD3DRender->SetD3DRSAlphaBlendEnable(TRUE );
}

void CDirectXBlender::BlendFunc(DWORD srcFunc, DWORD desFunc)
{
	m_pD3DRender->SetD3DRSSrcBlend( srcFunc);
	m_pD3DRender->SetD3DRSDestBlend( desFunc);
}

void CDirectXBlender::Enable()
{
	m_pD3DRender->SetD3DRSAlphaBlendEnable(TRUE );
}

void CDirectXBlender::Disable()
{
	m_pD3DRender->SetD3DRSAlphaBlendEnable(FALSE );
}

