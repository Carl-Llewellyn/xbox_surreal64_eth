#include "Network.h"

XNetwork g_xNet;

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
		int err = XNetDnsLookup(_name, hEvent, &pDns);
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
DWORD XNetwork::GetHostAddress(const char *host)
{
    struct hostent *phe;
    char *p;

    phe = gethostbyname( host );

            
    if(phe==NULL)
        return 0;
    
    p = *phe->h_addr_list;
	OutputDebugString("GetHostAddress succeeded for: ");
	OutputDebugString(host);
	OutputDebugString("\n");
    return *((DWORD*)p);
}

XNetwork::XNetwork(void)
{
}

XNetwork::~XNetwork(void)
{
}

bool XNetwork::IsXboxConnected(void)
{
	DWORD dwStatus = XNetGetEthernetLinkStatus();
	if(dwStatus != 0){
	   return true;
	}

	return false;
}

bool XNetwork::InitNetwork(void)
{
	OutputDebugString("Initializing Network...\n");
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	xnsp.cfgSockDefaultRecvBufsizeInK = 32;
	err = XNetStartup(&xnsp);

	if(err != 0){
		XNetCleanup();//Terminate Network
		OutputDebugString("Could not initialize Network!\n");
		return false;
	}
return true;
}

bool XNetwork::InitWinsock(void)
{
	OutputDebugString("Initializing Winsock\n");
	WSADATA wsaData;

    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
    {
        OutputDebugString("Error initializing Winsock 2.2\n");
        CleanupWinsock();
		CleanupNetwork();
		return false;
    }
return true;
}

//DownloadFile("myserver.com", "/myfolder/myfile.ini", "D:\\new.ini");
bool XNetwork::DownloadFile(const char *host, const char *path_and_filename, const char *save_dir)
{
    HANDLE fhand;
    string request;
    int sendret;
    int iRecv;
    int iResponseLength=0;
    int offset;
    DWORD dw;
    string res2;
    char recvBuffer[BUFFER_SIZE]={0};
    string response;
    const char lb[]="\r\n\r\n";
    const char http[]="http\x3a//";

	//struct hostent *h;
    struct sockaddr_in sa;
    SOCKET server1;


	sa.sin_addr.s_addr = GetHostAddress(host);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);

	server1 = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(connect(server1,(struct sockaddr *)&sa,sizeof(sa)) == SOCKET_ERROR)
    {
        OutputDebugString("connect() failed!\n");
		CleanupWinsock();
		CleanupNetwork();
		return false;
    }

	OutputDebugString("Connected to server at port 80...");

	//Construct the HTTP request header
	request += "GET ";
    request += path_and_filename;
    request += " HTTP/1.0";
    request += &lb[2];
    request += "Host: ";
    request += host;
    request += lb;
    OutputDebugString("HTTP request constructed successfully:\n");
    OutputDebugString(request.c_str());

	//Send the query
	sendret = send(server1, request.c_str(), request.length(), 0);
    if(sendret == SOCKET_ERROR)
    {
        OutputDebugString("send() failed!\n");
        CleanupWinsock();
		CleanupNetwork();
		return false;
    }
	OutputDebugString("HTTP request sent successfully...\n");

	//Fill the buffer
	OutputDebugString("Downloading file...\n");
	fhand = CreateFile(save_dir, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(fhand == INVALID_HANDLE_VALUE)
	{
        OutputDebugString("Could not create file!\n");
        CleanupWinsock();
		CleanupNetwork();
		return false;
	}

	char buf[1024];
	//Receive the data
	while((iRecv = recv(server1, recvBuffer, BUFFER_SIZE-1, 0)) > 0)
    {
        response.append(recvBuffer, iRecv);
        iResponseLength += iRecv;
        ZeroMemory(recvBuffer, BUFFER_SIZE);
		sprintf(buf, "%d\n", iResponseLength);
		OutputDebugString(buf);
	}
	if(iRecv==SOCKET_ERROR)
    {
        OutputDebugString("recv() failed!\n");
    }


	offset = response.find(lb) + 4;
    if(offset != string::npos)
    {
		char buffer[256];
        OutputDebugString("File starts at offset ");
		sprintf(buffer,"%d\n", offset);
		OutputDebugString(buffer);
        OutputDebugString("Initial response from server:\n");
        for(int j=0;j<offset;++j)
        {
			sprintf(buffer,"%c", response[j]);
            OutputDebugString(buffer);
        }
        res2.assign(response,offset,response.size());
        if(WriteFile(fhand, res2.data() ,res2.size(), &dw, 0) == 0)
        {
            OutputDebugString("Could not write to file!\n");
            CleanupWinsock();
			CleanupNetwork();
			return false;
        }
        else
        {
            OutputDebugString("File successfully downloaded and saved to ");
            OutputDebugString(save_dir);
			OutputDebugString("\n");
        }
    }


return true;
}


//Clean up Winsock
void XNetwork::CleanupWinsock(void){
	OutputDebugString("Cleaning up Winsock\n");
	WSACleanup();
}

//Clean up Network
void XNetwork::CleanupNetwork(void){
	OutputDebugString("Cleaning up Network\n");
	XNetCleanup();
}
