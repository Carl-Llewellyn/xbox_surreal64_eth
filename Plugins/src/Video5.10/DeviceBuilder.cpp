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

#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <xtl.h>


//========================================================================
CDeviceBuilder* CDeviceBuilder::m_pInstance=NULL;
DaedalusSupportedDeviceType CDeviceBuilder::m_deviceType = DIRECTX_DEVICE;
DaedalusSupportedDeviceType CDeviceBuilder::m_deviceGeneralType = DIRECTX_DEVICE;

CDeviceBuilder* CDeviceBuilder::GetBuilder(void)
{
	if( m_pInstance == NULL )
		CreateBuilder(m_deviceType);
	
	return m_pInstance;
}

void CDeviceBuilder::SelectDeviceType(DaedalusSupportedDeviceType type)
{
	if( type != m_deviceType && m_pInstance != NULL )
	{
		DeleteBuilder();
	}

	CDeviceBuilder::m_deviceType = type;
	switch(type)
	{
 
	case DIRECTX_DEVICE:
	case DIRECTX_9_DEVICE:
	case DIRECTX_8_DEVICE:
	case DIRECTX_7_DEVICE:
	case DIRECTX_6_DEVICE:
	case DIRECTX_5_DEVICE:
	case XBOX_DIRECTX_DEVICE:
		CDeviceBuilder::m_deviceGeneralType = DIRECTX_DEVICE;
		break;
	}
}

DaedalusSupportedDeviceType CDeviceBuilder::GetDeviceType(void)
{
	return CDeviceBuilder::m_deviceType;
}

DaedalusSupportedDeviceType CDeviceBuilder::GetGeneralDeviceType(void)
{
	return CDeviceBuilder::m_deviceGeneralType;
}

CDeviceBuilder* CDeviceBuilder::CreateBuilder(DaedalusSupportedDeviceType type)
{
	if( m_pInstance == NULL )
	{
		switch( type )
		{
 
		case 	DIRECTX_DEVICE:
		case 	DIRECTX_9_DEVICE:
		case 	DIRECTX_8_DEVICE:
		case 	DIRECTX_7_DEVICE:
		case 	DIRECTX_6_DEVICE:
		case 	DIRECTX_5_DEVICE:
		case 	XBOX_DIRECTX_DEVICE:
			m_pInstance = new DirectXDeviceBuilder();
			break;
		default:
			ErrorMsg("Error builder type");
			exit(1);
		}

		SAFE_CHECK(m_pInstance);
	}

	return m_pInstance;
}

void CDeviceBuilder::DeleteBuilder(void)
{
	delete m_pInstance;
	m_pInstance = NULL;
}

CDeviceBuilder::CDeviceBuilder() :
	m_pRender(NULL),
	m_pGraphicsContext(NULL),
	m_pColorCombiner(NULL),
	m_pAlphaBlender(NULL)
{
}

CDeviceBuilder::~CDeviceBuilder()
{
	DeleteGraphicsContext();
	DeleteRender();
	DeleteColorCombiner();
	DeleteAlphaBlender();
}

void CDeviceBuilder::DeleteGraphicsContext(void)
{
	if( m_pGraphicsContext != NULL )
	{
		delete m_pGraphicsContext;
		CGraphicsContext::g_pGraphicsContext = m_pGraphicsContext = NULL;
	}
}

void CDeviceBuilder::DeleteRender(void)
{
	if( m_pRender != NULL )
	{
		delete m_pRender;
		CDaedalusRender::g_pRender = m_pRender = NULL;
	}
}

void CDeviceBuilder::DeleteColorCombiner(void)
{
	if( m_pColorCombiner != NULL )
	{
		delete m_pColorCombiner;
		m_pColorCombiner = NULL;
	}
}

void CDeviceBuilder::DeleteAlphaBlender(void)
{
	if( m_pAlphaBlender != NULL )
	{
		delete m_pAlphaBlender;
		m_pAlphaBlender = NULL;
	}
}


//========================================================================
 


