#ifndef _XBOX_DEPP_H__COMMON_
#define _XBOX_DEPP_H__COMMON_

#if _MSC_VER > 1000
#pragma once
#endif //_MSC_VER > 1000

#include <xtl.h>

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

// Message Box arg's, unused on XBOX
#define MB_ICONERROR		0x00000010L
#define MB_ICONINFORMATION	0x00000040L
#define MB_OK				0x00000000L

// ShowWindow arg's. unused on XBOX
#define SW_HIDE			 0
#define SW_SHOW			 5

BOOL PathFileExists(const char *pszPath);
int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode);
BOOL IsWindow(HWND hWnd);
BOOL ShowWindow(HWND hWnd, int CmdShow);
BOOL SetWindowText(HWND hWnd, LPCTSTR lpString);
LONG SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong);
BOOL GetClientRect(HWND hWnd, LPRECT lpRect);
int ShowCursor(BOOL bShow);
int GetDlgCtrlID(HWND hWnd);
HWND GetDlgItem(HWND hDlg, int nIDDlgItem);
DWORD GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
BOOL StrTrim(LPSTR psz, LPCSTR pszTrimChars);

#endif //_XBOX_DEPP_H__COMMON_