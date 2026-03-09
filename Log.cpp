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

#include <xbapp.h>
#include <xbresource.h>
#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>
using namespace std;

void __cdecl Log(char *szMessage, ...)
{
	long	lSize = 0;
	char	szMsg[512];
	char	szTime[100];
	char	szOutput[768];
	char	szLogFile[256] = "T:\\Misc\\log.txt";
	time_t	rawtime;
	va_list vaArgList;
	
	va_start(vaArgList, szMessage);
	vsprintf(szMsg, szMessage, vaArgList);
	va_end(vaArgList);

	time (&rawtime);
	strftime(szTime, 100, "%Y:%m:%d::%H:%M:%S", localtime(&rawtime));
	
	sprintf(szOutput, "%s| %s\n", szTime, szMsg);
	
	char szLogFileTmp[256];
	sprintf(szLogFileTmp, "%s.tmp", szLogFile);
	
	// rename log file to tmp
	rename(szLogFile, szLogFileTmp);

	ifstream LogFileTmp (szLogFileTmp, ifstream::binary);
	ofstream LogFile (szLogFile, ofstream::binary);

	LogFileTmp.seekg(0,ifstream::end);
	lSize = LogFileTmp.tellg();
	LogFileTmp.seekg(0);
	
	// start a new log if > 5MB
	if (lSize > 5242880)
	{
		LogFileTmp.close();
		
		char szLogFileBak[256];
		sprintf(szLogFileBak, "%s.bak", szLogFile);
		
		// rem the bak file
		ifstream LogFileBak (szLogFileBak, ifstream::binary);
		if (LogFileBak.is_open())
		{
			LogFileBak.close();
			if (!DeleteFile(szLogFileBak))
			{
				OutputDebugString(szLogFileBak);
				OutputDebugStringA(" Could Not Be Deleted!\n");
			}
		}
		
		// rename tmp file to bak
		rename(szLogFileTmp, szLogFileBak);
		
		LogFile.write(szOutput, strlen(szOutput));
	}
	else
	{
		// prepend the new line if log exists
		if (lSize > 0)
		{
			char * szBuffer;
			szBuffer = new char [lSize];
			
			LogFileTmp.read(szBuffer, lSize);
			LogFile.write(szOutput, strlen(szOutput));
			LogFile.write(szBuffer, lSize);
			
			delete[] szBuffer;
		}
		else
		{
			LogFile.write(szOutput, strlen(szOutput));
		}
		
		// rem the temp file
		LogFileTmp.close();
		if (!DeleteFile(szLogFileTmp))
		{
			OutputDebugString(szLogFileTmp);
			OutputDebugStringA(" Could Not Be Deleted!\n");
		}
	}

	LogFile.close();

#ifdef DEBUG
	OutputDebugString("LOG: ");
	OutputDebugStringA(szOutput);
#endif
}