#ifndef _NETWORK_H__1964_
#define _NETWORK_H__1964_

#include "globals.h"

void NetProbe_Startup(void);
void NetProbe_Shutdown(void);

/* Queue one PI/SRAM interception event for async UDP send.
 * op: caller-defined opcode (e.g. read/write + region)
 * addr: physical cart address (masked form recommended)
 * value: sampled 32-bit value
 * rt/pc: optional CPU context for debugging
 */
void NetProbe_QueueEvent(uint32 op, uint32 addr, uint32 value, uint32 rt, uint32 pc);

#endif
