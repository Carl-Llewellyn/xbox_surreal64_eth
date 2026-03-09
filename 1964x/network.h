#ifndef _NETWORK_H__1964_
#define _NETWORK_H__1964_

#include "globals.h"

//start udp networking and worker thread
void NetProbe_Startup(void);

//stop udp networking and worker thread
void NetProbe_Shutdown(void);

//queue one event for async network send
void NetProbe_QueueEvent(uint32 op, uint32 addr, uint32 value, uint32 rt, uint32 pc, const uint8* payload, uint32 payload_len);

//copy latest inbound payload bytes (returns copied length)
uint32 NetProbe_CopyIncoming(uint8* out, uint32 out_len);

#endif
