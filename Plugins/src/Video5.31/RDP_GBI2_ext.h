/*
Copyright (C) 2002 Rice1964

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

// Some new GBI2 extension ucodes
void DLParser_GBI2_DL_Count(DWORD dwCmd0, DWORD dwCmd1)
{
	SP_Timing(DP_Minimal);
	DP_Timing(DP_Minimal);

	// This cmd is likely to execute number of ucode at the given address
	DWORD dwAddr = RDPSegAddr(dwCmd1);
	{
		g_dwPCindex++;
		g_dwPCStack[g_dwPCindex].addr = dwAddr;
		g_dwPCStack[g_dwPCindex].limit = (dwCmd0&0xFFFF);
	}
}


void DLParser_GBI2_0x8(DWORD dwCmd0, DWORD dwCmd1)
{
	if( (dwCmd0&0x00FFFFFF) == 0x2F && (dwCmd1&0xFF000000) == 0x80000000 )
	{
		// V-Rally 64
		RDP_S2DEX_SPObjLoadTxRectR(dwCmd0, dwCmd1);
	}
	else
	{
		DLParser_Nothing(dwCmd0, dwCmd1);
	}
}


