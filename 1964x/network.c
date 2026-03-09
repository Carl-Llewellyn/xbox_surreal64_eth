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
#define NET_PACKET_WORDS    8
#define NET_META_BYTES      (NET_PACKET_WORDS * 4)
#define NET_PAYLOAD_BYTES   32
#define NET_PACKET_BYTES    (NET_META_BYTES + NET_PAYLOAD_BYTES)

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
	uint32 payload_len;
	uint8 payload[NET_PAYLOAD_BYTES];
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
	volatile LONG incoming_len;
	uint8 incoming_payload[NET_PAYLOAD_BYTES];
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

static __forceinline uint32 ReadBE32(const uint8 *p)
{
	return ((uint32)p[0] << 24) | ((uint32)p[1] << 16) | ((uint32)p[2] << 8) | (uint32)p[3];
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
	uint32 copyLen = e->payload_len;

	if(copyLen > NET_PAYLOAD_BYTES) {
		copyLen = NET_PAYLOAD_BYTES;
	}
	memset(pkt, 0, sizeof(pkt));

	//w0 magic 'nsr2'
	PutBE32(pkt +  0, 0x4E535232);
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
	//w7 payload length in bytes (<= 32)
	PutBE32(pkt + 28, copyLen);

	//payload bytes start at offset 32
	if(copyLen > 0) {
		memcpy(pkt + NET_META_BYTES, e->payload, copyLen);
	}

	sendto(g_net.sock, (const char *)pkt, NET_PACKET_BYTES, 0, (SOCKADDR *)&g_net.remote, sizeof(g_net.remote));
}

//consume one inbound UDP packet and store a latest 32-byte payload snapshot
static void PollIncoming(void)
{
	uint8 pkt[NET_PACKET_BYTES];
	uint8 payload[NET_PAYLOAD_BYTES];
	struct sockaddr_in src;
	int srcLen = sizeof(src);
	int recvLen;
	int err;
	uint32 w2;
	uint32 addrLow;
	uint32 copyLen = 0;

	for(;;) {
		srcLen = sizeof(src);
		recvLen = recvfrom(g_net.sock, (char *)pkt, sizeof(pkt), 0, (SOCKADDR *)&src, &srcLen);
		if(recvLen <= 0) {
			err = WSAGetLastError();
			if(err == WSAEWOULDBLOCK) {
				return;
			}
			return;
		}

		memset(payload, 0, sizeof(payload));

		//allow direct 32-byte payload packets
		if(recvLen >= (int)NET_PAYLOAD_BYTES && pkt[0] == 'O' && pkt[1] == 'O' && pkt[2] == 'T') {
			copyLen = NET_PAYLOAD_BYTES;
			memcpy(payload, pkt, NET_PAYLOAD_BYTES);
		}
		//or nsr2 framed packets with payload at byte 32
		//only accept nsr2 packets targeting OOT incoming SRAM window (0x7A20-0x7A3F)
		//to avoid feeding echoed outbound probe packets (0x7A00 window) back into input.
		else if(recvLen >= (int)NET_PACKET_BYTES && ReadBE32(pkt) == 0x4E535232) {
			w2 = ReadBE32(pkt + 8);
			addrLow = (w2 & 0xFFFF);
			if(addrLow < 0x7A20 || addrLow >= 0x7A40) {
				continue;
			}

			copyLen = ReadBE32(pkt + 28);
			if(copyLen > NET_PAYLOAD_BYTES) {
				copyLen = NET_PAYLOAD_BYTES;
			}
			if(copyLen > 0) {
				memcpy(payload, pkt + NET_META_BYTES, copyLen);
			}
			if(copyLen < 3 || payload[0] != 'O' || payload[1] != 'O' || payload[2] != 'T') {
				continue;
			}
		}
		else {
			continue;
		}

		if(copyLen > 0) {
			memcpy((void *)g_net.incoming_payload, payload, NET_PAYLOAD_BYTES);
			InterlockedExchange(&g_net.incoming_len, (LONG)copyLen);
		}
	}
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
		PollIncoming();
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
void NetProbe_QueueEvent(uint32 op, uint32 addr, uint32 value, uint32 rt, uint32 pc, const uint8* payload, uint32 payload_len)
{
	NetEvent e;
	uint32 copyLen;

	if(!g_net.started) return;
	e.op = op;
	e.addr = addr;
	e.value = value;
	e.rt = rt;
	e.pc = pc;
	e.seq = (uint32)InterlockedIncrement(&g_net.seq);
	copyLen = payload_len;
	if(copyLen > NET_PAYLOAD_BYTES) {
		copyLen = NET_PAYLOAD_BYTES;
	}
	e.payload_len = copyLen;
	memset(e.payload, 0, sizeof(e.payload));
	if((payload != NULL) && (copyLen > 0)) {
		memcpy(e.payload, payload, copyLen);
	}
	Enqueue(&e);
}

uint32 NetProbe_CopyIncoming(uint8* out, uint32 out_len)
{
	uint32 copyLen;

	if(!g_net.started || out == NULL || out_len == 0) {
		return 0;
	}

	copyLen = (uint32)g_net.incoming_len;
	if(copyLen > NET_PAYLOAD_BYTES) {
		copyLen = NET_PAYLOAD_BYTES;
	}
	if(copyLen > out_len) {
		copyLen = out_len;
	}

	if(copyLen > 0) {
		memcpy(out, (const void *)g_net.incoming_payload, copyLen);
	}
	return copyLen;
}
