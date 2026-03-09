
// Ez0n3 - this file isn't used anymore

/*#pragma once

typedef struct _LaunchData
{
	// 1964 settings
	DWORD dw1964DynaMem;
	DWORD dw1964PagingMem;
	
	// Pj64 settings
	DWORD dwPJ64DynaMem;
	DWORD dwPJ64PagingMem;

	// common settings
	BOOL bUseLLERSP;
	
	// Ez0n3 - reinstate max video mem
	DWORD dwMaxVideoMem;
	
} LaunchData;*/


// example of old launch code
/*
LaunchData ld;

// 1964 settings
ld.dw1964DynaMem	= dw1964DynaMem;
ld.dw1964PagingMem	= dw1964PagingMem;
			
// Pj64 settings
ld.dwPJ64DynaMem	= dwPJ64DynaMem;
ld.dwPJ64PagingMem	= dwPJ64PagingMem;

// common settings
ld.bUseLLERSP		= bUseLLERSP;

// Ez0n3 - reinstate max video mem
ld.dwMaxVideoMem	= dwMaxVideoMem;

LAUNCH_DATA _ld;
memcpy(_ld.Data, &ld, sizeof(LaunchData));

switch (preferedemu)
{
	case _1964:
	{
		switch (videoplugin) {
	case 0 : XLaunchNewImage("D:\\1964-510.xbe", &_ld);
			 break;
	case 1 : XLaunchNewImage("D:\\1964-531.xbe", &_ld);
			 break;
	case 2 : XLaunchNewImage("D:\\1964-560.xbe", &_ld);
			 break;
	case 3 : XLaunchNewImage("D:\\1964-611.xbe", &_ld);
		break;}
		break;
	}
	case _Project64:
	{
		switch (videoplugin) {
	case 0 : XLaunchNewImage("D:\\Pj64-510.xbe", &_ld);
			 break;
	case 1 : XLaunchNewImage("D:\\Pj64-531.xbe", &_ld);
			 break;
	case 2 : XLaunchNewImage("D:\\Pj64-560.xbe", &_ld);
			 break;
	case 3 : XLaunchNewImage("D:\\Pj64-611.xbe", &_ld);
		break;}
		break;
	}
	case _UltraHLE:
	{
		XLaunchNewImage("D:\\UltraHLE.xbe", NULL);
	}
}
*/