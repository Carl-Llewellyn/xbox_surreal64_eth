#include "xtl.h"
#include "stdlib.h"
#include "stdio.h"
#include "io.h"
#include "HTTP.h"

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

/* Wrapper functions for gethostbyname() */
struct hostent* _cdecl gethostbyname( const char *name );
struct hostent* _cdecl gethostbyname(const char*);

