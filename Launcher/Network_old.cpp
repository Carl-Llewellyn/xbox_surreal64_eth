#include "Network.h"
//Basic Network / HTTP(1.1) support added by freakdave
/*
The code for the HTTP protocol and gethostbyname() was borrowed from XBMC.
The code for getting the host adress was borrowed from LinksBoks.
Rest is my code..
You can find my old http code in Network_old.cpp (actually it is working, but it lacks support for binary files, chunked transfer, etc..,
so i decided to take already existing sources. Why reinvent the wheel ?).
Note: All Update funtions are running as separate threads.

What's left to do:

- Add Download new executable/archive (PRIO: high)
- Add Download progress bar (PRIO: high)
- Clean up code (PRIO: normal)

- Implement a simple webserver (PRIO: very low)
	-> Edit settings and launch roms from pc
	-> Users will be able to access their *sites*(IP/DNS), browse their roms directory
	   and download/exchange their roms

How this code is meant to be working:
- Add Update option in the menu
- On entering the update option the code should create a new thread and call the Update function
- Surreal will then try to download and replace the requested file
- and clean up afterwards...

Misc: Don't forget to update HTTP.cpp, if you're planning to change the host(name)!

*/

//#define DEBUG // -> 192.168.0.26



/* Variables */
INT err; //Error

//Tells us whether or not an Ethernet cable is connected to the XBOX
//Deactivated for now..
BOOL IsXboxConnected()
{/*
	DWORD dwStatus = XNetGetEthernetLinkStatus();
	if(dwStatus != 0){
	   return TRUE;
   }
   else
   {
	   return FALSE;
   }*/
	return FALSE;
}

//Wrapper for gethostbyname() -> not implemented in Winsock for XDK
struct hostent* _cdecl gethostbyname(const char* _name)
{
	HostEnt *server = (HostEnt *)malloc( sizeof( HostEnt ) );
	struct hostent *host = (struct hostent *)malloc( sizeof( struct hostent ) );
	unsigned long addr = inet_addr( _name );

	WSAEVENT hEvent = WSACreateEvent();
	XNDNS* pDns = NULL;
	if( addr != INADDR_NONE )
	{
		host->h_addr_list[0] = (char *)malloc( 4 );
		memcpy( host->h_addr_list[0], &addr, 4 );
	}
	else
	{
		INT err = XNetDnsLookup(_name, hEvent, &pDns);
		WaitForSingleObject(hEvent, INFINITE);
		if( !pDns || pDns->iStatus )
		{
			if( pDns )
				XNetDnsRelease(pDns);
			free( host );
			return NULL;
		}

		host->h_addr_list[0] = (char *)malloc( 4 );
		memcpy( host->h_addr_list[0], &(pDns->aina[0].s_addr), 4 );

		XNetDnsRelease(pDns);
	}

	host->h_name = (char *)malloc( strlen( _name ) );
	strcpy( host->h_name, _name );
	host->h_aliases = 0;
	host->h_addrtype = AF_INET;
	host->h_length = 4;
	host->h_addr_list[1] = NULL;

	return host;
}


// Returns host adress (borrowed from LinksBoks)
DWORD GetHostAddress(const char *host)
{
    struct hostent *phe;
    char *p;

    phe = gethostbyname( host );
            
    if(phe==NULL)
        return 0;
    
    p = *phe->h_addr_list;
    return *((DWORD*)p);
}

//Initialize our Network
//TODO: Mess with Buffer sizes
BOOL InitNetwork(void){
	OutputDebugString("Initializing Network\n");
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	xnsp.cfgSockDefaultRecvBufsizeInK = 32;
	err = XNetStartup(&xnsp);

	if(err != 0){
		XNetCleanup();//Terminate Network
		OutputDebugString("Could not initialize Network\n");
		return FALSE;
	}
return TRUE;
}

//Init Winsock
BOOL InitWinsock(void){
OutputDebugString("Initializing Winsock\n");
WSADATA wsaData;
err = WSAStartup( MAKEWORD(2,2), &wsaData );

if(err != 0){
	WSACleanup();//Terminate Winsock
	XNetCleanup();//Terminate Network
	OutputDebugString("Could not initialize Winsock\n");
	return FALSE;
}
return TRUE;
}



//Clean up Network
BOOL TerminateNetwork(void){
OutputDebugString("Cleaning up Network\n");
XNetCleanup();
return TRUE;
}

//Clean up Winsock
BOOL TerminateWinsock(void){
OutputDebugString("Cleaning up Winsock\n");
WSACleanup();
return TRUE;
}


//Main UpdateRiceIni function (needs rewriting)
BOOL UpdateIni(int Version)
{
CHTTP http;
InitNetwork();
InitWinsock();

switch (Version)
{
	case 510:
#ifdef DEBUG
		http.Download("http://192.168.0.26/ini/RiceDaedalus5.1.0.ini","T:\\RiceDaedalus5.1.0.ini");
#else
		http.Download("http://freakdave.xbox-scene.com/ini/RiceDaedalus5.1.0.ini","T:\\RiceDaedalus5.1.0.ini");
#endif
		break;
	case 531:
#ifdef DEBUG
		http.Download("http://192.168.0.26/ini/RiceDaedalus5.3.1.ini","T:\\RiceDaedalus5.3.1.ini");
#else
		http.Download("http://freakdave.xbox-scene.com/ini/RiceDaedalus5.3.1.ini","T:\\RiceDaedalus5.3.1.ini");
#endif	
		break;
	case 560:
#ifdef DEBUG	
		http.Download("http://192.168.0.26/ini/RiceVideo5.6.0.ini","T:\\RiceVideo5.6.0.ini");
#else
		http.Download("http://freakdave.xbox-scene.com/ini/RiceVideo5.6.0.ini","T:\\RiceVideo5.6.0.ini");
#endif	
		break;
}

//At this point we're basically done
//Clean up Winsock
TerminateWinsock();
//Clean up Network
TerminateNetwork();
return TRUE;
}





