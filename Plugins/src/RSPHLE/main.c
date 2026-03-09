#include <xtl.h>
#include <stdio.h>
#include "Rsp.h"
#include "Hle.h"
#include "jpeg.h"

RSP_INFO rsp;
BOOL AHle = TRUE, GHle = TRUE, SHle = FALSE;

void _RSP_HLE_CloseDLL (void)
{
}

void _RSP_HLE_DllAbout (HWND hParent)
{
}
void _RSP_HLE_DllConfig (HWND hParent)
{
}

DWORD _RSP_HLE_DoRspCycles (DWORD Cycles)
{
	OSTask_t *task = (OSTask_t*)(rsp.DMEM + 0xFC0);
	unsigned int i, sum=0;
   
	if( task->type == 1 && task->data_ptr != 0 && GHle) 
	{
		if (rsp.ProcessDList != NULL) 
		{
			rsp.ProcessDList();
		}
      
		*rsp.SP_STATUS_RG |= 0x0203;
		
		if ((*rsp.SP_STATUS_RG & 0x40) != 0) 
		{
			*rsp.MI_INTR_RG |= 0x1;
			
			rsp.CheckInterrupts();
		}
		
		*rsp.DPC_STATUS_RG &= ~0x0002;
		
		return Cycles;
	} 
	else if (task->type == 2 && AHle) 
	{
		if (rsp.ProcessAList != NULL) 
		{
			rsp.ProcessAList();
		}
		
		*rsp.SP_STATUS_RG |= 0x0203;
		
		if ((*rsp.SP_STATUS_RG & 0x40) != 0 )
		{
			*rsp.MI_INTR_RG |= 0x1;
			
			rsp.CheckInterrupts();
		}
		
		return Cycles;
	} 
	else if (task->type == 7) 
	{
		rsp.ShowCFB();
	}
	
	*rsp.SP_STATUS_RG |= 0x203;
	
	if ((*rsp.SP_STATUS_RG & 0x40) != 0)
	{
		*rsp.MI_INTR_RG |= 0x1;
		
		rsp.CheckInterrupts();
	}
	
	if (task->ucode_size <= 0x1000)
	{
		for (i=0; i<(task->ucode_size/2); i++)
			sum += *(rsp.RDRAM + task->ucode + i);
	}
	else
	{
		for (i=0; i<(0x1000/2); i++)
			sum += *(rsp.IMEM + i);
	}

	if (task->ucode_size > 0x1000)
	{
		switch(sum)
		{
			case 0x9E2: // banjo tooie (U) boot code
			{
				int i,j;
				
				memcpy(rsp.IMEM + 0x120, rsp.RDRAM + 0x1e8, 0x1e8);
				
				for (j=0; j<0xfc; j++)
					for (i=0; i<8; i++)
						*(rsp.RDRAM+((0x2fb1f0+j*0xff0+i)^S8))=*(rsp.IMEM+((0x120+j*8+i)^S8));
			
				return Cycles;
			}
			case 0x9F2: // banjo tooie (E) + zelda oot (E) boot code
			{
				int i,j;
				
				memcpy(rsp.IMEM + 0x120, rsp.RDRAM + 0x1e8, 0x1e8);
				
				for (j=0; j<0xfc; j++)
					for (i=0; i<8; i++)
						*(rsp.RDRAM+((0x2fb1f0+j*0xff0+i)^S8))=*(rsp.IMEM+((0x120+j*8+i)^S8));
				
				return Cycles;
			}
		}
	}
	else
	{
		if (task->type == 4)
		{
			switch(sum)
			{
				case 0x278: // used by zelda during boot
				{	
					*rsp.SP_STATUS_RG |= 0x200;

					return Cycles;
				}
				case 0x2e4fc: // uncompress
				{	
					jpg_uncompress(task);

					return Cycles;
				}
			}
		}
	}

	return Cycles;
}

void _RSP_HLE_GetDllInfo (PLUGIN_INFO *PluginInfo)
{
   PluginInfo->Version = 0x0101;
   PluginInfo->Type = PLUGIN_TYPE_RSP;
   strcpy(PluginInfo->Name, "Hacktarux hle rsp plugin");
   PluginInfo->NormalMemory = TRUE;
   PluginInfo->MemoryBswaped = TRUE;
}

void _RSP_HLE_InitiateRSP (RSP_INFO Rsp_Info, DWORD *CycleCount)
{
   rsp = Rsp_Info;
}

void _RSP_HLE_RomClosed (void)
{
   int i;

   for (i=0; i<0x1000; i++)
   {
	   rsp.DMEM[i] = rsp.IMEM[i] = 0;
   }
}
