// NestopiaX 1.0 Synopsis Reader
// Created by nes6502

#include <xtl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include "Util.h"
#include "..\simpleini.h"
using namespace std;

// uncommented it will print all output to the console, commented will rename/write files
//#define DEBUG_SYNOPSIS_BUILD 1

// uncommented will make an ini file that can be used to cross reference crc1 to txt filename
// not sure if it should have a table like boxart, so using ini for now
#define SYNOPSIS_BUILD_TABLE 1

// just need one global
vector<string> vSynopsis;

extern char szPathMedia[256];
extern int iSynopsisWrap;


void WordWrap(const string& szString, vector<string>& vOutput, unsigned int iLength)
{
	istringstream iss(szString);
	string szLine;
	do {
		string szWord;
		iss >> szWord;

		if (szLine.length() + szWord.length() > iLength) {
			vOutput.push_back(szLine);
			szLine.clear();
		}
		szLine += szWord + " ";
	}while (iss);

	if (!szLine.empty()) {
		vOutput.push_back(szLine);
	}
}

void LoadSynopsis(DWORD crc)
{
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
	ini.SetMultiKey(false);
	ini.SetMultiLine(false);
	ini.SetSpaces(false);
	
	vSynopsis.clear(); // make sure it's empty
	
	char szIniFilename[256];
	sprintf(szIniFilename, "%ssynopsis\\synopsis.ini", szPathMedia);
	
	OutputDebugString(szIniFilename);
	rc = ini.LoadFile(szIniFilename);
	if (rc < 0) {
		OutputDebugStringA(" Failed to Load!\n");
		return;
	}
	OutputDebugStringA(" Successfully Loaded!\n");
	
	char szRomCrc[64];
	sprintf(szRomCrc, "%08X", crc);
	string szFileName = ini.GetValue(szRomCrc, "Filename", "");
	
	// 8 crc + 4 ext min (xxxxxxxx.txt is shortest)
	if (szFileName.length() < 12)
		return;
	
	char szTxtFilename[256];
	sprintf(szTxtFilename, "%ssynopsis\\%s", szPathMedia, szFileName.c_str());
	ifstream f(szTxtFilename);
	if(!f) {
		OutputDebugString(szTxtFilename);
		OutputDebugStringA(" Failed to Open!");
        return;
	}
	while(!f.eof()) {
		char line[20480];
		f.getline(line, 20480);
		string szLine = line;
		
		//vSynopsis.push_back(t);
		WordWrap(szLine, vSynopsis, iSynopsisWrap);
	}
	f.close();
}

void ClearSynopsis()
{
	vSynopsis.clear();
}

void DeleteSynopsis(const string& szSynopsisPath)
{
	HANDLE 				hFind;
	WIN32_FIND_DATA		oFindData;
	
	// remove the all the ini and txt files from the synopsis dir
	char szRemSynopsis[256];
	sprintf(szRemSynopsis, "%s*.*", szSynopsisPath.c_str());
	hFind = FindFirstFile(szRemSynopsis, &oFindData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if(oFindData.cFileName[0] == '.' || (oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				continue;
			} else {
				string szRemFilename (oFindData.cFileName);
			
				char ext[64];
				_splitpath(szRemFilename.c_str(), NULL, NULL, NULL, ext);
				if (stricmp(ext, ".ini") == 0 || stricmp(ext, ".txt") == 0)
				{
					char szRemFile[256];
					sprintf(szRemFile, "%s%s", szSynopsisPath.c_str(), szRemFilename.c_str());
					if (!DeleteFile(szRemFile)) {
						OutputDebugString(szRemFile);
						OutputDebugStringA(" Could Not Be Deleted!\n");
					}
				}
			}
		} while (FindNextFile(hFind, &oFindData));
	}
	FindClose(hFind);
}

