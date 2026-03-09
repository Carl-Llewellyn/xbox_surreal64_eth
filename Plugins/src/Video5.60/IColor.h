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

#ifndef _ICOLOR_H_
#define _ICOLOR_H_


class IColor {
public:
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;

	IColor(D3DCOLOR rgba)
	{
		*((D3DCOLOR*)this) = rgba;
	}

	IColor()
	{
		*((D3DCOLOR*)this) = 0;
	}

	IColor(D3DXCOLOR &rgba)
	{
		*((D3DCOLOR*)this) = DWORD(rgba);
	}

	inline IColor operator = (const IColor &sec) const
	{
		*((D3DCOLOR*)this) = *((D3DCOLOR*)&sec);
		return *this;
	}

	inline IColor operator = (D3DCOLOR col) const
	{
		*((D3DCOLOR*)this) = col;
		return *this;
	}

	inline IColor operator + (const IColor &sec) const
	{
		IColor newc;
		newc.r = (BYTE)min(DWORD(r)+DWORD(sec.r),0xFF);
		newc.g = (BYTE)min(DWORD(g)+DWORD(sec.g),0xFF);
		newc.b = (BYTE)min(DWORD(b)+DWORD(sec.b),0xFF);
		newc.a = (BYTE)min(DWORD(a)+DWORD(sec.a),0xFF);

		return newc;
	}

	inline IColor operator - (const IColor &sec) const
	{
		IColor newc;
		newc.r = max(int(r)-int(sec.r),0);
		newc.g = max(int(g)-int(sec.g),0);
		newc.b = max(int(b)-int(sec.b),0);
		newc.a = max(int(a)-int(sec.a),0);

		return newc;
	}
	inline IColor operator * (const IColor &sec) const
	{
		IColor newc;
		newc.r = (BYTE)min(DWORD(r)*DWORD(sec.r)/256,255);
		newc.g = (BYTE)min(DWORD(g)*DWORD(sec.g)/256,255);
		newc.b = (BYTE)min(DWORD(b)*DWORD(sec.b)/256,255);
		newc.a = (BYTE)min(DWORD(a)*DWORD(sec.a)/256,255);

		return newc;
	}

	inline IColor& operator += (const IColor &sec)
	{
		r = BYTE(min(DWORD(r)+DWORD(sec.r),255));
		g = BYTE(min(DWORD(g)+DWORD(sec.g),255));
		b = BYTE(min(DWORD(b)+DWORD(sec.b),255));
		a = BYTE(min(DWORD(a)+DWORD(sec.a),255));
		return *this;
	}

	inline IColor& operator -= (const IColor &sec)
	{
		r = BYTE(max(int(r)-int(sec.r),0));
		g = BYTE(max(int(g)-int(sec.g),0));
		b = BYTE(max(int(b)-int(sec.b),0));
		a = BYTE(max(int(a)-int(sec.a),0));
		return *this;
	}

	inline IColor& operator *= (const IColor &sec)
	{
		r = BYTE(min(DWORD(r)*DWORD(sec.r)/256,255));
		g = BYTE(min(DWORD(g)*DWORD(sec.g)/256,255));
		b = BYTE(min(DWORD(b)*DWORD(sec.b)/256,255));
		a = BYTE(min(DWORD(a)*DWORD(sec.a)/256,255));
		return *this;
	}
	
	inline IColor& operator += (D3DXCOLOR val)
	{
		IColor newc(val);
		return this->operator += (newc);
	}

	inline IColor& operator -= (D3DXCOLOR val)
	{
		IColor newc(val);
		return this->operator-=(newc);
	}

	inline IColor& operator *= (D3DXCOLOR val)
	{
		IColor newc(val);
		return this->operator*=(newc);
	}

	inline IColor operator + (D3DXCOLOR val) const
	{
		IColor newc(val);
		return this->operator+(newc);
	}

	inline IColor operator - (D3DXCOLOR val) const
	{
		IColor newc(val);
		return this->operator-(newc);
	}

	inline IColor operator * (D3DXCOLOR val) const
	{
		IColor newc(val);
		return this->operator*(newc);
	}

	inline operator D3DCOLOR() const
	{
		return *((D3DCOLOR*)this);
	}

	inline operator D3DXCOLOR() const
	{
		D3DXCOLOR rgba;
		rgba.r = (float)r/256.0f;
		rgba.g = (float)g/256.0f;
		rgba.b = (float)b/256.0f;
		rgba.a = (float)a/256.0f;

		return rgba;
	}

	inline void Complement()
	{
		r = 0xFF-r;
		g = 0xFF-g;
		b = 0xFF-b;
		a = 0xFF-a;
	}

	inline void AlphaReplicate()
	{
		r=g=b=a;
	}
};


#endif


