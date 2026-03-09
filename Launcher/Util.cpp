/**
 * Surreal 64 Launcher (C) 2003
 * 
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: buttza@hotmail.com, lantus@lantus-x.com
 */

#include "Util.h"

string Trim(const string &str)
{
	string szStr(str);
	string::size_type i;

	// remove all the whitespaces and tabs at the start of the string
	for (i = 0; i < szStr.length(); i++)
	{
		if (szStr[i] != ' ' && szStr[i] != '\t' && szStr[i] != '\n')
			break;
	}

	if (i != 0)
		szStr = szStr.erase(0, i);

	if (szStr.length() == 0)
		return szStr;

	// remove all the whitespaces and tabs at the end of the string
	for (i = szStr.length() - 1; i >= 0; i--)
	{
		if (szStr[i] != ' ' && szStr[i] != '\t' && szStr[i] != '\n')
			break;
	}

	if (i != 0)
		szStr = szStr.erase(i+1);

	return szStr;
}

string FixPath(const string &path)
{
	string szPath(path);

	if (szPath.substr(szPath.length() - 1) != "\\")
	{
		szPath = szPath + string("\\");
	}

	return szPath;
}

word *StringToWChar(const string &str)
{
	word *retVal = new word[(str.length()+1)*2];
	memset(retVal, 0, (str.length()+1)*2*sizeof(word));
	
	if (str.length() > 0)
		mbstowcs(retVal, str.c_str(), str.length());

	return retVal;
}

// right less than left
bool StringRLTL(const string& szElem1, const string& szElem2)
{
	string szLeft = szElem1;
	string szRight = szElem2;
	transform(szLeft.begin(), szLeft.end(), szLeft.begin(), ::tolower);
	transform(szRight.begin(), szRight.end(), szRight.begin(), ::tolower);
	
	return (szLeft < szRight);
}

string Truncate(const string &szString, int iLength, bool bEllipses)
{
	string szTruncated = szString;
	if ((int)szTruncated.length() > iLength) {
		string szTrunc (szTruncated, 0, (bEllipses ? (iLength-3) : iLength));
		if (bEllipses) {
			szTrunc = Trim(szTrunc);
			szTrunc.append("...");
		}
		szTruncated = szTrunc;
	}
	return Trim(szTruncated);
}

byte GetByteSwapType(byte *header)
{
	if(header[0] == 0x80 && header[1] == 0x37)
	{
		return 1;
	}
	else if (header[0] == 0x37 && header[1] == 0x80)
	{
		return 2;
	}
	else if(header[2] == 0x37 && header[3] == 0x80)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

bool ByteSwap(dword size, byte *image, byte bswapType)
{
	dword k;

	switch (bswapType)
	{
		case 1: // converts 0x40123780 to 0x80371240
		{
			for(k = 0; k < size; k += 4)
			{
				
				_asm
				{
					mov edx, image
					add edx, k
					mov eax, dword ptr[edx]
					bswap eax
					mov dword ptr[edx], eax
				}
			}

			return true;
		}

		case 2: // converts 0x12408037 to 0x80371240
		{
			k = size;
			_asm
			{
				mov edx, image
				mov ebx, dword ptr[k]
				jmp _LABEL3
			}

			_LABEL2:
			_asm
			{
				sub ebx, 8
				test ebx, ebx
				jz _LABELEXIT
			}

			_LABEL3:
			_asm
			{
				mov eax, dword ptr[edx]
				mov ecx, eax
				shr ecx, 0x10
				shl eax, 0x10
				or ecx, eax
				mov dword ptr[edx], ecx
				add edx, 4

				mov eax, dword ptr[edx]
				mov ecx, eax
				shr ecx, 0x10
				shl eax, 0x10
				or ecx, eax
				mov dword ptr[edx], ecx
				add edx, 4
				jmp _LABEL2
			}

			_LABELEXIT: ;
			return true;
		}

		case 0:
			return true;

		default:
			return false;
	}
}

void SwapRomName(byte *name)
{
	byte c;

	for(int i = 0; i < 20; i += 4)
	{
		c = name[i];
		name[i] = name[i + 3];
		name[i + 3] = c;

		c = name[i + 2];
		name[i + 2] = name[i + 1];
		name[i + 1] = c;
	}

	for(int i = 19; i >= 0; i--)
	{
		if(name[i] != ' ') break;
	}

	name[i + 1] = '\0';
}