// break synopsis file into smaller files based on rom crcs
void BuildSynopsis()
{
	OutputDebugString("-- Build Synopsis --\n");

	char szPathSynopsis[256];
	sprintf(szPathSynopsis, "%ssynopsis\\", szPathMedia);
	
	vector<string> vInput;
	vector<string> vOutput;
	
#ifdef SYNOPSIS_BUILD_TABLE
	CSimpleIniA ini;
	SI_Error rc;
	ini.SetUnicode(true);
    ini.SetMultiKey(false);
    ini.SetMultiLine(false);
	ini.SetSpaces(false);

	string szSynopsisTable;
	char szPathSynopsisTable[256];
	sprintf(szPathSynopsisTable, "%ssynopsis.ini", szPathSynopsis);
#endif //SYNOPSIS_BUILD_TABLE

#ifndef DEBUG_SYNOPSIS_BUILD
	DeleteSynopsis(string(szPathSynopsis));
#else
	/*char szExcludedChars[4096];
	memset(szExcludedChars, 0, sizeof(szExcludedChars));*/
#endif //DEBUG_SYNOPSIS_BUILD
	
	// input the entire file to a vector
	ifstream f("D:\\synopsis.txt");
	if(!f) {
		OutputDebugString("D:\\synopsis.txt Failed to Open!");
        return;
	}
	while(!f.eof()) {
		char line[20480];
		f.getline(line, 20480);
		string t = line;
		vInput.push_back(t);
	}
	f.close();

	int iLineCount = 0;
	int iNewLineCount = 0;
	bool bInDesc = false;
	vector<string> vFileNames;

	// loop through the synopsis vector and break into smaller vectors
	for (vector<string>::iterator i = vInput.begin(); i != vInput.end(); ++i)
	{
		string szLine (*i);

		// potential rom crc line
		int iLineLen = Trim(szLine).length();
		if ((iLineLen == 8 || (iLineLen > 8 && szLine.find(",") == 8)))
		{
			// check for valid chars
			int iCommaCount = 0;
			bool bIsValid = false;
			for (int j = 0; j < (int)szLine.length(); j++)
			{
				int iLC = -1;
				iLC = szLine.find_first_of("abcdefghijklmnopqrstuvwxyz"); // no lowercase chars
			
				// found a line with rom crcs
				if ( ((szLine[j] >= 48 && szLine[j] <= 57) || (szLine[j] >= 65 && szLine[j] <= 90) || szLine[j] == 44) && szLine[j] != ' ' && iLC == -1) { // >=0 and <=9 or >=A and <=Z or ==,
					bIsValid = true;
					if (szLine[j] == 44) {
						iCommaCount++;
						szLine[j] = '-'; // comma to dash
					}
				} else {
					bIsValid = false;
					break;
				}
			}
			
			// it's a valid crc line
			if (bIsValid)
			{
				// found the next crc, output what is currently in the output vector
				if (bInDesc)
				{
					bInDesc = false;

					// clean and trim the output for the file(s)
					string szFileOutput;
					if (vOutput.size() > 0)
					{
						int iDescLen = 0;
						int iDescLines = 0;
						bool bDescOutput = false;
						for (vector<string>::iterator j = vOutput.begin(); j != vOutput.end(); ++j)
						{
							string szAppend (Trim(*j));
							
							if (szAppend.find("further info yet", 0) != string::npos)
								szFileOutput.append("No Information Available");
							else
								szFileOutput.append(szAppend);
							
							szFileOutput.append("\n");
							
							iDescLen = szAppend.length();

							if (bDescOutput)
								iDescLines++;
								
							if (szAppend.find("________", 0) != string::npos)
								bDescOutput = true;
						}
						if (iDescLines <= 1 && iDescLen == 0) // there's only a \n in the body
							szFileOutput.append("No Information Available\n"); // no description
					}
					else
					{
						szFileOutput = "No Information Available"; // nothing to show
					}
					string szFileOutputTrim (Trim(szFileOutput));
					
					// rem trailing new lines and spaces
					while ((szFileOutputTrim.length() - 1) == '\n' || (szFileOutputTrim.length() - 1) == '\r' || (szFileOutputTrim.length() - 1) == ' ')
					{
						szFileOutputTrim.erase(szFileOutputTrim.length() - 1);
					}

					// write the output to new file(s)
					for (vector<string>::iterator j = vFileNames.begin(); j != vFileNames.end(); ++j)
					{
						string szFilename (*j);
						
						char szOutputFilename[256];
						sprintf(szOutputFilename, "%s%s.txt", szPathSynopsis, szFilename.c_str());
						
#ifdef DEBUG_SYNOPSIS_BUILD
						// print output to console
						OutputDebugString("Output Filename: ");
						OutputDebugString(szOutputFilename);
						OutputDebugString("\n");
						
						OutputDebugString("Output: \n");
						OutputDebugString(szFileOutputTrim.c_str());
						OutputDebugString("\n----------------------\n");
#else
						// print output to files
						ofstream outfile (szOutputFilename);
						if (outfile.is_open()) {
							outfile << szFileOutputTrim.c_str();
							outfile.close();
						} else {
							OutputDebugString(szOutputFilename);
							OutputDebugStringA(" Failed to Open!\n");
						}
#endif //DEBUG_SYNOPSIS_BUILD

#ifdef SYNOPSIS_BUILD_TABLE
						vector<string> vCrcs;
						
						string szFilenameTemp (szFilename);
						szFilenameTemp.append("-");
						for (int k = 0; k < (int)szFilenameTemp.length(); k++)
						{	
							if (strchr("-", szFilenameTemp[k])) {
								vCrcs.push_back(szFilenameTemp.substr((k-8), 8));
							}
						}
							
						if (vCrcs.size() > 0)
						{
#ifdef DEBUG_SYNOPSIS_BUILD
							OutputDebugString("Table Data: ");
							OutputDebugString("\n============================================================\n");
#endif //DEBUG_SYNOPSIS_BUILD
							for (vector<string>::iterator k = vCrcs.begin(); k != vCrcs.end(); ++k)
							{
								string szCrcFilename (*k);
								
#ifdef DEBUG_SYNOPSIS_BUILD
								// print output to console
								OutputDebugString("CRC: ");
								OutputDebugString(szCrcFilename.c_str());
								OutputDebugString("\n");
#else
								// print to ini
								char szFilenameIni[256];
								sprintf(szFilenameIni, "%s.txt", szFilename.c_str());
								ini.SetValue(szCrcFilename.c_str(), "Filename", szFilenameIni);
#endif //DEBUG_SYNOPSIS_BUILD
							}
#ifdef DEBUG_SYNOPSIS_BUILD
							OutputDebugString("Lookup: ");
							OutputDebugString(szOutputFilename);
							OutputDebugString("\n============================================================\n");
#endif //DEBUG_SYNOPSIS_BUILD
						}
						
						vCrcs.clear();
#endif //SYNOPSIS_BUILD_TABLE
					}

					vFileNames.clear();
					vOutput.clear();
				}
				
				// found next rom crc, fill the empty vectors
				if (!bInDesc)
				{
					// more than 4 crcs, need to split them
					if (iCommaCount > 3)
					{
						// EG: DD26FDA1,CB4A6BE3,67D20729,F696774C,593BD58B,C330786C,93945F48,5C0F2E30,3409CEB6,CEC8517A,916B8B5B,780B85A4
						int iCrcCount = iCommaCount + 1; // comma=dashes

						// up to 16 crcs
						if (iCrcCount >= 5 && iCrcCount <= 8 ) {
							vFileNames.push_back(Trim(szLine.substr(0, 35)));
							vFileNames.push_back(Trim(szLine.substr(36, (szLine.length() - 36))));
						}
						else if (iCrcCount >= 9 && iCrcCount <= 12 ) {
							vFileNames.push_back(Trim(szLine.substr(0, 35)));
							vFileNames.push_back(Trim(szLine.substr(36, 35)));
							vFileNames.push_back(Trim(szLine.substr(72, (szLine.length() - 72))));
						}
						else if (iCrcCount >= 13 && iCrcCount <= 16 ) {
							vFileNames.push_back(Trim(szLine.substr(0, 35)));
							vFileNames.push_back(Trim(szLine.substr(36, 35)));
							vFileNames.push_back(Trim(szLine.substr(72, 35)));
							vFileNames.push_back(Trim(szLine.substr(108, (szLine.length() - 108))));
						}
					}
					else
					{
						// less than 4 crcs, no splitting
						vFileNames.push_back(Trim(szLine));
					}
						
					bInDesc = true; // the next lines will be the body for this rom
				}

				continue; // skip this crc line, just need it for filenames
			}
			/*else
				bInDesc = true;*/
		}
		
		if (bInDesc)
		{
			if( // skip unwanted lines
				szLine.find("****", 0) != string::npos || // section dividers
				szLine.find("=-=-", 0) != string::npos || // body dividers
				szLine.find("http://", 0) != string::npos || // urls - cant wrap or click
				// MMQ notes
				szLine.find("Mega Man (?)", 0) != string::npos ||
				szLine.find(" - MM?", 0) != string::npos ||
				szLine.find(" -MM?", 0) != string::npos ||
				szLine.find(")- MM", 0) != string::npos ||
				szLine.find(". :/", 0) != string::npos ||
				szLine.find("-MM") == 0
			)
				continue;

			// replace MMQ "no info" with consistent one later
			if (strcmp(szLine.c_str(), "no info") == 0)
				continue;

			string szCleanLine;

			// loop through the line and pop out unwanted chars
			if ((int)szLine.length() > 0)
			{
			
				// list of allowed body chars - mind the 1st space ;)
				char szValidChars[] = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_-+={}[]|:;\"'<>,.?\\/~`";
				
				// excluded using szValidChars - these have no font equivalent - none of them show
				//  ∞íÂÜÊ£ãÈìîñÔøΩëÖó·„¸	Ì∑ÆéôÇ∏É≥Ø®´§ﬂ…ˆ•”©π»¶∫Î≠ºÕª¨Û–⁄¿◊ßŒÁ√°™ËÍ±ƒï Å’
				
#ifdef DEBUG_SYNOPSIS_BUILD
				/*OutputDebugString("Line To Clean: \n");
				OutputDebugString(szLine.c_str());
				OutputDebugString("\n");*/
#endif //DEBUG_SYNOPSIS_BUILD

				char szTempLine[4096];
				memset(&szTempLine[0], 0, sizeof(szTempLine));

				int iCharCount = 0;
				for (int j = 0; j < (int)szLine.length(); j++)
				{
					// need to test for special characters
					if (!strchr(szValidChars, szLine[j])) {
#ifdef DEBUG_SYNOPSIS_BUILD
						/*bool bUnique = true;
						for (int k = 0; k < (int)strlen(szExcludedChars); k++) {
							if (szExcludedChars[k] == szLine[j]) bUnique = false;
						}
						if (bUnique) {
							szExcludedChars[strlen(szExcludedChars)] = szLine[j];
						}*/
#endif //DEBUG_SYNOPSIS_BUILD

						//working title Eltale („Ç®„É´„ÉÜ„Ç§„É´) as its name.

						//if (strchr("∞", szLine[j])
							//szTempLine[iCharCount] = '';
						if (strchr("í", szLine[j]))
							szTempLine[iCharCount] = '\'';
						else if (strchr("Â", szLine[j]))
							szTempLine[iCharCount] = 'a';
						//else if (strchr("Ü", szLine[j]))
							//szTempLine[iCharCount] = '';
						//else if (strchr("Ê", szLine[j]))
							//szTempLine[iCharCount] = '';
						//else if (strchr("£", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("ã", szLine[j]))
							szTempLine[iCharCount] = '<';
						else if (strchr("È", szLine[j]))
							szTempLine[iCharCount] = 'e';
						else if (strchr("ì", szLine[j]))
							szTempLine[iCharCount] = '"';
						else if (strchr("î", szLine[j]))
							szTempLine[iCharCount] = '"';
						else if (strchr("ñ", szLine[j]))
							szTempLine[iCharCount] = '-';
						else if (strchr("Ô", szLine[j])){
							if (strchr("ø", szLine[j+1]) && strchr("Ω", szLine[j+2])){ // some kind of spacer "ÔøΩ"
								szTempLine[iCharCount] = '-';
							}else{
								szTempLine[iCharCount] = 'i';}}
						//else if (strchr("ø", szLine[j]))
							//szTempLine[iCharCount] = '';
						/*else if (strchr("Ω", szLine[j])){
							if (!strchr("ø", szLine[j-1])){
								strcat(szTempLine,"1/2");
								iCharCount += 2;}} // adds 1 later*/
						else if (strchr("ë", szLine[j]))
							szTempLine[iCharCount] = '\'';
						else if (strchr("Ö", szLine[j])){
							strcat(szTempLine,"...");
							iCharCount += 2;} // adds 1 later
						else if (strchr("ó", szLine[j]))
							szTempLine[iCharCount] = '-';
						else if (strchr("·", szLine[j]))
							szTempLine[iCharCount] = 'a';
						else if (strchr("„", szLine[j]))
							szTempLine[iCharCount] = 'a';
						else if (strchr("¸", szLine[j]))
							szTempLine[iCharCount] = 'u';
						else if (strchr("\t", szLine[j])){ // \t
							//if (strchr(" ", szLine[j-1]) && strchr(" ", szLine[j+1]))
								szTempLine[iCharCount] = ' ';}
						else if (strchr("Ì", szLine[j]))
							szTempLine[iCharCount] = 'i';
						else if (strchr("∑", szLine[j]))
							szTempLine[iCharCount] = '-';
						//else if (strchr("Æ", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("é", szLine[j]))
							szTempLine[iCharCount] = 'Z';
						else if (strchr("ô", szLine[j])){
							strcat(szTempLine,"tm");
							iCharCount += 1;} // adds 1 later
						else if (strchr("Ç", szLine[j]))
							szTempLine[iCharCount] = ',';
						//else if (strchr("∏", szLine[j]))
							//szTempLine[iCharCount] = '';	
						else if (strchr("É", szLine[j]))
							szTempLine[iCharCount] = 'f'; // just skip it?
						else if (strchr("≥", szLine[j]))
							szTempLine[iCharCount] = '3';
						else if (strchr("Ø", szLine[j]))
							szTempLine[iCharCount] = '-';
						//else if (strchr("®", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("´", szLine[j])){
							if (!strchr("É", szLine[j-1]) && !strchr("°", szLine[j-1])){ // needs better check
								strcat(szTempLine,"<<");
								iCharCount += 1;}} // adds 1 later
						//else if (strchr("§", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("ﬂ", szLine[j]))
							szTempLine[iCharCount] = 'B';
						else if (strchr("…", szLine[j]))
							szTempLine[iCharCount] = 'E';
						else if (strchr("ˆ", szLine[j]))
							szTempLine[iCharCount] = 'o';
						else if (strchr("•", szLine[j]))
							szTempLine[iCharCount] = 'Y';
						else if (strchr("”", szLine[j]))
							szTempLine[iCharCount] = 'O';
						//else if (strchr("©", szLine[j]))
							//szTempLine[iCharCount] = '';
						//else if (strchr("π", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("»", szLine[j]))
							szTempLine[iCharCount] = 'E';
						else if (strchr("¶", szLine[j]))
							szTempLine[iCharCount] = '|';
						//else if (strchr("∫", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("Î", szLine[j]))
							szTempLine[iCharCount] = 'e';
						else if (strchr("≠", szLine[j]))
							szTempLine[iCharCount] = '-';
						else if (strchr("º", szLine[j])){
							if (!strchr("•", szLine[j+1]) && !strchr("§", szLine[j+1])){ // needs better check
								strcat(szTempLine,"1/4");
								iCharCount += 2;}} // adds 1 later
						else if (strchr("Õ", szLine[j]))
							szTempLine[iCharCount] = 'I';
						else if (strchr("ª", szLine[j])){
							if (!strchr("Õ", szLine[j-1]) && !strchr("Å", szLine[j-1])){ // needs better check
								strcat(szTempLine,">>");
								iCharCount += 1;}} // adds 1 later
						//else if (strchr("¨", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("Û", szLine[j]))
							szTempLine[iCharCount] = 'o';
						else if (strchr("–", szLine[j]))
							szTempLine[iCharCount] = 'D';
						else if (strchr("⁄", szLine[j]))
							szTempLine[iCharCount] = 'U';
						else if (strchr("¿", szLine[j]))
							szTempLine[iCharCount] = 'A';
						else if (strchr("◊", szLine[j]))
							szTempLine[iCharCount] = 'x';	
						else if (strchr("ß", szLine[j]))
							szTempLine[iCharCount] = 'S';	
						else if (strchr("Œ", szLine[j]))
							szTempLine[iCharCount] = 'I';
						else if (strchr("Á", szLine[j]))
							szTempLine[iCharCount] = 'c';
						else if (strchr("√", szLine[j]))
							szTempLine[iCharCount] = 'A';
						else if (strchr("°", szLine[j])){
							if (strchr(" ", szLine[j-1]) && strchr(" ", szLine[j+1]))
								szTempLine[iCharCount] = ' ';} //'i'
						else if (strchr("™", szLine[j]))
							szTempLine[iCharCount] = 'a';
						else if (strchr("Ë", szLine[j]))
							szTempLine[iCharCount] = 'e';
						else if (strchr("Í", szLine[j]))
							szTempLine[iCharCount] = 'e';
						//else if (strchr("±", szLine[j]))
							//szTempLine[iCharCount] = '';
						else if (strchr("ƒ", szLine[j]))
							szTempLine[iCharCount] = 'A';
						else if (strchr("ï", szLine[j]))
							szTempLine[iCharCount] = '-';
						else if (strchr(" ", szLine[j]))
							szTempLine[iCharCount] = 'E';
						//else if (strchr("Å", szLine[j]))
							//szTempLine[iCharCount] = '';		
						else if (strchr("’", szLine[j]))
							szTempLine[iCharCount] = 'O';
						else
							continue;
							
						iCharCount++;
					}
					else {
						// clean up multiple question marks
						if (strchr("?", szLine[j]) && strchr("?", szLine[(j+1)]))
							continue;
						
						// clean up multiple spaces
						if (strchr(" ", szLine[j]) && strchr(" ", szLine[(j+1)]))
							continue;
						
						szTempLine[iCharCount] = szLine[j];
						iCharCount++;
					}
				}
				szTempLine[iCharCount] = '\0';
				szCleanLine = string(szTempLine);
				
#ifdef DEBUG_SYNOPSIS_BUILD
				/*OutputDebugString("Line Post Clean: \n");
				OutputDebugString(szCleanLine.c_str());
				OutputDebugString("\n");*/
#endif //DEBUG_SYNOPSIS_BUILD
			}
			else {
				szCleanLine = "\n";
			}
			
			// skip lines that only contain "*"
			if (strcmp(szCleanLine.c_str(), "*") == 0)
				continue;

			// clean up multiple newlines
			if ((int)Trim(szCleanLine).length() == 0)
				iNewLineCount++;
			else
				iNewLineCount = 0;
				
			if (szLine.find("________", 0) != string::npos)
				iNewLineCount++; // it's going to add one later
				
			if (iNewLineCount > 1)
				continue;

			//some have a single char after "." and before the newline - looks like a typo
			if (szLine.find(".[") == ((int)szCleanLine.length() - 2) ||
				szLine.find(".\\") == ((int)szCleanLine.length() - 2))
					szCleanLine.erase((int)szCleanLine.length() - 1);
			
			// push back the clean data
			vOutput.push_back(Trim(szCleanLine));
			
			// adds a newline under the dashes between info and desc - where "*" was
			if (szLine.find("________", 0) != string::npos) // && (szCleanLine[0] != '\n' && szCleanLine[0] != '\r')
				vOutput.push_back("\n");
		}

		iLineCount++;
	}
    
	// empty all the vectors
	vFileNames.clear();
	vOutput.clear();
	vInput.clear();
	