CGraphicsContext * DirectXDeviceBuilder::CreateGraphicsContext(void)
{
	 
	if( m_pGraphicsContext == NULL )
	{
		m_pGraphicsContext = new CDirectXGraphicsContext();
		SAFE_CHECK(m_pGraphicsContext);
		CGraphicsContext::g_pGraphicsContext = m_pGraphicsContext;
	}

	return m_pGraphicsContext;
}

CDaedalusRender * DirectXDeviceBuilder::CreateRender(void)
{
	if( m_pRender == NULL )
	{
		m_pRender = new D3DRender();
		SAFE_CHECK(m_pRender);
		CDaedalusRender::g_pRender = m_pRender;
	}

	return m_pRender;
}

CTexture * DirectXDeviceBuilder::CreateTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget)
{
	return new CDirectXTexture(dwWidth, dwHeight, asRenderTarget);
}

CColorCombiner * DirectXDeviceBuilder::CreateColorCombiner(CDaedalusRender *pRender)
{
	if( m_pColorCombiner == NULL )
	{
		int type = g_dwDirectXCombinerType;

		if( g_dwDirectXCombinerType == DX_BEST_FIT )
		{
			char buf[200];
			strcpy(buf, CGraphicsContext::g_pGraphicsContext->GetDeviceStr());
			strlwr(buf);
			if( strstr(buf, "tnt") != NULL )
			{
				type = DX_NVIDIA_TNT;
			}
			else if( strstr(buf, "geforce") != NULL && strstr(buf,"ti") == NULL && strstr(buf,"geforce3") == NULL )
			{
				// Geforce 2, Geforce 256, but not geforce 2/3/4 ti, Geforce 2 MX, Geforce 4 MX
				type = DX_NVIDIA_TNT;
			}
			else
			{
				extern D3DCAPS8 g_D3DDeviceCaps;
				bool m_bCapsTxtOpAdd = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_ADD)!=0;
				bool m_bCapsTxtOpLerp = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_LERP )!=0;
				bool m_bCapsTxtOpSub = (g_D3DDeviceCaps.TextureOpCaps & D3DTEXOPCAPS_SUBTRACT )!=0;

				int m_dwCapsMaxTxt=g_D3DDeviceCaps.MaxSimultaneousTextures;
				int m_dwCapsMaxStages=g_D3DDeviceCaps.MaxTextureBlendStages;

				if( m_dwCapsMaxTxt == 1 || m_dwCapsMaxStages == 1)
				{
					type = DX_LOWEST_END;
				}
				else if( !m_bCapsTxtOpLerp )
				{
					type = DX_MIDDLE_LEVEL;
				}
				else
				{
					type = DX_HIGHER_END;
				}
			}
		}
	
		type = DX_HIGHER_END;

		switch( type )
		{
		case DX_BEST_FIT:
			m_pColorCombiner = new CDirectXColorCombiner(pRender);
			break;
		case DX_LOWEST_END:
			m_pColorCombiner = new CDirectXColorCombinerLowestEnd(pRender);
			break;
		case DX_LOWER_END:
			m_pColorCombiner = new CDirectXColorCombinerLowerEnd(pRender);
			break;
		case DX_MIDDLE_LEVEL:
			m_pColorCombiner = new CDirectXColorCombinerMiddleLevel(pRender);
			break;
		case DX_HIGHER_END:
			m_pColorCombiner = new CDirectXColorCombinerHigherEnd(pRender);
			break;
		case DX_NVIDIA_TNT:
		case DX_NVIDIA_GEFORCE_PLUS:
			//m_pColorCombiner = new CNvTNTDirectXCombiner(pRender);
			m_pColorCombiner = new CDirectXColorCombinerMiddleLevel(pRender);
			break;
		case DX_ATI_RADEON_PLUS:
			m_pColorCombiner = new CDirectXColorCombinerATIDadeonPlus(pRender);
			break;
		}
	
		SAFE_CHECK(m_pColorCombiner);
	}

	return m_pColorCombiner;
}

CBlender * DirectXDeviceBuilder::CreateAlphaBlender(CDaedalusRender *pRender)
{
	if( m_pAlphaBlender == NULL )
	{
		m_pAlphaBlender = new CDirectXBlender(pRender);
		SAFE_CHECK(m_pAlphaBlender);
	}

	return m_pAlphaBlender;
}


