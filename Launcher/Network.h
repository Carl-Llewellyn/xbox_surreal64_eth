#pragma once

#include "Launcher.h"
#include <xvoice.h>
#include <xonline.h>
#include "ftplib.h"

#define BUFFER_SIZE (4096)

/* DNS stuff (borrowed from XBMP) */
struct  hostent {
       char    * h_name;		/* official name of host */
       char    **h_aliases;		/* alias list */
       short   h_addrtype;		/* host address type */
       short   h_length;		/* length of address */
       char    *h_addr_list[4];	/* list of addresses */
#define h_addr  h_addr_list[0]  /* address, for backward compat */
};

typedef struct {
struct hostent server;
char name[128];
char addr[16];
char* addr_list[4];
} HostEnt;


class XNetwork
{
public:
	XNetwork(void);
	virtual ~XNetwork(void);
	bool InitNetwork(void);
	bool InitWinsock(void);
	void CleanupNetwork(void);
	void CleanupWinsock(void);

	bool IsXboxConnected(void);

	DWORD GetHostAddress(const char *host);

	bool DownloadFile(const char *host, const char *path_and_filename, const char *save_dir);



private:
	int err; 
	bool err1, err2, err3;
};


extern XNetwork g_xNet;
