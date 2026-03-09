/*
Copyright (C) 2001 StrmnNrmn

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

#ifndef __RDP_H__
#define __RDP_H__

#include "TextureCache.h"
#include "ultra_sptask.h"

#define MAX_DL_STACK_SIZE	32

typedef void (*RDPInstruction)(DWORD dwCmd0, DWORD dwCmd1);

#define StartRDP() { status.bRDPHalted = FALSE;  }
#define StopRDP()  { status.bRDPHalted = TRUE;  }

void RDP_DumpRSPCode(char * szName, DWORD dwCRC, DWORD * pBase, DWORD dwPCBase, DWORD dwLen);
void RDP_DumpRSPData(char * szName, DWORD dwCRC, DWORD * pBase, DWORD dwPCBase, DWORD dwLen);
#endif