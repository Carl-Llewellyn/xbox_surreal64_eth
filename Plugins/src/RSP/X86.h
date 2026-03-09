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

enum x86RegValues {
	x86_EAX = 0, x86_EBX = 1, x86_ECX = 2, x86_EDX = 3,
	x86_ESI = 4, x86_EDI = 5, x86_EBP = 6, x86_ESP = 7
};

enum mmxRegValues {
	x86_MM0 = 0, x86_MM1 = 1, x86_MM2 = 2, x86_MM3 = 3, 
	x86_MM4 = 4, x86_MM5 = 5, x86_MM6 = 6, x86_MM7 = 7
};

enum sseRegValues {
	x86_XMM0 = 0, x86_XMM1 = 1, x86_XMM2 = 2, x86_XMM3 = 3, 
	x86_XMM4 = 4, x86_XMM5 = 5, x86_XMM6 = 6, x86_XMM7 = 7
};

void RSP_AdcX86RegToX86Reg				( int Destination, int Source );
void RSP_AdcX86regToVariable			( int x86reg, void * Variable, char * VariableName );
void RSP_AdcX86regHalfToVariable		( int x86reg, void * Variable, char * VariableName );
void RSP_AdcConstToX86reg				( BYTE Constant, int x86reg );
void RSP_AdcConstToVariable				( void *Variable, char *VariableName, BYTE Constant );
void RSP_AdcConstHalfToVariable			( void *Variable, char *VariableName, BYTE Constant );
void RSP_AddConstToVariable				( DWORD Const, void *Variable, char *VariableName );
void RSP_AddConstToX86Reg				( int x86Reg, DWORD Const );
void RSP_AddVariableToX86reg			( int x86reg, void * Variable, char * VariableName );
void RSP_AddX86regToVariable			( int x86reg, void * Variable, char * VariableName );
void RSP_AddX86regHalfToVariable		( int x86reg, void * Variable, char * VariableName );
void RSP_AddX86RegToX86Reg				( int Destination, int Source );
void RSP_AndConstToVariable				( DWORD Const, void *Variable, char *VariableName );
void RSP_AndConstToX86Reg				( int x86Reg, DWORD Const );
void RSP_AndVariableToX86Reg			( void * Variable, char * VariableName, int x86Reg );
void RSP_AndVariableToX86regHalf		( void * Variable, char * VariableName, int x86Reg );
void RSP_AndX86RegToVariable			( void * Variable, char * VariableName, int x86Reg );
void RSP_AndX86RegToX86Reg				( int Destination, int Source );
void RSP_AndX86RegHalfToX86RegHalf		( int Destination, int Source );
void RSP_Call_Direct					( void * FunctAddress, char * FunctName );
void RSP_Call_Indirect					( void * FunctAddress, char * FunctName );
void RSP_CondMoveEqual					( int Destination, int Source );
void RSP_CondMoveNotEqual				( int Destination, int Source );
void RSP_CondMoveGreater				( int Destination, int Source );
void RSP_CondMoveGreaterEqual			( int Destination, int Source );
void RSP_CondMoveLess					( int Destination, int Source );
void RSP_CondMoveLessEqual				( int Destination, int Source );
void RSP_CompConstToVariable			( DWORD Const, void * Variable, char * VariableName );
void RSP_CompConstHalfToVariable		( WORD Const, void * Variable, char * VariableName );
void RSP_CompConstToX86reg				( int x86Reg, DWORD Const );
void RSP_CompX86regToVariable			( int x86Reg, void * Variable, char * VariableName );
void RSP_CompVariableToX86reg			( int x86Reg, void * Variable, char * VariableName );
void RSP_CompX86RegToX86Reg				( int Destination, int Source );
void RSP_Cwd							( void );
void RSP_Cwde							( void );
void RSP_DecX86reg						( int x86Reg );
void RSP_DivX86reg						( int x86reg );
void RSP_idivX86reg						( int x86reg );
void RSP_imulX86reg						( int x86reg );
void RSP_ImulX86RegToX86Reg				( int Destination, int Source );
void RSP_IncX86reg						( int x86Reg );
void RSP_JaeLabel32						( char * Label, DWORD Value );
void RSP_JaLabel8						( char * Label, BYTE Value );
void RSP_JaLabel32						( char * Label, DWORD Value );
void RSP_JbLabel8						( char * Label, BYTE Value );
void RSP_JbLabel32						( char * Label, DWORD Value );
void RSP_JeLabel8						( char * Label, BYTE Value );
void RSP_JeLabel32						( char * Label, DWORD Value );
void RSP_JgeLabel8						( char * Label, BYTE Value );
void RSP_JgeLabel32						( char * Label, DWORD Value );
void RSP_JgLabel8						( char * Label, BYTE Value );
void RSP_JgLabel32						( char * Label, DWORD Value );
void RSP_JleLabel8						( char * Label, BYTE Value );
void RSP_JleLabel32						( char * Label, DWORD Value );
void RSP_JlLabel8						( char * Label, BYTE Value );
void RSP_JlLabel32						( char * Label, DWORD Value );
void RSP_JumpX86Reg						( int x86reg );
void RSP_JmpLabel8						( char * Label, BYTE Value );
void RSP_JmpLabel32						( char * Label, DWORD Value );
void RSP_JneLabel8						( char * Label, BYTE Value );
void RSP_JneLabel32						( char * Label, DWORD Value );
void RSP_JnsLabel8						( char * Label, BYTE Value );
void RSP_JnsLabel32						( char * Label, DWORD Value );
void RSP_JsLabel32						( char * Label, DWORD Value );
void RSP_LeaSourceAndOffset				( int x86DestReg, int x86SourceReg, int offset );
void RSP_MoveConstByteToN64Mem			( BYTE Const, int AddrReg );
void RSP_MoveConstHalfToN64Mem			( WORD Const, int AddrReg );
void RSP_MoveConstByteToVariable		( BYTE Const,void *Variable, char *VariableName );
void RSP_MoveConstHalfToVariable		( WORD Const, void *Variable, char *VariableName );
void RSP_MoveConstToN64Mem				( DWORD Const, int AddrReg );
void RSP_MoveConstToN64MemDisp			( DWORD Const, int AddrReg, BYTE Disp );
void RSP_MoveConstToVariable			( DWORD Const, void *Variable, char *VariableName );
void RSP_MoveConstToX86reg				( DWORD Const, int x86reg );
void RSP_MoveOffsetToX86reg				( DWORD Const, char * VariableName, int x86reg );
void RSP_MoveX86regByteToX86regPointer	( int Source, int AddrReg );
void RSP_MoveX86regHalfToX86regPointer	( int Source, int AddrReg );
void RSP_MoveX86regHalfToX86regPointerDisp ( int Source, int AddrReg, BYTE Disp);
void RSP_MoveX86regToX86regPointer		( int Source, int AddrReg );
void RSP_MoveX86RegToX86regPointerDisp	( int Source, int AddrReg, BYTE Disp );
void RSP_MoveX86regPointerToX86regByte	( int Destination, int AddrReg );
void RSP_MoveX86regPointerToX86regHalf	( int Destination, int AddrReg );
void RSP_MoveX86regPointerToX86reg		( int Destination, int AddrReg );
void RSP_MoveN64MemDispToX86reg			( int x86reg, int AddrReg, BYTE Disp );
void RSP_MoveN64MemToX86reg				( int x86reg, int AddrReg );
void RSP_MoveN64MemToX86regByte			( int x86reg, int AddrReg );
void RSP_MoveN64MemToX86regHalf			( int x86reg, int AddrReg );
void RSP_MoveX86regByteToN64Mem			( int x86reg, int AddrReg );
void RSP_MoveX86regByteToVariable		( int x86reg, void * Variable, char * VariableName );
void RSP_MoveX86regHalfToN64Mem			( int x86reg, int AddrReg );
void RSP_MoveX86regHalfToVariable		( int x86reg, void * Variable, char * VariableName );
void RSP_MoveX86regToN64Mem				( int x86reg, int AddrReg );
void RSP_MoveX86regToN64MemDisp			( int x86reg, int AddrReg, BYTE Disp );
void RSP_MoveX86regToVariable			( int x86reg, void * Variable, char * VariableName );
void RSP_MoveX86RegToX86Reg				( int Source, int Destination );
void RSP_MoveVariableToX86reg			( void *Variable, char *VariableName, int x86reg );
void RSP_MoveVariableToX86regByte		( void *Variable, char *VariableName, int x86reg );
void RSP_MoveVariableToX86regHalf		( void *Variable, char *VariableName, int x86reg );
void RSP_MoveSxX86RegHalfToX86Reg		( int Source, int Destination );
void RSP_MoveSxX86RegPtrDispToX86RegHalf( int AddrReg, BYTE Disp, int Destination );
void RSP_MoveSxN64MemToX86regByte		( int x86reg, int AddrReg );
void RSP_MoveSxN64MemToX86regHalf		( int x86reg, int AddrReg );
void RSP_MoveSxVariableToX86regHalf		( void *Variable, char *VariableName, int x86reg );
void RSP_MoveZxX86RegHalfToX86Reg		( int Source, int Destination );
void RSP_MoveZxX86RegPtrDispToX86RegHalf( int AddrReg, BYTE Disp, int Destination );
void RSP_MoveZxN64MemToX86regByte		( int x86reg, int AddrReg );
void RSP_MoveZxN64MemToX86regHalf		( int x86reg, int AddrReg );
void RSP_MoveZxVariableToX86regHalf		( void *Variable, char *VariableName, int x86reg );
void RSP_MulX86reg						( int x86reg );
void RSP_NegateX86reg					( int x86reg );
void RSP_OrConstToVariable				( DWORD Const, void * Variable, char * VariableName );
void RSP_OrConstToX86Reg				( DWORD Const, int  x86Reg );
void RSP_OrVariableToX86Reg				( void * Variable, char * VariableName, int x86Reg );
void RSP_OrVariableToX86regHalf			( void * Variable, char * VariableName, int x86Reg );
void RSP_OrX86RegToVariable				( void * Variable, char * VariableName, int x86Reg );
void RSP_OrX86RegToX86Reg				( int Destination, int Source );
void RSP_Popad							( void );
void RSP_Pushad							( void );
void RSP_Push							( int x86reg );
void RSP_Pop							( int x86reg );
void RSP_PushImm32						( char * String, DWORD Value );
void RSP_Ret							( void );
void RSP_Seta							( int x86reg );
void RSP_Setae							( int x86reg );
void RSP_Setl							( int x86reg );
void RSP_Setb							( int x86reg );
void RSP_Setg							( int x86reg );
void RSP_Setz							( int x86reg );
void RSP_Setnz							( int x86reg );
void RSP_SetlVariable					( void * Variable, char * VariableName );
void RSP_SetleVariable					( void * Variable, char * VariableName );
void RSP_SetgVariable					( void * Variable, char * VariableName );
void RSP_SetgeVariable					( void * Variable, char * VariableName );
void RSP_SetbVariable					( void * Variable, char * VariableName );
void RSP_SetaVariable					( void * Variable, char * VariableName );
void RSP_SetzVariable					( void * Variable, char * VariableName );
void RSP_SetnzVariable					( void * Variable, char * VariableName );
void RSP_ShiftLeftSign					( int x86reg );
void RSP_ShiftLeftSignImmed				( int x86reg, BYTE Immediate );
void RSP_ShiftLeftSignVariableImmed		( void *Variable, char *VariableName, BYTE Immediate );
void RSP_ShiftRightSignImmed			( int x86reg, BYTE Immediate );
void RSP_ShiftRightSignVariableImmed	( void *Variable, char *VariableName, BYTE Immediate );
void RSP_ShiftRightUnsign				( int x86reg );
void RSP_ShiftRightUnsignImmed			( int x86reg, BYTE Immediate );
void RSP_ShiftRightUnsignVariableImmed	( void *Variable, char *VariableName, BYTE Immediate );
void RSP_ShiftLeftDoubleImmed			( int Destination, int Source, BYTE Immediate );
void RSP_ShiftRightDoubleImmed			( int Destination, int Source, BYTE Immediate );
void RSP_SubConstFromVariable			( DWORD Const, void *Variable, char *VariableName );
void RSP_SubConstFromX86Reg				( int x86Reg, DWORD Const );
void RSP_SubVariableFromX86reg			( int x86reg, void * Variable, char * VariableName );
void RSP_SubX86RegToX86Reg				( int Destination, int Source );
void RSP_SubX86regFromVariable			( int x86reg, void * Variable, char * VariableName );
void RSP_SbbX86RegToX86Reg				( int Destination, int Source );
void RSP_TestConstToVariable			( DWORD Const, void * Variable, char * VariableName );
void RSP_TestConstToX86Reg				( DWORD Const, int x86reg );
void RSP_TestX86RegToX86Reg				( int Destination, int Source );
void RSP_XorConstToX86Reg				( int x86Reg, DWORD Const );
void RSP_XorX86RegToX86Reg				( int Source, int Destination );
void RSP_XorVariableToX86reg			( void *Variable, char *VariableName, int x86reg );
void RSP_XorX86RegToVariable			( void *Variable, char *VariableName, int x86reg );
void RSP_XorConstToVariable				( void *Variable, char *VariableName, DWORD Const );

