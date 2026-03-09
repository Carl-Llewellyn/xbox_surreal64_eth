/*
 * RSP Compiler plug in for Project 64 (A Nintendo 64 emulator).
 *
 * (c) Copyright 2001 jabo (jabo@emulation64.com) and
 * zilmar (zilmar@emulation64.com)
 *
 * pj64 homepage: www.pj64.net
 * 
 * Permission to use, copy, modify and distribute Project64 in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Project64 is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Project64 or software derived from Project64.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so if they want them.
 *
 */

#include <xtl.h>
#include <stdio.h>
#include "opcode.h"
#include "RSP.h"
#include "CPU.h"
#include "RSP Registers.h"
#include "RSP Command.h"
#include "rspmemory.h"
#include "breakpoint.h"

#define RSP_MaxCommandLines		30

#define RSP_Status_PC            1
#define RSP_Status_BP            2

#define IDC_LIST					1000
#define IDC_ADDRESS					1001
#define IDC_FUNCTION_COMBO			1002
#define IDC_GO_BUTTON				1003
#define IDC_BREAK_BUTTON			1004
#define IDC_STEP_BUTTON				1005
#define IDC_SKIP_BUTTON				1006
#define IDC_BP_BUTTON				1007
#define IDC_R4300I_REGISTERS_BUTTON	1008
#define IDC_R4300I_DEBUGGER_BUTTON	1009
#define IDC_RSP_REGISTERS_BUTTON	1010
#define IDC_MEMORY_BUTTON			1011
#define IDC_SCRL_BAR				1012

void Paint_RSP_Commands (HWND hDlg);
void RSP_Commands_Setup ( HWND hDlg );
LRESULT CALLBACK RSP_Commands_Proc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


typedef struct {
	DWORD Location;
	DWORD opcode;
	char  String[150];
    DWORD status;
} RSPCOMMANDLINE;

RSPCOMMANDLINE RSPCommandLine[30];
HWND RSPCommandshWnd, hList, hAddress, hFunctionlist, hGoButton, hBreakButton,
	hStepButton, hSkipButton, hBPButton, hR4300iRegisters, hR4300iDebugger, hRSPRegisters,
	hMemory, hScrlBar;
BOOL InRSPCommandsWindow;
char CommandName[100];


void Create_RSP_Commands_Window ( int Child ) {
	DWORD ThreadID;

	if ( Child ) {
		InRSPCommandsWindow = TRUE;
		 

		InRSPCommandsWindow = FALSE;
		memset(RSPCommandLine,0,sizeof(RSPCommandLine));
		SetRSPCommandToRunning();
	} else {
		if (!InRSPCommandsWindow) {
			Stepping_Commands = TRUE;
			CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Create_RSP_Commands_Window,
				(LPVOID)TRUE,0, &ThreadID);	
		} else {
			 
		}	
	}
}

void Disable_RSP_Commands_Window ( void ) {
 
}

int DisplayRSPCommand (DWORD location, int InsertPos) {
	DWORD LinesUsed = 1;
 
	return LinesUsed;
}

void DumpRSPCode (void) {
	 
}

void DumpRSPData (void) {
 
}

void DrawRSPCommand ( LPARAM lParam ) {	
	 

}


void Enable_RSP_Commands_Window ( void ) {
	 
}

void Enter_RSP_Commands_Window ( void ) {
    Create_RSP_Commands_Window ( FALSE );
}

void Paint_RSP_Commands (HWND hDlg) {
	 
}

void RefreshRSPCommands ( void ) {
 
}

LRESULT CALLBACK RSP_Commands_Proc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {	
	 
	return TRUE;
}

void RSP_Commands_Setup ( HWND hDlg ) {
#define WindowWidth  457
#define WindowHeight 494
	 

}