#ifndef DEBUG_SYNOPSIS_BUILD
	// rename file so that it won't run again
	char oldname[] = "D:\\synopsis.txt";
	//char newname[] = "D:\\synopsis.txt.bak";
	
	// put it in the synopsis dir? so you have a copy of the file used to build?
	char newname[256];
	sprintf(newname, "%ssynopsis.txt", szPathSynopsis);
	
	OutputDebugString(oldname);
	int result = rename(oldname, newname);
	if (result == 0)
		OutputDebugStringA(" Renamed Successfully!\n");
	else
		OutputDebugStringA(" Rename Failed!\n");
	
#else
	// show what got excluded
	/*if (strlen(szExcludedChars)) {
		szExcludedChars[(strlen(szExcludedChars))] = '\0';
		OutputDebugString("ExcludedChars: ");
		OutputDebugStringA(szExcludedChars);
		OutputDebugStringA("\n");
	}*/
#endif //DEBUG_SYNOPSIS_BUILD

#ifdef SYNOPSIS_BUILD_TABLE
	OutputDebugString(szPathSynopsisTable);
	rc = ini.SaveFile(szPathSynopsisTable);
    if (rc < 0) {
		OutputDebugStringA(" Failed to Save!\n");
		return;
	}
	OutputDebugStringA(" Successfully Saved!\n");
#endif //SYNOPSIS_BUILD_TABLE
}