#define _MMX_SHUFFLE(a, b, c, d)	\
	((BYTE)(((a) << 6) | ((b) << 4) | ((c) << 2) | (d)))

void MmxMoveRegToReg				( int Dest, int Source );
void MmxMoveQwordRegToVariable		( int Dest, void *Variable, char *VariableName );
void MmxMoveQwordVariableToReg		( int Dest, void *Variable, char *VariableName );
void MmxPandRegToReg				( int Dest, int Source );
void MmxPandnRegToReg				( int Dest, int Source );
void MmxPandVariableToReg			( void * Variable, char * VariableName, int Dest );
void MmxPorRegToReg					( int Dest, int Source );
void MmxPorVariableToReg			( void * Variable, char * VariableName, int Dest );
void MmxXorRegToReg					( int Dest, int Source );
void MmxShuffleMemoryToReg			( int Dest, void * Variable, char * VariableName, BYTE Immed );
void MmxPmullwRegToReg				( int Dest, int Source );
void MmxPmullwVariableToReg			( int Dest, void * Variable, char * VariableName );
void MmxPmulhuwRegToReg				( int Dest, int Source );
void MmxPmulhwRegToReg				( int Dest, int Source );
void MmxPmulhwRegToVariable			( int Dest, void * Variable, char * VariableName );
void MmxPsrlwImmed					( int Dest, BYTE Immed );
void MmxPsrawImmed					( int Dest, BYTE Immed );
void MmxPsllwImmed					( int Dest, BYTE Immed );
void MmxPaddswRegToReg				( int Dest, int Source );
void MmxPaddswVariableToReg			( int Dest, void * Variable, char * VariableName );
void MmxPaddwRegToReg				( int Dest, int Source );
void MmxPackSignedDwords			( int Dest, int Source );
void MmxUnpackLowWord				( int Dest, int Source );
void MmxUnpackHighWord				( int Dest, int Source );
void MmxCompareGreaterWordRegToReg	( int Dest, int Source );
void MmxEmptyMultimediaState		( void );

