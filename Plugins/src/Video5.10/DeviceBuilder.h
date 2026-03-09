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

#ifndef _DEVICE_BUILDER_H
#define _DEVICE_BUILDER_H

//========================================================================

class CDeviceBuilder
{
public:
	virtual CGraphicsContext * CreateGraphicsContext(void)=0;
	virtual CDaedalusRender * CreateRender(void)=0;
	virtual CTexture * CreateTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget = false)=0;
	virtual CColorCombiner * CreateColorCombiner(CDaedalusRender *pRender)=0;
	virtual CBlender * CreateAlphaBlender(CDaedalusRender *pRender)=0;

	void DeleteGraphicsContext(void);
	void DeleteRender(void);
	void DeleteColorCombiner(void);
	void DeleteAlphaBlender(void);

	static void DeleteBuilder(void);
	static CDeviceBuilder* GetBuilder(void);
	static void SelectDeviceType(DaedalusSupportedDeviceType type);
	static DaedalusSupportedDeviceType GetDeviceType(void);
	static DaedalusSupportedDeviceType GetGeneralDeviceType(void);
	static DaedalusSupportedDeviceType m_deviceGeneralType;
protected:
	CDeviceBuilder();
	~CDeviceBuilder();

	static CDeviceBuilder* CreateBuilder(DaedalusSupportedDeviceType type);
	static DaedalusSupportedDeviceType m_deviceType;
	static CDeviceBuilder* m_pInstance;

	CDaedalusRender* m_pRender;
	CGraphicsContext* m_pGraphicsContext;
	CColorCombiner* m_pColorCombiner;
	CBlender* m_pAlphaBlender;
};


class OGLDeviceBuilder : public CDeviceBuilder
{
	friend class CDeviceBuilder;
public:
	CGraphicsContext * CreateGraphicsContext(void);
	CDaedalusRender * CreateRender(void);
	CTexture * CreateTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget = false);
	CColorCombiner * CreateColorCombiner(CDaedalusRender *pRender);
	CBlender * CreateAlphaBlender(CDaedalusRender *pRender);

protected:
	OGLDeviceBuilder() {};
	~OGLDeviceBuilder() {};

};

class DirectXDeviceBuilder : public CDeviceBuilder
{
	friend class CDeviceBuilder;
public:
	CGraphicsContext * CreateGraphicsContext(void);
	CDaedalusRender * CreateRender(void);
	CTexture * CreateTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget = false);
	CColorCombiner * CreateColorCombiner(CDaedalusRender *pRender);
	CBlender * CreateAlphaBlender(CDaedalusRender *pRender);

protected:
	DirectXDeviceBuilder() {};
	~DirectXDeviceBuilder() {};
};

/*
class GlideDeviceBuilder : public CDeviceBuilder
{
public:
	CGraphicsContext * CreateGraphicsContext(void);
	CDaedalusRender * CreateRender(void);
	CTexture * CreateTexture(DWORD dwWidth, DWORD dwHeight, bool asRenderTarget = false);
	CColorCombiner * CreateColorCombiner(void);
	CBlender * CreateAlphaBlender(void);
};
*/


#endif