char * RSPSpecialName ( DWORD OpCode, DWORD PC ) {
	OPCODE command;
	command.Hex = OpCode;
		
	switch (command.funct) {
	case RSP_SPECIAL_SLL:
		if (command.rd != 0) {
			sprintf(CommandName,"SLL\t%s, %s, 0x%X",GPR_Name(command.rd),
				GPR_Name(command.rt), command.sa);
		} else {
			sprintf(CommandName,"NOP");
		}
		break;
	case RSP_SPECIAL_SRL:
		sprintf(CommandName,"SRL\t%s, %s, 0x%X",GPR_Name(command.rd),
			GPR_Name(command.rt), command.sa);
		break;
	case RSP_SPECIAL_SRA:
		sprintf(CommandName,"SRA\t%s, %s, 0x%X",GPR_Name(command.rd),
			GPR_Name(command.rt), command.sa);
		break;
	case RSP_SPECIAL_SLLV:
		sprintf(CommandName,"SLLV\t%s, %s, %s",GPR_Name(command.rd),
			GPR_Name(command.rt), GPR_Name(command.rs));
		break;
	case RSP_SPECIAL_SRLV:
		sprintf(CommandName,"SRLV\t%s, %s, %s",GPR_Name(command.rd),
			GPR_Name(command.rt), GPR_Name(command.rs));
		break;
	case RSP_SPECIAL_SRAV:
		sprintf(CommandName,"SRAV\t%s, %s, %s",GPR_Name(command.rd),
			GPR_Name(command.rt), GPR_Name(command.rs));
		break;
	case RSP_SPECIAL_JR:
		sprintf(CommandName,"JR\t%s",GPR_Name(command.rs));
		break;
	case RSP_SPECIAL_JALR:
		sprintf(CommandName,"JALR\t%s, %s",GPR_Name(command.rd),GPR_Name(command.rs));
		break;
	case RSP_SPECIAL_BREAK:
		sprintf(CommandName,"BREAK");
		break;
	case RSP_SPECIAL_ADD:
		sprintf(CommandName,"ADD\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_ADDU:
		sprintf(CommandName,"ADDU\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_SUB:
		sprintf(CommandName,"SUB\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_SUBU:
		sprintf(CommandName,"SUBU\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_AND:
		sprintf(CommandName,"AND\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_OR:
		sprintf(CommandName,"OR\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_XOR:
		sprintf(CommandName,"XOR\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_NOR:
		sprintf(CommandName,"NOR\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_SLT:
		sprintf(CommandName,"SLT\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	case RSP_SPECIAL_SLTU:
		sprintf(CommandName,"SLTU\t%s, %s, %s",GPR_Name(command.rd),GPR_Name(command.rs),
			GPR_Name(command.rt));
		break;
	default:
		sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
			command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
	}
	return CommandName;
}

char * RSPRegimmName ( DWORD OpCode, DWORD PC ) {
	OPCODE command;
	command.Hex = OpCode;
		
	switch (command.rt) {
	case RSP_REGIMM_BLTZ:
		sprintf(CommandName,"BLTZ\t%s, 0x%03X",GPR_Name(command.rs),
			(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		break;
	case RSP_REGIMM_BGEZ:
		sprintf(CommandName,"BGEZ\t%s, 0x%03X",GPR_Name(command.rs),
			(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		break;
	case RSP_REGIMM_BLTZAL:
		sprintf(CommandName,"BLTZAL\t%s, 0x%03X",GPR_Name(command.rs),
			(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		break;
	case RSP_REGIMM_BGEZAL:
		if (command.rs == 0) {
			sprintf(CommandName,"BAL\t0x%03X",(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		} else {
			sprintf(CommandName,"BGEZAL\t%s, 0x%03X",GPR_Name(command.rs),
				(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		}	
		break;
	default:
		sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
			command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
	}
	return CommandName;
}

char * RSPCop0Name ( DWORD OpCode, DWORD PC ) {
	OPCODE command;
	command.Hex = OpCode;
	switch (command.rs) {
	case RSP_COP0_MF:
		sprintf(CommandName,"MFC0\t%s, %s",GPR_Name(command.rt),COP0_Name(command.rd));
		break;
	case RSP_COP0_MT:
		sprintf(CommandName,"MTC0\t%s, %s",GPR_Name(command.rt),COP0_Name(command.rd));
		break;
	default:
		sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
			command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
	}
	return CommandName;
}

char * RSPCop2Name ( DWORD OpCode, DWORD PC ) {
	OPCODE command;
	command.Hex = OpCode;
		
	if ( ( command.rs & 0x10 ) == 0 ) {
		switch (command.rs) {
		case RSP_COP2_MF:
			sprintf(CommandName,"MFC2\t%s, $v%d [%d]",GPR_Name(command.rt),
				command.rd, command.sa >> 1);
			break;
		case RSP_COP2_CF:		
			sprintf(CommandName,"CFC2\t%s, %d",GPR_Name(command.rt),
				command.rd % 4);
			break;
		case RSP_COP2_MT:
			sprintf(CommandName,"MTC2\t%s, $v%d [%d]",GPR_Name(command.rt),
				command.rd, command.sa >> 1);
			break;
		case RSP_COP2_CT:		
			sprintf(CommandName,"CTC2\t%s, %d",GPR_Name(command.rt),
				command.rd % 4);
			break;
		default:
			sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
				command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
		}
	} else {
		switch (command.funct) {
		case RSP_VECTOR_VMULF:
			sprintf(CommandName,"VMULF\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMULU:
			sprintf(CommandName,"VMULU\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRNDP:
			sprintf(CommandName,"VRNDP\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMULQ:
			sprintf(CommandName,"VMULQ\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMUDL:
			sprintf(CommandName,"VMUDL\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMUDM:
			sprintf(CommandName,"VMUDM\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMUDN:
			sprintf(CommandName,"VMUDN\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMUDH:
			sprintf(CommandName,"VMUDH\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMACF:
			sprintf(CommandName,"VMACF\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMACU:
			sprintf(CommandName,"VMACU\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRNDN:
			sprintf(CommandName,"VRNDN\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMACQ:
			sprintf(CommandName,"VMACQ\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMADL:
			sprintf(CommandName,"VMADL\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMADM:
			sprintf(CommandName,"VMADM\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMADN:
			sprintf(CommandName,"VMADN\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMADH:
			sprintf(CommandName,"VMADH\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VADD:
			sprintf(CommandName,"VADD\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VSUB:
			sprintf(CommandName,"VSUB\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VABS:
			sprintf(CommandName,"VABS\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VADDC:
			sprintf(CommandName,"VADDC\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VSUBC:
			sprintf(CommandName,"VSUBC\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VSAW:
			sprintf(CommandName,"VSAW\t$v%d [%d], $v%d, $v%d ",command.sa, (command.rs & 0xF),
				command.rd, command.rt);
			break;
		case RSP_VECTOR_VLT:
			sprintf(CommandName,"VLT\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VEQ:
			sprintf(CommandName,"VEQ\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VNE:
			sprintf(CommandName,"VNE\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VGE:
			sprintf(CommandName,"VGE\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VCL:
			sprintf(CommandName,"VCL\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VCH:
			sprintf(CommandName,"VCH\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VCR:
			sprintf(CommandName,"VCR\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMRG:
			sprintf(CommandName,"VMRG\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VAND:
			sprintf(CommandName,"VAND\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VNAND:
			sprintf(CommandName,"VNAND\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VOR:
			sprintf(CommandName,"VOR\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VNOR:
			sprintf(CommandName,"VNOR\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VXOR:
			sprintf(CommandName,"VXOR\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VNXOR:
			sprintf(CommandName,"VNXOR\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRCP:
			sprintf(CommandName,"VRCP\t$v%d [%d], $v%d%s",command.sa, (command.rd & 0x7), 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRCPL:
			sprintf(CommandName,"VRCPL\t$v%d [%d], $v%d%s",command.sa, (command.rd & 0x7), 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRCPH:
			sprintf(CommandName,"VRCPH\t$v%d [%d], $v%d%s",command.sa, (command.rd & 0x7), 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VMOV:
			sprintf(CommandName,"VMOV\t$v%d, $v%d, $v%d%s",command.sa, command.rd, 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRSQ:
			sprintf(CommandName,"VRSQ\t$v%d [%d], $v%d%s",command.sa, (command.rd & 0x7), 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRSQL:
			sprintf(CommandName,"VRSQL\t$v%d [%d], $v%d%s",command.sa, (command.rd & 0x7), 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VRSQH:
			sprintf(CommandName,"VRSQH\t$v%d [%d], $v%d%s",command.sa, (command.rd & 0x7), 
				command.rt, ElementSpecifier(command.rs & 0xF));
			break;
		case RSP_VECTOR_VNOOP:
			sprintf(CommandName,"VNOOP");
			break;
		default:
			sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
				command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
		}
	}
	return CommandName;
}

char * RSPLc2Name ( DWORD OpCode, DWORD PC ) {
	OPCODE command;
	command.Hex = OpCode;

	switch (command.rd) {
	case RSP_LSC2_BV:
		sprintf(CommandName,"LBV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			command.voffset, GPR_Name(command.base));
		break;
	case RSP_LSC2_SV:
		sprintf(CommandName,"LSV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 1), GPR_Name(command.base));
		break;
	case RSP_LSC2_LV:
		sprintf(CommandName,"LLV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 2), GPR_Name(command.base));
		break;
	case RSP_LSC2_DV:
		sprintf(CommandName,"LDV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 3), GPR_Name(command.base));
		break;
	case RSP_LSC2_QV:
		sprintf(CommandName,"LQV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_RV:
		sprintf(CommandName,"LRV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_PV:
		sprintf(CommandName,"LPV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 3), GPR_Name(command.base));
		break;
	case RSP_LSC2_UV:
		sprintf(CommandName,"LUV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 3), GPR_Name(command.base));
		break;
	case RSP_LSC2_HV:
		sprintf(CommandName,"LHV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_FV:
		sprintf(CommandName,"LFV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_WV:
		sprintf(CommandName,"LWV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_TV:
		sprintf(CommandName,"LTV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	default:
		sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
			command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
	}
	return CommandName;
}

char * RSPSc2Name ( DWORD OpCode, DWORD PC ) {
	OPCODE command;
	command.Hex = OpCode;

	switch (command.rd) {
	case RSP_LSC2_BV:
		sprintf(CommandName,"SBV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			command.voffset, GPR_Name(command.base));
		break;
	case RSP_LSC2_SV:
		sprintf(CommandName,"SSV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 1), GPR_Name(command.base));
		break;
	case RSP_LSC2_LV:
		sprintf(CommandName,"SLV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 2), GPR_Name(command.base));
		break;
	case RSP_LSC2_DV:
		sprintf(CommandName,"SDV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 3), GPR_Name(command.base));
		break;
	case RSP_LSC2_QV:
		sprintf(CommandName,"SQV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_RV:
		sprintf(CommandName,"SRV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_PV:
		sprintf(CommandName,"SPV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 3), GPR_Name(command.base));
		break;
	case RSP_LSC2_UV:
		sprintf(CommandName,"SUV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 3), GPR_Name(command.base));
		break;
	case RSP_LSC2_HV:
		sprintf(CommandName,"SHV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_FV:
		sprintf(CommandName,"SFV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_WV:
		sprintf(CommandName,"SWV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	case RSP_LSC2_TV:
		sprintf(CommandName,"STV\t$v%d [%d], 0x%04X (%s)",command.rt, command.del, 
			(command.voffset << 4), GPR_Name(command.base));
		break;
	default:
		sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
			command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
	}
	return CommandName;
}

char * RSPOpcodeName ( DWORD OpCode, DWORD PC ) {
	OPCODE command;
	command.Hex = OpCode;
		
	switch (command.op) {
	case RSP_SPECIAL:
		return RSPSpecialName(OpCode,PC);
		break;
	case RSP_REGIMM:
		return RSPRegimmName(OpCode,PC);
		break;
	case RSP_J:
		sprintf(CommandName,"J\t0x%03X",(command.target << 2) & 0xFFC);
		break;
	case RSP_JAL:
		sprintf(CommandName,"JAL\t0x%03X",(command.target << 2) & 0xFFC);
		break;
	case RSP_BEQ:
		if (command.rs == 0 && command.rt == 0) {
			sprintf(CommandName,"B\t0x%03X",(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		} else if (command.rs == 0 || command.rt == 0){
			sprintf(CommandName,"BEQZ\t%s, 0x%03X",GPR_Name(command.rs == 0 ? command.rt : command.rs),
				(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		} else {
			sprintf(CommandName,"BEQ\t%s, %s, 0x%03X",GPR_Name(command.rs),GPR_Name(command.rt),
				(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		}
		break;
	case RSP_BNE:
		sprintf(CommandName,"BNE\t%s, %s, 0x%03X",GPR_Name(command.rs),GPR_Name(command.rt),
			(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		break;
	case RSP_BLEZ:
		sprintf(CommandName,"BLEZ\t%s, 0x%03X",GPR_Name(command.rs),(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		break;
	case RSP_BGTZ:
		sprintf(CommandName,"BGTZ\t%s, 0x%03X",GPR_Name(command.rs),(PC + ((short)command.offset << 2) + 4) & 0xFFC);
		break;
	case RSP_ADDI:
		sprintf(CommandName,"ADDI\t%s, %s, 0x%04X",GPR_Name(command.rt), GPR_Name(command.rs),
			command.immediate);
		break;
	case RSP_ADDIU:
		sprintf(CommandName,"ADDIU\t%s, %s, 0x%04X",GPR_Name(command.rt), GPR_Name(command.rs),
			command.immediate);
		break;
	case RSP_SLTI:
		sprintf(CommandName,"SLTI\t%s, %s, 0x%04X",GPR_Name(command.rt), GPR_Name(command.rs),
			command.immediate);
		break;
	case RSP_SLTIU:
		sprintf(CommandName,"SLTIU\t%s, %s, 0x%04X",GPR_Name(command.rt), GPR_Name(command.rs),
			command.immediate);
		break;
	case RSP_ANDI:
		sprintf(CommandName,"ANDI\t%s, %s, 0x%04X",GPR_Name(command.rt), GPR_Name(command.rs),
			command.immediate);
		break;
	case RSP_ORI:
		sprintf(CommandName,"ORI\t%s, %s, 0x%04X",GPR_Name(command.rt), GPR_Name(command.rs),
			command.immediate);
		break;
	case RSP_XORI:
		sprintf(CommandName,"XORI\t%s, %s, 0x%04X",GPR_Name(command.rt), GPR_Name(command.rs),
			command.immediate);
		break;
	case RSP_LUI:
		sprintf(CommandName,"LUI\t%s, 0x%04X",GPR_Name(command.rt), command.immediate);
		break;
	case RSP_CP0:
		return RSPCop0Name(OpCode,PC);
		break;
	case RSP_CP2:
		return RSPCop2Name(OpCode,PC);
		break;
	case RSP_LB:
		sprintf(CommandName,"LB\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_LH:
		sprintf(CommandName,"LH\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_LW:
		sprintf(CommandName,"LW\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_LBU:
		sprintf(CommandName,"LBU\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_LHU:
		sprintf(CommandName,"LHU\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_SB:
		sprintf(CommandName,"SB\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_SH:
		sprintf(CommandName,"SH\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_SW:
		sprintf(CommandName,"SW\t%s, 0x%04X(%s)",GPR_Name(command.rt), command.offset,
			GPR_Name(command.base));
		break;
	case RSP_LC2:
		return RSPLc2Name(OpCode,PC);
		break;
	case RSP_SC2:
		return RSPSc2Name(OpCode,PC);
		break;
	default:
		sprintf(CommandName,"RSP: Unknown\t%02X %02X %02X %02X",
			command.Ascii[3],command.Ascii[2],command.Ascii[1],command.Ascii[0]);
	}
	return CommandName;
}

void SetRSPCommandToRunning ( void ) { 	
	 
}

void SetRSPCommandToStepping ( void ) { 	
	 
}

void SetRSPCommandViewto ( UINT NewLocation ) {
	unsigned int location;
	char Value[20];

	if (InRSPCommandsWindow == FALSE) { return; }

	//GetWindowText(hAddress,Value,sizeof(Value));
	location = AsciiToHex(Value) & ~3;

	if ( NewLocation < location || NewLocation >= location + 120 ) {
		sprintf(Value,"%03X",NewLocation);
		//SetWindowText(hAddress,Value);
	} else {
		RefreshRSPCommands();
	}
}