void SseMoveAlignedVariableToReg	( void *Variable, char *VariableName, int sseReg );
void SseMoveAlignedRegToVariable	( int sseReg, void *Variable, char *VariableName );
void SseMoveAlignedN64MemToReg		( int sseReg, int AddrReg );
void SseMoveAlignedRegToN64Mem		( int sseReg, int AddrReg );
void SseMoveUnalignedVariableToReg	( void *Variable, char *VariableName, int sseReg );
void SseMoveUnalignedRegToVariable	( int sseReg, void *Variable, char *VariableName );
void SseMoveUnalignedN64MemToReg	( int sseReg, int AddrReg );
void SseMoveUnalignedRegToN64Mem	( int sseReg, int AddrReg );
void SseMoveRegToReg				( int Dest, int Source );
void SseXorRegToReg					( int Dest, int Source );

typedef struct {
	union {
		struct {
			unsigned Reg0 : 2;
			unsigned Reg1 : 2;
			unsigned Reg2 : 2;
			unsigned Reg3 : 2;
		};
		unsigned UB:8;
	};
} SHUFFLE;

void SseShuffleReg					( int Dest, int Source, BYTE Immed );

void x86_SetBranch32b(void * JumpByte, void * Destination);
void x86_SetBranch8b(void * JumpByte, void * Destination);