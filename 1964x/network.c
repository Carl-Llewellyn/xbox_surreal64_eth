#include <xtl.h>
#include <winsockx.h>
#include <xonline.h>
#include <memory.h>
#pragma comment(lib, "xnet.lib")

#include "network.h"

//udp bind port on xbox
#define NET_LOCAL_PORT  55364
//udp target port on host
#define NET_REMOTE_PORT 55365
//udp target host ip
#define NET_REMOTE_IP   "10.10.10.1"

//fixed packet layout
#define NET_PACKET_WORDS 8
#define NET_PACKET_BYTES (NET_PACKET_WORDS * 4)

//single producer single consumer ring
#define NET_RING_SIZE    256
#define NET_RING_MASK    (NET_RING_SIZE - 1)

//one queued network event
//op is caller-defined event type
//addr is cart/physical address context
//value is caller payload field
//rt and pc are optional cpu context fields
typedef struct NetEvent_s {
	uint32 op;
	uint32 addr;
	uint32 value;
	uint32 rt;
	uint32 pc;
	uint32 seq;
} NetEvent;

//global network module state
//widx is written by emu thread
//ridx is written by net thread
typedef struct NetState_s {
	volatile LONG started;
	volatile LONG run_thread;
	volatile LONG widx;
	volatile LONG ridx;
	volatile LONG drops;
	volatile LONG seq;
	SOCKET sock;
	struct sockaddr_in remote;
	HANDLE thread;
	NetEvent ring[NET_RING_SIZE];
} NetState;

//module state
static NetState g_net = {0};

//write one u32 into packet bytes in big-endian order
static __forceinline void PutBE32(uint8 *p, uint32 w)
{
	p[0] = (uint8)(w >> 24);
	p[1] = (uint8)(w >> 16);
	p[2] = (uint8)(w >> 8);
	p[3] = (uint8)w;
}

//enqueue one event from emu thread
//this never blocks and drops when ring is full
static __forceinline BOOL Enqueue(const NetEvent *e)
{
	LONG w = g_net.widx;
	LONG r = g_net.ridx;
	LONG n = (w + 1) & NET_RING_MASK;
	if(n == r) {
		InterlockedIncrement(&g_net.drops);
		return FALSE;
	}
	g_net.ring[w & NET_RING_MASK] = *e;
	InterlockedExchange(&g_net.widx, n);
	return TRUE;
}

//dequeue one event from net thread
//returns false when queue is empty
static __forceinline BOOL Dequeue(NetEvent *e)
{
	LONG r = g_net.ridx;
	LONG w = g_net.widx;
	if(r == w) return FALSE;
	*e = g_net.ring[r & NET_RING_MASK];
	InterlockedExchange(&g_net.ridx, (r + 1) & NET_RING_MASK);
	return TRUE;
}

//encode and send one udp packet for a queued event
static void SendEvent(const NetEvent *e)
{
	uint8 pkt[NET_PACKET_BYTES];

	//w0 magic 'nsr1'
	PutBE32(pkt +  0, 0x4E535231);
	//w1 sequence
	PutBE32(pkt +  4, e->seq);
	//w2 compact op/rt/addr low
	PutBE32(pkt +  8, (e->op << 24) | (e->rt << 16) | (e->addr & 0xFFFF));
	//w3 full addr
	PutBE32(pkt + 12, e->addr);
	//w4 value payload
	PutBE32(pkt + 16, e->value);
	//w5 pc context
	PutBE32(pkt + 20, e->pc);
	//w6 current drop counter
	PutBE32(pkt + 24, (uint32)g_net.drops);
	//w7 reserved
	PutBE32(pkt + 28, 0);

	sendto(g_net.sock, (const char *)pkt, NET_PACKET_BYTES, 0, (SOCKADDR *)&g_net.remote, sizeof(g_net.remote));
}

//background sender thread
//drains queued events and transmits packets
static DWORD WINAPI NetThreadProc(LPVOID p)
{
	while(g_net.run_thread) {
		NetEvent e;
		while(Dequeue(&e)) {
			SendEvent(&e);
		}
		Sleep(1);
	}
	return 0;
}

//public startup
//initializes xnet/winsock, binds udp socket, and starts sender thread
void NetProbe_Startup(void)
{
	WSADATA wsa;
	XNetStartupParams xnsp;
	u_long nb = 1;
	struct sockaddr_in local;

	//skip if already initialized
	if(g_net.started) return;
	//reset module state before init
	memset(&g_net, 0, sizeof(g_net));
	g_net.sock = INVALID_SOCKET;

	//start xbox network stack
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(xnsp);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	if(XNetStartup(&xnsp) != 0) return;
	//start winsock api
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return;

	//socket start: create udp socket handle
	g_net.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(g_net.sock == INVALID_SOCKET) return;
	//set socket non-blocking
	ioctlsocket(g_net.sock, FIONBIO, &nb);

	//bind socket to local xbox port
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(NET_LOCAL_PORT);
	if(bind(g_net.sock, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR) return;

	//configure default remote host endpoint
	memset(&g_net.remote, 0, sizeof(g_net.remote));
	g_net.remote.sin_family = AF_INET;
	g_net.remote.sin_addr.s_addr = inet_addr(NET_REMOTE_IP);
	g_net.remote.sin_port = htons(NET_REMOTE_PORT);

	//thread start: launch sender worker thread
	g_net.run_thread = 1;
	g_net.thread = CreateThread(NULL, 0, NetThreadProc, NULL, 0, NULL);
	if(g_net.thread == NULL) {
		g_net.run_thread = 0;
		return;
	}

	//mark network module ready for enqueue calls
	g_net.started = 1;
}

//public shutdown
//stops thread and releases network resources
void NetProbe_Shutdown(void)
{
	if(g_net.thread != NULL) {
		g_net.run_thread = 0;
		WaitForSingleObject(g_net.thread, 500);
		CloseHandle(g_net.thread);
		g_net.thread = NULL;
	}

	if(g_net.sock != INVALID_SOCKET) {
		closesocket(g_net.sock);
		g_net.sock = INVALID_SOCKET;
	}

	WSACleanup();
	XNetCleanup();
	g_net.started = 0;
}

//public enqueue api used by intercept code
//this is safe to call from emu thread hot paths
void NetProbe_QueueEvent(uint32 op, uint32 addr, uint32 value, uint32 rt, uint32 pc)
{
	NetEvent e;
	if(!g_net.started) return;
	e.op = op;
	e.addr = addr;
	e.value = value;
	e.rt = rt;
	e.pc = pc;
	e.seq = (uint32)InterlockedIncrement(&g_net.seq);
	Enqueue(&e);
}
